/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>

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

KoPathPoint::KoPathPoint( const KoPathPoint & pathPoint )
: m_pointGroup( 0 )
{
    m_shape = pathPoint.m_shape;
    m_point = pathPoint.m_point;
    m_controlPoint1 = pathPoint.m_controlPoint1;
    m_controlPoint2 = pathPoint.m_controlPoint2;
    m_properties = pathPoint.m_properties;
}

KoPathPoint& KoPathPoint::operator=( const KoPathPoint &rhs )
{
    if( this == &rhs )
        return (*this);

    m_shape = rhs.m_shape;
    m_point = rhs.m_point;
    m_controlPoint1 = rhs.m_controlPoint1;
    m_controlPoint2 = rhs.m_controlPoint2;
    m_properties = rhs.m_properties;
    //m_pointGroup = rhs.m_pointGroup;

    return (*this);
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
    if( properties & HasControlPoint1 == 0 || properties & HasControlPoint2 == 0 )
    {
        // strip smooth and symmetric flags if points has not two control points
        properties &= ~IsSmooth;
        properties &= ~IsSymmetric;
    }
    if( properties & KoPathPoint::StartSubpath )
    {
        properties &= ~CloseSubpath;
    }
    m_properties = properties;
    m_shape->update();
}

void KoPathPoint::setProperty( KoPointProperty property )
{
    switch( property )
    {
        case StartSubpath:
            m_properties &= ~CloseSubpath;
        break;
        case CloseSubpath:
            m_properties &= ~StartSubpath;
            m_properties |= CanHaveControlPoint2;
        break;
        case CanHaveControlPoint1:
            if( m_properties & StartSubpath )
                return;
        break;
        case HasControlPoint1:
            if( m_properties & CanHaveControlPoint1 == 0 )
                return;
        break;
        case CanHaveControlPoint2:
            if( m_properties & CloseSubpath )
                return;
        break;
        case HasControlPoint2:
            if( m_properties & CanHaveControlPoint2 == 0 )
                return;
        break;
        case IsSmooth:
            if( m_properties & HasControlPoint1 == 0 && m_properties & HasControlPoint2 == 0 )
                return;
            m_properties &= ~IsSymmetric;
        break;
        case IsSymmetric:
            if( m_properties & HasControlPoint1 == 0 && m_properties & HasControlPoint2 == 0 )
                return;
            m_properties &= ~IsSmooth;
        break;
        default: return;
    }
    m_properties |= property;
}

void KoPathPoint::unsetProperty( KoPointProperty property )
{
    switch( property )
    {
        case StartSubpath:
            m_properties |= CanHaveControlPoint1;
        break;
        case CloseSubpath:
            m_properties |= CanHaveControlPoint2;
        break;
        case CanHaveControlPoint1:
            m_properties &= ~HasControlPoint1;
            m_properties &= ~IsSmooth;
            m_properties &= ~IsSymmetric;
        break;
        case CanHaveControlPoint2:
            m_properties &= ~HasControlPoint2;
            m_properties &= ~IsSmooth;
            m_properties &= ~IsSymmetric;
        break;
        case HasControlPoint1:
        case HasControlPoint2:
            m_properties &= ~IsSmooth;
            m_properties &= ~IsSymmetric;
        break;
        case IsSmooth:
        case IsSymmetric:
            // no others depend on these
        break;
        default: return;
    }
    m_properties &= ~property;
}

bool KoPathPoint::activeControlPoint1()
{
    return ( properties() & HasControlPoint1 && properties() & CanHaveControlPoint1 );
}

