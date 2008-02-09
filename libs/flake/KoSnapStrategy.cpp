/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoSnapStrategy.h"
#include <KoPathShape.h>
#include <KoPathPoint.h>
//#include <kdebug.h>
#include <math.h>


KoSnapStrategy::KoSnapStrategy( KoSnapStrategy::SnapType type )
    : m_snapType(type)
{
}

QPainterPath KoSnapStrategy::decoration() const
{
    return m_decoration;
}

void KoSnapStrategy::setDecoration( const QPainterPath &decoration )
{
    m_decoration = decoration;
}

QPointF KoSnapStrategy::snappedPosition() const
{
    return m_snappedPosition;
}

void KoSnapStrategy::setSnappedPosition( const QPointF &position )
{
    m_snappedPosition= position;
}

KoSnapStrategy::SnapType KoSnapStrategy::type() const
{
    return m_snapType;
}

double KoSnapStrategy::fastDistance( const QPointF &p1, const QPointF &p2 )
{
    double dx = p1.x()-p2.x();
    double dy = p1.y()-p2.y();
    return dx*dx + dy*dy;
}

OrthogonalSnapStrategy::OrthogonalSnapStrategy()
    : KoSnapStrategy( KoSnapStrategy::Orthogonal )
{
}

bool OrthogonalSnapStrategy::snapToPoints( const QPointF &mousePosition, KoSnapProxy * proxy, double maxSnapDistance )
{
    QPointF horzSnap, vertSnap;
    double minVertDist = HUGE_VAL;
    double minHorzDist = HUGE_VAL;

    QList<KoShape*> shapes = proxy->shapes();
    foreach( KoShape * shape, shapes )
    {
        QList<QPointF> points = proxy->pointsFromShape( shape );
        foreach( QPointF point, points )
        {
            double dx = fabs( point.x() - mousePosition.x() );
            if( dx < minHorzDist && dx < maxSnapDistance )
            {
                minHorzDist = dx;
                horzSnap = point;
            }
            double dy = fabs( point.y() - mousePosition.y() );
            if( dy < minVertDist && dy < maxSnapDistance )
            {
                minVertDist = dy;
                vertSnap = point;
            }
        }
    }

    QPointF snappedPoint = mousePosition;

    if( minHorzDist < HUGE_VAL )
        snappedPoint.setX( horzSnap.x() );
    if( minVertDist < HUGE_VAL )
        snappedPoint.setY( vertSnap.y() );

    QPainterPath decoration;

    if( minHorzDist < HUGE_VAL )
    {
        decoration.moveTo( horzSnap );
        decoration.lineTo( snappedPoint );
    }

    if( minVertDist < HUGE_VAL )
    {
        decoration.moveTo( vertSnap );
        decoration.lineTo( snappedPoint );
    }

    setDecoration( decoration );
    setSnappedPosition( snappedPoint );

    return (minHorzDist < HUGE_VAL || minVertDist < HUGE_VAL);
}

NodeSnapStrategy::NodeSnapStrategy()
    : KoSnapStrategy( KoSnapStrategy::Node )
{
}

bool NodeSnapStrategy::snapToPoints( const QPointF &mousePosition, KoSnapProxy * proxy, double maxSnapDistance )
{
    double maxDistance = maxSnapDistance*maxSnapDistance;
    double minDistance = HUGE_VAL;

    QRectF rect( -maxSnapDistance, -maxSnapDistance, maxSnapDistance, maxSnapDistance );
    rect.moveCenter( mousePosition );
    QList<QPointF> points = proxy->pointsInRect( rect );

    QPointF snappedPoint = mousePosition;

    foreach( QPointF point, points )
    {
        double distance = fastDistance( mousePosition, point );
        if( distance < maxDistance && distance < minDistance )
        {
            snappedPoint = point;
            minDistance = distance;
        }
    }

    QPainterPath decoration;

    if( minDistance < HUGE_VAL )
    {
        QRectF ellipse( -10, -10, 10, 10 );
        ellipse.moveCenter( snappedPoint );
        decoration.addEllipse( ellipse );
    }

    setDecoration( decoration );
    setSnappedPosition( snappedPoint );

    return (minDistance < HUGE_VAL);
}

ExtensionSnapStrategy::ExtensionSnapStrategy()
    : KoSnapStrategy( KoSnapStrategy::Extension )
{
}

bool ExtensionSnapStrategy::snapToPoints( const QPointF &mousePosition, KoSnapProxy * proxy, double maxSnapDistance )
{
    double maxDistance = maxSnapDistance*maxSnapDistance;
    double minDistance = HUGE_VAL;

    QPointF snappedPoint = mousePosition;
    QPointF startPoint;

    QList<KoShape*> shapes = proxy->shapes( true );
    foreach( KoShape * shape, shapes )
    {
        KoPathShape * path = dynamic_cast<KoPathShape*>( shape );
        if( ! path )
            continue;

        QMatrix matrix = path->absoluteTransformation(0);

        int subpathCount = path->subpathCount();
        for( int subpathIndex = 0; subpathIndex < subpathCount; ++subpathIndex )
        {
            if( path->isClosedSubpath( subpathIndex ) )
                continue;

            int pointCount = path->pointCountSubpath( subpathIndex );

            // check the extension from the start point
            KoPathPoint * first = path->pointByIndex( KoPathPointIndex( subpathIndex, 0 )  );
            QPointF firstSnapPosition = mousePosition;
            if( snapToExtension( firstSnapPosition, first, matrix ) )
            {
                double distance = fastDistance( firstSnapPosition, mousePosition );
                if( distance < maxDistance && distance < minDistance )
                {
                    minDistance = distance;
                    snappedPoint = firstSnapPosition;
                    startPoint = matrix.map( first->point() );
                }
            }

            // now check the extension from the last point
            KoPathPoint * last = path->pointByIndex( KoPathPointIndex( subpathIndex, pointCount-1 )  );
            QPointF lastSnapPosition = mousePosition;
            if( snapToExtension( lastSnapPosition, last, matrix ) )
            {
                double distance = fastDistance( lastSnapPosition, mousePosition );
                if( distance < maxDistance && distance < minDistance )
                {
                    minDistance = distance;
                    snappedPoint = lastSnapPosition;
                    startPoint = matrix.map( last->point() );
                }
            }
        }
    }

    QPainterPath decoration;

    if( minDistance < HUGE_VAL )
    {
        decoration.moveTo( startPoint );
        decoration.lineTo( snappedPoint );
    }

    setDecoration( decoration );
    setSnappedPosition( snappedPoint );

    return (minDistance < HUGE_VAL);
}

