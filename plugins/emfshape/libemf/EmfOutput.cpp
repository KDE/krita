/*
  Copyright 2008 Brad Hards  <bradh@frogmouth.net>
  Copyright 2009 Inge Wallin <inge@lysator.liu.se>

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

#include "EmfOutput.h"

#include <QDebug>

#include <math.h>

namespace Libemf
{

/*****************************************************************************/
DebugOutput::DebugOutput()
{
}

DebugOutput::~DebugOutput()
{
}

void DebugOutput::init( const Header *header )
{
    qDebug() << "Initialising DebugOutput";
    qDebug() << "image size:" << header->bounds().size();
}

void DebugOutput::cleanup( const Header *header )
{
    qDebug() << "Cleanup DebugOutput";
    qDebug() << "image size:" << header->bounds().size();
}

void DebugOutput::eof()
{
    qDebug() << "EMR_EOF";
}

void DebugOutput::setPixelV( QPoint &point, quint8 red, quint8 green, quint8 blue, quint8 reserved )
{
    Q_UNUSED( reserved );
    qDebug() << "EMR_SETPIXELV:" << point << QColor( red, green, blue );
}

void DebugOutput::beginPath()
{
    qDebug() << "EMR_BEGINPATH";
}

void DebugOutput::closeFigure()
{
    qDebug() << "EMR_CLOSEFIGURE";
}

void DebugOutput::endPath()
{
    qDebug() << "EMR_ENDPATH";
}

void DebugOutput::saveDC()
{
    qDebug() << "EMR_SAVEDC";
}

void DebugOutput::restoreDC( qint32 savedDC )
{
    qDebug() << "EMR_RESTOREDC" << savedDC;
}

void DebugOutput::setMetaRgn()
{
    qDebug() << "EMR_SETMETARGN";
}

void DebugOutput::setWindowOrgEx( const QPoint &origin )
{
    qDebug() << "EMR_SETWINDOWORGEX" << origin;
}

void DebugOutput::setWindowExtEx( const QSize &size )
{
    qDebug() << "EMR_SETWINDOWEXTEX" << size;
}

void DebugOutput::setViewportOrgEx( const QPoint &origin )
{
    qDebug() << "EMR_SETVIEWPORTORGEX" << origin;
}

void DebugOutput::setViewportExtEx( const QSize &size )
{
    qDebug() << "EMR_SETVIEWPORTEXTEX" << size;
}

void DebugOutput::deleteObject( const quint32 ihObject )
{
    qDebug() << "EMR_DELETEOBJECT:" << ihObject;
}

void DebugOutput::arc( const QRect &box, const QPoint &start, const QPoint &end )
{
    qDebug() << "EMR_ARC" << box << start << end;
}

void DebugOutput::chord( const QRect &box, const QPoint &start, const QPoint &end )
{
    qDebug() << "EMR_CHORD" << box << start << end;
}

void DebugOutput::pie( const QRect &box, const QPoint &start, const QPoint &end )
{
    qDebug() << "EMR_PIE" << box << start << end;
}

void DebugOutput::ellipse( const QRect &box )
{
    qDebug() << "EMR_ELLIPSE:" << box;
}

void DebugOutput::rectangle( const QRect &box )
{
    qDebug() << "EMR_RECTANGLE:" << box;
}

void DebugOutput::modifyWorldTransform( quint32 mode, float M11, float M12,
					float M21, float M22, float Dx, float Dy )
{
    qDebug() << "EMR_MODIFYWORLDTRANSFORM:" << mode << QMatrix ( M11, M12, M21, M22, Dx, Dy );
}

void DebugOutput::setWorldTransform( float M11, float M12, float M21,
				     float M22, float Dx, float Dy )
{
    qDebug() << "EMR_SETWORLDTRANSFORM:" << QMatrix ( M11, M12, M21, M22, Dx, Dy );
}

void DebugOutput::setMapMode( quint32 mapMode )
{
    QString modeAsText;
    switch ( mapMode ) {
    case 0x1:
	modeAsText = QString( "map mode - text" );
	break;
    case 0x6:
	modeAsText = QString( "map mode - twips" );
	break;
    case 0x7:
	modeAsText = QString( "map mode - isotropic" );
	break;
    case 0x8:
	modeAsText = QString( "map mode - anisotropic" );
	break;
    default:
	modeAsText = QString( "unexpected map mode: %1").arg( mapMode );
    }
    qDebug() << "EMR_SETMAPMODE:" << modeAsText;

}

