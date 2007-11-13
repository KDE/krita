/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoPathShape.h"
#include "KoPathPoint.h"
#include "KoPointGroup.h"
#include "KoShapeBorderModel.h"
#include "KoViewConverter.h"
#include "KoPathShapeLoader.h"
#include "KoShapeSavingContext.h"
#include "KoShapeLoadingContext.h"

#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoUnit.h>
#include <KoGenStyle.h>
#include <KoStyleStack.h>
#include <KoOasisLoadingContext.h>

#include <KDebug>
#include <QtGui/QPainter>

class KoPathShape::Private
{
public:
    Private() : fillRule( Qt::OddEvenFill )
    {
    }

    Qt::FillRule fillRule;
};

KoPathShape::KoPathShape()
    : d( new Private() ) // while we don't actually have any private data, just leave it as this.
{
}

KoPathShape::~KoPathShape()
{
    clear();
    delete d;
}

void KoPathShape::saveOdf( KoShapeSavingContext & context ) const
{
    context.xmlWriter().startElement( "draw:path" );
    saveOdfAttributes( context, OdfMandatories | OdfSize | OdfPosition | OdfTransformation );

    context.xmlWriter().addAttribute( "svg:d", toString( QMatrix() ) );

    context.xmlWriter().endElement();
}

bool KoPathShape::loadOdf( const KoXmlElement & element, KoShapeLoadingContext &context ) 
{
    loadOdfAttributes( element, context, OdfMandatories );

    // first clear the path data from the default path
    clear();

    if( element.localName() == "line" )
    {
        QPointF start;
        start.setX( KoUnit::parseValue( element.attributeNS( KoXmlNS::svg, "x1", "" ) ) );
        start.setY( KoUnit::parseValue( element.attributeNS( KoXmlNS::svg, "y1", "" ) ) );
        QPointF end;
        end.setX( KoUnit::parseValue( element.attributeNS( KoXmlNS::svg, "x2", "" ) ) );
        end.setY( KoUnit::parseValue( element.attributeNS( KoXmlNS::svg, "y2", "" ) ) );
        moveTo( start );
        lineTo( end );
    }
    else if( element.localName() == "polyline" || element.localName() == "polygon" )
    {
        QString points = element.attributeNS( KoXmlNS::draw, "points" ).simplified();
        points.replace( ',', ' ' );
        points.remove( '\r' );
        points.remove( '\n' );
        bool firstPoint = true;
        QStringList coordinateList = points.split( ' ' );
        for( QStringList::Iterator it = coordinateList.begin(); it != coordinateList.end(); ++it)
        {
            QPointF point;
            point.setX( (*it).toDouble() );
            ++it;
            point.setY( (*it).toDouble() );
            if( firstPoint )
            {
                moveTo( point );
                firstPoint = false;
            }
            else
                lineTo( point );
        }
        if( element.localName() == "polygon" ) 
            close();
    }
    else // path loading
    {
        KoPathShapeLoader loader( this );
        loader.parseSvg( element.attributeNS( KoXmlNS::svg, "d" ), true );
    }

    QPointF pos;
    pos.setX( KoUnit::parseValue( element.attributeNS( KoXmlNS::svg, "x", QString() ) ) );
    pos.setY( KoUnit::parseValue( element.attributeNS( KoXmlNS::svg, "y", QString() ) ) );
    setPosition( pos );

    applyViewboxTransformation( element );

    normalize();

    loadOdfAttributes( element, context, OdfTransformation );

    return true;
}

QString KoPathShape::saveStyle( KoGenStyle &style, KoShapeSavingContext &context ) const
{
    style.addProperty( "svg:fill-rule", d->fillRule == Qt::OddEvenFill ? "evenodd" : "nonzero" );

    return KoShape::saveStyle( style, context );
}

void KoPathShape::loadStyle( const KoXmlElement & element, KoShapeLoadingContext &context )
{
    KoShape::loadStyle( element, context );
    KoStyleStack &styleStack = context.koLoadingContext().styleStack();
    if ( styleStack.hasProperty( KoXmlNS::svg, "fill-rule" ) )
    {
        QString rule = styleStack.property( KoXmlNS::svg, "fill-rule" );
        d->fillRule = rule == "nonzero" ?  Qt::WindingFill : Qt::OddEvenFill;
    }
}

QRectF KoPathShape::loadOdfViewbox( const KoXmlElement & element ) const
{
    QRectF viewbox;

    QString data = element.attributeNS( KoXmlNS::svg, "viewBox" );
    if( ! data.isEmpty() )
    {
        data.replace( ',', ' ' );
        QStringList coordinates = data.simplified().split( ' ', QString::SkipEmptyParts );
        if( coordinates.count() == 4 )
        {
            viewbox.setRect( coordinates[0].toDouble(), coordinates[1].toDouble(),
                             coordinates[2].toDouble(), coordinates[3].toDouble() );
        }
    }

    return viewbox;
}

