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

#include "KoPathShape.h"

#include <QDebug>
#include <QPainter>

KoPathPoint::KoPathPoint( KoPathPoint & pathPoint )
: m_pointGroup( 0 )
{
    m_shape = pathPoint.m_shape;
    m_point = pathPoint.m_point;
    m_controlPoint1 = pathPoint.m_controlPoint1;
    m_controlPoint2 = pathPoint.m_controlPoint2;
    m_properties = pathPoint.m_properties;
}

void KoPathPoint::setPoint( const QPointF & point ) 
{ 
    m_point = point; 
    m_shape->update();
}

void KoPathPoint::setControlPoint1( const QPointF & point ) 
{ 
    m_controlPoint1 = point; 
    m_properties |= HasControlPoint1; 
    m_shape->update(); 
}

void KoPathPoint::setControlPoint2( const QPointF & point ) 
{ 
    m_controlPoint2 = point; 
    m_properties |= HasControlPoint2; 
    m_shape->update();
}

void KoPathPoint::setProperties( KoPointProperties properties ) 
{ 
    m_properties = properties; 
    m_shape->update(); 
}

void KoPathPoint::map( const QMatrix &matrix, bool mapGroup )
{ 
    if ( m_pointGroup && mapGroup )
    {
        m_pointGroup->map( matrix );
    }
    else
    {
        m_point = matrix.map( m_point ); 
        m_controlPoint1 = matrix.map( m_controlPoint1 );
        m_controlPoint2 = matrix.map( m_controlPoint2 );
    }
    m_shape->update(); 
}

void KoPathPoint::removeFromGroup() 
{ 
    if ( m_pointGroup ) 
        m_pointGroup->remove( this ); 
    m_pointGroup = 0; 
}

void KoPathPoint::addToGroup( KoPointGroup *pointGroup ) 
{ 
    if ( m_pointGroup && m_pointGroup != pointGroup )
    {
        //TODO error message as this should not happen
        removeFromGroup();
    }
    m_pointGroup = pointGroup; 
}

void KoPointGroup::add( KoPathPoint * point ) 
{ 
    m_points.insert( point ); 
    point->addToGroup( this );
}

void KoPointGroup::remove( KoPathPoint * point ) 
{ 
    if ( m_points.remove( point ) ) 
    {    
        point->removeFromGroup();
        if ( m_points.size() == 1 )
        {
            ( * m_points.begin() )->removeFromGroup();
            //commit suicide as it is no longer used
            delete this;
        }
    }
}

void KoPointGroup::map( const QMatrix &matrix )
{
    QSet<KoPathPoint *>::iterator it = m_points.begin();
    for ( ; it != m_points.end(); ++it )
    {
        ( *it )->map( matrix, false );
    }
}

KoPathShape::KoPathShape()
{
}

KoPathShape::~KoPathShape()
{
}

void KoPathShape::paint( QPainter &painter, const KoViewConverter &converter )
{
    applyConversion( painter, converter );
    QPainterPath path( outline() );
    
    painter.setBrush( background() );
    painter.drawPath( path );
    //paintDebug( painter );
}