void DebugOutput::setBkMode( const quint32 backgroundMode )
{
    if ( backgroundMode == TRANSPARENT ) {
        qDebug() << "EMR_SETBKMODE: Transparent";
    } else if ( backgroundMode == OPAQUE ) {
        qDebug() << "EMR_SETBKMODE: Opaque";
    } else {
        qDebug() << "EMR_SETBKMODE: Unexpected value -" << backgroundMode;
        Q_ASSERT( 0 );
    }
}

void DebugOutput::setPolyFillMode( const quint32 polyFillMode )
{
    if ( polyFillMode == ALTERNATE ) {
	qDebug() << "EMR_SETPOLYFILLMODE: OddEvenFill";
    } else if ( polyFillMode == WINDING ) {
	qDebug() << "EMR_SETPOLYFILLMODE: WindingFill";
    } else {
	qDebug() << "EMR_SETPOLYFILLMODE: Unexpected value -" << polyFillMode;
	Q_ASSERT( 0 );
    }
}

void DebugOutput::setLayout( const quint32 layoutMode )
{
    qDebug() << "EMR_SETLAYOUT:" << layoutMode;
}

void DebugOutput::extCreateFontIndirectW( const ExtCreateFontIndirectWRecord &extCreateFontIndirectW )
{
    qDebug() << "EMR_CREATEFONTINDIRECTW:" << extCreateFontIndirectW.fontFace();
}

void DebugOutput::setTextAlign( const quint32 textAlignMode )
{
    qDebug() << "EMR_SETTEXTALIGN:" << textAlignMode;
}

void DebugOutput::setTextColor( const quint8 red, const quint8 green, const quint8 blue,
				const quint8 reserved )
{
    Q_UNUSED( reserved );
    qDebug() << "EMR_SETTEXTCOLOR" << QColor( red, green, blue );
}

void DebugOutput::setBkColor( const quint8 red, const quint8 green, const quint8 blue,
                              const quint8 reserved )
{
    Q_UNUSED( reserved );
    qDebug() << "EMR_SETBKCOLOR" << QColor( red, green, blue );
}

void DebugOutput::createPen( quint32 ihPen, quint32 penStyle, quint32 x, quint32 y,
			       quint8 red, quint8 green, quint8 blue, quint8 reserved )
{
    Q_UNUSED( y );
    Q_UNUSED( reserved );

    qDebug() << "EMR_CREATEPEN" << "ihPen:" << ihPen << ", penStyle:" << penStyle
             << "width:" << x << "color:" << QColor( red, green, blue );
}

void DebugOutput::createBrushIndirect( quint32 ihBrush, quint32 BrushStyle, quint8 red,
				       quint8 green, quint8 blue, quint8 reserved,
				       quint32 BrushHatch )
{
    Q_UNUSED( reserved );

    qDebug() << "EMR_CREATEBRUSHINDIRECT:" << ihBrush << "style:" << BrushStyle
             << "Colour:" << QColor( red, green, blue ) << ", Hatch:" << BrushHatch;
}

void DebugOutput::selectObject( const quint32 ihObject )
{
    qDebug() << "EMR_SELECTOBJECT" << ihObject;
}

void DebugOutput::extTextOutA( const ExtTextOutARecord &extTextOutA )
{
    qDebug() << "EMR_EXTTEXTOUTA:" << extTextOutA.referencePoint()
             << extTextOutA.textString();
}

void DebugOutput::extTextOutW( const QPoint &referencePoint, const QString &textString )
{
    qDebug() << "EMR_EXTTEXTOUTW:" << referencePoint << textString;
}

void DebugOutput::moveToEx( const quint32 x, const quint32 y )
{
    qDebug() << "EMR_MOVETOEX" << QPoint( x, y );
}

void DebugOutput::lineTo( const QPoint &finishPoint )
{
    qDebug() << "EMR_LINETO" << finishPoint;
}

void DebugOutput::arcTo( const QRect &box, const QPoint &start, const QPoint &end )
{
    qDebug() << "EMR_ARCTO" << box << start << end;
}

void DebugOutput::polygon16( const QRect &bounds, const QList<QPoint> points )
{
    qDebug() << "EMR_POLYGON16" << bounds << points;
}