void KoPathShape::applyViewboxTransformation( const KoXmlElement & element )
{
    // apply viewbox transformation
    QRectF viewBox = loadOdfViewbox( element );
    if( ! viewBox.isEmpty() )
    {
        // load the desired size
        QSizeF size;
        size.setWidth( KoUnit::parseValue( element.attributeNS( KoXmlNS::svg, "width", QString() ) ) );
        size.setHeight( KoUnit::parseValue( element.attributeNS( KoXmlNS::svg, "height", QString() ) ) );

        // load the desired position
        QPointF pos;
        pos.setX( KoUnit::parseValue( element.attributeNS( KoXmlNS::svg, "x", QString() ) ) );
        pos.setY( KoUnit::parseValue( element.attributeNS( KoXmlNS::svg, "y", QString() ) ) );

        // create matrix to transform original path data into desired size and position
        QMatrix viewMatrix;
        viewMatrix.translate( -viewBox.left(), -viewBox.top() );
        viewMatrix.scale( size.width()/viewBox.width(), size.height()/viewBox.height() );
        viewMatrix.translate( pos.x(), pos.y() );

        // transform the path data
        map( viewMatrix );
    }
}

void KoPathShape::clear()
{
    foreach( KoSubpath *subpath, m_subpaths )
    {
        foreach( KoPathPoint *point, *subpath )
            delete point;
        delete subpath;
    }
    m_subpaths.clear();
}

void KoPathShape::paint( QPainter &painter, const KoViewConverter &converter )
{
    applyConversion( painter, converter );
    QPainterPath path( outline() );
    path.setFillRule( d->fillRule );

    painter.setBrush( background() );
    painter.drawPath( path );
    //paintDebug( painter );
}

#ifndef NDEBUG
void KoPathShape::paintDebug( QPainter &painter )
{
    KoSubpathList::const_iterator pathIt( m_subpaths.begin() );
    int i = 0;

    QPen pen( Qt::black );
    painter.save();
    painter.setPen( pen );
    for ( ; pathIt != m_subpaths.end(); ++pathIt )
    {
        KoSubpath::const_iterator it( ( *pathIt )->begin() );
        for ( ; it != ( *pathIt )->end(); ++it )
        {
            ++i;
            KoPathPoint *point = ( *it );
            QRectF r( point->point(), QSizeF( 5, 5 ) );
            r.translate( -2.5, -2.5 );
            QPen pen( Qt::black );
            painter.setPen( pen );
            if ( point->group() )
            {
                QBrush b( Qt::blue );
                painter.setBrush( b );
            }
            else if ( point->properties() & KoPathPoint::CanHaveControlPoint1 && point->properties() & KoPathPoint::CanHaveControlPoint2 )
            {
                QBrush b( Qt::red );
                painter.setBrush( b );
            }
            else if ( point->properties() & KoPathPoint::CanHaveControlPoint1 ) 
            {
                QBrush b( Qt::yellow );
                painter.setBrush( b );
            }
            else if ( point->properties() & KoPathPoint::CanHaveControlPoint2 ) 
            {
                QBrush b( Qt::darkYellow );
                painter.setBrush( b );
            }
            painter.drawEllipse( r );
        }
    }
    painter.restore();
    kDebug(30006) <<"nop =" << i;
}

void KoPathShape::debugPath()
{
    KoSubpathList::iterator pathIt( m_subpaths.begin() );
    for ( ; pathIt != m_subpaths.end(); ++pathIt )
    {
        KoSubpath::const_iterator it( ( *pathIt )->begin() );
        for ( ; it != ( *pathIt )->end(); ++it )
        {
            kDebug(30006) <<"p:" << ( *pathIt ) <<"," << *it <<"," << ( *it )->point() <<"," << ( *it )->properties() <<"," << ( *it )->group();
        }
    }
}
#endif

QPointF KoPathShape::shapeToDocument( const QPointF &point ) const
{
    return absoluteTransformation(0).map( point );
}

QRectF KoPathShape::shapeToDocument( const QRectF &rect ) const 
{
    return absoluteTransformation(0).mapRect( rect );
}

QPointF KoPathShape::documentToShape( const QPointF &point ) const
{
    return absoluteTransformation(0).inverted().map( point );
}

QRectF KoPathShape::documentToShape( const QRectF &rect ) const 
{
    return absoluteTransformation(0).inverted().mapRect( rect );
}


void KoPathShape::paintPoints( QPainter &painter, const KoViewConverter &converter, int handleRadius )
{
    applyConversion( painter, converter );

    KoSubpathList::const_iterator pathIt( m_subpaths.begin() );

    QRectF handle = converter.viewToDocument( handleRect( QPoint(0,0), handleRadius ) );

    for ( ; pathIt != m_subpaths.end(); ++pathIt )
    {
        KoSubpath::const_iterator it( ( *pathIt )->begin() );
        for ( ; it != ( *pathIt )->end(); ++it )
        {
            KoPathPoint *point = ( *it );
            point->paint( painter, handle.size(), KoPathPoint::Node );
        }
    }
}

