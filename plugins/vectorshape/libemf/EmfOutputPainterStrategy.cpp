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
    m_path( 0 ), 
    m_currentlyBuildingPath( false ), 
    m_image( 0 ), 
    m_currentCoords()
{
}

OutputPainterStrategy::OutputPainterStrategy(QPainter &painter, QSize &size, 
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

OutputPainterStrategy::~OutputPainterStrategy()
{
    //delete m_painter;
    delete m_path;
    delete m_image;
}

void OutputPainterStrategy::init( const Header *header )
{
    QSize  emfSize = header->bounds().size();

#if 0
    qDebug("emfOrigin  = %d, %d", header->bounds().x(), header->bounds().y());
    qDebug("emfSize    = %d, %d", emfSize.width(), emfSize.height() );
    qDebug("emfFrame   = %d, %d, %d, %d", 
           header->frame().x(), header->frame().y(),
           header->frame().width(), header->frame().height());
    qDebug("emfBounds   = %d, %d, %d, %d", 
           header->bounds().x(), header->bounds().y(),
           header->bounds().width(), header->bounds().height());
    qDebug("emfSize    = %d, %d", emfSize.width(), emfSize.height() );
    qDebug("outputSize = %d, %d", m_outputSize.width(), m_outputSize.height() );

    qDebug("Device = %d, %d", header->device().width(), header->device().height() );
    qDebug("Millimeters = %d, %d", header->millimeters().width(), header->millimeters().height() );
    kDebug(31000) << "Foo";
#endif

    // This is restored in cleanup().
    m_painter->save();

    //m_painter->setPen(QColor(255,0,0));
    //m_painter->drawRect( 0, 0, m_outputSize.width(), m_outputSize.height());// Note possible translation
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
    }
    kDebug(31000) << "scale = " << scaleX << ", " << scaleY;

    // Transform the EMF object so that it fits in the shape.
    m_painter->scale( scaleX, scaleY );
    m_painter->translate(-header->bounds().left(), -header->bounds().top());

    // Calculate translation if we should center the Emf in the
    // area and keep the aspect ratio.
    if ( m_keepAspectRatio ) {
        m_painter->translate((m_outputSize.width() - emfSize.width() * scaleX) / 2,
                             (m_outputSize.height() - emfSize.height() * scaleY) / 2);
    }
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
    delete( m_path );
    m_path = new QPainterPath;
    m_currentlyBuildingPath = true;
}

void OutputPainterStrategy::closeFigure()
{
    m_path->closeSubpath();
}

void OutputPainterStrategy::endPath()
{
    m_path->setFillRule( m_fillRule );
    m_currentlyBuildingPath = false;
}

void OutputPainterStrategy::saveDC()
{
    m_painter->save();
    ++m_painterSaves;
}

void OutputPainterStrategy::restoreDC( const qint32 savedDC )
{
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
    QSize windowSize = m_painter->window().size();
    m_painter->setWindow( QRect( origin, windowSize ) );
}

void OutputPainterStrategy::setWindowExtEx( const QSize &size )
{
    QPoint windowOrigin = m_painter->window().topLeft();
    m_painter->setWindow( QRect( windowOrigin, size ) );
}

void OutputPainterStrategy::setViewportOrgEx( const QPoint &origin )
{
    QSize viewportSize = m_painter->viewport().size();
    m_painter->setViewport( QRect( origin, viewportSize ) );
}

void OutputPainterStrategy::setViewportExtEx( const QSize &size )
{
    QPoint viewportOrigin = m_painter->viewport().topLeft();
    m_painter->setViewport( QRect( viewportOrigin, size ) );
}