void KoPathShape::paintDebug( QPainter &painter )
{
    QList<KoSubpath>::const_iterator pathIt( m_points.begin() );
    int i = 0;

    QPen pen( Qt::black );
    painter.save();
    painter.setPen( pen );
    for ( ; pathIt != m_points.end(); ++pathIt )
    {
        KoSubpath::const_iterator it( ( *pathIt ).begin() );
        for ( ; it != ( *pathIt ).end(); ++it )
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
    qDebug() << "nop = " << i;
}

const QPainterPath KoPathShape::KoPathShape::outline() const
{
    QList<KoSubpath>::const_iterator pathIt( m_points.begin() );
    QPainterPath path;
    for ( ; pathIt != m_points.end(); ++pathIt )
    {
        KoSubpath::const_iterator it( ( *pathIt ).begin() );
        KoPathPoint * lastPoint( *it );
        bool activeCP = false;
        for ( ; it != ( *pathIt ).end(); ++it )
        {
            if ( ( *it )->properties() & KoPathPoint::StartSubpath )
            {
                //qDebug() << "moveTo" << ( *it )->point();
                path.moveTo( ( *it )->point() );
            }
            else if ( activeCP || ( *it )->properties() & KoPathPoint::HasControlPoint1 )
            {
                //qDebug() << "activeCP " << activeCP 
                //    << "lastPoint->controlPoint2()" << lastPoint->controlPoint2()
                //    << "lastPoint->point()" << lastPoint->point() << ( *it )->controlPoint1();

                path.cubicTo( activeCP ? lastPoint->controlPoint2() : lastPoint->point()
                            , ( *it )->properties() & KoPathPoint::HasControlPoint1 ? ( *it )->controlPoint1() : ( *it )->point()
                            , ( *it )->point() );
            }
            else
            {
                //qDebug() << "lineTo" << ( *it )->point();
                path.lineTo( ( *it )->point() );
            }
            if ( ( *it )->properties() & KoPathPoint::CloseSubpath )
            {
                //qDebug() << "closeSubpath";
                path.closeSubpath();
            }

            if ( ( *it )->properties() & KoPathPoint::HasControlPoint2 )
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
    qDebug() << "KoPathShape::boundingRect = " << bb;
    return transformationMatrix( 0 ).mapRect( bb );
}


QSizeF KoPathShape::size() const
{
    // don't call boundingRect here as it uses transformationMatrix which leads to invinit reccursion
    return outline().boundingRect().size();
}

QPointF KoPathShape::position() const
{
    //return boundingRect().topLeft();
    return KoShape::position();
}

void KoPathShape::resize( const QSizeF &newSize )
{
    QSizeF oldSize = size();
    double zoomX = newSize.width() / oldSize.width(); 
    double zoomY = newSize.height() / oldSize.height(); 
    QMatrix matrix( zoomX, 0, 0, zoomY, 0, 0 );

    qDebug() << "resize" << zoomX << "," << zoomY << "," << newSize;
    map( matrix );
    KoShape::resize( newSize );
}

KoPathPoint * KoPathShape::moveTo( const QPointF &p )
{
    KoPathPoint * point = new KoPathPoint( this, p, KoPathPoint::StartSubpath | KoPathPoint::CanHaveControlPoint2 );
    KoSubpath path;
    path.push_back( point );
    m_points.push_back( path );
    return point;
}

KoPathPoint * KoPathShape::lineTo( const QPointF &p )
{
    if ( m_points.empty() )
    {
        moveTo( QPointF( 0, 0 ) );
    }
    KoPathPoint * point = new KoPathPoint( this, p, KoPathPoint::CanHaveControlPoint1 );
    KoPathPoint * lastPoint = m_points.last().last();
    updateLast( lastPoint );
    m_points.last().push_back( point );
    return point;
}

KoPathPoint * KoPathShape::curveTo( const QPointF &c1, const QPointF &c2, const QPointF &p )
{
    if ( m_points.empty() )
    {
        moveTo( QPointF( 0, 0 ) );
    }
    KoPathPoint * lastPoint = m_points.last().last();
    updateLast( lastPoint );
    lastPoint->setControlPoint2( c1 );
    KoPathPoint * point = new KoPathPoint( this, p, KoPathPoint::CanHaveControlPoint1 );
    point->setControlPoint1( c2 );
    m_points.last().push_back( point );
    return point;
}

void KoPathShape::close()
{
    if ( m_points.empty() )
    {
        return;
    }
    KoPathPoint * lastPoint = m_points.last().last();
    lastPoint->setProperties( lastPoint->properties() | KoPathPoint::CloseSubpath | KoPathPoint::CanHaveControlPoint2 );
    KoPathPoint * firstPoint = m_points.last().first();
    firstPoint->setProperties( firstPoint->properties() | KoPathPoint::CanHaveControlPoint1 );
}

void KoPathShape::normalize()
{
    QPointF tl( outline().boundingRect().topLeft() );
    QMatrix matrix;
    matrix.translate( -tl.x(), -tl.y() );
    map( matrix );
}

void KoPathShape::map( const QMatrix &matrix )
{
    QList<KoSubpath>::iterator pathIt( m_points.begin() );
    for ( ; pathIt != m_points.end(); ++pathIt )
    {
        KoSubpath::iterator it( ( *pathIt ).begin() );
        for ( ; it != ( *pathIt ).end(); ++it )
        {
            ( *it )->map( matrix );
        }
    }
}

void KoPathShape::updateLast( KoPathPoint * lastPoint )
{
    if ( lastPoint->properties() & KoPathPoint::CloseSubpath )
    {
        KoPathPoint * subpathStart = m_points.last().first();
        KoPathPoint * newLastPoint = new KoPathPoint( *subpathStart );
        KoPointGroup * group = subpathStart->group();
        if ( group == 0 )
        {
            group = new KoPointGroup();
            group->add( subpathStart );
        }
        group->add( newLastPoint );

        KoSubpath path;
        path.push_back( newLastPoint );
        m_points.push_back( path );
        lastPoint = newLastPoint;
        lastPoint->setProperties( KoPathPoint::Normal );
    }
    lastPoint->setProperties( lastPoint->properties() | KoPathPoint::CanHaveControlPoint2 );
}