void DebugOutput::polyLine( const QRect &bounds, const QList<QPoint> points )
{
    qDebug() << "EMR_POLYLINE" << bounds << points;
}

void DebugOutput::polyLine16( const QRect &bounds, const QList<QPoint> points )
{
    qDebug() << "EMR_POLYLINE16" << bounds << points;
}

void DebugOutput::polyPolyLine16( const QRect &bounds, const QList< QVector< QPoint > > &points )
{
    qDebug() << "EMR_POLYPOLYLINE16" << bounds << points;
}

void DebugOutput::polyPolygon16( const QRect &bounds, const QList< QVector< QPoint > > &points )
{
    qDebug() << "EMR_POLYPOLYGON16" << bounds << points;
}

void DebugOutput::polyLineTo16( const QRect &bounds, const QList<QPoint> points )
{
    qDebug() << "EMR_POLYLINETO16" << bounds << points;
}

void DebugOutput::polyBezier16( const QRect &bounds, const QList<QPoint> points )
{
    qDebug() << "EMR_POLYBEZIER16" << bounds << points;
}

void DebugOutput::polyBezierTo16( const QRect &bounds, const QList<QPoint> points )
{
    qDebug() << "EMR_POLYBEZIERTO16" << bounds << points;
}

void DebugOutput::fillPath( const QRect &bounds )
{
    qDebug() << "EMR_FILLPATH" << bounds;
}

void DebugOutput::strokeAndFillPath( const QRect &bounds )
{
    qDebug() << "EMR_STROKEANDFILLPATH" << bounds;
}

void DebugOutput::strokePath( const QRect &bounds )
{
    qDebug() << "EMR_STROKEPATH" << bounds;
}

void DebugOutput::setClipPath( quint32 regionMode )
{
   qDebug() << "EMR_SETCLIPPATH:" << regionMode;
}

void DebugOutput::bitBlt( BitBltRecord bitBltRecord )
{
    qDebug() << "EMR_BITBLT:" << bitBltRecord.destinationRectangle();
}

void DebugOutput::setStretchBltMode( const quint32 stretchMode )
{
    switch ( stretchMode ) {
    case 0x01:
        qDebug() << "EMR_STRETCHBLTMODE: STRETCH_ANDSCANS";
        break;
    case 0x02:
        qDebug() << "EMR_STRETCHBLTMODE: STRETCH_ORSCANS";
        break;
    case 0x03:
        qDebug() << "EMR_STRETCHBLTMODE: STRETCH_DELETESCANS";
        break;
    case 0x04:
        qDebug() << "EMR_STRETCHBLTMODE: STRETCH_HALFTONE";
        break;
    default:
        qDebug() << "EMR_STRETCHBLTMODE - unknown stretch mode:" << stretchMode;
    }
}

void DebugOutput::stretchDiBits( StretchDiBitsRecord stretchDiBitsRecord )
{
    qDebug() << "EMR_STRETCHDIBITS:" << stretchDiBitsRecord.sourceRectangle()
             << "," << stretchDiBitsRecord.destinationRectangle();
}


// ================================================================
//                         Class PainterOutput


PainterOutput::PainterOutput() :
    m_path( 0 ), 
    m_currentlyBuildingPath( false ), 
    m_image( 0 ), 
    m_currentCoords()
{
}

PainterOutput::PainterOutput(QPainter &painter, QSize &size, 
                             bool keepAspectRatio)
    : m_path( 0 )
    , m_currentlyBuildingPath( false )
    , m_image( 0 )
    , m_currentCoords()
{
    m_painter         = &painter;
    m_painterSaves    = 0;
    m_outputSize      = size;
    m_keepAspectRatio = keepAspectRatio;
}

PainterOutput::~PainterOutput()
{
    //delete m_painter;
    delete m_path;
    delete m_image;
}

void PainterOutput::init( const Header *header )
{
    QSize  emfSize = header->bounds().size();

    //qDebug("emfSize    = %d, %d", emfSize.width(), emfSize.height() );
    //qDebug("outputSize = %d, %d", m_outputSize.width(), m_outputSize.height() );

    // Calculate how much the painter should be resized to fill the
    // outputSize with output.
    qreal  scaleX = qreal( m_outputSize.width() )  / emfSize.width();
    qreal  scaleY = qreal( m_outputSize.height() ) / emfSize.height();
    if ( m_keepAspectRatio ) {
        // Use the smaller value so that we don't get an overflow in
        // any direction.
        if ( scaleX > scaleY )
            scaleX = scaleY;
        else
            scaleY = scaleX;

    // FIXME: Calculate translation if we should center the Emf in the
    //        area and keep the aspect ration.
    }

    // This is restored in cleanup().
    m_painter->save();

    m_painter->scale( scaleX, scaleY );
}