void OutputPainterStrategy::createPen( quint32 ihPen, quint32 penStyle, quint32 x, quint32 y,
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
        kDebug(33100) << "UserStyle pen not yet supported";
        Q_ASSERT( 0 );
        break;
    case PS_ALTERNATE:
        kDebug(33100) << "Alternate pen not yet supported";
        Q_ASSERT( 0 );
        break;
    default:
        kDebug(33100) << "unexpected pen type" << (penStyle & 0xF);
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
        kDebug(33100) << "unexpected cap style" << (penStyle & PS_ENDCAP_FLAT);
        Q_ASSERT( 0 ); 
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

int OutputPainterStrategy::convertFontWeight( quint32 emfWeight )
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
    QPoint centrePoint = box.center();

    qreal startAngle = angleFromArc( centrePoint, start );

    qreal endAngle = angleFromArc( centrePoint, end );

    qreal spanAngle = angularSpan( startAngle, endAngle );

    m_painter->drawArc( box, startAngle*16, spanAngle*16 );
}

void OutputPainterStrategy::chord( const QRect &box, const QPoint &start, const QPoint &end )
{
    QPoint centrePoint = box.center();

    qreal startAngle = angleFromArc( centrePoint, start );

    qreal endAngle = angleFromArc( centrePoint, end );

    qreal spanAngle = angularSpan( startAngle, endAngle );

    m_painter->drawChord( box, startAngle*16, spanAngle*16 ); 
}

void OutputPainterStrategy::pie( const QRect &box, const QPoint &start, const QPoint &end )
{
    QPoint centrePoint = box.center();

    qreal startAngle = angleFromArc( centrePoint, start );

    qreal endAngle = angleFromArc( centrePoint, end );

    qreal spanAngle = angularSpan( startAngle, endAngle );

    m_painter->drawPie( box, startAngle*16, spanAngle*16 );
}

void OutputPainterStrategy::ellipse( const QRect &box )
{
    kDebug(31000) << "ellipse at " << box;
    m_painter->drawEllipse( box );
}

void OutputPainterStrategy::rectangle( const QRect &box )
{
    m_painter->drawRect( box );
}

void OutputPainterStrategy::setMapMode( const quint32 mapMode )
{
    kDebug(33100) << "Set map mode not yet implemented" << mapMode;
}

void OutputPainterStrategy::setBkMode( const quint32 backgroundMode )
{
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
    m_textAlignMode = textAlignMode;
}

void OutputPainterStrategy::setTextColor( const quint8 red, const quint8 green, const quint8 blue,
                                          const quint8 reserved )
{
    Q_UNUSED( reserved );

    m_textPen.setColor( QColor( red, green, blue ) );
}

void OutputPainterStrategy::setBkColor( const quint8 red, const quint8 green, const quint8 blue,
                                        const quint8 reserved )
{
    Q_UNUSED( reserved );

    m_painter->setBackground( QBrush( QColor( red, green, blue ) ) );
}

void OutputPainterStrategy::modifyWorldTransform( const quint32 mode, float M11, float M12,
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

void OutputPainterStrategy::setWorldTransform( float M11, float M12, float M21,
                                               float M22, float Dx, float Dy )
{
    QMatrix matrix( M11, M12, M21, M22, Dx, Dy);

    m_painter->setWorldMatrix( matrix );
}

void OutputPainterStrategy::extTextOutA( const ExtTextOutARecord &extTextOutA )
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
            kDebug(33100) << "Unexpected vertical positioning mode:" << m_textAlignMode;
    }
    // TODO: Handle the rest of the test alignment mode flags

    m_painter->drawText( position, extTextOutA.textString() );

    kDebug(33100) << "extTextOutA: ref.point = " 
                  << extTextOutA.referencePoint().x() << extTextOutA.referencePoint().y()
                  << ", Text = " << extTextOutA.textString().toLatin1().data();

    m_painter->restore();
}

void OutputPainterStrategy::extTextOutW( const QPoint &referencePoint, const QString &textString )
{
    m_painter->save();

    m_painter->setPen( m_textPen );
    m_painter->drawText( referencePoint, textString );

    kDebug(33100) << "extTextOutW: ref.point = " << referencePoint.x() << " " << referencePoint.y()
                  << " Text = " << textString.toLatin1().data();

    m_painter->restore();
}

