/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006-2008 Jan Hambrecht <jaham@gmx.net>
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
#include "KoShapeShadow.h"

#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoUnit.h>
#include <KoGenStyle.h>
#include <KoStyleStack.h>
#include <KoOdfLoadingContext.h>

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

KoShape * KoPathShape::cloneShape() const
{
    // TODO implement cloning
    return 0;
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
    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
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
            else if ( point->activeControlPoint1() && point->activeControlPoint2() )
            {
                QBrush b( Qt::red );
                painter.setBrush( b );
            }
            else if ( point->activeControlPoint1() )
            {
                QBrush b( Qt::yellow );
                painter.setBrush( b );
            }
            else if ( point->activeControlPoint2() )
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
    QPainterPath path;
    foreach( KoSubpath * subpath, m_subpaths )
    {
        KoPathPoint * lastPoint = subpath->first();
        bool activeCP = false;
        foreach( KoPathPoint * currPoint, *subpath )
        {
            KoPathPoint::KoPointProperties currProperties = currPoint->properties();
            if ( currPoint == subpath->first() )
            {
                if ( currProperties & KoPathPoint::StartSubpath )
                {
                    path.moveTo( currPoint->point() );
                }
            }
            else if ( activeCP || currPoint->activeControlPoint1() )
            {
                path.cubicTo( 
                    activeCP ? lastPoint->controlPoint2() : lastPoint->point(),
                    currPoint->activeControlPoint1() ? currPoint->controlPoint1() : currPoint->point(),
                    currPoint->point() );
            }
            else
            {
                path.lineTo( currPoint->point() );
            }
            if ( currProperties & KoPathPoint::CloseSubpath && currProperties & KoPathPoint::StopSubpath )
            {
                // add curve when there is a curve on the way to the first point
                KoPathPoint * firstPoint = subpath->first();
                if ( currPoint->activeControlPoint2() || firstPoint->activeControlPoint1() )
                {
                    path.cubicTo( 
                        currPoint->activeControlPoint2() ? currPoint->controlPoint2() : currPoint->point(),
                        firstPoint->activeControlPoint1() ? firstPoint->controlPoint1() : firstPoint->point(),
                        firstPoint->point() );
                }
                path.closeSubpath();
            }

            if ( currPoint->activeControlPoint2() )
            {
                activeCP = true;
            }
            else
            {
                activeCP = false;
            }
            lastPoint = currPoint;
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
    bb = absoluteTransformation( 0 ).mapRect( bb );
    if( shadow() )
    {
        KoInsets insets;
        shadow()->insets( this, insets );
        bb.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
    }
    return bb;
}


QSizeF KoPathShape::size() const
{
    // don't call boundingRect here as it uses absoluteTransformation 
    // which itself uses size() -> leads to infinite reccursion
    return outline().boundingRect().size();
}

void KoPathShape::setSize( const QSizeF &newSize )
{
    QSizeF oldSize = size();
    double zoomX = oldSize.width() == 0.0 ? 1.0 : newSize.width() / oldSize.width();
    double zoomY = oldSize.height() == 0.0 ? 1.0 : newSize.height() / oldSize.height();
    QMatrix matrix( zoomX, 0, 0, zoomY, 0, 0 );

    //qDebug() << "setSize" << zoomX << "," << zoomY << "," << newSize;
    KoShape::setSize( newSize );
    map( matrix );
}

KoPathPoint * KoPathShape::moveTo( const QPointF &p )
{
    KoPathPoint * point = new KoPathPoint( this, p, KoPathPoint::StartSubpath|KoPathPoint::StopSubpath );
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
    KoPathPoint * point = new KoPathPoint( this, p, KoPathPoint::StopSubpath );
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
    KoPathPoint * point = new KoPathPoint( this, p, KoPathPoint::StopSubpath );
    point->setControlPoint1( c2 );
    m_subpaths.last()->push_back( point );
    return point;
}

KoPathPoint * KoPathShape::curveTo( const QPointF &c, const QPointF &p )
{
    if( m_subpaths.empty() )
        moveTo( QPointF( 0, 0 ) );

    KoPathPoint * lastPoint = m_subpaths.last()->last();
    updateLast( &lastPoint );
    lastPoint->setControlPoint2( c );
    KoPathPoint * point = new KoPathPoint( this, p, KoPathPoint::StopSubpath );
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
    QPointF tl( outline().boundingRect().topLeft() );
    QMatrix matrix;
    matrix.translate( -tl.x(), -tl.y() );
    map( matrix );

    // keep the top left point of the object
    applyTransformation( matrix.inverted() );

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
    // check if we are about to add a new point to a closed subpath
    if ( ( *lastPoint )->properties() & KoPathPoint::StopSubpath
      && ( *lastPoint )->properties() & KoPathPoint::CloseSubpath )
    {
        // get the first point of the subpath
        KoPathPoint * subpathStart = m_subpaths.last()->first();
        // clone the first point of the subpath...
        KoPathPoint * newLastPoint = new KoPathPoint( *subpathStart );
        // ... and make it a normal point
        newLastPoint->setProperties( KoPathPoint::Normal );
        // make a point group of the first point and its clone
        KoPointGroup * group = subpathStart->group();
        if ( group == 0 )
        {
            group = new KoPointGroup();
            group->add( subpathStart );
        }
        group->add( newLastPoint );

        // now start a new subpath with the cloned start point
        KoSubpath *path = new KoSubpath;
        path->push_back( newLastPoint );
        m_subpaths.push_back( path );
        *lastPoint = newLastPoint;
    }
    else
    {
        // the subpath was not closed so the formerly last point
        // of the subpath is no end point anymore
        ( *lastPoint )->unsetProperty( KoPathPoint::StopSubpath );
    }
    ( *lastPoint )->unsetProperty( KoPathPoint::CloseSubpath );
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

QList<KoPathSegment> KoPathShape::segmentsAt( const QRectF &r )
{
    QList<KoPathSegment> segments;
    int subpathCount = m_subpaths.count();
    for( int subpathIndex = 0; subpathIndex < subpathCount; ++subpathIndex )
    {
        KoSubpath * subpath = m_subpaths[subpathIndex];
        int pointCount = subpath[subpathIndex].count();
        bool subpathClosed = isClosedSubpath( subpathIndex );
        for( int pointIndex = 0; pointIndex < pointCount; ++pointIndex )
        {
            if( pointIndex == (pointCount-1) && ! subpathClosed )
                break;
            KoPathSegment s( subpath->at( pointIndex ), subpath->at( (pointIndex+1)%pointCount ) );
            QRectF controlRect = s.controlPointRect();
            if( ! r.intersects( controlRect ) && ! controlRect.contains( r ) )
                continue;
            QRectF bound = s.boundingRect();
            if( ! r.intersects( bound ) && ! bound.contains( r ) )
                continue;

            segments.append( s );
        }
    }
    return segments;
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
        // check if we have a (closing) segment starting from the last point
        if ( (index == subpath->size()-1) && point->properties() & KoPathPoint::CloseSubpath )
            index = 0;
        else
            ++index;

        if ( index < subpath->size() )
        {
            segment = KoPathSegment( point, subpath->at( index ) );
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

    KoPathPoint::KoPointProperties properties = point->properties();
    properties &= ~KoPathPoint::StartSubpath;
    properties &= ~KoPathPoint::StopSubpath;
    properties &= ~KoPathPoint::CloseSubpath;
    // check if new point starts subpath
    if ( pointIndex.second == 0 )
    {
        properties |= KoPathPoint::StartSubpath;
        // subpath was closed
        if (  subpath->last()->properties() & KoPathPoint::CloseSubpath )
        {
            // keep the path closed
            properties |= KoPathPoint::CloseSubpath;
        }
        // old first point does not start the subpath anymore
        subpath->first()->unsetProperty( KoPathPoint::StartSubpath );
    }
    // check if new point stops subpath
    else if ( pointIndex.second == subpath->size() )
    {
        properties |= KoPathPoint::StopSubpath;
        // subpath was closed
        if ( subpath->last()->properties() & KoPathPoint::CloseSubpath )
        {
            // keep the path closed
            properties = properties | KoPathPoint::CloseSubpath;
        }
        // old last point does not end subpath anymore
        subpath->last()->unsetProperty( KoPathPoint::StopSubpath );
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

    // check if we removed the first point
    if ( pointIndex.second == 0 )
    {
        // first point removed, set new StartSubpath
        subpath->first()->setProperty( KoPathPoint::StartSubpath );
        // check if path was closed
        if ( subpath->last()->properties() & KoPathPoint::CloseSubpath )
        {
            // keep path closed
            subpath->first()->setProperty( KoPathPoint::CloseSubpath );
        }
    }
    // check if we removed the last point
    else if ( pointIndex.second == subpath->size() ) // use size as point is already removed
    {
        // last point removed, set new StopSubpath
        subpath->last()->setProperty( KoPathPoint::StopSubpath );
        // check if path was closed
        if ( point->properties() & KoPathPoint::CloseSubpath )
        {
            // keep path closed
            subpath->last()->setProperty( KoPathPoint::CloseSubpath );
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
    newSubpath->first()->setProperty( KoPathPoint::StartSubpath );
    // the last point of the old subpath is now an ending node
    subpath->last()->setProperty( KoPathPoint::StopSubpath );

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

    // the last point of the subpath does not end the subpath anymore
    subpath->last()->unsetProperty( KoPathPoint::StopSubpath );
    // the first point of the next subpath does not start a subpath anymore
    nextSubpath->first()->unsetProperty( KoPathPoint::StartSubpath );

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
    oldStartPoint->unsetProperty( KoPathPoint::StartSubpath );
    // the old end node no longer closes the subpath
    subpath->last()->unsetProperty( KoPathPoint::StopSubpath );

    // reorder the subpath
    for ( int i = 0; i < pointIndex.second; ++i )
    {
        subpath->append( subpath->takeFirst() );
    }
    // make the first point a start node
    subpath->first()->setProperty( KoPathPoint::StartSubpath );
    // make the last point an end node
    subpath->last()->unsetProperty( KoPathPoint::StopSubpath );

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
    oldStartPoint->unsetProperty( KoPathPoint::StartSubpath );
    // the old end node no longer ends the subpath
    subpath->last()->unsetProperty( KoPathPoint::StopSubpath );

    // reorder the subpath
    for ( int i = 0; i < pointIndex.second; ++i )
    {
        subpath->append( subpath->takeFirst() );
    }
    subpath->first()->setProperty( KoPathPoint::StartSubpath );
    subpath->last()->setProperty( KoPathPoint::StopSubpath );

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
    firstProps &= ~KoPathPoint::StopSubpath;
    lastProps |= KoPathPoint::StopSubpath;
    lastProps &= ~KoPathPoint::StartSubpath;
    if( firstProps & KoPathPoint::CloseSubpath )
    {
        firstProps |= KoPathPoint::CloseSubpath;
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

    subpath->last()->setProperty( KoPathPoint::CloseSubpath );
    subpath->first()->setProperty( KoPathPoint::CloseSubpath );
}

void KoPathShape::closeMergeSubpath( KoSubpath *subpath )
{
    if ( ! subpath || subpath->size() < 2 )
        return;

    KoPathPoint * lastPoint = subpath->last();
    KoPathPoint * firstPoint = subpath->first();

    // check if first and last points are coincident
    if ( lastPoint->point() == firstPoint->point() )
    {
        // we are removing the current last point and
        // reuse its first control point if active
        firstPoint->setProperty( KoPathPoint::StartSubpath );
        firstPoint->setProperty( KoPathPoint::CloseSubpath );
        if ( lastPoint->activeControlPoint1() )
            firstPoint->setControlPoint1( lastPoint->controlPoint1() );
        // remove last point
        delete subpath->takeLast();
        // the new last point closes the subpath now
        lastPoint = subpath->last();
        lastPoint->setProperty( KoPathPoint::StopSubpath );
        lastPoint->setProperty( KoPathPoint::CloseSubpath );
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
            // first point of subpath ?
            if ( it == ( *pathIt )->begin() )
            {
                if ( ( *it )->properties() & KoPathPoint::StartSubpath )
                {
                    QPointF p = matrix.map( (*it)->point() );
                    d += QString( "M%1 %2" ).arg( p.x() ).arg( p.y() );
                }
            }
            // end point of curve ?
            else if ( activeCP || ( *it )->activeControlPoint1() )
            {
                QPointF cp1 = matrix.map( activeCP ? lastPoint->controlPoint2() : ( *it )->controlPoint1() );
                QPointF cp2 = matrix.map( ( *it )->activeControlPoint1() ? ( *it )->controlPoint1() : lastPoint->controlPoint2() );
                QPointF p = matrix.map( (*it)->point() );
                d += QString( "C%1 %2 %3 %4 %5 %6" )
                        .arg( cp1.x() ).arg( cp1.y() )
                        .arg( cp2.x() ).arg( cp2.y() )
                        .arg( p.x() ).arg( p.y() );
            }
            // end point of line
            else
            {
                QPointF p = matrix.map( (*it)->point() );
                d += QString( "L%1 %2" ).arg( p.x() ).arg( p.y() );
            }
            // last point closes subpath ?
            if ( ( *it )->properties() & KoPathPoint::StopSubpath
              && ( *it )->properties() & KoPathPoint::CloseSubpath )
            {
                // add curve when there is a curve on the way to the first point
                KoPathPoint * firstPoint = ( *pathIt )->first();
                if ( ( *it )->activeControlPoint2() || firstPoint->activeControlPoint1() )
                {
                    QPointF cp1 = matrix.map( ( *it )->activeControlPoint2() ? ( *it )->controlPoint2() : firstPoint->controlPoint1() );
                    QPointF cp2 = matrix.map( firstPoint->activeControlPoint1() ? firstPoint->controlPoint1() : ( *it )->controlPoint2() );
                    QPointF p = matrix.map( firstPoint->point() );

                    d += QString( "C%1 %2 %3 %4 %5 %6" )
                            .arg( cp1.x() ).arg( cp1.y() )
                            .arg( cp2.x() ).arg( cp2.y() )
                            .arg( p.x() ).arg( p.y() );
                }
                d += QString( "Z" );
            }

            activeCP = ( *it )->activeControlPoint2();
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

KoPathShape * KoPathShape::fromQPainterPath( const QPainterPath &path )
{
    KoPathShape * shape = new KoPathShape();

    int elementCount = path.elementCount();
    for( int i = 0; i < elementCount; i++ )
    {
        QPainterPath::Element element = path.elementAt( i );
        switch( element.type )
        {
            case QPainterPath::MoveToElement:
                shape->moveTo( QPointF( element.x, element.y ) );
                break;
            case QPainterPath::LineToElement:
                shape->lineTo( QPointF( element.x, element.y ) );
                break;
            case QPainterPath::CurveToElement:
                shape->curveTo( QPointF( element.x, element.y ),
                                QPointF( path.elementAt(i+1).x, path.elementAt(i+1).y),
                                         QPointF( path.elementAt(i+2).x, path.elementAt(i+2).y) );
                break;
            default:
                continue;
        }
    }

    shape->normalize();
    return shape;
}