QRectF KoPathShape::handleRect( const QPointF &p, double radius ) const
{
    return QRectF( p.x()-radius, p.y()-radius, 2*radius, 2*radius );
}

const QPainterPath KoPathShape::outline() const
{
    KoSubpathList::const_iterator pathIt( m_subpaths.begin() );
    QPainterPath path;
    for ( ; pathIt != m_subpaths.end(); ++pathIt )
    {
        KoSubpath::const_iterator it( ( *pathIt )->begin() );
        KoPathPoint * lastPoint( *it );
        bool activeCP = false;
        for ( ; it != ( *pathIt )->end(); ++it )
        {
            if ( it == ( *pathIt )->begin() )
            {
                if ( ( *it )->properties() & KoPathPoint::StartSubpath )
                {
                    //qDebug() << "moveTo(" << ( *it )->point() << ")";
                    path.moveTo( ( *it )->point() );
                }
            }
            else if ( activeCP || ( *it )->activeControlPoint1() )
            {
                //qDebug() << "cubicTo(" << ( activeCP ? lastPoint->controlPoint2() : lastPoint->point() )
                //         << "," << ( ( *it )->activeControlPoint1() ? ( *it )->controlPoint1() : ( *it )->point() )
                //         << "," << ( *it )->point() << ")";

                path.cubicTo( activeCP ? lastPoint->controlPoint2() : lastPoint->point()
                            , ( *it )->activeControlPoint1() ? ( *it )->controlPoint1() : ( *it )->point()
                            , ( *it )->point() );
            }
            else
            {
                //qDebug() << "lineTo(" << ( *it )->point() << ")";
                path.lineTo( ( *it )->point() );
            }
            if ( ( *it )->properties() & KoPathPoint::CloseSubpath )
            {
                // add curve when there is a curve on the way to the first point
                KoPathPoint * firstPoint = ( *pathIt )->first();
                if ( ( *it )->activeControlPoint2() || firstPoint->activeControlPoint1() )
                {
                    //qDebug() << "cubicTo(" << ( ( *it )->activeControlPoint2() ? ( *it )->controlPoint2() : ( *it )->point() )
                    //         << "," << ( firstPoint->activeControlPoint1() ? firstPoint->controlPoint1() : firstPoint->point() )
                    //         << "," << firstPoint->point() << ")";
                    path.cubicTo( (*it)->activeControlPoint2() ? ( *it )->controlPoint2() : ( *it )->point()
                                , firstPoint->activeControlPoint1() ? firstPoint->controlPoint1() : firstPoint->point()
                                , firstPoint->point() );
                }
                //qDebug() << "closeSubpath()";
                path.closeSubpath();
            }

            if ( ( *it )->activeControlPoint2() )
            {
                activeCP = true;
            }
            else
            {
                activeCP = false;
            }
            lastPoint = *it;
        }
    }
    return path;
}

QRectF KoPathShape::boundingRect() const
{
    QRectF bb( outline().boundingRect() );
    if( border() )
    {
        KoInsets inset;
        border()->borderInsets( this, inset );
        bb.adjust( -inset.left, -inset.top, inset.right, inset.bottom );
    }
    //qDebug() << "KoPathShape::boundingRect = " << bb;
    return absoluteTransformation( 0 ).mapRect( bb );
}


QSizeF KoPathShape::size() const
{
    // don't call boundingRect here as it uses absoluteTransformation which leads to infinite reccursion
    return outline().boundingRect().size();
}

QPointF KoPathShape::position() const
{
    //return boundingRect().topLeft();
    return KoShape::position();
}

void KoPathShape::setSize( const QSizeF &newSize )
{
    QSizeF oldSize = size();
    double zoomX = newSize.width() / oldSize.width(); 
    double zoomY = newSize.height() / oldSize.height(); 
    QMatrix matrix( zoomX, 0, 0, zoomY, 0, 0 );

    //qDebug() << "setSize" << zoomX << "," << zoomY << "," << newSize;
    KoShape::setSize( newSize );
    map( matrix );
}

KoPathPoint * KoPathShape::moveTo( const QPointF &p )
{
    KoPathPoint * point = new KoPathPoint( this, p, KoPathPoint::StartSubpath | KoPathPoint::CanHaveControlPoint2 );
    KoSubpath * path = new KoSubpath;
    path->push_back( point );
    m_subpaths.push_back( path );
    return point;
}

KoPathPoint * KoPathShape::lineTo( const QPointF &p )
{
    if ( m_subpaths.empty() )
    {
        moveTo( QPointF( 0, 0 ) );
    }
    KoPathPoint * point = new KoPathPoint( this, p, KoPathPoint::CanHaveControlPoint1 );
    KoPathPoint * lastPoint = m_subpaths.last()->last();
    updateLast( &lastPoint );
    m_subpaths.last()->push_back( point );
    return point;
}