void PainterOutput::cleanup( const Header *header )
{
    Q_UNUSED( header );

    // Restore all the save()s that were done during the processing.
    for (int i = 0; i < m_painterSaves; ++i)
        m_painter->restore();
    m_painterSaves = 0;

    // Restore the painter to what it was before init() was called.
    m_painter->restore();
}


void PainterOutput::eof()
{
}

void PainterOutput::setPixelV( QPoint &point, quint8 red, quint8 green, quint8 blue, quint8 reserved )
{
    Q_UNUSED( reserved );

    m_painter->save();

    QPen pen;
    pen.setColor( QColor( red, green, blue ) );
    m_painter->setPen( pen );
    m_painter->drawPoint( point );

    m_painter->restore();
}

QImage* PainterOutput::image()
{
    return m_image;
}

   
void PainterOutput::beginPath()
{
    delete( m_path );
    m_path = new QPainterPath;
    m_currentlyBuildingPath = true;
}

void PainterOutput::closeFigure()
{
    m_path->closeSubpath();
}

void PainterOutput::endPath()
{
    m_path->setFillRule( m_fillRule );
    m_currentlyBuildingPath = false;
}

void PainterOutput::saveDC()
{
    m_painter->save();
    ++m_painterSaves;
}

void PainterOutput::restoreDC( const qint32 savedDC )
{
    for (int i = -1; i >= savedDC; --i) {
        m_painter->restore();
    }
}

void PainterOutput::setMetaRgn()
{
    qDebug() << "EMR_SETMETARGN not yet implemented";
}

void PainterOutput::setWindowOrgEx( const QPoint &origin )
{
    QSize windowSize = m_painter->window().size();
    m_painter->setWindow( QRect( origin, windowSize ) );
}

void PainterOutput::setWindowExtEx( const QSize &size )
{
    QPoint windowOrigin = m_painter->window().topLeft();
    m_painter->setWindow( QRect( windowOrigin, size ) );
}

void PainterOutput::setViewportOrgEx( const QPoint &origin )
{
    QSize viewportSize = m_painter->viewport().size();
    m_painter->setViewport( QRect( origin, viewportSize ) );
}

void PainterOutput::setViewportExtEx( const QSize &size )
{
    QPoint viewportOrigin = m_painter->viewport().topLeft();
    m_painter->setViewport( QRect( viewportOrigin, size ) );
}

void PainterOutput::createPen( quint32 ihPen, quint32 penStyle, quint32 x, quint32 y,
			       quint8 red, quint8 green, quint8 blue, quint8 reserved )
{
    Q_UNUSED( y );
    Q_UNUSED( reserved );

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
        qDebug() << "UserStyle pen not yet supported";
        Q_ASSERT( 0 );
        break;
    case PS_ALTERNATE:
        qDebug() << "Alternate pen not yet supported";
        Q_ASSERT( 0 );
        break;
    default:
        qDebug() << "unexpected pen type" << (penStyle & 0xF);
        Q_ASSERT( 0 );
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
        qDebug() << "unexpected cap style" << (penStyle & PS_ENDCAP_FLAT);
        Q_ASSERT( 0 ); 
    }
    pen.setWidth( x );

    m_objectTable.insert( ihPen,  pen );
}

void PainterOutput:: createBrushIndirect( quint32 ihBrush, quint32 brushStyle, 
                                          quint8 red, quint8 green, quint8 blue,
                                          quint8 reserved,
					  quint32 brushHatch )
{
    Q_UNUSED( reserved );
    Q_UNUSED( brushHatch );

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

int PainterOutput::convertFontWeight( quint32 emfWeight )
{
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

void PainterOutput::extCreateFontIndirectW( const ExtCreateFontIndirectWRecord &extCreateFontIndirectW )
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

void PainterOutput::selectStockObject( const quint32 ihObject )
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

void PainterOutput::selectObject( const quint32 ihObject )
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
	    qDebug() << "Unexpected type:" << obj.typeName();
	}
    }
}

void PainterOutput::deleteObject( const quint32 ihObject )
{
    m_objectTable.take( ihObject );
}

