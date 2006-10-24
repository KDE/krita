/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>

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

#include "KoRectangleShape.h"

#include <QDebug>
#include <QPainter>

KoRectangleShape::KoRectangleShape()
: m_cornerRadiusX( 0 )
, m_cornerRadiusY( 0 )
{
    m_handles.push_back( QPointF( 100, 0 ) );
    m_handles.push_back( QPointF( 100, 0 ) );
    QSizeF size( 100, 100 );
    createPath( size );
    m_points = *m_subpaths[0];
    updatePath( size );
}

KoRectangleShape::~KoRectangleShape()
{
}

void KoRectangleShape::moveHandleAction( int handleId, const QPointF & point, Qt::KeyboardModifiers modifiers )
{
    Q_UNUSED( modifiers );
    QPointF p( point );

    double width2 = size().width() / 2.0;
    double height2 = size().height() / 2.0;
    switch ( handleId )
    {
        case 0:
            if ( p.x() < width2 )
            {
                p.setX( width2 ); 
            }
            else if ( p.x() > size().width() )
            {
                p.setX( size().width() );
            }
            p.setY( 0 );
            m_cornerRadiusX = ( size().width() - p.x() ) / ( size().width() / 2.0 ) * 100.0;
            // this is needed otherwise undo/redo might not end in the same result
            if ( 100 - m_cornerRadiusX < 1e-10 )
                m_cornerRadiusX = 100;
            break;
        case 1:
            if ( p.y() < 0 )
            {
                p.setY( 0 );
            }
            else if ( p.y() > height2 )
            {
                p.setY( height2 ); 
            }
            p.setX( size().width() );
            m_cornerRadiusY = p.y() / ( size().height() / 2.0 ) * 100.0;
            if ( 100 - m_cornerRadiusY < 1e-10 )
                m_cornerRadiusY = 100;
            break;
    }

    m_handles[handleId] = p;
}

void KoRectangleShape::updatePath( const QSizeF &size )
{
    double rx = 0;
    double ry = 0;
    if ( m_cornerRadiusX > 0 && m_cornerRadiusY > 0 )
    {
        rx = size.width() / 200.0 * m_cornerRadiusX;
        ry = size.height() / 200.0 * m_cornerRadiusY;
    }

    double x2 = size.width() - rx;
    double y2 = size.height() - ry;

    // the points of the object must not be deleted and recreated as that will 
    // break command history
    int cp = 0;
    QPointF curvePoints[12];

    m_points[cp]->setPoint( QPointF( rx, 0 ) );
    m_points[cp]->unsetProperty( KoPathPoint::HasControlPoint1 );
    m_points[cp]->unsetProperty( KoPathPoint::HasControlPoint2 );
    
    if ( m_cornerRadiusX < 100 || m_cornerRadiusY == 0 )
    {
        m_points[++cp]->setPoint( QPointF( x2, 0 ) );
        m_points[cp]->unsetProperty( KoPathPoint::HasControlPoint1 );
        m_points[cp]->unsetProperty( KoPathPoint::HasControlPoint2 );
    }

    if ( rx )
    {
        arcToCurve( rx, ry, 90, -90, m_points[cp]->point(), curvePoints );
        m_points[cp]->setControlPoint2( curvePoints[0] );
        m_points[++cp]->setControlPoint1( curvePoints[1] );
        m_points[cp]->setPoint( curvePoints[2] );
        m_points[cp]->unsetProperty( KoPathPoint::HasControlPoint2 );
    }

    if ( m_cornerRadiusY < 100 || m_cornerRadiusX == 0 )
    {
        m_points[++cp]->setPoint( QPointF( size.width(), y2 ) );
        m_points[cp]->unsetProperty( KoPathPoint::HasControlPoint1 );
        m_points[cp]->unsetProperty( KoPathPoint::HasControlPoint2 );
    }

    if ( rx )
    {
        arcToCurve( rx, ry, 0, -90, m_points[cp]->point(), curvePoints );
        m_points[cp]->setControlPoint2( curvePoints[0] );
        m_points[++cp]->setControlPoint1( curvePoints[1] );
        m_points[cp]->setPoint( curvePoints[2] );
        m_points[cp]->unsetProperty( KoPathPoint::HasControlPoint2 );
    }

    if ( m_cornerRadiusX < 100 || m_cornerRadiusY == 0 )
    {
        m_points[++cp]->setPoint( QPointF( rx, size.height() ) );
        m_points[cp]->unsetProperty( KoPathPoint::HasControlPoint1 );
        m_points[cp]->unsetProperty( KoPathPoint::HasControlPoint2 );
    }

    if ( rx )
    {
        arcToCurve( rx, ry, 270, -90, m_points[cp]->point(), curvePoints );
        m_points[cp]->setControlPoint2( curvePoints[0] );
        m_points[++cp]->setControlPoint1( curvePoints[1] );
        m_points[cp]->setPoint( curvePoints[2] );
        m_points[cp]->unsetProperty( KoPathPoint::HasControlPoint2 );
    }

    if ( m_cornerRadiusY < 100 || m_cornerRadiusX == 0 )
    {
        m_points[++cp]->setPoint( QPointF( 0, ry ) );
        m_points[cp]->unsetProperty( KoPathPoint::HasControlPoint1 );
        m_points[cp]->unsetProperty( KoPathPoint::HasControlPoint2 );
    }

    if ( rx )
    {
        arcToCurve( rx, ry, 180, -90, m_points[cp]->point(), curvePoints );
        m_points[cp++]->setControlPoint2( curvePoints[0] );
        m_points[0]->setControlPoint1( curvePoints[1] );
        m_points[0]->setPoint( curvePoints[2] );
    }
    
    qDebug() << "KoRectangleShape" << cp;
    m_subpaths[0]->clear();
    for ( int i = 0; i < cp; ++i )
    {
        if ( i != cp - 1 )
        {
            m_points[i]->unsetProperty( KoPathPoint::CloseSubpath );
        }
        else
        {
            m_points[i]->setProperty( KoPathPoint::CloseSubpath );
        }
        m_subpaths[0]->push_back( m_points[i] );
    }
}

void KoRectangleShape::createPath( const QSizeF &size )
{
    double rx = size.width() / 4;
    double ry = size.height() / 4;
    double x2 = size.width() - rx;
    double y2 = size.height() - ry;
    moveTo( QPointF( rx, 0 ) );
    lineTo( QPointF( x2, 0 ) );
    arcTo( rx, ry, 90, -90 );
    lineTo( QPointF( size.width(), y2 ) );
    arcTo( rx, ry, 0, -90 );
    lineTo( QPointF( rx, size.height() ) );
    arcTo( rx, ry, 270, -90 );
    lineTo( QPointF( 0, ry ) );
    arcTo( rx, ry, 180, -90 );
    closeMerge();
}