KoPathPoint * KoPathShape::curveTo( const QPointF &c1, const QPointF &c2, const QPointF &p )
{
    if ( m_subpaths.empty() )
    {
        moveTo( QPointF( 0, 0 ) );
    }
    KoPathPoint * lastPoint = m_subpaths.last()->last();
    updateLast( &lastPoint );
    lastPoint->setControlPoint2( c1 );
    KoPathPoint * point = new KoPathPoint( this, p, KoPathPoint::CanHaveControlPoint1 );
    point->setControlPoint1( c2 );
    m_subpaths.last()->push_back( point );
    return point;
}

KoPathPoint * KoPathShape::arcTo( double rx, double ry, double startAngle, double sweepAngle )
{
    if ( m_subpaths.empty() )
    {
        moveTo( QPointF( 0, 0 ) );
    }

    KoPathPoint * lastPoint = m_subpaths.last()->last();
    if ( lastPoint->properties() & KoPathPoint::CloseSubpath )
    {
        lastPoint = m_subpaths.last()->first();
    }
    QPointF startpoint( lastPoint->point() );

    KoPathPoint * newEndPoint = lastPoint;

    QPointF curvePoints[12];
    int pointCnt = arcToCurve( rx, ry, startAngle, sweepAngle, startpoint, curvePoints );
    for ( int i = 0; i < pointCnt; i += 3 )
    {
        newEndPoint = curveTo( curvePoints[i], curvePoints[i+1], curvePoints[i+2] );
    }
    return newEndPoint;
}

int KoPathShape::arcToCurve( double rx, double ry, double startAngle, double sweepAngle, const QPointF & offset, QPointF * curvePoints ) const
{
    int pointCnt = 0;

    // check Parameters
    if ( sweepAngle == 0 )
        return pointCnt;
    if (  sweepAngle > 360 )
        sweepAngle = 360;
    else if (  sweepAngle < -360 )
        sweepAngle = - 360;

    if ( rx == 0 || ry == 0 )
    {
        //TODO
    }

    // split angles bigger than 90° so that it gives a good aproximation to the circle
    double parts = ceil( qAbs( sweepAngle / 90.0 ) );

    double sa_rad = startAngle * M_PI / 180.0;
    double partangle = sweepAngle / parts;
    double endangle = startAngle + partangle;
    double se_rad = endangle * M_PI / 180.0;
    double sinsa = sin( sa_rad );
    double cossa = cos( sa_rad );
    double kappa = 4.0 / 3.0 * tan( ( se_rad - sa_rad ) / 4 );

    // startpoint is at the last point is the path but when it is closed
    // it is at the first point
    QPointF startpoint( offset );

    //center berechnen
    QPointF center( startpoint - QPointF( cossa * rx, -sinsa * ry ) );

    //kDebug(30006) <<"kappa" << kappa <<"parts" << parts;;
    
    for ( int part = 0; part < parts; ++part )
    {
        // start tangent
        curvePoints[pointCnt++] = QPointF( startpoint - QPointF( sinsa * rx * kappa, cossa * ry * kappa ) );

        double sinse = sin( se_rad );
        double cosse = cos( se_rad );

        // end point
        QPointF endpoint( center + QPointF( cosse * rx, -sinse * ry ) );
        // end tangent
        curvePoints[pointCnt++] = QPointF( endpoint - QPointF( -sinse * rx * kappa, -cosse * ry * kappa ) );
        curvePoints[pointCnt++] = endpoint;

        // set the endpoint as next start point
        startpoint = endpoint;
        sinsa = sinse;
        cossa = cosse;
        endangle += partangle;
        se_rad = endangle * M_PI / 180.0;
    }

    return pointCnt;
}

void KoPathShape::close()
{
    if ( m_subpaths.empty() )
    {
        return;
    }
    closeSubpath( m_subpaths.last() );
}

void KoPathShape::closeMerge()
{
    if ( m_subpaths.empty() )
    {
        return;
    }
    closeMergeSubpath( m_subpaths.last() );
}

QPointF KoPathShape::normalize()
{
    QPointF oldTL( boundingRect().topLeft() );

    QPointF tl( outline().boundingRect().topLeft() );
    QMatrix matrix;
    matrix.translate( -tl.x(), -tl.y() );
    map( matrix );

    // keep the top left point of the object
    QPointF newTL( boundingRect().topLeft() );
    QPointF diff( oldTL - newTL );
    setAbsolutePosition( absolutePosition() + diff );
    return tl;
}

void KoPathShape::map( const QMatrix &matrix )
{
    KoSubpathList::iterator pathIt( m_subpaths.begin() );
    for ( ; pathIt != m_subpaths.end(); ++pathIt )
    {
        KoSubpath::iterator it( ( *pathIt )->begin() );
        for ( ; it != ( *pathIt )->end(); ++it )
        {
            ( *it )->map( matrix );
        }
    }
}