void PainterOutput::arc( const QRect &box, const QPoint &start, const QPoint &end )
{
    QPoint centrePoint = box.center();

    qreal startAngle = angleFromArc( centrePoint, start );

    qreal endAngle = angleFromArc( centrePoint, end );

    qreal spanAngle = angularSpan( startAngle, endAngle );

    m_painter->drawArc( box, startAngle*16, spanAngle*16 );
}

void PainterOutput::chord( const QRect &box, const QPoint &start, const QPoint &end )
{
    QPoint centrePoint = box.center();

    qreal startAngle = angleFromArc( centrePoint, start );

    qreal endAngle = angleFromArc( centrePoint, end );

    qreal spanAngle = angularSpan( startAngle, endAngle );

    m_painter->drawChord( box, startAngle*16, spanAngle*16 ); 
}

void PainterOutput::pie( const QRect &box, const QPoint &start, const QPoint &end )
{
    QPoint centrePoint = box.center();

    qreal startAngle = angleFromArc( centrePoint, start );

    qreal endAngle = angleFromArc( centrePoint, end );

    qreal spanAngle = angularSpan( startAngle, endAngle );

    m_painter->drawPie( box, startAngle*16, spanAngle*16 );
}

void PainterOutput::ellipse( const QRect &box )
{
    m_painter->drawEllipse( box );
}

void PainterOutput::rectangle( const QRect &box )
{
    m_painter->drawRect( box );
}

void PainterOutput::setMapMode( const quint32 mapMode )
{
    qDebug() << "Set map mode not yet implemented" << mapMode;
}

void PainterOutput::setBkMode( const quint32 backgroundMode )
{
    if ( backgroundMode == TRANSPARENT ) {
        m_painter->setBackgroundMode( Qt::TransparentMode );
    } else if ( backgroundMode == OPAQUE ) {
        m_painter->setBackgroundMode( Qt::OpaqueMode );
    } else {
        qDebug() << "EMR_SETBKMODE: Unexpected value -" << backgroundMode;
        Q_ASSERT( 0 );
    }
}

void PainterOutput::setPolyFillMode( const quint32 polyFillMode )
{
    if ( polyFillMode == ALTERNATE ) {
	m_fillRule = Qt::OddEvenFill;
    } else if ( polyFillMode == WINDING ) {
	m_fillRule = Qt::WindingFill;
    } else {
	qDebug() << "EMR_SETPOLYFILLMODE: Unexpected value -" << polyFillMode;
	Q_ASSERT( 0 );
    }
}

void PainterOutput::setLayout( const quint32 layoutMode )
{
    if ( layoutMode == LAYOUT_LTR ) {
        m_painter->setLayoutDirection( Qt::LeftToRight );
    } else if ( layoutMode == LAYOUT_RTL ) {
        m_painter->setLayoutDirection( Qt::RightToLeft );
    } else {
        qDebug() << "EMR_SETLAYOUT: Unexpected value -" << layoutMode;
        Q_ASSERT( 0 );
    }
}

void PainterOutput::setTextAlign( const quint32 textAlignMode )
{
    m_textAlignMode = textAlignMode;
}

void PainterOutput::setTextColor( const quint8 red, const quint8 green, const quint8 blue,
				const quint8 reserved )
{
    Q_UNUSED( reserved );

    m_textPen.setColor( QColor( red, green, blue ) );
}

void PainterOutput::setBkColor( const quint8 red, const quint8 green, const quint8 blue,
                                const quint8 reserved )
{
    Q_UNUSED( reserved );

    m_painter->setBackground( QBrush( QColor( red, green, blue ) ) );
}

void PainterOutput::modifyWorldTransform( const quint32 mode, float M11, float M12,
					  float M21, float M22, float Dx, float Dy )
{
    QMatrix matrix( M11, M12, M21, M22, Dx, Dy);

    if ( mode == MWT_IDENTITY ) {
        m_painter->setWorldMatrix( QMatrix() );
    } else if ( mode == MWT_LEFTMULTIPLY ) {
	m_painter->setWorldMatrix( matrix, true );
    } else if ( mode == MWT_RIGHTMULTIPLY ) {
        QMatrix currentMatrix = m_painter->worldMatrix();
        QMatrix newMatrix = currentMatrix * matrix;
        m_painter->setWorldMatrix( newMatrix );
    } else if ( mode == MWT_SET ) {
	m_painter->setWorldMatrix( matrix );
    } else {
	qWarning() << "Unimplemented transform mode" << mode;
    }
}

