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

#include "SnapGuide.h"
#include <KoPathShape.h>
#include <KoPathPoint.h>
#include <KoViewConverter.h>

#include <QtGui/QPainter>

#include <math.h>

SnapGuide::SnapGuide( KoPathShape * path )
    : m_path( path )
{
}

SnapGuide::~SnapGuide()
{
}

QPointF SnapGuide::snap( const QPointF &mousePosition, double maxSnapDistance )
{
    QMatrix m = m_path->absoluteTransformation(0);

    QList<QPointF> pathPoints;

    int subpathCount = m_path->subpathCount();
    for( int subpathIndex = 0; subpathIndex < subpathCount; ++subpathIndex )
    {
        int pointCount = m_path->pointCountSubpath( subpathIndex );
        for( int pointIndex = 0; pointIndex < pointCount; ++pointIndex )
        {
            KoPathPoint * p = m_path->pointByIndex( KoPathPointIndex( subpathIndex, pointIndex )  );
            if( ! p )
                continue;

            pathPoints.append( m.map( p->point() ) );
        }
    }

    return snapToPoints( mousePosition, pathPoints, maxSnapDistance );
}

QRectF SnapGuide::boundingRect()
{
    return m_lineGuide.boundingRect().adjusted( -2, -2, 2, 2 );
}

void SnapGuide::paint( QPainter &painter, const KoViewConverter &converter )
{
    if( m_lineGuide.isEmpty() )
        return;

    QPen pen( Qt::red );
    pen.setStyle( Qt::DotLine );
    painter.setPen( pen );
    painter.setBrush( Qt::NoBrush );
    painter.drawPath( m_lineGuide );
}

double SnapGuide::fastDistance( const QPointF &p1, const QPointF &p2 )
{
    double dx = p1.x()-p2.x();
    double dy = p1.y()-p2.y();
    return dx*dx + dy*dy;
}

OrthogonalSnapGuide::OrthogonalSnapGuide( KoPathShape * path )
    : SnapGuide( path )
{
}

QPointF OrthogonalSnapGuide::snapToPoints( const QPointF &mousePosition, QList<QPointF> &pathPoints, double maxSnapDistance )
{
    m_lineGuide = QPainterPath();

    QPointF horzSnap, vertSnap;
    double minVertDist = HUGE_VAL;
    double minHorzDist = HUGE_VAL;

    foreach( QPointF point, pathPoints )
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
    QPointF snappedPoint = mousePosition;
    if( minHorzDist < HUGE_VAL )
        snappedPoint.setX( horzSnap.x() );
    if( minVertDist < HUGE_VAL )
        snappedPoint.setY( vertSnap.y() );

    if( minHorzDist < HUGE_VAL )
    {
        m_lineGuide.moveTo( horzSnap );
        m_lineGuide.lineTo( snappedPoint );
    }

    if( minVertDist < HUGE_VAL )
    {
        m_lineGuide.moveTo( vertSnap );
        m_lineGuide.lineTo( snappedPoint );
    }

    return snappedPoint;
}