void KoPathShape::updateLast( KoPathPoint ** lastPoint )
{
    if ( ( *lastPoint )->properties() & KoPathPoint::CloseSubpath )
    {
        KoPathPoint * subpathStart = m_subpaths.last()->first();
        KoPathPoint * newLastPoint = new KoPathPoint( *subpathStart );
        newLastPoint->setProperties( KoPathPoint::Normal );
        KoPointGroup * group = subpathStart->group();
        if ( group == 0 )
        {
            group = new KoPointGroup();
            group->add( subpathStart );
        }
        group->add( newLastPoint );

        KoSubpath *path = new KoSubpath;
        path->push_back( newLastPoint );
        m_subpaths.push_back( path );
        *lastPoint = newLastPoint;
    }
    ( *lastPoint )->setProperties( ( *lastPoint )->properties() | KoPathPoint::CanHaveControlPoint2 );
}

QList<KoPathPoint*> KoPathShape::pointsAt( const QRectF &r )
{
    QList<KoPathPoint*> result;

    KoSubpathList::iterator pathIt( m_subpaths.begin() );
    for ( ; pathIt != m_subpaths.end(); ++pathIt )
    {
        KoSubpath::iterator it( ( *pathIt )->begin() );
        for ( ; it != ( *pathIt )->end(); ++it )
        {
            if( r.contains( (*it)->point() ) )
                result.append( *it );
            else if( (*it)->activeControlPoint1() && r.contains( (*it)->controlPoint1() ) )
                result.append( *it );
            else if( (*it)->activeControlPoint2() && r.contains( (*it)->controlPoint2() ) )
                result.append( *it );
        }
    }
    return result;
}


KoPathPointIndex KoPathShape::pathPointIndex( const KoPathPoint *point ) const
{
    for ( int subpathIndex = 0; subpathIndex < m_subpaths.size(); ++subpathIndex )
    {
        KoSubpath * subpath = m_subpaths.at( subpathIndex );
        for ( int pointPos = 0; pointPos < subpath->size(); ++pointPos )
        {
            if ( subpath->at( pointPos ) == point )
            {
                return KoPathPointIndex( subpathIndex, pointPos );
            }
        }
    }
    return KoPathPointIndex( -1, -1 );
}

KoPathPoint * KoPathShape::pointByIndex( const KoPathPointIndex &pointIndex ) const
{
    KoSubpath * subpath = subPath( pointIndex.first );

    if ( subpath == 0 || pointIndex.second < 0 || pointIndex.second >= subpath->size() )
        return 0;

    return subpath->at( pointIndex.second );
}

KoPathSegment KoPathShape::segmentByIndex( const KoPathPointIndex &pointIndex ) const
{
    KoPathSegment segment( 0, 0 );

    KoSubpath * subpath = subPath( pointIndex.first );

    if ( subpath != 0 && pointIndex.second >= 0 && pointIndex.second < subpath->size() )
    {
        KoPathPoint * point = subpath->at( pointIndex.second );
        int index = pointIndex.second;
        ++index;
        if ( point->properties() & KoPathPoint::CloseSubpath )
        {
            index = 0;
        }

        if ( index < subpath->size() )
        {
            segment.first = point;
            segment.second = subpath->at( index );
        }
    }
    return segment;
}

int KoPathShape::pointCount() const
{
    int i = 0;
    KoSubpathList::const_iterator pathIt( m_subpaths.begin() );
    for ( ; pathIt != m_subpaths.end(); ++pathIt )
    {
        i += (*pathIt)->size();
    }

    return i;
}

int KoPathShape::subpathCount() const
{
    return m_subpaths.count();
}

int KoPathShape::pointCountSubpath( int subpathIndex ) const
{
    KoSubpath * subpath = subPath( subpathIndex );

    if ( subpath == 0 )
        return -1;

    return subpath->size();
}

bool KoPathShape::isClosedSubpath( int subpathIndex )
{
    KoSubpath * subpath = subPath( subpathIndex );

    if ( subpath == 0 )
        return false;

    return subpath->last()->properties() & KoPathPoint::CloseSubpath;
}

bool KoPathShape::insertPoint( KoPathPoint* point, const KoPathPointIndex &pointIndex )
{
    KoSubpath * subpath = subPath( pointIndex.first );

    if ( subpath == 0 || pointIndex.second < 0 || pointIndex.second > subpath->size() )
        return false;

    KoPathPoint::KoPointProperties properties( point->properties() | KoPathPoint::CanHaveControlPoint1 | KoPathPoint::CanHaveControlPoint2 );
    properties = properties & ~KoPathPoint::StartSubpath;
    properties = properties & ~KoPathPoint::CloseSubpath;
    if ( pointIndex.second == 0 )
    {
        properties = properties | KoPathPoint::StartSubpath;
        if ( !( subpath->last()->properties() & KoPathPoint::CloseSubpath ) )
        {
            // there is no control point 1 when subpath is not closed
            properties = properties & ~KoPathPoint::CanHaveControlPoint1;
        }
        subpath->first()->setProperties( subpath->first()->properties() & ~KoPathPoint::StartSubpath | KoPathPoint::CanHaveControlPoint1 );
    }
    else if ( pointIndex.second == subpath->size() )
    {
        if ( subpath->last()->properties() & KoPathPoint::CloseSubpath )
        {
            // keep the path closed
            properties = properties | KoPathPoint::CloseSubpath;
        }
        else
        {
            // there is no control point 2 when subpath is not closed
            properties = properties & ~KoPathPoint::CanHaveControlPoint2;
            properties = properties & ~KoPathPoint::CloseSubpath;
        }
        subpath->last()->setProperties( subpath->last()->properties() & ~KoPathPoint::CloseSubpath | KoPathPoint::CanHaveControlPoint2 );
    }
    else
    {
        properties = properties & ~KoPathPoint::StartSubpath;
        properties = properties & ~KoPathPoint::CloseSubpath;
    }
    point->setProperties( properties );
    subpath->insert( pointIndex.second , point );
    return true;
}

