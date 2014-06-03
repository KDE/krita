/*
  Copyright 2008        Brad Hards  <bradh@frogmouth.net>
  Copyright 2009 - 2010 Inge Wallin <inge@lysator.liu.se>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "EmfOutputPainterStrategy.h"

#include <math.h>

#include <kdebug.h>

#include "EmfObjects.h"


#define DEBUG_EMFPAINT 0
#define DEBUG_PAINTER_TRANSFORM 0

namespace Libemf
{


static QPainter::CompositionMode  rasteropToQtComposition(long rop);

// ================================================================
//                         Class OutputPainterStrategy


OutputPainterStrategy::OutputPainterStrategy()
    : m_header( 0 )
    , m_path( 0 )
    , m_currentlyBuildingPath( false )
    , m_fillRule(Qt::OddEvenFill)
    , m_mapMode(MM_TEXT)
    , m_textAlignMode(TA_NOUPDATECP) // == TA_TOP == TA_LEFT
    , m_currentCoords()
{
    m_painter         = 0;
    m_painterSaves    = 0;
    m_outputSize      = QSize();
    m_keepAspectRatio = true;
}

OutputPainterStrategy::OutputPainterStrategy(QPainter &painter, QSize &size,
                                             bool keepAspectRatio)
    : m_header( 0 )
    , m_path( 0 )
    , m_currentlyBuildingPath( false )
    , m_windowExtIsSet(false)
    , m_viewportExtIsSet(false)
    , m_windowViewportIsSet(false)
    , m_fillRule(Qt::OddEvenFill)
    , m_mapMode(MM_TEXT)
    , m_textAlignMode(TA_NOUPDATECP) // == TA_TOP == TA_LEFT
    , m_currentCoords()
{
    m_painter         = &painter;
    m_painterSaves    = 0;
    m_outputSize      = size;
    m_keepAspectRatio = keepAspectRatio;
}

OutputPainterStrategy::~OutputPainterStrategy()
{
    delete m_header;
    delete m_path;
}

void OutputPainterStrategy::paintBounds(const Header *header)
{
    // The rectangle is in device coordinates.
    QRectF  rect(header->bounds());
    m_painter->save();

    // Draw a simple cross in a rectangle to show the bounds.
    m_painter->setPen(QPen(QColor(172, 196, 206)));
    m_painter->drawRect(rect);
    m_painter->drawLine(rect.topLeft(), rect.bottomRight());
    m_painter->drawLine(rect.bottomLeft(), rect.topRight());

    m_painter->restore();
}

void OutputPainterStrategy::init( const Header *header )
{
    // Save the header since we need the frame and bounds inside the drawing.
    m_header = new Header(*header);

    QSize  headerBoundsSize = header->bounds().size();

#if DEBUG_EMFPAINT
    kDebug(31000) << "----------------------------------------------------------------------";
    kDebug(31000) << "Shape size               =" << m_outputSize.width() << m_outputSize.height() << " pt";
    kDebug(31000) << "----------------------------------------------------------------------";
    kDebug(31000) << "Boundary box (dev units) =" << header->bounds().x() << header->bounds().y()
                  << header->bounds().width() << header->bounds().height();
    kDebug(31000) << "Frame (phys size)        =" << header->frame().x() << header->frame().y()
                  << header->frame().width() << header->frame().height() << " *0.01 mm";

    kDebug(31000) << "Device =" << header->device().width() << header->device().height();
    kDebug(31000) << "Millimeters =" << header->millimeters().width()
                  << header->millimeters().height();
#endif

#if DEBUG_PAINTER_TRANSFORM
    printPainterTransform("In init, before save:");
#endif

    // This is restored in cleanup().
    m_painter->save();

    // Calculate how much the painter should be resized to fill the
    // outputSize with output.
    qreal  scaleX = qreal( m_outputSize.width() )  / headerBoundsSize.width();
    qreal  scaleY = qreal( m_outputSize.height() ) / headerBoundsSize.height();
    if ( m_keepAspectRatio ) {
        // Use the smaller value so that we don't get an overflow in
        // any direction.
        if ( scaleX > scaleY )
            scaleX = scaleY;
        else
            scaleY = scaleX;
    }
#if DEBUG_EMFPAINT
    kDebug(31000) << "scale = " << scaleX << ", " << scaleY;
#endif

    // Transform the EMF object so that it fits in the shape.  The
    // topleft of the EMF will be the top left of the shape.
    m_painter->scale( scaleX, scaleY );
    m_painter->translate(-header->bounds().left(), -header->bounds().top());
#if DEBUG_PAINTER_TRANSFORM
    printPainterTransform("after fitting into shape");
#endif

    // Calculate translation if we should center the EMF in the
    // area and keep the aspect ratio.
#if 0 // Should apparently be upper left.  See bug 265868
    if ( m_keepAspectRatio ) {
        m_painter->translate((m_outputSize.width() / scaleX - headerBoundsSize.width()) / 2,
                             (m_outputSize.height() / scaleY - headerBoundsSize.height()) / 2);
#if DEBUG_PAINTER_TRANSFORM
        printPainterTransform("after translation for keeping center in the shape");
#endif
    }
#endif

    m_outputTransform = m_painter->transform();
    m_worldTransform = QTransform();

    // For calculations of window / viewport during the painting
    m_windowOrg = QPoint(0, 0);
    m_viewportOrg = QPoint(0, 0);
    m_windowExtIsSet = false;
    m_viewportExtIsSet = false;
    m_windowViewportIsSet = false;

#if DEBUG_EMFPAINT
    paintBounds(header);
#endif
}

void OutputPainterStrategy::cleanup( const Header *header )
{
    Q_UNUSED( header );

#if DEBUG_EMFPAINT
    if (m_painterSaves > 0)
        kDebug(33100) << "WARNING: UNRESTORED DC's:" << m_painterSaves;
#endif

    // Restore all the save()s that were done during the processing.
    for (int i = 0; i < m_painterSaves; ++i)
        m_painter->restore();
    m_painterSaves = 0;

    // Restore the painter to what it was before init() was called.
    m_painter->restore();
}


void OutputPainterStrategy::eof()
{
}

void OutputPainterStrategy::setPixelV( QPoint &point, quint8 red, quint8 green, quint8 blue,
                                       quint8 reserved )
{
    Q_UNUSED( reserved );

#if DEBUG_EMFPAINT
    kDebug(31000) << point << red << green << blue;
#endif

    m_painter->save();

    QPen pen;
    pen.setColor( QColor( red, green, blue ) );
    m_painter->setPen( pen );
    m_painter->drawPoint( point );

    m_painter->restore();
}


void OutputPainterStrategy::beginPath()
{
#if DEBUG_EMFPAINT
    kDebug(31000);
#endif

    delete( m_path );
    m_path = new QPainterPath;
    m_currentlyBuildingPath = true;
}

void OutputPainterStrategy::closeFigure()
{
#if DEBUG_EMFPAINT
    kDebug(31000);
#endif

    m_path->closeSubpath();
}

void OutputPainterStrategy::endPath()
{
#if DEBUG_EMFPAINT
    kDebug(31000);
#endif

    m_path->setFillRule( m_fillRule );
    m_currentlyBuildingPath = false;
}

void OutputPainterStrategy::saveDC()
{
#if DEBUG_EMFPAINT
    kDebug(31000);
#endif

    // A little trick here: Save the worldTransform in the painter.
    // If we didn't do this, we would have to create a separate stack
    // for these.
    //
    // FIXME: We should collect all the parts of the DC that are not
    //        stored in the painter and save them separately.
    QTransform  savedTransform = m_painter->worldTransform();
    m_painter->setWorldTransform(m_worldTransform);

    m_painter->save();
    ++m_painterSaves;

    m_painter->setWorldTransform(savedTransform);
}

void OutputPainterStrategy::restoreDC( const qint32 savedDC )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << savedDC;
#endif

    // Note that savedDC is always negative
    for (int i = 0; i < -savedDC; ++i) {
        if (m_painterSaves > 0) {
            m_painter->restore();
            --m_painterSaves;
        }
        else {
            kDebug(33100) << "restoreDC(): try to restore painter without save" << savedDC - i;
            break;
        }
    }

    // We used a trick in saveDC() and stored the worldTransform in
    // the painter.  Now restore the full transformation.
    m_worldTransform = m_painter->worldTransform();
    QTransform newMatrix = m_worldTransform * m_outputTransform;
    m_painter->setWorldTransform( newMatrix );
}

void OutputPainterStrategy::setMetaRgn()
{
    kDebug(33100) << "EMR_SETMETARGN not yet implemented";
}


// ----------------------------------------------------------------
//                 World Transform, Window and Viewport


// General note about coordinate spaces and transforms:
//
// There are several coordinate spaces in use when drawing an EMF file:
//  1. The object space, in which the objects' coordinates are expressed inside the EMF.
//     In general there are several of these.
//  2. The page space, which is where they end up being painted in the EMF picture.
//     The union of these form the bounding box of the EMF.
//  3. (possibly) the output space, where the EMF picture itself is placed
//     and/or scaled, rotated, etc
//
// The transform between spaces 1. and 2. is called the World Transform.
// The world transform can be changed either through calls to change
// the window or viewport or through calls to setWorldTransform() or
// modifyWorldTransform().
//
// The transform between spaces 2. and 3. is the transform that the QPainter
// already contains when it is given to us.  We need to save this and reapply
// it after the world transform has changed. We call this transform the Output
// Transform in lack of a better word. (Some sources call it the Device Transform.)
//

// An unanswered question:
//
// The file mp07_embedded_ppt.pptx in the test files contains the
// following sequence of records:
// - SetWorldTransform
// - ModifyWorldTransform
// - SetViewportOrg  <-- doesn't change anything
// - SetWindowOrg    <-- doesn't change anything
// - ExtTextOutw
//
// I was previously under the impression that whenever a
// Set{Window,Viewport}{Org,Ext} record was encountered, the world
// transform was supposed to be recalculated. But in this file, it
// destroys the world transform. The question is which of the
// following alternatives is true:
// 
// 1. The world transform should only be recalculated if the
//    Set{Window,Viewport}{Org,Ext} record actually changes anything.
//
// 2. The transformations set by {Set,Modify}WorldTransform records
//    should always be reapplied after a change in window or viewport.
//
// 3. Something else
//
// I have for now implemented option 1. See the FIXME's in
// SetWindowOrg et al.
//


// Set Window and Viewport
void OutputPainterStrategy::recalculateWorldTransform()
{
    m_worldTransform = QTransform();

    // If neither the window nor viewport extension is set, then there
    // is no way to perform the calculation.  Just give up.
    if (!m_windowExtIsSet && !m_viewportExtIsSet)
        return;

    // Negative window extensions mean flip the picture.  Handle this here.
    bool  flip = false;
    qreal midpointX = 0.0;
    qreal midpointY = 0.0;
    qreal scaleX = 1.0;
    qreal scaleY = 1.0;
    if (m_windowExt.width() < 0) {
        midpointX = m_windowOrg.x() + m_windowExt.width() / qreal(2.0);
        scaleX = -1.0;
        flip = true;
    }
    if (m_windowExt.height() < 0) {
        midpointY = m_windowOrg.y() + m_windowExt.height() / qreal(2.0);
        scaleY = -1.0;
        flip = true;
    }
    if (flip) {
        //kDebug(31000) << "Flipping" << midpointX << midpointY << scaleX << scaleY;
        m_worldTransform.translate(midpointX, midpointY);
        m_worldTransform.scale(scaleX, scaleY);
        m_worldTransform.translate(-midpointX, -midpointY);
        //kDebug(31000) << "After flipping for window" << mWorldTransform;
    }

    // Update the world transform if both window and viewport are set...
    // FIXME: Check windowExt == 0 in any direction
    if (m_windowExtIsSet && m_viewportExtIsSet) {
        // Both window and viewport are set.
        qreal windowViewportScaleX = qreal(m_viewportExt.width()) / qreal(m_windowExt.width());
        qreal windowViewportScaleY = qreal(m_viewportExt.height()) / qreal(m_windowExt.height());

        m_worldTransform.translate(-m_windowOrg.x(), -m_windowOrg.y());
        m_worldTransform.scale(windowViewportScaleX, windowViewportScaleY);
        m_worldTransform.translate(m_viewportOrg.x(), m_viewportOrg.y());
    }

    // ...and apply it to the painter
    m_painter->setWorldTransform(m_worldTransform);
    m_windowViewportIsSet = true;

    // Apply the output transform.
    QTransform newMatrix = m_worldTransform * m_outputTransform;
    m_painter->setWorldTransform( newMatrix );
}


void OutputPainterStrategy::setWindowOrgEx( const QPoint &origin )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << origin;
#endif

    // FIXME: See unanswered question at the start of this section.
    if (m_windowOrg == origin) {
        //kDebug(31000) << "same origin as before";
        return;
    }

    m_windowOrg = origin;

    recalculateWorldTransform();
}

void OutputPainterStrategy::setWindowExtEx( const QSize &size )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << size;
#endif

    // FIXME: See unanswered question at the start of this section.
    if (m_windowExt == size) {
        //kDebug(31000) << "same extension as before";
        return;
    }

    m_windowExt = size;
    m_windowExtIsSet = true;

    recalculateWorldTransform();
}

void OutputPainterStrategy::setViewportOrgEx( const QPoint &origin )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << origin;
#endif

    // FIXME: See unanswered question at the start of this section.
    if (m_viewportOrg == origin) {
        //kDebug(31000) << "same origin as before";
        return;
    }

    m_viewportOrg = origin;

    recalculateWorldTransform();
}

void OutputPainterStrategy::setViewportExtEx( const QSize &size )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << size;
#endif

    // FIXME: See unanswered question at the start of this section.
    if (m_viewportExt == size) {
        //kDebug(31000) << "same extension as before";
        return;
    }

    m_viewportExt = size;
    m_viewportExtIsSet = true;

    recalculateWorldTransform();
}



void OutputPainterStrategy::modifyWorldTransform( quint32 mode, float M11, float M12,
                                                  float M21, float M22, float Dx, float Dy )
{
#if DEBUG_EMFPAINT
    if (mode == MWT_IDENTITY)
        kDebug(31000) << "Identity matrix";
    else
        kDebug(31000) << mode << M11 << M12 << M21 << M22 << Dx << Dy;
#endif

    QTransform matrix( M11, M12, M21, M22, Dx, Dy);

    if ( mode == MWT_IDENTITY ) {
        m_worldTransform = QTransform();
    } else if ( mode == MWT_LEFTMULTIPLY ) {
        m_worldTransform = matrix * m_worldTransform;
    } else if ( mode == MWT_RIGHTMULTIPLY ) {
        m_worldTransform = m_worldTransform * matrix;
    } else if ( mode == MWT_SET ) {
        m_worldTransform = matrix;
    } else {
	qWarning() << "Unimplemented transform mode" << mode;
    }

    // Apply the output transform.
    QTransform newMatrix = m_worldTransform * m_outputTransform;
    m_painter->setWorldTransform( newMatrix );
}

void OutputPainterStrategy::setWorldTransform( float M11, float M12, float M21,
                                               float M22, float Dx, float Dy )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << M11 << M12 << M21 << M22 << Dx << Dy;
#endif

    QTransform matrix( M11, M12, M21, M22, Dx, Dy);

    m_worldTransform = matrix;

    // Apply the output transform.
    QTransform newMatrix = m_worldTransform * m_outputTransform;
    m_painter->setWorldTransform( newMatrix );
}


// ----------------------------------------------------------------


void OutputPainterStrategy::createPen( quint32 ihPen, quint32 penStyle, quint32 x, quint32 y,
                                       quint8 red, quint8 green, quint8 blue, quint8 reserved )
{
    Q_UNUSED( y );
    Q_UNUSED( reserved );

#if DEBUG_EMFPAINT
    kDebug(31000) << ihPen << hex << penStyle << dec << x << y
                  << red << green << blue << reserved;
#endif

    QPen pen;
    pen.setColor( QColor( red, green, blue ) );

    if ( penStyle & PS_GEOMETRIC ) {
	pen.setCosmetic( false );
    } else {
	pen.setCosmetic( true );
    }

    switch ( penStyle & 0xF ) {
    case PS_SOLID:
        pen.setStyle( Qt::SolidLine );
        break;
    case PS_DASH:
        pen.setStyle( Qt::DashLine );
        break;
    case PS_DOT:
        pen.setStyle( Qt::DotLine );
        break;
    case PS_DASHDOT:
        pen.setStyle( Qt::DashDotLine );
        break;
    case PS_DASHDOTDOT:
        pen.setStyle( Qt::DashDotDotLine );
        break;
    case PS_NULL:
        pen.setStyle( Qt::NoPen );
        break;
    case PS_INSIDEFRAME:
        // FIXME: We don't properly support this
        pen.setStyle( Qt::SolidLine );
        break;
    case PS_USERSTYLE:
        kDebug(33100) << "UserStyle pen not yet supported, using SolidLine";
        pen.setStyle( Qt::SolidLine );
        break;
    case PS_ALTERNATE:
        kDebug(33100) << "Alternate pen not yet supported, using DashLine";
        pen.setStyle( Qt::DashLine );
        break;
    default:
        kDebug(33100) << "unexpected pen type, using SolidLine" << (penStyle & 0xF);
        pen.setStyle( Qt::SolidLine );
    }

    switch ( penStyle & PS_ENDCAP_FLAT ) {
    case PS_ENDCAP_ROUND:
        pen.setCapStyle( Qt::RoundCap );
        break;
    case PS_ENDCAP_SQUARE:
        pen.setCapStyle( Qt::SquareCap );
        break;
    case PS_ENDCAP_FLAT:
        pen.setCapStyle( Qt::FlatCap );
        break;
    default:
        kDebug(33100) << "unexpected cap style, using SquareCap" << (penStyle & PS_ENDCAP_FLAT);
        pen.setCapStyle( Qt::SquareCap );
    }
    pen.setWidth( x );

    m_objectTable.insert( ihPen,  pen );
}

void OutputPainterStrategy::createBrushIndirect( quint32 ihBrush, quint32 brushStyle,
                                                 quint8 red, quint8 green, quint8 blue,
                                                 quint8 reserved,
                                                 quint32 brushHatch )
{
    Q_UNUSED( reserved );
    Q_UNUSED( brushHatch );

#if DEBUG_EMFPAINT
    kDebug(31000) << ihBrush << hex << brushStyle << dec
                  << red << green << blue << reserved << brushHatch;
#endif

    QBrush brush;

    switch ( brushStyle ) {
    case BS_SOLID:
	brush.setStyle( Qt::SolidPattern );
	break;
    case BS_NULL:
	brush.setStyle( Qt::NoBrush );
	break;
    case BS_HATCHED:
	brush.setStyle( Qt::CrossPattern );
	break;
    case BS_PATTERN:
	Q_ASSERT( 0 );
	break;
    case BS_INDEXED:
	Q_ASSERT( 0 );
	break;
    case BS_DIBPATTERN:
	Q_ASSERT( 0 );
	break;
    case BS_DIBPATTERNPT:
	Q_ASSERT( 0 );
	break;
    case BS_PATTERN8X8:
	Q_ASSERT( 0 );
	break;
    case BS_DIBPATTERN8X8:
	Q_ASSERT( 0 );
	break;
    case BS_MONOPATTERN:
	Q_ASSERT( 0 );
	break;
    default:
	Q_ASSERT( 0 );
    }

    brush.setColor( QColor( red, green, blue ) );

    // TODO: Handle the BrushHatch enum.

    m_objectTable.insert( ihBrush, brush );
}

void OutputPainterStrategy::createMonoBrush( quint32 ihBrush, Bitmap *bitmap )
{

    QImage  pattern(bitmap->image());
    QBrush  brush(pattern);

    m_objectTable.insert( ihBrush, brush );
}


void OutputPainterStrategy::extCreateFontIndirectW( const ExtCreateFontIndirectWRecord &extCreateFontIndirectW )
{
    QFont font( extCreateFontIndirectW.fontFace() );

    font.setWeight( convertFontWeight( extCreateFontIndirectW.weight() ) );

    if ( extCreateFontIndirectW.height() < 0 ) {
	font.setPixelSize( -1 * extCreateFontIndirectW.height() );
    } else if ( extCreateFontIndirectW.height() > 0 ) {
        font.setPixelSize( extCreateFontIndirectW.height() );
    } // zero is "use a default size" which is effectively no-op here.

    // .snp files don't always provide 0x01 for italics
    if ( extCreateFontIndirectW.italic() != 0x00 ) {
	font.setItalic( true );
    }

    if ( extCreateFontIndirectW.underline() != 0x00 ) {
	font.setUnderline( true );
    }

    m_objectTable.insert( extCreateFontIndirectW.ihFonts(), font );
}

void OutputPainterStrategy::selectStockObject( const quint32 ihObject )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << ihObject;
#endif

    switch ( ihObject ) {
    case WHITE_BRUSH:
	m_painter->setBrush( QBrush( Qt::white ) );
	break;
    case LTGRAY_BRUSH:
	m_painter->setBrush( QBrush( Qt::lightGray ) );
	break;
    case GRAY_BRUSH:
	m_painter->setBrush( QBrush( Qt::gray ) );
	break;
    case DKGRAY_BRUSH:
	m_painter->setBrush( QBrush( Qt::darkGray ) );
	break;
    case BLACK_BRUSH:
	m_painter->setBrush( QBrush( Qt::black ) );
	break;
    case NULL_BRUSH:
	m_painter->setBrush( QBrush() );
	break;
    case WHITE_PEN:
	m_painter->setPen( QPen( Qt::white ) );
	break;
    case BLACK_PEN:
	m_painter->setPen( QPen( Qt::black ) );
	break;
    case NULL_PEN:
	m_painter->setPen( QPen( Qt::NoPen ) );
	break;
    case OEM_FIXED_FONT:
    case ANSI_FIXED_FONT:
    case SYSTEM_FIXED_FONT:
        {
            QFont  font(QString("Fixed"));
            m_painter->setFont(font);
            break;
        }
    case ANSI_VAR_FONT:
    case DEFAULT_GUI_FONT:      // Not sure if this is true, but it should work well
        {
            QFont  font(QString("Helvetica")); // Could also be "System"
            m_painter->setFont(font);
            break;
        }
	break;
    case SYSTEM_FONT:
	// TODO: handle this
	break;
    case DEVICE_DEFAULT_FONT:
	// TODO: handle this
	break;
    case DEFAULT_PALETTE:
	break;
    case DC_BRUSH:
        // FIXME
	break;
    case DC_PEN:
        // FIXME
	break;
    default:
	qWarning() << "Unexpected stock object:" << ( ihObject & 0x8000000 );
    }
}

void OutputPainterStrategy::selectObject( const quint32 ihObject )
{
#if DEBUG_EMFPAINT
    kDebug(33100) << hex << ihObject << dec;
#endif

    if ( ihObject & 0x80000000 ) {
	selectStockObject( ihObject );
    } else {
	QVariant obj = m_objectTable.value( ihObject );

	switch ( obj.type() ) {
	case QVariant::Pen :
	    m_painter->setPen( obj.value<QPen>() );
	    break;
	case QVariant::Brush :
	    m_painter->setBrush( obj.value<QBrush>() );
	    break;
	case QVariant::Font :
	    m_painter->setFont( obj.value<QFont>() );
	    break;
	default:
	    kDebug(33100) << "Unexpected type:" << obj.typeName();
	}
    }
}

void OutputPainterStrategy::deleteObject( const quint32 ihObject )
{
    m_objectTable.take( ihObject );
}

void OutputPainterStrategy::arc( const QRect &box, const QPoint &start, const QPoint &end )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << box << start << end;
#endif

    QPoint centrePoint = box.center();

    qreal startAngle = angleFromArc( centrePoint, start );
    qreal endAngle   = angleFromArc( centrePoint, end );
    qreal spanAngle  = angularSpan( startAngle, endAngle );

    m_painter->drawArc( box, startAngle*16, spanAngle*16 );
}

void OutputPainterStrategy::chord( const QRect &box, const QPoint &start, const QPoint &end )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << box << start << end;
#endif

    QPoint centrePoint = box.center();

    qreal startAngle = angleFromArc( centrePoint, start );
    qreal endAngle   = angleFromArc( centrePoint, end );
    qreal spanAngle  = angularSpan( startAngle, endAngle );

    m_painter->drawChord( box, startAngle*16, spanAngle*16 );
}

void OutputPainterStrategy::pie( const QRect &box, const QPoint &start, const QPoint &end )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << box << start << end;
#endif

    QPoint centrePoint = box.center();

    qreal startAngle = angleFromArc( centrePoint, start );
    qreal endAngle   = angleFromArc( centrePoint, end );
    qreal spanAngle  = angularSpan( startAngle, endAngle );

    m_painter->drawPie( box, startAngle*16, spanAngle*16 );
}

void OutputPainterStrategy::ellipse( const QRect &box )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << box;
#endif

    m_painter->drawEllipse( box );
}

void OutputPainterStrategy::rectangle( const QRect &box )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << box;
#endif

    m_painter->drawRect( box );
}

void OutputPainterStrategy::setMapMode( const quint32 mapMode )
{
#if DEBUG_EMFPAINT
    kDebug(33100) << "Set map mode:" << mapMode;
#endif

    m_mapMode = (MapMode)mapMode;
}

void OutputPainterStrategy::setBkMode( const quint32 backgroundMode )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << backgroundMode;
#endif

    if ( backgroundMode == TRANSPARENT ) {
        m_painter->setBackgroundMode( Qt::TransparentMode );
    } else if ( backgroundMode == OPAQUE ) {
        m_painter->setBackgroundMode( Qt::OpaqueMode );
    } else {
        kDebug(33100) << "EMR_SETBKMODE: Unexpected value -" << backgroundMode;
        Q_ASSERT( 0 );
    }
}

void OutputPainterStrategy::setPolyFillMode( const quint32 polyFillMode )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << polyFillMode;
#endif

    if ( polyFillMode == ALTERNATE ) {
	m_fillRule = Qt::OddEvenFill;
    } else if ( polyFillMode == WINDING ) {
	m_fillRule = Qt::WindingFill;
    } else {
	kDebug(33100) << "EMR_SETPOLYFILLMODE: Unexpected value -" << polyFillMode;
	Q_ASSERT( 0 );
    }
}

void OutputPainterStrategy::setLayout( const quint32 layoutMode )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << layoutMode;
#endif

    if ( layoutMode == LAYOUT_LTR ) {
        m_painter->setLayoutDirection( Qt::LeftToRight );
    } else if ( layoutMode == LAYOUT_RTL ) {
        m_painter->setLayoutDirection( Qt::RightToLeft );
    } else {
        kDebug(33100) << "EMR_SETLAYOUT: Unexpected value -" << layoutMode;
        Q_ASSERT( 0 );
    }
}

void OutputPainterStrategy::setTextAlign( const quint32 textAlignMode )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << textAlignMode;
#endif

    m_textAlignMode = textAlignMode;
}

void OutputPainterStrategy::setTextColor( const quint8 red, const quint8 green, const quint8 blue,
                                          const quint8 reserved )
{
    Q_UNUSED( reserved );

#if DEBUG_EMFPAINT
    kDebug(31000) << red << green << blue << reserved;
#endif

    m_textPen.setColor( QColor( red, green, blue ) );
}

void OutputPainterStrategy::setBkColor( const quint8 red, const quint8 green, const quint8 blue,
                                        const quint8 reserved )
{
    Q_UNUSED( reserved );

#if DEBUG_EMFPAINT
    kDebug(31000) << red << green << blue << reserved;
#endif

    m_painter->setBackground( QBrush( QColor( red, green, blue ) ) );
}


#define DEBUG_TEXTOUT 0

void OutputPainterStrategy::extTextOut( const QRect &bounds, const EmrTextObject &textObject )
{
    const QPoint  &referencePoint = textObject.referencePoint();
    const QString &text = textObject.textString();

#if DEBUG_EMFPAINT
    kDebug(31000) << "Ref point: " << referencePoint
                  << "options: " << hex << textObject.options() << dec
                  << "rectangle: " << textObject.rectangle()
                  << "text: " << textObject.textString();
#endif

    int  x = referencePoint.x();
    int  y = referencePoint.y();

    // The TA_UPDATECP flag tells us to use the current position
    if (m_textAlignMode & TA_UPDATECP) {
        // (left, top) position = current logical position
#if DEBUG_EMFPAINT
        kDebug(31000) << "TA_UPDATECP: use current logical position";
#endif
        x = m_currentCoords.x();
        y = m_currentCoords.y();
    }

    QFontMetrics  fm = m_painter->fontMetrics();
    int textWidth  = fm.width(text) + fm.descent(); // fm.width(text) isn't right with Italic text
    int textHeight = fm.height();

    // Make (x, y) be the coordinates of the upper left corner of the
    // rectangle surrounding the text.
    //
    // FIXME: Handle RTL text.

    // Horizontal align.  Default is TA_LEFT.
    if ((m_textAlignMode & TA_HORZMASK) == TA_CENTER)
        x -= (textWidth / 2);
    else if ((m_textAlignMode & TA_HORZMASK) == TA_RIGHT)
        x -= textWidth;

    // Vertical align.  Default is TA_TOP
    if ((m_textAlignMode & TA_VERTMASK) == TA_BASELINE)
        y -= (textHeight - fm.descent());
    else if ((m_textAlignMode & TA_VERTMASK) == TA_BOTTOM) {
        y -= textHeight;
    }

#if DEBUG_EMFPAINT
    kDebug(31000) << "textWidth = " << textWidth << "height = " << textHeight;

    kDebug(31000) << "font = " << m_painter->font()
                  << "pointSize = " << m_painter->font().pointSize()
                  << "ascent = " << fm.ascent() << "descent = " << fm.descent()
                  << "height = " << fm.height()
                  << "leading = " << fm.leading();
    kDebug(31000) << "actual point = " << x << y;
#endif

    // Debug code that paints a rectangle around the output area.
#if DEBUG_TEXTOUT
    m_painter->save();
    m_painter->setWorldTransform(m_outputTransform);
    m_painter->setPen(Qt::black);
    m_painter->drawRect(bounds);
    m_painter->restore();
#endif

    // Actual painting starts here.
    m_painter->save();

    // Find out how much we have to scale the text to make it fit into
    // the output rectangle.  Normally this wouldn't be necessary, but
    // when fonts are switched, the replacement fonts are sometimes
    // wider than the original fonts.
    QRect  worldRect(m_worldTransform.mapRect(QRect(x, y, textWidth, textHeight)));
    //kDebug(31000) << "rects:" << QRect(x, y, textWidth, textHeight) << worldRect;
    qreal  scaleX = qreal(1.0);
    qreal  scaleY = qreal(1.0);
    if (bounds.width() < worldRect.width())
        scaleX = qreal(bounds.width()) / qreal(worldRect.width());
    if (bounds.height() < worldRect.height())
        scaleY = qreal(bounds.height()) / qreal(worldRect.height());
    //kDebug(31000) << "scale:" << scaleX << scaleY;

    if (scaleX < qreal(1.0) || scaleY < qreal(1.0)) {
        m_painter->translate(-x, -y);
        m_painter->scale(scaleX, scaleY);
        m_painter->translate(x / scaleX, y / scaleY);
    }

    // Use the special pen defined by mTextPen for text.
    QPen  savePen = m_painter->pen();
    m_painter->setPen(m_textPen);
    m_painter->drawText(int(x / scaleX), int(y / scaleY), textWidth, textHeight,
                        Qt::AlignLeft|Qt::AlignTop, text);
    m_painter->setPen(savePen);

    m_painter->restore();
}

void OutputPainterStrategy::moveToEx( const qint32 x, const qint32 y )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << x << y;
#endif

    if ( m_currentlyBuildingPath )
        m_path->moveTo( QPoint( x, y ) );
    else
        m_currentCoords = QPoint( x, y );
}

void OutputPainterStrategy::lineTo( const QPoint &finishPoint )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << finishPoint;
#endif

    if ( m_currentlyBuildingPath )
        m_path->lineTo( finishPoint );
    else {
        m_painter->drawLine( m_currentCoords, finishPoint );
        m_currentCoords = finishPoint;
    }
}

void OutputPainterStrategy::arcTo( const QRect &box, const QPoint &start, const QPoint &end )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << box << start << end;
#endif

    QPoint centrePoint = box.center();

    qreal startAngle = angleFromArc( centrePoint, start );
    qreal endAngle   = angleFromArc( centrePoint, end );
    qreal spanAngle  = angularSpan( startAngle, endAngle );

    m_path->arcTo( box, startAngle, spanAngle );
}

void OutputPainterStrategy::polygon16( const QRect &bounds, const QList<QPoint> points )
{
    Q_UNUSED( bounds );

#if DEBUG_EMFPAINT
    kDebug(31000) << bounds << points;
#endif

    QVector<QPoint> pointVector = points.toVector();
    m_painter->drawPolygon( pointVector.constData(), pointVector.size(), m_fillRule );
}

void OutputPainterStrategy::polyLine( const QRect &bounds, const QList<QPoint> points )
{
    Q_UNUSED( bounds );

#if DEBUG_EMFPAINT
    kDebug(31000) << bounds << points;
#endif

    QVector<QPoint> pointVector = points.toVector();
    m_painter->drawPolyline( pointVector.constData(), pointVector.size() );
}

void OutputPainterStrategy::polyLine16( const QRect &bounds, const QList<QPoint> points )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << bounds << points;
#endif

    polyLine( bounds, points );
}

void OutputPainterStrategy::polyPolygon16( const QRect &bounds, const QList< QVector< QPoint > > &points )
{
    Q_UNUSED( bounds );

#if DEBUG_EMFPAINT
    kDebug(31000) << bounds << points;
#endif

    for ( int i = 0; i < points.size(); ++i ) {
        m_painter->drawPolygon( points[i].constData(), points[i].size(), m_fillRule );
    }
}

void OutputPainterStrategy::polyPolyLine16( const QRect &bounds, const QList< QVector< QPoint > > &points )
{
    Q_UNUSED( bounds );

#if DEBUG_EMFPAINT
    kDebug(31000) << bounds << points;
#endif

    for ( int i = 0; i < points.size(); ++i ) {
        m_painter->drawPolyline( points[i].constData(), points[i].size() );
    }
}

void OutputPainterStrategy::polyLineTo16( const QRect &bounds, const QList<QPoint> points )
{
    Q_UNUSED( bounds );

#if DEBUG_EMFPAINT
    kDebug(31000) << bounds << points;
#endif

    for ( int i = 0; i < points.count(); ++i ) {
	m_path->lineTo( points[i] );
    }
}

void OutputPainterStrategy::polyBezier16( const QRect &bounds, const QList<QPoint> points )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << bounds << points;
#endif

    Q_UNUSED( bounds );
    QPainterPath path;
    path.moveTo( points[0] );
    for ( int i = 1; i < points.count(); i+=3 ) {
	path.cubicTo( points[i], points[i+1], points[i+2] );
    }
    m_painter->drawPath( path );
}

void OutputPainterStrategy::polyBezierTo16( const QRect &bounds, const QList<QPoint> points )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << bounds << points;
#endif

    Q_UNUSED( bounds );
    for ( int i = 0; i < points.count(); i+=3 ) {
	m_path->cubicTo( points[i], points[i+1], points[i+2] );
    }
}

void OutputPainterStrategy::fillPath( const QRect &bounds )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << bounds;
#endif

    Q_UNUSED( bounds );
    m_painter->fillPath( *m_path, m_painter->brush() );
}

void OutputPainterStrategy::strokeAndFillPath( const QRect &bounds )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << bounds;
#endif

    Q_UNUSED( bounds );
    m_painter->drawPath( *m_path );
}

void OutputPainterStrategy::strokePath( const QRect &bounds )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << bounds;
#endif

    Q_UNUSED( bounds );
    m_painter->strokePath( *m_path, m_painter->pen() );
}

void OutputPainterStrategy::setClipPath( const quint32 regionMode )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << hex << regionMode << dec;
#endif

    switch ( regionMode ) {
    case RGN_AND:
        m_painter->setClipPath( *m_path, Qt::IntersectClip );
        break;
    case RGN_OR:
        m_painter->setClipPath( *m_path, Qt::UniteClip );
        break;
    case RGN_COPY:
        m_painter->setClipPath( *m_path, Qt::ReplaceClip );
        break;
    default:
        qWarning() <<  "Unexpected / unsupported clip region mode:" << regionMode;
        Q_ASSERT( 0 );
    }
}

void OutputPainterStrategy::bitBlt( BitBltRecord &bitBltRecord )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << bitBltRecord.xDest() << bitBltRecord.yDest()
                  << bitBltRecord.cxDest() << bitBltRecord.cyDest()
                  << hex << bitBltRecord.rasterOperation() << dec
                  << bitBltRecord.bkColorSrc();
#endif

    QRect target( bitBltRecord.xDest(), bitBltRecord.yDest(),
                  bitBltRecord.cxDest(), bitBltRecord.cyDest() );
    // 0x00f00021 is the PatCopy raster operation which just fills a rectangle with a brush.
    // This seems to be the most common one.
    //
    // FIXME: Implement the rest of the raster operations.
    if (bitBltRecord.rasterOperation() == 0x00f00021) {
        // Would have been nice if we didn't have to pull out the
        // brush to use it with fillRect()...
        QBrush brush = m_painter->brush();
        m_painter->fillRect(target, brush);
    }
    else if ( bitBltRecord.hasImage() ) {
        m_painter->drawImage( target, bitBltRecord.image() );
    }
}

void OutputPainterStrategy::setStretchBltMode( const quint32 stretchMode )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << hex << stretchMode << dec;
#endif

    switch ( stretchMode ) {
    case 0x01:
        kDebug(33100) << "EMR_STRETCHBLTMODE: STRETCH_ANDSCANS";
        break;
    case 0x02:
        kDebug(33100) << "EMR_STRETCHBLTMODE: STRETCH_ORSCANS";
        break;
    case 0x03:
        kDebug(33100) << "EMR_STRETCHBLTMODE: STRETCH_DELETESCANS";
        break;
    case 0x04:
        kDebug(33100) << "EMR_STRETCHBLTMODE: STRETCH_HALFTONE";
        break;
    default:
        kDebug(33100) << "EMR_STRETCHBLTMODE - unknown stretch mode:" << stretchMode;
    }
}

void OutputPainterStrategy::stretchDiBits( StretchDiBitsRecord &record )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << "Bounds:    " << record.bounds();
    kDebug(31000) << "Dest rect: "
                  << record.xDest() << record.yDest() << record.cxDest() << record.cyDest();
    kDebug(31000) << "Src rect:  "
                  << record.xSrc() << record.ySrc() << record.cxSrc() << record.cySrc();
    kDebug(31000) << "Raster op: " << hex << record.rasterOperation() << dec;
                  //<< record.bkColorSrc();
    kDebug(31000) << "usageSrc: " << record.usageSrc();
#endif

    QPoint targetPosition( record.xDest(), record.yDest() );
    QSize  targetSize( record.cxDest(), record.cyDest() );

    QPoint sourcePosition( record.xSrc(), record.ySrc() );
    QSize  sourceSize( record.cxSrc(), record.cySrc() );

    // special cases, from [MS-EMF] Section 2.3.1.7:
    // "This record specifies a mirror-image copy of the source bitmap to the
    // destination if the signs of the height or width fields differ. That is,
    // if cxSrc and cxDest have different signs, this record specifies a mirror
    // image of the source bitmap along the x-axis. If cySrc and cyDest have
    // different signs, this record specifies a mirror image of the source
    //  bitmap along the y-axis."
    QRect target( targetPosition, targetSize );
    QRect source( sourcePosition, sourceSize );
#if DEBUG_EMFPAINT
    //kDebug(31000) << "image size" << record.image()->size();
    kDebug(31000) << "Before transformation:";
    kDebug(31000) << "    target" << target;
    kDebug(31000) << "    source" << source;
#endif
    if ( source.width() < 0 && target.width() > 0 ) {
        sourceSize.rwidth() *= -1;
        sourcePosition.rx() -= sourceSize.width();
        source = QRect( sourcePosition, sourceSize );
    }
    if  ( source.width() > 0 && target.width() < 0 ) {
        targetSize.rwidth() *= -1;
        targetPosition.rx() -= targetSize.width();
        target = QRect( targetPosition, targetSize );
    }
    if ( source.height() < 0 && target.height() > 0 ) {
        sourceSize.rheight() *= -1;
        sourcePosition.ry() -= sourceSize.height();
        source = QRect( sourcePosition, sourceSize );
    }
    if  ( source.height() > 0 && target.height() < 0 ) {
        targetSize.rheight() *= -1;
        targetPosition.ry() -= targetSize.height();
        target = QRect( targetPosition, targetSize );
    }
#if DEBUG_EMFPAINT
    kDebug(31000) << "After transformation:";
    kDebug(31000) << "    target" << target;
    kDebug(31000) << "    source" << source;
    QImage image = record.image();
    kDebug(31000) << "Image" << image.size();

#endif

    QPainter::RenderHints      oldRenderHints = m_painter->renderHints();
    QPainter::CompositionMode  oldCompMode    = m_painter->compositionMode();
    m_painter->setRenderHints(0); // Antialiasing makes composition modes invalid
    m_painter->setCompositionMode(rasteropToQtComposition(record.rasterOperation()));
    m_painter->drawImage(target, record.image(), source);
    m_painter->setCompositionMode(oldCompMode);
    m_painter->setRenderHints(oldRenderHints);
}


// ----------------------------------------------------------------
//                         Private functions


void OutputPainterStrategy::printPainterTransform(const char *leadText)
{
    QTransform  transform;

    recalculateWorldTransform();

    kDebug(31000) << leadText << "world transform " << m_worldTransform
                  << "incl output transform: " << m_painter->transform();
}


qreal OutputPainterStrategy::angleFromArc( const QPoint &centrePoint, const QPoint &radialPoint )
{
    double dX = radialPoint.x() - centrePoint.x();
    double dY = centrePoint.y() - radialPoint.y();
    // Qt angles are in degrees. atan2 returns radians
    return ( atan2( dY, dX ) * 180 / M_PI );
}

qreal OutputPainterStrategy::angularSpan( const qreal startAngle, const qreal endAngle )
{
    qreal spanAngle = endAngle - startAngle;

    if ( spanAngle <= 0 ) {
        spanAngle += 360;
    }

    return spanAngle;
}

int OutputPainterStrategy::convertFontWeight( quint32 emfWeight )
{
    // FIXME: See how it's done in the wmf library and check if this is suitable here.

    if ( emfWeight == 0 ) {
        return QFont::Normal;
    } else if ( emfWeight <= 200 ) {
        return QFont::Light;
    } else if ( emfWeight <= 450 ) {
        return QFont::Normal;
    } else if ( emfWeight <= 650 ) {
        return QFont::DemiBold;
    } else if ( emfWeight <= 850 ) {
        return QFont::Bold;
    } else {
        return QFont::Black;
    }
}

static QPainter::CompositionMode  rasteropToQtComposition(long rop)
{
    // Code copied from filters/libkowmf/qwmf.cc
    // FIXME: Should be cleaned up

    /* TODO: Ternary raster operations
    0x00C000CA  dest = (source AND pattern)
    0x00F00021  dest = pattern
    0x00FB0A09  dest = DPSnoo
    0x005A0049  dest = pattern XOR dest   */
    static const struct OpTab {
        long winRasterOp;
        QPainter::CompositionMode qtRasterOp;
    } opTab[] = {
        // ### untested (conversion from Qt::RasterOp)
        { 0x00CC0020, QPainter::CompositionMode_Source }, // CopyROP
        { 0x00EE0086, QPainter::RasterOp_SourceOrDestination }, // OrROP
        { 0x008800C6, QPainter::RasterOp_SourceAndDestination }, // AndROP
        { 0x00660046, QPainter::RasterOp_SourceXorDestination }, // XorROP
        // ----------------------------------------------------------------
        // FIXME: Checked above this, below is still todo
        // ----------------------------------------------------------------
        { 0x00440328, QPainter::CompositionMode_DestinationOut }, // AndNotROP
        { 0x00330008, QPainter::CompositionMode_DestinationOut }, // NotCopyROP
        { 0x001100A6, QPainter::CompositionMode_SourceOut }, // NandROP
        { 0x00C000CA, QPainter::CompositionMode_Source }, // CopyROP
        { 0x00BB0226, QPainter::CompositionMode_Destination }, // NotOrROP
        { 0x00F00021, QPainter::CompositionMode_Source }, // CopyROP
        { 0x00FB0A09, QPainter::CompositionMode_Source }, // CopyROP
        { 0x005A0049, QPainter::CompositionMode_Source }, // CopyROP
        { 0x00550009, QPainter::CompositionMode_DestinationOut }, // NotROP
        { 0x00000042, QPainter::CompositionMode_Clear }, // ClearROP
        { 0x00FF0062, QPainter::CompositionMode_Source } // SetROP
    };

    int i;
    for (i = 0 ; i < 15 ; i++)
        if (opTab[i].winRasterOp == rop)
            break;

    if (i < 15)
        return opTab[i].qtRasterOp;
    else
        return QPainter::CompositionMode_Source;
}

} // xnamespace...
