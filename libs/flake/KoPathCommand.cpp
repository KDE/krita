/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
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

#include "KoPathCommand.h"
#include <klocale.h>
#include <math.h>

KoPointMoveCommand::KoPointMoveCommand( KoPathShape *shape, KoPathPoint *point, const QPointF &offset, KoPathPoint::KoPointType pointType )
: m_shape( shape )
, m_offset( offset )
, m_pointType( pointType )
{
    m_points << point;
}

KoPointMoveCommand::KoPointMoveCommand( KoPathShape *shape, const QList<KoPathPoint*> &points, const QPointF &offset )
: m_shape( shape )
, m_points( points )
, m_offset( offset )
, m_pointType( KoPathPoint::Node )
{
}

void KoPointMoveCommand::execute()
{
    QRectF oldControlRect = m_shape->outline().controlPointRect();

    if( m_pointType == KoPathPoint::Node )
    {
        foreach( KoPathPoint *p, m_points )
        {
            p->setPoint( p->point() + m_offset );
            if( p->properties() & KoPathPoint::HasControlPoint1 )
                p->setControlPoint1( p->controlPoint1() + m_offset );
            if( p->properties() & KoPathPoint::HasControlPoint2 )
                p->setControlPoint2( p->controlPoint2() + m_offset );
        }
    }
    else if( m_pointType == KoPathPoint::ControlPoint1 )
    {
        KoPathPoint *p = m_points.first();
        p->setControlPoint1( p->controlPoint1() + m_offset );
        if( p->properties() & KoPathPoint::IsSymmetric )
        {
            // set the other control point so that it lies on the line between the moved
            // control point and the point, with the same distance to the point as the moved point
            p->setControlPoint2( 2.0 * p->point() - p->controlPoint1() );
        }
        else if( p->properties() & KoPathPoint::IsSmooth )
        {
            // move the other control point so that it lies on the line through point and control point
            // keeping its distance to the point
            QPointF direction = p->point() - p->controlPoint1();
            direction /= sqrt( direction.x()*direction.x() + direction.y()*direction.y() );
            QPointF distance = p->point() - p->controlPoint2();
            qreal length = sqrt( distance.x()*distance.x() + distance.y()*distance.y() );
            p->setControlPoint2( p->point() + length * direction );
        }
    }
    else if( m_pointType == KoPathPoint::ControlPoint2 )
    {
        KoPathPoint *p = m_points.first();
        p->setControlPoint2( p->controlPoint2() + m_offset );
        if( p->properties() & KoPathPoint::IsSymmetric )
        {
            // set the other control point so that it lies on the line between the moved
            // control point and the point, with the same distance to the point as the moved point
            p->setControlPoint1( 2.0 * p->point() - p->controlPoint2() );
        }
        else if( p->properties() & KoPathPoint::IsSmooth )
        {
            // move the other control point so that it lies on the line through point and control point
            // keeping its distance to the point
            QPointF direction = p->point() - p->controlPoint2();
            direction /= sqrt( direction.x()*direction.x() + direction.y()*direction.y() );
            QPointF distance = p->point() - p->controlPoint1();
            qreal length = sqrt( distance.x()*distance.x() + distance.y()*distance.y() );
            p->setControlPoint1( p->point() + length * direction );
        }
    }

    // the bounding rect has changed -> normalize and adjust position of shape
    QPointF offset = m_shape->normalize();
    m_shape->moveBy( -offset.x(), -offset.y() );

    // adjust the old control rect as the repainting is relative to the new shape position
    oldControlRect.translate( offset );
    QRectF repaintRect = oldControlRect.unite( m_shape->outline().controlPointRect() );
    // TODO use the proper adjustment if the actual point size could be retrieved
    repaintRect.adjust( -5.0, -5.0, 5.0, 5.0 );
    m_shape->repaint( repaintRect );
}

void KoPointMoveCommand::unexecute()
{
    m_offset *= -1.0;
    execute();
    m_offset *= -1.0;
}

QString KoPointMoveCommand::name() const
{
    return i18n( "Move points" );
}