KoPathPoint * KoPathShape::removePoint( const KoPathPointIndex &pointIndex )
{
    KoSubpath * subpath = subPath( pointIndex.first );

    if ( subpath == 0 || pointIndex.second < 0 || pointIndex.second >= subpath->size() )
        return 0;

    KoPathPoint * point = subpath->takeAt( pointIndex.second );

    if ( pointIndex.second == 0 )
    {
        // first point removed, set new StartSubpath
        if ( subpath->last()->properties() & KoPathPoint::CloseSubpath )
        {
            subpath->first()->setProperties( subpath->first()->properties() | KoPathPoint::StartSubpath );
        }
        else
        {
            subpath->first()->setProperties( ( subpath->first()->properties() & ~KoPathPoint::CanHaveControlPoint1 ) | KoPathPoint::StartSubpath );
        }
    }
    else if ( pointIndex.second == subpath->size() ) // use size as point is already removed
    {
        // last point removed, change last point to be the end
        if ( point->properties() & KoPathPoint::CloseSubpath )
        {
            subpath->last()->setProperties( subpath->last()->properties() | KoPathPoint::CloseSubpath );
        }
        else
        {
            subpath->last()->setProperties( subpath->last()->properties() & ~KoPathPoint::CanHaveControlPoint2 );
        }
    }

    return point;
}

bool KoPathShape::breakAfter( const KoPathPointIndex &pointIndex )
{
    KoSubpath * subpath = subPath( pointIndex.first );

    if ( subpath == 0 || pointIndex.second < 0 || pointIndex.second > subpath->size() - 2 
         || subpath->last()->properties() & KoPathPoint::CloseSubpath )
        return false;

    KoSubpath * newSubpath = new KoSubpath;

    int size = subpath->size();
    for ( int i = pointIndex.second + 1; i < size; ++i )
    {
        newSubpath->append( subpath->takeAt( pointIndex.second + 1 ) );
    }
    // now make the first point of the new subpath a starting node
    newSubpath->first()->setProperties( newSubpath->first()->properties() & ~KoPathPoint::CanHaveControlPoint1 | KoPathPoint::StartSubpath );
    // the last point of the old subpath is now an ending node 
    subpath->last()->setProperties( subpath->last()->properties() & ~KoPathPoint::CanHaveControlPoint2 );

    // insert the new subpath after the broken one
    m_subpaths.insert( pointIndex.first + 1, newSubpath );

    return true;
}

bool KoPathShape::join( int subpathIndex )
{
    KoSubpath * subpath = subPath( subpathIndex );
    KoSubpath * nextSubpath = subPath( subpathIndex + 1 );

    if ( subpath == 0 || nextSubpath == 0 || subpath->last()->properties() & KoPathPoint::CloseSubpath 
         || nextSubpath->last()->properties() & KoPathPoint::CloseSubpath )
        return false;

    subpath->last()->setProperties( subpath->last()->properties() | KoPathPoint::CanHaveControlPoint2 );
    nextSubpath->first()->setProperties( nextSubpath->first()->properties() & ~KoPathPoint::StartSubpath | KoPathPoint::CanHaveControlPoint1 );
    
    // append the second subpath to the first
    foreach( KoPathPoint * p, *nextSubpath )
        subpath->append( p );

    // remove the nextSubpath from path
    m_subpaths.removeAt( subpathIndex + 1 );

    // delete it as it is no longer possible to use it
    delete nextSubpath;

    return true;
}

bool KoPathShape::moveSubpath( int oldSubpathIndex, int newSubpathIndex )
{
    KoSubpath * subpath = subPath( oldSubpathIndex );

    if ( subpath == 0 || newSubpathIndex >= m_subpaths.size() )
        return false;

    m_subpaths.removeAt( oldSubpathIndex );
    m_subpaths.insert( newSubpathIndex, subpath );

    return true;
}

