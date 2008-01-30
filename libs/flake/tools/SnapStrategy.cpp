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

#include "SnapStrategy.h"

#include <math.h>


SnapStrategy::SnapStrategy( SnapGuide::SnapType type )
    : m_snapType(type)
{
}

QPainterPath SnapStrategy::decoration() const
{
    return m_decoration;
}

void SnapStrategy::setDecoration( const QPainterPath &decoration )
{
    m_decoration = decoration;
}

QPointF SnapStrategy::snappedPosition() const
{
    return m_snappedPosition;
}

void SnapStrategy::setSnappedPosition( const QPointF &position )
{
    m_snappedPosition= position;
}

SnapGuide::SnapType SnapStrategy::type() const
{
    return m_snapType;
}

double SnapStrategy::fastDistance( const QPointF &p1, const QPointF &p2 )
{
    double dx = p1.x()-p2.x();
    double dy = p1.y()-p2.y();
    return dx*dx + dy*dy;
}

OrthogonalSnapStrategy::OrthogonalSnapStrategy()
    : SnapStrategy( SnapGuide::Orthogonal )
{
}

bool OrthogonalSnapStrategy::snapToPoints( const QPointF &mousePosition, SnapProxy * proxy, double maxSnapDistance )
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
    : SnapStrategy( SnapGuide::Node )
{
}

bool NodeSnapStrategy::snapToPoints( const QPointF &mousePosition, SnapProxy * proxy, double maxSnapDistance )
{
    double maxDistance = maxSnapDistance*maxSnapDistance;
    double minDistance = HUGE_VAL;

    QRectF rect( -maxSnapDistance, -maxSnapDistance, maxSnapDistance, maxSnapDistance );
    rect.moveCenter( mousePosition );
    QList<QPointF> points = proxy->pointsInRect( rect );

    QPointF snappedPoint = mousePosition;

    foreach( QPointF point, points )
    {
        QPointF diffVec = mousePosition-point;
        double distance = diffVec.x()*diffVec.x() + diffVec.y()*diffVec.y();
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