void PainterOutput::setWorldTransform( float M11, float M12, float M21,
				       float M22, float Dx, float Dy )
{
    QMatrix matrix( M11, M12, M21, M22, Dx, Dy);

    m_painter->setWorldMatrix( matrix );
}

void PainterOutput::extTextOutA( const ExtTextOutARecord &extTextOutA )
{
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
            qDebug() << "Unexpected vertical positioning mode:" << m_textAlignMode;
    }
    // TODO: Handle the rest of the test alignment mode flags

    m_painter->drawText( position, extTextOutA.textString() );

    m_painter->restore();
}

void PainterOutput::extTextOutW( const QPoint &referencePoint, const QString &textString )
{
    m_painter->save();

    m_painter->setPen( m_textPen );
    m_painter->drawText( referencePoint, textString );

    m_painter->restore();
}

void PainterOutput::moveToEx( const quint32 x, const quint32 y )
{
#if 1
    if ( currentlyBuildingPath() )
        m_path->moveTo( QPoint( x, y ) );
    else
        m_currentCoords = QPoint( x, y );

#else
    // This is what the code looked before.  I think the
    // interpretation above is the correct one.
    if ( ! currentlyBuildingPath() ) {
        beginPath();
    }
    m_path->moveTo( QPoint( x, y ) );
#endif
}

void PainterOutput::lineTo( const QPoint &finishPoint )
{
#if 1
    if ( currentlyBuildingPath() )
        m_path->lineTo( finishPoint );
    else {
        m_painter->drawLine( m_currentCoords, finishPoint );
        m_currentCoords = finishPoint;
    }

#else
    // This is what the code looked before.  I think the
    // interpretation above is the correct one.
    m_path->lineTo( finishPoint );
#endif
}

qreal PainterOutput::angleFromArc( const QPoint &centrePoint, const QPoint &radialPoint )
{
    double dX = radialPoint.x() - centrePoint.x();
    double dY = centrePoint.y() - radialPoint.y();
    // Qt angles are in degrees. atan2 returns radians
    return ( atan2( dY, dX ) * 180 / M_PI );
}

qreal PainterOutput::angularSpan( const qreal startAngle, const qreal endAngle )
{
    qreal spanAngle = endAngle - startAngle;
    
    if ( spanAngle <= 0 ) {
        spanAngle += 360;
    }
    
    return spanAngle;
}

void PainterOutput::arcTo( const QRect &box, const QPoint &start, const QPoint &end )
{
    QPoint centrePoint = box.center();

    qreal startAngle = angleFromArc( centrePoint, start );

    qreal endAngle = angleFromArc( centrePoint, end );

    qreal spanAngle = angularSpan( startAngle, endAngle );

    m_path->arcTo( box, startAngle, spanAngle );
}

void PainterOutput::polygon16( const QRect &bounds, const QList<QPoint> points )
{
    Q_UNUSED( bounds );
    QVector<QPoint> pointVector = points.toVector();
    m_painter->drawPolygon( pointVector.constData(), pointVector.size(), m_fillRule );
}

void PainterOutput::polyLine( const QRect &bounds, const QList<QPoint> points )
{
    Q_UNUSED( bounds );
    QVector<QPoint> pointVector = points.toVector();
    m_painter->drawPolyline( pointVector.constData(), pointVector.size() );
}

void PainterOutput::polyLine16( const QRect &bounds, const QList<QPoint> points )
{
    polyLine( bounds, points );
}

void PainterOutput::polyPolygon16( const QRect &bounds, const QList< QVector< QPoint > > &points )
{
    Q_UNUSED( bounds );

    for ( int i = 0; i < points.size(); ++i ) {
        m_painter->drawPolygon( points[i].constData(), points[i].size(), m_fillRule );
    }
}

void PainterOutput::polyPolyLine16( const QRect &bounds, const QList< QVector< QPoint > > &points )
{
    Q_UNUSED( bounds );

    for ( int i = 0; i < points.size(); ++i ) {
        m_painter->drawPolyline( points[i].constData(), points[i].size() );
    }
}

void PainterOutput::polyLineTo16( const QRect &bounds, const QList<QPoint> points )
{
    Q_UNUSED( bounds );
    for ( int i = 0; i < points.count(); ++i ) {
	m_path->lineTo( points[i] );
    }
}