KoPathPointIndex KoPathShape::openSubpath( const KoPathPointIndex &pointIndex )
{
    KoSubpath * subpath = subPath( pointIndex.first );

    if ( subpath == 0 || pointIndex.second < 0 || pointIndex.second >= subpath->size() 
         || !( subpath->last()->properties() & KoPathPoint::CloseSubpath ) )
        return KoPathPointIndex( -1, -1 );

    KoPathPoint * oldStartPoint = subpath->first();
    // the old starting node no longer starts the subpath
    oldStartPoint->setProperties( oldStartPoint->properties() & ~KoPathPoint::StartSubpath | KoPathPoint::CanHaveControlPoint1 );
    // the old end node no longer closes the subpath
    subpath->last()->setProperties( subpath->last()->properties() & ~KoPathPoint::CloseSubpath | KoPathPoint::CanHaveControlPoint2 );

    // reorder the subpath
    for ( int i = 0; i < pointIndex.second; ++i )
    {
        subpath->append( subpath->takeFirst() );
    }
    // make the first point a start node
    subpath->first()->setProperties( subpath->first()->properties() & ~KoPathPoint::CanHaveControlPoint1 | KoPathPoint::StartSubpath );
    // make the last point a end node 
    subpath->last()->setProperties( subpath->last()->properties() & ~KoPathPoint::CanHaveControlPoint2 );

    return pathPointIndex( oldStartPoint );
}

KoPathPointIndex KoPathShape::closeSubpath( const KoPathPointIndex &pointIndex )
{
    KoSubpath * subpath = subPath( pointIndex.first );

    if ( subpath == 0 || pointIndex.second < 0 || pointIndex.second >= subpath->size() 
         || subpath->last()->properties() & KoPathPoint::CloseSubpath )
        return KoPathPointIndex( -1, -1 );

    KoPathPoint * oldStartPoint = subpath->first();
    // the old starting node no longer starts the subpath
    oldStartPoint->setProperties( oldStartPoint->properties() & ~KoPathPoint::StartSubpath | KoPathPoint::CanHaveControlPoint1 );
    // the old end node no longer closes the subpath
    subpath->last()->setProperties( subpath->last()->properties() | KoPathPoint::CanHaveControlPoint2 );
    
    // reorder the subpath
    for ( int i = 0; i < pointIndex.second; ++i )
    {
        subpath->append( subpath->takeFirst() );
    }
    subpath->first()->setProperties( subpath->first()->properties() | KoPathPoint::StartSubpath );

    closeSubpath( subpath );
    return pathPointIndex( oldStartPoint );
}

bool KoPathShape::reverseSubpath( int subpathIndex )
{
    KoSubpath * subpath = subPath( subpathIndex );

    if ( subpath == 0 )
        return false;

    int size = subpath->size();
    for( int i = 0; i < size; ++i )
    {
        KoPathPoint *p = subpath->takeAt( i );
        p->reverse();
        subpath->prepend( p );
    }

    // adjust the position dependent properties
    KoPathPoint *first = subpath->first();
    KoPathPoint *last = subpath->last();

    KoPathPoint::KoPointProperties firstProps = first->properties();
    KoPathPoint::KoPointProperties lastProps = last->properties();

    firstProps |= KoPathPoint::StartSubpath;
    lastProps &= ~KoPathPoint::StartSubpath;
    if( firstProps & KoPathPoint::CloseSubpath )
    {
        firstProps &= ~KoPathPoint::CloseSubpath;
        lastProps |= KoPathPoint::CloseSubpath;
    }
    first->setProperties( firstProps );
    last->setProperties( lastProps );

    return true;
}

KoSubpath * KoPathShape::removeSubpath( int subpathIndex )
{
    KoSubpath * subpath = subPath( subpathIndex );

    if ( subpath != 0 )
        m_subpaths.removeAt( subpathIndex );

    return subpath;
}

bool KoPathShape::addSubpath( KoSubpath * subpath, int subpathIndex )
{
    if ( subpathIndex < 0 || subpathIndex > m_subpaths.size() )
        return false;
    
    m_subpaths.insert( subpathIndex, subpath );

    return true;
}

bool KoPathShape::combine( KoPathShape *path )
{
    if( ! path )
        return false;

    QMatrix pathMatrix = path->absoluteTransformation(0);
    QMatrix myMatrix = absoluteTransformation(0).inverted();

    foreach( KoSubpath* subpath, path->m_subpaths )
    {
        KoSubpath *newSubpath = new KoSubpath();

        foreach( KoPathPoint* point, *subpath )
        {
            KoPathPoint *newPoint = new KoPathPoint( *point );
            newPoint->map( pathMatrix );
            newPoint->map( myMatrix );
            newPoint->setParent( this );
            newSubpath->append( newPoint );
        }
        m_subpaths.append( newSubpath );
    }
    normalize();
    return true;
}

bool KoPathShape::separate( QList<KoPathShape*> & separatedPaths )
{
    if( ! m_subpaths.size() )
        return false;

    QMatrix myMatrix = absoluteTransformation(0);

    foreach( KoSubpath* subpath, m_subpaths )
    {
        KoPathShape *shape = new KoPathShape();
        if( ! shape ) continue;

        shape->setBorder( border() );
        shape->setShapeId( shapeId() );

        KoSubpath *newSubpath = new KoSubpath();

        foreach( KoPathPoint* point, *subpath )
        {
            KoPathPoint *newPoint = new KoPathPoint( *point );
            newPoint->map( myMatrix );
            newSubpath->append( newPoint );
        }
        shape->m_subpaths.append( newSubpath );
        shape->normalize();
        separatedPaths.append( shape );
    }
    return true;
}