bool ExtensionSnapStrategy::snapToExtension( QPointF &position, KoPathPoint * point, const QMatrix &matrix )
{
    QPointF direction = extensionDirection( point, matrix );
    QPointF extensionStart = matrix.map( point->point() );
    QPointF extensionStop = matrix.map( point->point() ) + direction;
    float posOnExtension = project( extensionStart, extensionStop, position );
    if( posOnExtension < 0.0 )
        return false;

    position = extensionStart + posOnExtension * direction;
    return true;
}

double ExtensionSnapStrategy::project( const QPointF &lineStart, const QPointF &lineEnd, const QPointF &point )
{
    QPointF diff = lineEnd - lineStart;
    QPointF relPoint = point - lineStart;
    double diffLength = sqrt( diff.x()*diff.x() + diff.y()*diff.y() );
    diff /= diffLength;
    // project mouse position relative to stop position on extension line
    double scalar = relPoint.x()*diff.x() + relPoint.y()*diff.y();
    return scalar /= diffLength;
}

QPointF ExtensionSnapStrategy::extensionDirection( KoPathPoint * point, const QMatrix &matrix )
{
    KoPathShape * path = point->parent();
    KoPathPointIndex index = path->pathPointIndex( point );

    /// check if it is a start point
    if( point->properties() & KoPathPoint::StartSubpath )
    {
        if( point->properties() & KoPathPoint::HasControlPoint2 )
        {
            return matrix.map(point->point()) - matrix.map(point->controlPoint2());
        }
        else
        {
            KoPathPoint * next = path->pointByIndex( KoPathPointIndex( index.first, index.second+1 ) );
            if( next->properties() & KoPathPoint::HasControlPoint1 )
                return matrix.map(point->point()) - matrix.map(next->controlPoint1());
            else
                return matrix.map(point->point()) - matrix.map(next->point());
        }
    }
    else
    {
        if( point->properties() & KoPathPoint::HasControlPoint1 )
        {
            return matrix.map(point->point()) - matrix.map(point->controlPoint1());
        }
        else
        {
            KoPathPoint * prev = path->pointByIndex( KoPathPointIndex( index.first, index.second-1 ) );
            if( prev->properties() & KoPathPoint::HasControlPoint2 )
                return matrix.map(point->point()) - matrix.map(prev->controlPoint2());
            else
                return matrix.map(point->point()) - matrix.map(prev->point());
        }
    }
}

IntersectionSnapStrategy::IntersectionSnapStrategy()
    : KoSnapStrategy( KoSnapStrategy::Intersection )
{
}

bool IntersectionSnapStrategy::snapToPoints( const QPointF &mousePosition, KoSnapProxy * proxy, double maxSnapDistance )
{
    double maxDistance = maxSnapDistance*maxSnapDistance;
    double minDistance = HUGE_VAL;

    QRectF rect( -maxSnapDistance, -maxSnapDistance, maxSnapDistance, maxSnapDistance );
    rect.moveCenter( mousePosition );
    QList<KoShape*> shapes = proxy->shapesInRect( rect, true );

    QList<KoPathSegment> segments;
    foreach( KoShape * shape, shapes )
    {
        KoPathShape * path = dynamic_cast<KoPathShape*>( shape );
        if( ! path )
            continue;

        QMatrix m = shape->absoluteTransformation(0);
        // transform segments to document coordinates
        foreach( KoPathSegment s, path->segmentsAt( path->documentToShape( rect ) ) )
        {
            segments.append( s.mapped( m ) );
        }
    }
    QPointF snappedPoint = mousePosition;

    //kDebug() << "found" << segments.count() << "segments in roi";

    int segmentCount = segments.count();
    for( int i = 0; i < segmentCount; ++i )
    {
        KoPathSegment s1 = segments[i];
        for( int j = i+1; j < segmentCount; ++j )
        {
            QList<QPointF> isects = s1.intersections( segments[j] );
            //kDebug() << isects.count() << "intersections found";
            foreach( QPointF point, isects )
            {
                if( ! rect.contains( point ) )
                    continue;
                double distance = fastDistance( mousePosition, point );
                if( distance < maxDistance && distance < minDistance )
                {
                    snappedPoint = point;
                    minDistance = distance;
                }
            }
        }
    }

    QPainterPath decoration;

    //kDebug() << "min distance =" << minDistance;

    if( minDistance < HUGE_VAL )
    {
        QRectF rectangle( -10, -10, 10, 10 );
        rectangle.moveCenter( snappedPoint );
        decoration.addRect( rectangle );
    }

    setDecoration( decoration );
    setSnappedPosition( snappedPoint );

    return (minDistance < HUGE_VAL);
}