void PainterOutput::polyBezier16( const QRect &bounds, const QList<QPoint> points )
{
    Q_UNUSED( bounds );
    QPainterPath path;
    path.moveTo( points[0] );
    for ( int i = 1; i < points.count(); i+=3 ) {
	path.cubicTo( points[i], points[i+1], points[i+2] );
    }
    m_painter->drawPath( path );
}

void PainterOutput::polyBezierTo16( const QRect &bounds, const QList<QPoint> points )
{
    Q_UNUSED( bounds );
    for ( int i = 0; i < points.count(); i+=3 ) {
	m_path->cubicTo( points[i], points[i+1], points[i+2] );
    }
}

void PainterOutput::fillPath( const QRect &bounds )
{
    Q_UNUSED( bounds );
    m_painter->fillPath( *m_path, m_painter->brush() );
}

void PainterOutput::strokeAndFillPath( const QRect &bounds )
{
    Q_UNUSED( bounds );
    m_painter->drawPath( *m_path );
}

void PainterOutput::strokePath( const QRect &bounds )
{
    Q_UNUSED( bounds );
    m_painter->strokePath( *m_path, m_painter->pen() );
}

void PainterOutput::setClipPath( const quint32 regionMode )
{
    switch ( regionMode ) {
        case RGN_AND:
        {
            m_painter->setClipPath( *m_path, Qt::IntersectClip );
        }
        break;
        case RGN_OR:
        {
            m_painter->setClipPath( *m_path, Qt::UniteClip );
        }
        break;
        case RGN_COPY:
        {
            m_painter->setClipPath( *m_path, Qt::ReplaceClip );
        }
        break;
        default:
        {
            qWarning() <<  "Unexpected / unsupported clip region mode:" << regionMode; 
            Q_ASSERT( 0 );
        }
    }
}

bool PainterOutput::currentlyBuildingPath() const
{
    return ( m_currentlyBuildingPath );
}

void PainterOutput::bitBlt( BitBltRecord bitBltRecord )
{
    QRect target( bitBltRecord.xDest(), bitBltRecord.yDest(),
                  bitBltRecord.cxDest(), bitBltRecord.cyDest() );
    if ( bitBltRecord.hasImage() ) {
        m_painter->drawImage( target, *(bitBltRecord.image()) );
    }
}

void PainterOutput::setStretchBltMode( const quint32 stretchMode )
{
    switch ( stretchMode ) {
    case 0x01:
        qDebug() << "EMR_STRETCHBLTMODE: STRETCH_ANDSCANS";
        break;
    case 0x02:
        qDebug() << "EMR_STRETCHBLTMODE: STRETCH_ORSCANS";
        break;
    case 0x03:
        qDebug() << "EMR_STRETCHBLTMODE: STRETCH_DELETESCANS";
        break;
    case 0x04:
        qDebug() << "EMR_STRETCHBLTMODE: STRETCH_HALFTONE";
        break;
    default:
        qDebug() << "EMR_STRETCHBLTMODE - unknown stretch mode:" << stretchMode;
    }
}

void PainterOutput::stretchDiBits( StretchDiBitsRecord stretchDiBitsRecord )
{
    QPoint targetPosition( stretchDiBitsRecord.xDest(),
                           stretchDiBitsRecord.yDest() );
    QSize targetSize( stretchDiBitsRecord.cxDest(),
                      stretchDiBitsRecord.cyDest() );

    QPoint sourcePosition( stretchDiBitsRecord.xSrc(),
                           stretchDiBitsRecord.ySrc() );
    QSize sourceSize( stretchDiBitsRecord.cxSrc(),
                      stretchDiBitsRecord.cySrc() );

    // special cases, from [MS-EMF] Section 2.3.1.7:
    // "This record specifies a mirror-image copy of the source bitmap to the
    // destination if the signs of the height or width fields differ. That is,
    // if cxSrc and cxDest have different signs, this record specifies a mirror
    // image of the source bitmap along the x-axis. If cySrc and cyDest have
    // different signs, this record specifies a mirror image of the source
    //  bitmap along the y-axis."
    QRect target( targetPosition, targetSize );
    QRect source( sourcePosition, sourceSize );
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
    m_painter->drawImage( target, *(stretchDiBitsRecord.image()), source );
}

} // xnamespace...
