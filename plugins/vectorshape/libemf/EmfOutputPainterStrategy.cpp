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

#include <KDebug>


namespace Libemf
{


// ================================================================
//                         Class OutputPainterStrategy


OutputPainterStrategy::OutputPainterStrategy() :
    m_header( 0 ),
    m_path( 0 ),
    m_currentlyBuildingPath( false ),
    m_image( 0 ),
    m_currentCoords()
{
}

OutputPainterStrategy::OutputPainterStrategy(QPainter &painter, QSize &size,
                                             bool keepAspectRatio)
    : m_header( 0 )
    , m_path( 0 )
    , m_currentlyBuildingPath( false )
    , m_image( 0 )
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
    //delete m_painter;
    delete m_path;
    delete m_image;
}

void OutputPainterStrategy::paintBounds(const Header *header)
{
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
    //
    // To be precise, it seems that the StretchDiBits record uses the
    // physical size (stored in header.frame()) to specify where to
    // draw the picture rather than logical coordinates.
    m_header = new Header(*header);

    QSize  outputSize = header->bounds().size();

#if DEBUG_EMFPAINT
    kDebug(31000) << "----------------------------------------------------------------------";
    kDebug(31000) << "emfFrame (phys size) =" << header->frame().x() << header->frame().y()
                  << header->frame().width() << header->frame().height();
    kDebug(31000) << "emfBounds (log size) =" << header->bounds().x() << header->bounds().y()
                  << header->bounds().width() << header->bounds().height();
    kDebug(31000) << "outputSize           =" << m_outputSize.width() << m_outputSize.height();

    kDebug(31000) << "Device =" << header->device().width() << header->device().height();
    kDebug(31000) << "Millimeters =" << header->millimeters().width()
                  << header->millimeters().height();
#endif

    // This is restored in cleanup().
    m_painter->save();

    // Calculate how much the painter should be resized to fill the
    // outputSize with output.
    qreal  scaleX = qreal( m_outputSize.width() )  / outputSize.width();
    qreal  scaleY = qreal( m_outputSize.height() ) / outputSize.height();
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

    // Transform the EMF object so that it fits in the shape.
    m_painter->scale( scaleX, scaleY );
    m_painter->translate(-header->bounds().left(), -header->bounds().top());

    // Calculate translation if we should center the Emf in the
    // area and keep the aspect ratio.
    if ( m_keepAspectRatio ) {
        m_painter->translate((m_outputSize.width() / scaleX - outputSize.width()) / 2,
                             (m_outputSize.height() / scaleY - outputSize.height()) / 2);
    }

#if DEBUG_EMFPAINT
    paintBounds(header);
#endif
}

void OutputPainterStrategy::cleanup( const Header *header )
{
    Q_UNUSED( header );

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

QImage* OutputPainterStrategy::image()
{
    return m_image;
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

    m_painter->save();
    ++m_painterSaves;
}

void OutputPainterStrategy::restoreDC( const qint32 savedDC )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << savedDC;
#endif

    for (int i = 0; i < savedDC; ++i) {
        if (m_painterSaves > 0) {
            m_painter->restore();
            --m_painterSaves;
        }
        else {
            kDebug(33100) << "restoreDC(): try to restore painter without save";
        }
    }
}

void OutputPainterStrategy::setMetaRgn()
{
    kDebug(33100) << "EMR_SETMETARGN not yet implemented";
}

void OutputPainterStrategy::setWindowOrgEx( const QPoint &origin )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << origin;
#endif
    return;
    QSize windowSize = m_painter->window().size();
    m_painter->setWindow( QRect( origin, windowSize ) );
}

void OutputPainterStrategy::setWindowExtEx( const QSize &size )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << size;
#endif
    return;
    QPoint windowOrigin = m_painter->window().topLeft();
    m_painter->setWindow( QRect( windowOrigin, size ) );
}

void OutputPainterStrategy::setViewportOrgEx( const QPoint &origin )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << origin;
#endif

    return;
    QSize viewportSize = m_painter->viewport().size();
    m_painter->setViewport( QRect( origin, viewportSize ) );
}

void OutputPainterStrategy::setViewportExtEx( const QSize &size )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << size;
#endif

    return;
    QPoint viewportOrigin = m_painter->viewport().topLeft();
    m_painter->setViewport( QRect( viewportOrigin, size ) );
}

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

void OutputPainterStrategy:: createBrushIndirect( quint32 ihBrush, quint32 brushStyle,
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
	Q_ASSERT( 0 );
	break;
    case ANSI_FIXED_FONT:
	Q_ASSERT( 0 );
	break;
    case ANSI_VAR_FONT:
	Q_ASSERT( 0 );
	break;
    case SYSTEM_FONT:
	// TODO: handle this
	break;
    case DEVICE_DEFAULT_FONT:
	// TODO: handle this
	break;
    case DEFAULT_PALETTE:
	Q_ASSERT( 0 );
	break;
    case SYSTEM_FIXED_FONT:
	Q_ASSERT( 0 );
	break;
    case DEFAULT_GUI_FONT:
	Q_ASSERT( 0 );
	break;
    case DC_BRUSH:
	Q_ASSERT( 0 );
	break;
    case DC_PEN:
	Q_ASSERT( 0 );
	break;
    default:
	qWarning() << "Unexpected stock object:" << ( ihObject & 0x8000000 );
    }
}