bool KoPathPoint::activeControlPoint2()
{
    return ( properties() & HasControlPoint2 && properties() & CanHaveControlPoint2 );
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

void KoPathPoint::paint(QPainter &painter, const QSizeF &size, bool selected)
{
    QRectF handle( QPointF(-0.5*size.width(),0-0.5*size.height()), size );

    if( selected )
    {
        if( activeControlPoint1() )
        {
            painter.drawLine( point(), controlPoint1() );
            painter.drawEllipse( handle.translated( controlPoint1() ) );
        }
        if( activeControlPoint2() )
        {
            painter.drawLine( point(), controlPoint2() );
            painter.drawEllipse( handle.translated( controlPoint2() ) );
        }
    }

    if( properties() & IsSmooth )
        painter.drawRect( handle.translated( point() ) );
    else if( properties() & IsSymmetric )
    {
        QWMatrix matrix;
        matrix.rotate( 45.0 );
        QPolygonF poly( handle );
        poly = matrix.map( poly );
        poly.translate( point() );
        painter.drawPolygon( poly );
    }
    else
        painter.drawEllipse( handle.translated( point() ) );
}

void KoPathPoint::setParent( KoPathShape* parent )
{
    m_shape = parent;
}

void KoPathPoint::reverse()
{
    qSwap( m_controlPoint1, m_controlPoint2 );
    KoPointProperties newProps = Normal;
    if( m_properties & CanHaveControlPoint1 )
        newProps |= CanHaveControlPoint2;
    if( m_properties & CanHaveControlPoint2 )
        newProps |= CanHaveControlPoint1;
    if( m_properties & HasControlPoint1 )
        newProps |= HasControlPoint2;
    if( m_properties & HasControlPoint2 )
        newProps |= HasControlPoint1;
    newProps |= m_properties & IsSmooth;
    newProps |= m_properties & IsSymmetric;
    newProps |= m_properties & StartSubpath;
    newProps |= m_properties & CloseSubpath;
    qDebug() << "oldProps = " << m_properties;
    m_properties = newProps;
    qDebug() << "newProps = " << m_properties;
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
    qDebug() << "nop = " << i;
}
#endif

void KoPathShape::paintDecorations(QPainter &painter, const KoViewConverter &converter, bool selected)
{
    if( ! selected ) return;

    applyConversion( painter, converter );

    KoSubpathList::const_iterator pathIt( m_subpaths.begin() );

    painter.save();
    painter.setRenderHint( QPainter::Antialiasing, true );
    painter.setBrush( Qt::blue );
    painter.setPen( Qt::blue );

    QRectF handle = converter.viewToDocument( handleRect( QPoint(0,0) ) );

    for ( ; pathIt != m_subpaths.end(); ++pathIt )
    {
        KoSubpath::const_iterator it( ( *pathIt )->begin() );
        for ( ; it != ( *pathIt )->end(); ++it )
        {
            KoPathPoint *point = ( *it );
            point->paint( painter, handle.size(), false );
        }
    }
    painter.restore();
}

QRectF KoPathShape::handleRect( const QPointF &p ) const
{
    const qreal handleRadius = 3.0;
    return QRectF( p.x()-handleRadius, p.y()-handleRadius, 2*handleRadius, 2*handleRadius );
}

const QPainterPath KoPathShape::KoPathShape::outline() const
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
            if ( ( *it )->properties() & KoPathPoint::StartSubpath )
            {
                //qDebug() << "moveTo(" << ( *it )->point() << ")";
                path.moveTo( ( *it )->point() );
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
    //qDebug() << "KoPathShape::boundingRect = " << bb;
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
    updateLast( lastPoint );
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
    updateLast( lastPoint );
    lastPoint->setControlPoint2( c1 );
    KoPathPoint * point = new KoPathPoint( this, p, KoPathPoint::CanHaveControlPoint1 );
    point->setControlPoint1( c2 );
    m_subpaths.last()->push_back( point );
    return point;
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
    KoPathPoint * lastPoint = m_subpaths.last()->last();
    KoPathPoint * firstPoint = m_subpaths.last()->first();

    if ( lastPoint->point() == firstPoint->point() )
    {
        firstPoint->setProperties( firstPoint->properties() | KoPathPoint::CanHaveControlPoint1 );
        if ( lastPoint->activeControlPoint1() )
            firstPoint->setControlPoint1( lastPoint->controlPoint1() );
        removePoint( lastPoint );
        // remove point
        delete lastPoint;
        lastPoint = m_subpaths.last()->last();
        lastPoint->setProperties( lastPoint->properties() | KoPathPoint::CloseSubpath );
    }
    else
    {
        close();
    }
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
    moveBy( diff.x(), diff.y() );
    return diff;
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

void KoPathShape::updateLast( KoPathPoint * lastPoint )
{
    if ( lastPoint->properties() & KoPathPoint::CloseSubpath )
    {
        KoPathPoint * subpathStart = m_subpaths.last()->first();
        KoPathPoint * newLastPoint = new KoPathPoint( *subpathStart );
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
        lastPoint = newLastPoint;
        lastPoint->setProperties( KoPathPoint::Normal );
    }
    lastPoint->setProperties( lastPoint->properties() | KoPathPoint::CanHaveControlPoint2 );
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

KoPointPosition KoPathShape::removePoint( KoPathPoint *point )
{
    KoSubpathList::iterator pathIt( m_subpaths.begin() );
    for ( ; pathIt != m_subpaths.end(); ++pathIt )
    {
        int index = ( *pathIt )->indexOf( point );
        if( index != -1 )
        {
            ( *pathIt )->removeAt( index );
            point->setParent( 0 );

            if ( !( *pathIt )->isEmpty() )
            {
                // the first point of the sub path has been removed
                if ( index == 0 )
                {
                    if ( ( *pathIt )->last()->properties() & KoPathPoint::CloseSubpath )
                    {
                        ( *pathIt )->first()->setProperties( ( *pathIt )->first()->properties() | KoPathPoint::StartSubpath );
                    }
                    else
                    {
                        ( *pathIt )->first()->setProperties( ( ( *pathIt )->first()->properties() & ~KoPathPoint::CanHaveControlPoint1 ) | KoPathPoint::StartSubpath );
                    }
                }
                // the last point of the sub path has been removed
                else if ( index == ( *pathIt )->size() )
                {
                    if ( point->properties() & KoPathPoint::CloseSubpath )
                    {
                        ( *pathIt )->last()->setProperties( ( *pathIt )->last()->properties() | KoPathPoint::CloseSubpath );
                    }
                    else
                    {
                        ( *pathIt )->last()->setProperties( ( *pathIt )->last()->properties() & ~KoPathPoint::CanHaveControlPoint2 );
                    }
                }
                return QPair<KoSubpath*, int>( *pathIt, index );
            }
        }
    }
    return KoPointPosition( 0, 0 );
}

void KoPathShape::insertPoint( KoPathPoint* point, KoSubpath* subpath, int position )
{
    if ( position == 0 )
    {
        subpath->first()->setProperties( subpath->first()->properties() & ~KoPathPoint::StartSubpath | KoPathPoint::CanHaveControlPoint1 );
    }
    else if ( position == subpath->size() )
    {
        subpath->last()->setProperties( subpath->last()->properties() & ~KoPathPoint::CloseSubpath | KoPathPoint::CanHaveControlPoint2 );
    }
    subpath->insert( position, point );
    point->setParent( this );
}

KoPathPoint* KoPathShape::nextPoint( KoPathPoint* point )
{
    KoSubpathList::iterator pathIt( m_subpaths.begin() );
    for ( ; pathIt != m_subpaths.end(); ++pathIt )
    {
        int index = ( *pathIt )->indexOf( point );
        if( index != -1 )
        {
            if( index >= ( *pathIt )->size()-1 )
                return 0;
            else
                return ( *pathIt )->value( index+1 );
        }
    }
    return 0;
}

#if 0

KoPathPoint* KoPathShape::prevPoint( KoPathPoint* point )
{
    KoSubpathList::iterator pathIt( m_subpaths.begin() );
    for ( ; pathIt != m_subpaths.end(); ++pathIt )
    {
        int index = ( *pathIt )->indexOf( point );
        if( index != -1 )
        {
            if( index == 0 )
                return 0;
            else
                return ( *pathIt )->value( index-1 );
        }
    }
    return 0;
}

bool KoPathShape::insertPointAfter( KoPathPoint *point, KoPathPoint *prevPoint )
{
   KoSubpathList::iterator pathIt( m_subpaths.begin() );
    for ( ; pathIt != m_subpaths.end(); ++pathIt )
    {
        int index = ( *pathIt )->indexOf( prevPoint );
        if( index != -1 )
        {
            // we insert after the last point
            if( index >= ( *pathIt )->size() )
                ( *pathIt )->append( point );
            else
                ( *pathIt )->insert( index+1, point );
            return true;
        }
    }
    return false;
}

bool KoPathShape::insertPointBefore( KoPathPoint *point, KoPathPoint *nextPoint )
{
    KoSubpathList::iterator pathIt( m_subpaths.begin() );
    for ( ; pathIt != m_subpaths.end(); ++pathIt )
    {
        int index = ( *pathIt )->indexOf( nextPoint );
        if( index != -1 )
        {
            // we insert before the first point
            if( index == 0 )
                ( *pathIt )->prepend( point );
            else
                ( *pathIt )->insert( index-1, point );
            return true;
        }
    }
    return false;
}
#endif

KoPathPoint* KoPathShape::splitAt( const KoPathSegment &segment, double t )
{
    if( t < 0.0 || t > 1.0 )
        return 0;

    KoSubpath *subPath = 0;
    int index = 0;
    // first check if the segment is part of the path
    KoSubpathList::iterator pathIt( m_subpaths.begin() );
    for ( ; pathIt != m_subpaths.end(); ++pathIt )
    {
        index = ( *pathIt )->indexOf( segment.first );

        if( index != -1 )
        {
            subPath = *pathIt;
            if( index == ( *pathIt )->size()-1 )
                return 0;
            if( ( *pathIt )->at( index + 1 ) != segment.second )
                return 0;
            break;
        }
    }
    // both segment points were not found
    if( ! subPath )
        return 0;

    KoPathPoint *splitPoint = 0;

    // check if we have a curve
    if( segment.first->properties() & KoPathPoint::HasControlPoint2 || segment.second->properties() & KoPathPoint::HasControlPoint1 )
    {
        QPointF q[4] ={ 
           segment.first->point(), 
           segment.first->activeControlPoint2() ? segment.first->controlPoint2() : segment.first->point(), 
           segment.second->activeControlPoint1() ? segment.second->controlPoint1() : segment.second->point(), 
           segment.second->point()
        };
        QPointF p[3];
        // the De Casteljau algorithm.
        for( unsigned short j = 1; j <= 3; ++j )
        {
            for( unsigned short i = 0; i <= 3 - j; ++i )
            {
                q[ i ] = ( 1.0 - t ) * q[ i ] + t * q[ i + 1 ];
            }
            // modify the new segment.
            p[j - 1] = q[ 0 ];
        }
        splitPoint = new KoPathPoint( this, p[2], KoPathPoint::CanHaveControlPoint1|KoPathPoint::CanHaveControlPoint2 );
        // modify the second control point of the segment start point
        segment.first->setControlPoint2( p[0] );

        splitPoint->setControlPoint1( p[1] );
        splitPoint->setControlPoint2( q[1] );

        // modify the first control point of the segment end point
        segment.second->setControlPoint1( q[2] );
    }
    else
    {
        QPointF splitPointPos = segment.first->point() + t * (segment.second->point() - segment.first->point());
        // easy, just a line
        splitPoint = new KoPathPoint( this, splitPointPos, KoPathPoint::CanHaveControlPoint1|KoPathPoint::CanHaveControlPoint2 );
    }

    subPath->insert( index+1, splitPoint );

    return splitPoint;
}

bool KoPathShape::breakAt( KoPathPoint *breakPoint, KoPathPoint* &insertedPoint )
{
    if( ! breakPoint )
        return false;

    KoPointPosition pointPos = findPoint( breakPoint );
    if( ! pointPos.first )
        return false;

    KoSubpath *subpath = pointPos.first;
    KoPathPoint *newPoint = 0;

    if( &insertedPoint )
        insertedPoint = 0;

    // check if the subpath is closed
    if( subpath->last()->properties() & KoPathPoint::CloseSubpath )
    {
        // break at the first subpath point
        if( pointPos.second == 0 )
        {
            // duplicate the first point, which becomes the new last point
            newPoint = new KoPathPoint( *subpath->first() );
            newPoint->unsetProperty( KoPathPoint::StartSubpath );
            newPoint->unsetProperty( KoPathPoint::CanHaveControlPoint2 );
            subpath->last()->unsetProperty( KoPathPoint::CloseSubpath );
            subpath->append( newPoint );
        }
        // break at the last point
        else if( pointPos.second == subpath->size()-1 )
        {
            // duplicate the last point, which becomes the new first point
            newPoint = new KoPathPoint( *subpath->last() );
            // the last point no longer closes the subpath
            subpath->last()->unsetProperty( KoPathPoint::CloseSubpath );
            // the first point no longer starts the subpath 
            subpath->first()->unsetProperty( KoPathPoint::StartSubpath );
            // the new point start the open subpath
            newPoint->setProperty( KoPathPoint::StartSubpath );
            newPoint->unsetProperty( KoPathPoint::CanHaveControlPoint1 );
            subpath->prepend( newPoint );
        }
        // break in between
        else
        {
            // duplicate the break point, which becomes the new first point
            newPoint = new KoPathPoint( *breakPoint );
            // the old starting node no longer starts the subpath
            subpath->first()->unsetProperty( KoPathPoint::StartSubpath );
            subpath->first()->setProperty( KoPathPoint::CanHaveControlPoint1 );
            // the old end node no longer closes nor ends the subpath
            subpath->last()->unsetProperty( KoPathPoint::CloseSubpath );
            // now reorder the subpath
            for( int i = 0; i <= pointPos.second; ++i )
                subpath->append( subpath->takeFirst() );

            // make the breakpoint an ending node
            breakPoint->unsetProperty( KoPathPoint::CanHaveControlPoint2 );
            // make the new point a starting node 
            newPoint->setProperty( KoPathPoint::StartSubpath );
            newPoint->unsetProperty( KoPathPoint::CanHaveControlPoint1 );
            subpath->prepend( newPoint );
        }
    }
    else
    {
        // the subpath is not closed, so breaking at the end nodes has no effect
        if( pointPos.second == 0 || pointPos.second == subpath->size()-1 )
            return false;

        // now break the subpath into two subpaths
        KoSubpath *newSubpath = new KoSubpath;

        // copy the break point and make it a starting node
        newPoint = new KoPathPoint( *breakPoint );
        newPoint->setProperty( KoPathPoint::StartSubpath );
        newPoint->unsetProperty( KoPathPoint::CanHaveControlPoint1 );
        newSubpath->append( newPoint );

        // make the break point an end node
        breakPoint->unsetProperty( KoPathPoint::CanHaveControlPoint2 );

        int size = subpath->size();
        for( int i = pointPos.second+1; i < size; ++i )
            newSubpath->append( subpath->takeAt( pointPos.second+1 ) );

        m_subpaths.append( newSubpath );
    }

    if( &insertedPoint )
        insertedPoint = newPoint;

    return true;
}

bool KoPathShape::breakAt( const KoPathSegment &segment )
{
    if( ! segment.first || ! segment.second )
        return false;
    if( segment.first == segment.second )
        return false;
    KoPointPosition pos1 = findPoint( segment.first );
    if( ! pos1.first )
        return false;
    KoPointPosition pos2 = findPoint( segment.second );
    if( ! pos2.first )
        return false;
    if( pos1.first != pos2.first )
        return false;

    // get ordered point indeces 
    KoSubpath *subpath = pos1.first;
    int index1 = pos1.second < pos2.second ? pos1.second : pos2.second;
    int index2 = pos1.second > pos2.second ? pos1.second : pos2.second;
    KoPathPoint *p1 = (*subpath)[index1];
    KoPathPoint *p2 = (*subpath)[index2];

    // check if the path is closed
    if( subpath->last()->properties() & KoPathPoint::CloseSubpath )
    {
        // check if we break at the closing segment
        if( index1 == 0 && index2 == subpath->size()-1 )
        {
            // just unclose the subpath
            p2->setProperties( p2->properties() & ~KoPathPoint::CloseSubpath );
        }
        else
        {
            // we break in between -> reorder the subpath
            // make first point the new endpoint
            p1->unsetProperty( KoPathPoint::CanHaveControlPoint2 );
            // make second point the new start point
            p2->setProperty( KoPathPoint::StartSubpath );
            // the last point no longer closes the subpath
            subpath->last()->unsetProperty( KoPathPoint::CloseSubpath );
            // the first no longer start the subath
            subpath->first()->unsetProperty( KoPathPoint::StartSubpath );
            // now reorder the subpath
            for( int i = 0; i <= index1; ++i )
                subpath->append( subpath->takeFirst() );
        }
    }
    else
    {
        // subpath is not closed, breaking between first and last has no effect
        if( index1 == 0 && index2 == subpath->size()-1 )
            return true;

        // break into two subpaths
        KoSubpath *newSubpath = new KoSubpath;
        int size = subpath->size();
        for( int i = index2; i < size; ++i )
            newSubpath->append( subpath->takeAt( index2 ) );

        // now make the first point of the new subpath a starting node
        newSubpath->first()->setProperty( KoPathPoint::StartSubpath );
        newSubpath->first()->unsetProperty( KoPathPoint::CanHaveControlPoint1 );
        // the last point of the old subpath is now an ending node 
        subpath->last()->unsetProperty( KoPathPoint::CanHaveControlPoint2 );
        m_subpaths.append( newSubpath );
    }
    repaint();
    return true;
}

bool KoPathShape::joinBetween( KoPathPoint *endPoint1, KoPathPoint *endPoint2 )
{
    if( endPoint1 == endPoint2 )
        return false;

    KoPointPosition pos1 = findPoint( endPoint1 );
    if( ! pos1.first )
        return false;
    // check if first point is end or start node
    if( pos1.second != 0 && pos1.second != pos1.first->size()-1 )
        return false;
    KoPointPosition pos2 = findPoint( endPoint2 );
    if( ! pos2.first )
        return false;
    // check if second point is end or start node
    if( pos2.second != 0 && pos2.second != pos2.first->size()-1 )
        return false;

    // check if one of the subpaths is already closed
    if( pos1.first->last()->properties() & KoPathPoint::CloseSubpath )
        return false;
    if( pos2.first->last()->properties() & KoPathPoint::CloseSubpath )
        return false;

    // check if points are end nodes of same subpath
    if( pos1.first == pos2.first )
    {
        // just close the subpath
        closeSubpath( pos1.first );
    }
    else
    {
        // merge the two subpaths
        if( pos1.second == 0 )
            reverseSubpath( *pos1.first );

        // the last point is no longer an end node
        pos1.first->last()->setProperty( KoPathPoint::CanHaveControlPoint2 );

        if( pos2.second != 0 )
            reverseSubpath( *pos2.first );

        // the first point does not start a subpath anymore
        pos2.first->first()->unsetProperty( KoPathPoint::StartSubpath );

        // append the second subpath to the first
        foreach( KoPathPoint* p, *pos2.first )
            pos1.first->append( p );

        // delete the second subpaths
        int index = m_subpaths.indexOf( pos2.first );
        m_subpaths.removeAt( index );
    }

    return true;
}

KoPointPosition KoPathShape::findPoint( KoPathPoint* point )
{
    KoSubpathList::iterator pathIt( m_subpaths.begin() );
    for ( ; pathIt != m_subpaths.end(); ++pathIt )
    {
        int index = ( *pathIt )->indexOf( point );
        if( index != -1 )
            return KoPointPosition( *pathIt, index );
    }
    return KoPointPosition( 0, 0 );
}

void KoPathShape::closeSubpath( KoSubpath *subpath )
{
    KoPathPoint * lastPoint = subpath->last();
    lastPoint->setProperties( lastPoint->properties() | KoPathPoint::CloseSubpath | KoPathPoint::CanHaveControlPoint2 );
    KoPathPoint * firstPoint = subpath->first();
    firstPoint->setProperties( firstPoint->properties() | KoPathPoint::CanHaveControlPoint1 );
}

void KoPathShape::reverseSubpath( KoSubpath &subpath )
{
    int size = subpath.size();
    for( int i = 0; i < size; ++i )
    {
        KoPathPoint *p = subpath.takeAt( i );
        p->reverse();
        subpath.prepend( p );
    }

    // adjust the position dependent properties
    KoPathPoint *first = subpath.first();
    KoPathPoint *last = subpath.last();

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
}