void KoPathShape::closeSubpath( KoSubpath *subpath )
{
    if( ! subpath )
        return;

    KoPathPoint * lastPoint = subpath->last();
    lastPoint->setProperties( lastPoint->properties() | KoPathPoint::CloseSubpath | KoPathPoint::CanHaveControlPoint2 );
    KoPathPoint * firstPoint = subpath->first();
    firstPoint->setProperties( firstPoint->properties() | KoPathPoint::CanHaveControlPoint1 );
}

void KoPathShape::closeMergeSubpath( KoSubpath *subpath )
{
    if ( ! subpath || subpath->size() < 2 )
        return;

    KoPathPoint * lastPoint = subpath->last();
    KoPathPoint * firstPoint = subpath->first();

    if ( lastPoint->point() == firstPoint->point() )
    {
        firstPoint->setProperties( firstPoint->properties() | KoPathPoint::CanHaveControlPoint1 );
        if ( lastPoint->activeControlPoint1() )
            firstPoint->setControlPoint1( lastPoint->controlPoint1() );
        removePoint( pathPointIndex( lastPoint ) );
        // remove point
        delete lastPoint;
        lastPoint = subpath->last();
        lastPoint->setProperties( lastPoint->properties() | KoPathPoint::CanHaveControlPoint2 | KoPathPoint::CloseSubpath );
    }
    else
    {
        closeSubpath( subpath );
    }
}

KoSubpath * KoPathShape::subPath( int subpathIndex ) const
{
    if ( subpathIndex < 0 || subpathIndex >= m_subpaths.size() )
        return 0;

    return m_subpaths.at( subpathIndex );
}

QString KoPathShape::pathShapeId() const
{
    return KoPathShapeId;
}

QString KoPathShape::toString( const QMatrix &matrix ) const
{
    QString d;

    KoSubpathList::const_iterator pathIt( m_subpaths.begin() );
    for ( ; pathIt != m_subpaths.end(); ++pathIt )
    {
        KoSubpath::const_iterator it( ( *pathIt )->begin() );
        KoPathPoint * lastPoint( *it );
        bool activeCP = false;
        for ( ; it != ( *pathIt )->end(); ++it )
        {
            if ( it == ( *pathIt )->begin() )
            {
                if ( ( *it )->properties() & KoPathPoint::StartSubpath )
                {
                    QPointF p = matrix.map( (*it)->point() );
                    d += QString( "M%1 %2" ).arg( p.x() ).arg( p.y() );
                }
            }
            else if ( activeCP || ( *it )->activeControlPoint1() )
            {
                QPointF cp1 = matrix.map( activeCP ? lastPoint->controlPoint2() : lastPoint->point() );
                QPointF cp2 = matrix.map( ( *it )->activeControlPoint1() ? ( *it )->controlPoint1() : ( *it )->point() );
                QPointF p = matrix.map( (*it)->point() );
                d += QString( "C%1 %2 %3 %4 %5 %6" )
                        .arg( cp1.x() ).arg( cp1.y() )
                        .arg( cp2.x() ).arg( cp2.y() )
                        .arg( p.x() ).arg( p.y() );
            }
            else
            {
                QPointF p = matrix.map( (*it)->point() );
                d += QString( "L%1 %2" ).arg( p.x() ).arg( p.y() );
            }
            if ( ( *it )->properties() & KoPathPoint::CloseSubpath )
            {
                // add curve when there is a curve on the way to the first point
                KoPathPoint * firstPoint = ( *pathIt )->first();
                if ( ( *it )->activeControlPoint2() || firstPoint->activeControlPoint1() )
                {
                    QPointF cp1 = matrix.map( ( *it )->activeControlPoint2() ? ( *it )->controlPoint2() : ( *it )->point() );
                    QPointF cp2 = matrix.map( firstPoint->activeControlPoint1() ? firstPoint->controlPoint1() : firstPoint->point() );
                    QPointF p = matrix.map( firstPoint->point() );

                    d += QString( "C%1 %2 %3 %4 %5 %6" )
                            .arg( cp1.x() ).arg( cp1.y() )
                            .arg( cp2.x() ).arg( cp2.y() )
                            .arg( p.x() ).arg( p.y() );
                }
                d += QString( "Z" );
            }

            if ( ( *it )->activeControlPoint2() )
            {
                activeCP = true;
            }
            else
            {
                activeCP = false;
            }
            lastPoint = *it;
        }
    }

    return d;
}

Qt::FillRule KoPathShape::fillRule() const
{
    return d->fillRule;
}

void KoPathShape::setFillRule( Qt::FillRule fillRule )
{
    d->fillRule = fillRule;
}