void OutputPainterStrategy::selectObject( const quint32 ihObject )
{
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

void OutputPainterStrategy::modifyWorldTransform( const quint32 mode, float M11, float M12,
                                                  float M21, float M22, float Dx, float Dy )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << mode << M11 << M12 << M21 << M22 << Dx << Dy;
#endif

    QTransform matrix( M11, M12, M21, M22, Dx, Dy);

    if ( mode == MWT_IDENTITY ) {
        m_painter->setWorldTransform( QTransform() );
    } else if ( mode == MWT_LEFTMULTIPLY ) {
	m_painter->setWorldTransform( matrix, true );
    } else if ( mode == MWT_RIGHTMULTIPLY ) {
        QTransform currentMatrix = m_painter->worldTransform();
        QTransform newMatrix = currentMatrix * matrix;
        m_painter->setWorldTransform( newMatrix );
    } else if ( mode == MWT_SET ) {
	m_painter->setWorldTransform( matrix );
    } else {
	qWarning() << "Unimplemented transform mode" << mode;
    }
}

void OutputPainterStrategy::setWorldTransform( float M11, float M12, float M21,
                                               float M22, float Dx, float Dy )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << M11 << M12 << M21 << M22 << Dx << Dy;
#endif

    QTransform matrix( M11, M12, M21, M22, Dx, Dy);

    m_painter->setWorldTransform( matrix );
}

void OutputPainterStrategy::extTextOutA( const ExtTextOutARecord &extTextOutA )
{
#if DEBUG_EMFPAINT
    //kDebug(31000) << extTextOutA;  FIXME
#endif

    m_painter->save();

    m_painter->setPen( m_textPen );

    QPoint position = extTextOutA.referencePoint();
    QFontMetrics fontMetrics = m_painter->fontMetrics();
    switch ( m_textAlignMode & TA_VERTMASK ) {
        case TA_TOP:
            position += QPoint( 0, fontMetrics.ascent() );
            break;
        case TA_BOTTOM:
            position -= QPoint( 0, fontMetrics.descent() );
            break;
        case TA_BASELINE:
            // do nothing
            break;
        default:
            kDebug(33100) << "Unexpected vertical positioning mode:" << m_textAlignMode;
    }
    // TODO: Handle the rest of the test alignment mode flags

    m_painter->drawText( position, extTextOutA.textString() );

    kDebug(33100) << "extTextOutA: ref.point = "
                  << extTextOutA.referencePoint().x() << extTextOutA.referencePoint().y()
                  << ", Text = " << extTextOutA.textString().toLatin1().data();

    m_painter->restore();
}

void OutputPainterStrategy::extTextOutW( const QPoint &referencePoint, const QString &text )
{
#if DEBUG_EMFPAINT
    kDebug(31000) << referencePoint << text;
#endif

    int  x = referencePoint.x();
    int  y = referencePoint.y();

    // The TA_UPDATECP flag tells us to use the current position
    if (m_textAlignMode & TA_UPDATECP) {
        // (left, top) position = current logical position
        x = m_currentCoords.x();
        y = m_currentCoords.y();
    }

    QFontMetrics  fm = m_painter->fontMetrics();
    int width  = fm.width(text) + fm.descent();    // fm.width(text) isn't right with Italic text
    int height = fm.height();

    // Horizontal align.  These flags are supposed to be mutually exclusive.
    if ((m_textAlignMode & TA_CENTER) == TA_CENTER)
        x -= (width / 2);
    else if ((m_textAlignMode & TA_RIGHT) == TA_RIGHT)
        x -= width;

    // Vertical align.
    if ((m_textAlignMode & TA_BASELINE) == TA_BASELINE)
        y -= fm.ascent();  // (height - fm.descent()) is used in qwmf.  This should be the same.
    else if ((m_textAlignMode & TA_BOTTOM) == TA_BOTTOM) {
        y -= height;

#if DEBUG_EMFPAINT
        kDebug(31000) << "font = " << m_painter->font() << " pointSize = " << m_painter->font().pointSize()
                      << "ascent = " << fm.ascent() << " height = " << fm.height()
                      << "leading = " << fm.leading();
#endif
    }

    // Use the special pen defined by mTextPen for text.
    QPen  savePen = m_painter->pen();
    m_painter->setPen(m_textPen);

    m_painter->drawText(x, y, width, height, Qt::AlignLeft|Qt::AlignTop, text);

    m_painter->setPen(savePen);
}

void OutputPainterStrategy::moveToEx( const quint32 x, const quint32 y )
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
#endif

    // SRCCOPY is the simplest case.  TODO: implement the rest.
    if (record.rasterOperation() == 0x00cc0020) {
        // For some reason, the target coordinates for the picture are
        // expressed in physical coordinates (Frame in the header)
        // instead of logical coordinates like all the other types of
        // records.  Therefore we have to rescale the target from
        // physical to logical coordinates.
        qreal scaleX = qreal(m_header->frame().width()) / qreal(m_header->bounds().width());
        qreal scaleY = qreal(m_header->frame().height()) / qreal(m_header->bounds().height());
#if DEBUG_EMFPAINT
        kDebug(31000) << "Scale = " << scaleX << scaleY;
#endif
        QRectF realTarget(QPoint(target.x() / scaleX, target.y() / scaleY),
                          QSize(target.width() / scaleX, target.height() / scaleY));
#if DEBUG_EMFPAINT
        kDebug(31000) << "    realTarget" << realTarget;
#endif
        m_painter->drawImage(realTarget, record.image(), source);
    }
}


// ----------------------------------------------------------------
//                         Private functions


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

} // xnamespace...
