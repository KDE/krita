/* This file is part of the KDE project
 * Copyright (C) 2007 Boudewijn Rempt <boud@kde.org>
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#include "KoConnectionShape.h"

#include <KoViewConverter.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoXmlWriter.h>
#include <KoStoreDevice.h>
#include <KoUnit.h>

#include <kdebug.h>

#include <QPainter>

// XXX: Add editable text in path shapes so we can get a label here

struct KoConnectionShape::Private
{
    Private()
    : shape1(0), shape2(0), connectionPointIndex1(-1), connectionPointIndex2(-1)
    , connectionType(Standard)
    {}
    KoSubpath points;
    KoShape * shape1;
    KoShape * shape2;
    int connectionPointIndex1;
    int connectionPointIndex2;
    Type connectionType;
};


KoConnectionShape::KoConnectionShape()
    : d ( new Private )
{
    m_handles.push_back( QPointF( 0, 0 ) );
    m_handles.push_back( QPointF( 140, 140 ) );

    moveTo( m_handles[0] );
    lineTo( m_handles[1] );

    d->points = *m_subpaths[0];
    updatePath( QSizeF( 140, 140 ) );
}

KoConnectionShape::~KoConnectionShape()
{
    delete d;
}

void KoConnectionShape::paint( QPainter& painter, const KoViewConverter& converter ) {
    // TODO find a better place to update the connections (maybe inside the shape manager?)
    if( isParametricShape() )
        updateConnections();
}

void KoConnectionShape::saveOdf( KoShapeSavingContext & context ) const
{
}

bool KoConnectionShape::loadOdf( const KoXmlElement & element, KoShapeLoadingContext &context )
{
    return false;
}

void KoConnectionShape::moveHandleAction( int handleId, const QPointF & point, Qt::KeyboardModifiers modifiers )
{
    if ( handleId > m_handles.size() ) 
        return;

    m_handles[handleId] = point;
}

void KoConnectionShape::updatePath( const QSizeF &size )
{
    const double MinimumEscapeLength = 20.0;

    clear();
    switch( d->connectionType )
    {
        case Standard:
        {
            QList<QPointF> edges1;
            QList<QPointF> edges2;

            QPointF direction1 = escapeDirection( 0 );
            QPointF direction2 = escapeDirection( 1 );

            QPointF edgePoint1 = m_handles[0] + MinimumEscapeLength * direction1;
            QPointF edgePoint2 = m_handles[1] + MinimumEscapeLength * direction2;

            edges1.append( edgePoint1 );
            edges2.prepend( edgePoint2 );

            if( handleConnected( 0 ) && handleConnected( 1 ) )
            {
                QPointF intersection;
                bool connected = false;
                do
                {
                    // first check if directions from current edge points intersect
                    if( intersects( edgePoint1, direction1, edgePoint2, direction2, intersection ) )
                    {
                        // directions intersect, we have another edge point and be done
                        edges1.append( intersection );
                        break;
                    }

                    // check if we are going toward the other handle
                    double sp = scalarProd( direction1, edgePoint2-edgePoint1 );
                    if( sp >= 0.0 )
                    {
                        // if we are having the same direction, go all the way toward
                        // the other handle, else only go half the way
                        if( direction1 == direction2 )
                            edgePoint1 += sp * direction1;
                        else
                            edgePoint1 += 0.5 * sp * direction1;
                        edges1.append( edgePoint1 );
                        // switch direction
                        direction1 = perpendicularDirection( edgePoint1, direction1, edgePoint2 );
                    }
                    else
                    {
                        // we are not going into the same direction, so switch direction
                        direction1 = perpendicularDirection( edgePoint1, direction1, edgePoint2 );
                    }
                }
                while( ! connected );
            }
            moveTo( m_handles[0] );
            uint edgeCount1 = edges1.count();
            for( uint i = 0; i < edgeCount1; ++i )
                lineTo( edges1[i] );
            uint edgeCount2 = edges2.count();
            for( uint i = 0; i < edgeCount2; ++i )
                lineTo( edges2[i] );
            lineTo( m_handles[1] );

            break;
        }
        case Lines:
        {
            QPointF direction1 = escapeDirection( 0 );
            QPointF direction2 = escapeDirection( 1 );
            moveTo( m_handles[0] );
            if( ! direction1.isNull() )
                lineTo( m_handles[0] + MinimumEscapeLength * direction1 );
            if( ! direction2.isNull() )
                lineTo( m_handles[1] + MinimumEscapeLength * direction2 );
            lineTo( m_handles[1] );
            break;
        }
        case Straight:
            moveTo( m_handles[0] );
            lineTo( m_handles[1] );
            break;
        case Curve:
            // TODO
            QPointF direction1 = escapeDirection( 0 );
            QPointF direction2 = escapeDirection( 1 );
            moveTo( m_handles[0] );
            if( ! direction1.isNull() && ! direction2.isNull() )
            {
                QPointF curvePoint1 = m_handles[0] + 5.0 * MinimumEscapeLength * direction1;
                QPointF curvePoint2 = m_handles[1] + 5.0 * MinimumEscapeLength * direction2;
                curveTo( curvePoint1, curvePoint2, m_handles[1] );
            }
            else
            {
                lineTo( m_handles[1] );
            }
            break;
    }
    normalize();
}

void KoConnectionShape::setConnection1( KoShape * shape1, int connectionPointIndex1 )
{
    d->shape1 = shape1;
    d->connectionPointIndex1 = connectionPointIndex1;
}

void KoConnectionShape::setConnection2( KoShape * shape2, int connectionPointIndex2 )
{
    d->shape2 = shape2;
    d->connectionPointIndex2 = connectionPointIndex2;
}

KoConnection KoConnectionShape::connection1() const
{
    return KoConnection( d->shape1, d->connectionPointIndex1 );
}

KoConnection KoConnectionShape::connection2() const
{
    return KoConnection( d->shape2, d->connectionPointIndex2 );
}

void KoConnectionShape::updateConnections()
{
    bool updateHandles = false;
    if( handleConnected( 0 ) )
    {
        QList<QPointF> connectionPoints = d->shape1->connectionPoints();
        if( d->connectionPointIndex1 < connectionPoints.count() )
        {
            // map connection point into our shape coordinates
            QPointF p = documentToShape( d->shape1->absoluteTransformation(0).map( connectionPoints[d->connectionPointIndex1] ) );
            if( m_handles[0] != p )
            {
                m_handles[0] = p;
                updateHandles = true;
            }
        }
    }
    if( handleConnected( 1 ) )
    {
        QList<QPointF> connectionPoints = d->shape2->connectionPoints();
        if( d->connectionPointIndex2 < connectionPoints.count() )
        {
            // map connection point into our shape coordinates
            QPointF p = documentToShape( d->shape2->absoluteTransformation(0).map( connectionPoints[d->connectionPointIndex2] ) );
            if( m_handles[1] != p )
            {
                m_handles[1] = p;
                updateHandles = true;
            }
        }
    }
    if( updateHandles )
    {
        update(); // ugly, for repainting the connection we just changed
        updatePath( QSizeF() );
        update(); // ugly, for repainting the connection we just changed
    }
}

KoConnectionShape::Type KoConnectionShape::connectionType() const
{
    return d->connectionType;
}

void KoConnectionShape::setConnectionType( Type connectionType )
{
    d->connectionType = connectionType;
    updatePath( size() );
}

bool KoConnectionShape::handleConnected( int handleId ) const
{
    if( handleId == 0 && d->shape1 && d->connectionPointIndex1 >= 0 )
        return true;
    if( handleId == 1 && d->shape2 && d->connectionPointIndex2 >= 0 )
        return true;

    return false;
}

QPointF KoConnectionShape::escapeDirection( int handleId ) const
{
    QPointF direction;
    if( handleConnected( handleId ) )
    {
        QPointF handlePoint = absoluteTransformation(0).map( m_handles[handleId] );
        QPointF centerPoint;
        if( handleId == 0 )
            centerPoint = d->shape1->absolutePosition( KoFlake::CenteredPositon );
        else
            centerPoint = d->shape2->absolutePosition( KoFlake::CenteredPositon );

        double angle = atan2( handlePoint.y()-centerPoint.y(), handlePoint.x()-centerPoint.x() );
        if( angle < 0.0 )
            angle += 2.0*M_PI;
        angle *= 180.0 / M_PI;
        if( angle >= 45.0 && angle < 135.0 )
            direction = QPointF( 0.0, 1.0 );
        else if( angle >= 135.0 && angle < 225.0 )
            direction = QPointF( -1.0, 0.0 );
        else if( angle >= 225.0 && angle < 315.0 )
            direction = QPointF( 0.0, -1.0 );
        else
            direction = QPointF( 1.0, 0.0 );
    }
    return direction;
}

bool KoConnectionShape::intersects( const QPointF &p1, const QPointF &d1, const QPointF &p2, const QPointF &d2, QPointF &isect )
{
    double sp1 = scalarProd( d1, p2-p1 );
    if( sp1 < 0.0 )
        return false;

    double sp2 = scalarProd( d2, p1-p2 );
    if( sp2 < 0.0 )
        return false;

    // use cross product to check if rays intersects at all
    double cp = crossProd( d1, d2 );
    if( cp == 0.0 )
    {
        // rays are parallel or coincidient
        if( p1.x() == p2.x() && d1.x() == 0.0 && d1.y() != d2.y() )
        {
            // vertical, coincident
            isect = 0.5 * (p1+p2);
        }
        else if( p1.y() == p2.y() && d1.y() == 0.0 && d1.x() != d2.x() )
        {
            // horizontal, coincident
            isect = 0.5 * (p1+p2);
        }
        else
            return false;
    }
    else
    {
        // they are intersecting normally
        isect = p1 + sp1 * d1;
    }

    return true;
}

double KoConnectionShape::scalarProd( const QPointF &v1, const QPointF &v2 )
{
    return v1.x()*v2.x() + v1.y()*v2.y();
}

double KoConnectionShape::crossProd( const QPointF &v1, const QPointF &v2 )
{
    return (v1.x()*v2.y() - v1.y()*v2.x() );
}

QPointF KoConnectionShape::perpendicularDirection( const QPointF &p1, const QPointF &d1, const QPointF &p2 )
{
    QPointF perpendicular( d1.y(), -d1.x() );
    double sp = scalarProd( perpendicular, p2-p1 );
    if( sp < 0.0 )
        perpendicular *= -1.0;

    return perpendicular;
}