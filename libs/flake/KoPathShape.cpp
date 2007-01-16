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
#include "KoInsets.h"
#include "KoShapeBorderModel.h"
#include "KoViewConverter.h"

#include <QDebug>
#include <QPainter>
#include <math.h>

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

bool KoPathPoint::activeControlPoint1() const
{
    return ( properties() & HasControlPoint1 && properties() & CanHaveControlPoint1 );
}

bool KoPathPoint::activeControlPoint2() const
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

void KoPathPoint::paint( QPainter &painter, const QSizeF &size, KoPointTypes types, bool active )
{
    QRectF handle( QPointF( -0.5 * size.width(), -0.5 * size.height() ), size );

    bool drawControlPoint1 = types & ControlPoint1 && ( !active || activeControlPoint1() );
    bool drawControlPoint2 = types & ControlPoint2 && ( !active || activeControlPoint2() );

    // draw lines at the bottom
    if ( drawControlPoint2 )
        painter.drawLine( point(), controlPoint2() );

    if ( drawControlPoint1 )
        painter.drawLine( point(), controlPoint1() );

    // the point is lowest 
    if ( types & Node )
    {
        if ( properties() & IsSmooth )
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

    // then comes control point 2
    if ( drawControlPoint2 )
        painter.drawEllipse( handle.translated( controlPoint2() ) );

    // then comes control point 1
    if ( drawControlPoint1 )
        painter.drawEllipse( handle.translated( controlPoint1() ) );
}

void KoPathPoint::setParent( KoPathShape* parent )
{
    // don't set to zero
    Q_ASSERT( parent );
    m_shape = parent;
}

QRectF KoPathPoint::boundingRect( bool active ) const
{
    QRectF rect( m_point, QSize( 1, 1 ) );
    if ( !active || activeControlPoint1() )
    {
        QRectF r1( m_point, QSize( 1, 1 ) );
        r1.setBottomRight( m_controlPoint1 );
        rect = rect.unite( r1 );
    }
    if ( !active || activeControlPoint2() )
    {
        QRectF r2( m_point, QSize( 1, 1 ) );
        r2.setBottomRight( m_controlPoint2 );
        rect = rect.unite( r2 );
    }
    return m_shape->shapeToDocument( rect );
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
    m_properties = newProps;
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
    clear();
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

void KoPathShape::debugPath()
{
    KoSubpathList::iterator pathIt( m_subpaths.begin() );
    for ( ; pathIt != m_subpaths.end(); ++pathIt )
    {
        KoSubpath::const_iterator it( ( *pathIt )->begin() );
        for ( ; it != ( *pathIt )->end(); ++it )
        {
            qDebug() << "p:" << ( *pathIt ) << "," << *it << "," << ( *it )->point() << "," << ( *it )->properties() << "," << ( *it )->group();
        }
    }
}

QPointF KoPathShape::shapeToDocument( const QPointF &point ) const
{
    return transformationMatrix(0).map( point );
}

QRectF KoPathShape::shapeToDocument( const QRectF &rect ) const 
{
    return transformationMatrix(0).mapRect( rect );
}

QPointF KoPathShape::documentToShape( const QPointF &point ) const
{
    return transformationMatrix(0).inverted().map( point );
}

QRectF KoPathShape::documentToShape( const QRectF &rect ) const 
{
    return transformationMatrix(0).inverted().mapRect( rect );
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
    if( m_border )
    {
        KoInsets inset;
        m_border->borderInsets( this, inset );
        bb.adjust( -inset.left, -inset.top, inset.right, inset.bottom );
    }
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

    qDebug() << "kappa" << kappa << "parts" << parts;
    
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

void KoPathShape::update()
{
    notifyChanged();
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
    else if ( pointIndex.second == subpath->size() ) // use size as point is allreay removed
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

    QMatrix pathMatrix = path->transformationMatrix(0);
    QMatrix myMatrix = transformationMatrix(0).inverted();

    foreach( KoSubpath* subpath, path->m_subpaths )
    {
        KoSubpath *newSubpath = new KoSubpath();

        foreach( KoPathPoint* point, *subpath )
        {
            KoPathPoint *newPoint = new KoPathPoint( *point );
            newPoint->map( pathMatrix );
            newPoint->map( myMatrix );
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

    QMatrix myMatrix = transformationMatrix(0);

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
    if ( ! subpath )
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