void OutputPainterStrategy::moveToEx( const quint32 x, const quint32 y )
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

void OutputPainterStrategy::lineTo( const QPoint &finishPoint )
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

void OutputPainterStrategy::arcTo( const QRect &box, const QPoint &start, const QPoint &end )
{
    QPoint centrePoint = box.center();

    qreal startAngle = angleFromArc( centrePoint, start );

    qreal endAngle = angleFromArc( centrePoint, end );

    qreal spanAngle = angularSpan( startAngle, endAngle );

    m_path->arcTo( box, startAngle, spanAngle );
}

void OutputPainterStrategy::polygon16( const QRect &bounds, const QList<QPoint> points )
{
    Q_UNUSED( bounds );
    QVector<QPoint> pointVector = points.toVector();
    m_painter->drawPolygon( pointVector.constData(), pointVector.size(), m_fillRule );
}

void OutputPainterStrategy::polyLine( const QRect &bounds, const QList<QPoint> points )
{
    Q_UNUSED( bounds );
    QVector<QPoint> pointVector = points.toVector();
    m_painter->drawPolyline( pointVector.constData(), pointVector.size() );
}

void OutputPainterStrategy::polyLine16( const QRect &bounds, const QList<QPoint> points )
{
    polyLine( bounds, points );
}

void OutputPainterStrategy::polyPolygon16( const QRect &bounds, const QList< QVector< QPoint > > &points )
{
    Q_UNUSED( bounds );

    for ( int i = 0; i < points.size(); ++i ) {
        m_painter->drawPolygon( points[i].constData(), points[i].size(), m_fillRule );
    }
}

void OutputPainterStrategy::polyPolyLine16( const QRect &bounds, const QList< QVector< QPoint > > &points )
{
    Q_UNUSED( bounds );

    for ( int i = 0; i < points.size(); ++i ) {
        m_painter->drawPolyline( points[i].constData(), points[i].size() );
    }
}

void OutputPainterStrategy::polyLineTo16( const QRect &bounds, const QList<QPoint> points )
{
    Q_UNUSED( bounds );
    for ( int i = 0; i < points.count(); ++i ) {
	m_path->lineTo( points[i] );
    }
}

void OutputPainterStrategy::polyBezier16( const QRect &bounds, const QList<QPoint> points )
{
    kDebug(31000) << "bounds: " << bounds << ", points = " << points;

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
    kDebug(31000) << "bounds: " << bounds << ", points = " << points;

    Q_UNUSED( bounds );
    for ( int i = 0; i < points.count(); i+=3 ) {
	m_path->cubicTo( points[i], points[i+1], points[i+2] );
    }
}

void OutputPainterStrategy::fillPath( const QRect &bounds )
{
    Q_UNUSED( bounds );
    m_painter->fillPath( *m_path, m_painter->brush() );
}

void OutputPainterStrategy::strokeAndFillPath( const QRect &bounds )
{
    Q_UNUSED( bounds );
    m_painter->drawPath( *m_path );
}

void OutputPainterStrategy::strokePath( const QRect &bounds )
{
    Q_UNUSED( bounds );
    m_painter->strokePath( *m_path, m_painter->pen() );
}

void OutputPainterStrategy::setClipPath( const quint32 regionMode )
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

bool OutputPainterStrategy::currentlyBuildingPath() const
{
    return ( m_currentlyBuildingPath );
}

void OutputPainterStrategy::bitBlt( BitBltRecord bitBltRecord )
{
    QRect target( bitBltRecord.xDest(), bitBltRecord.yDest(),
                  bitBltRecord.cxDest(), bitBltRecord.cyDest() );
    if ( bitBltRecord.hasImage() ) {
        m_painter->drawImage( target, *(bitBltRecord.image()) );
    }
}

void OutputPainterStrategy::setStretchBltMode( const quint32 stretchMode )
{
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

void OutputPainterStrategy::stretchDiBits( StretchDiBitsRecord stretchDiBitsRecord )
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
