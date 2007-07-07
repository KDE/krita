/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>
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

#include "KoPathPoint.h"
#include "KoPathShape.h"
#include "KoPointGroup.h"
//   #include "KoShapeBorderModel.h"
//   #include "KoViewConverter.h"
//   #include "KoPathShapeLoader.h"
//
//   #include <KoShapeSavingContext.h>
//   #include <KoXmlWriter.h>
//   #include <KoXmlNS.h>
//   #include <KoUnit.h>
//
//   #include <KDebug>
#include <QtGui/QPainter>
#include <QPointF>
//   #include <QtGui/QPainterPath>
//   #include <math.h>

class KoPathPoint::Private {
public:
    Private() : shape(0), properties(Normal), pointGroup(0) {}
    KoPathShape * shape;
    QPointF point;
    QPointF controlPoint1;
    QPointF controlPoint2;
    KoPointProperties properties;
    KoPointGroup * pointGroup;
};

KoPathPoint::KoPathPoint( const KoPathPoint & pathPoint )
: d(new Private())
{
    d->shape = pathPoint.d->shape;
    d->point = pathPoint.d->point;
    d->controlPoint1 = pathPoint.d->controlPoint1;
    d->controlPoint2 = pathPoint.d->controlPoint2;
    d->properties = pathPoint.d->properties;
}

KoPathPoint::KoPathPoint()
    : d(new Private())
{
}

KoPathPoint::KoPathPoint( KoPathShape * path, const QPointF & point, KoPointProperties properties)
    : d(new Private())
{
    d->shape = path;
    d->point = point;
    d->properties = properties;
}

KoPathPoint::~KoPathPoint() {
    delete d;
}

KoPathPoint& KoPathPoint::operator=( const KoPathPoint &rhs )
{
    if( this == &rhs )
        return (*this);

    d->shape = rhs.d->shape;
    d->point = rhs.d->point;
    d->controlPoint1 = rhs.d->controlPoint1;
    d->controlPoint2 = rhs.d->controlPoint2;
    d->properties = rhs.d->properties;
    //d->pointGroup = rhs.d->pointGroup;

    return (*this);
}

void KoPathPoint::setPoint( const QPointF & point ) 
{
    d->point = point;
    d->shape->update();
}

void KoPathPoint::setControlPoint1( const QPointF & point ) 
{ 
    d->controlPoint1 = point; 
    d->properties |= HasControlPoint1; 
    d->shape->update(); 
}

void KoPathPoint::setControlPoint2( const QPointF & point ) 
{ 
    d->controlPoint2 = point; 
    d->properties |= HasControlPoint2; 
    d->shape->update();
}

void KoPathPoint::setProperties( KoPointProperties properties ) 
{
    if( (properties & HasControlPoint1) == 0 || (properties & HasControlPoint2) == 0 )
    {
        // strip smooth and symmetric flags if points has not two control points
        properties &= ~IsSmooth;
        properties &= ~IsSymmetric;
    }
    d->properties = properties;
    d->shape->update();
}

void KoPathPoint::setProperty( KoPointProperty property )
{
    switch( property )
    {
        case StartSubpath:
            d->properties &= ~CloseSubpath;
        break;
        case CloseSubpath:
            d->properties &= ~StartSubpath;
            d->properties |= CanHaveControlPoint2;
        break;
        case CanHaveControlPoint1:
            if( d->properties & StartSubpath )
                return;
        break;
        case HasControlPoint1:
            if( (d->properties & CanHaveControlPoint1) == 0 )
                return;
        break;
        case CanHaveControlPoint2:
            if( d->properties & CloseSubpath )
                return;
        break;
        case HasControlPoint2:
            if( (d->properties & CanHaveControlPoint2) == 0 )
                return;
        break;
        case IsSmooth:
            if( (d->properties & HasControlPoint1) == 0 && (d->properties & HasControlPoint2) == 0 )
                return;
            d->properties &= ~IsSymmetric;
        break;
        case IsSymmetric:
            if( (d->properties & HasControlPoint1) == 0 && (d->properties & HasControlPoint2) == 0 )
                return;
            d->properties &= ~IsSmooth;
        break;
        default: return;
    }
    d->properties |= property;
}

void KoPathPoint::unsetProperty( KoPointProperty property )
{
    switch( property )
    {
        case StartSubpath:
            d->properties |= CanHaveControlPoint1;
        break;
        case CloseSubpath:
            d->properties |= CanHaveControlPoint2;
        break;
        case CanHaveControlPoint1:
            d->properties &= ~HasControlPoint1;
            d->properties &= ~IsSmooth;
            d->properties &= ~IsSymmetric;
        break;
        case CanHaveControlPoint2:
            d->properties &= ~HasControlPoint2;
            d->properties &= ~IsSmooth;
            d->properties &= ~IsSymmetric;
        break;
        case HasControlPoint1:
        case HasControlPoint2:
            d->properties &= ~IsSmooth;
            d->properties &= ~IsSymmetric;
        break;
        case IsSmooth:
        case IsSymmetric:
            // no others depend on these
        break;
        default: return;
    }
    d->properties &= ~property;
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
    if ( d->pointGroup && mapGroup )
    {
        d->pointGroup->map( matrix );
    }
    else
    {
        d->point = matrix.map( d->point ); 
        d->controlPoint1 = matrix.map( d->controlPoint1 );
        d->controlPoint2 = matrix.map( d->controlPoint2 );
    }
    d->shape->update(); 
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
            QMatrix matrix;
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
    d->shape = parent;
}

QRectF KoPathPoint::boundingRect( bool active ) const
{
    QRectF rect( d->point, QSize( 1, 1 ) );
    if ( !active || activeControlPoint1() )
    {
        QRectF r1( d->point, QSize( 1, 1 ) );
        r1.setBottomRight( d->controlPoint1 );
        rect = rect.unite( r1 );
    }
    if ( !active || activeControlPoint2() )
    {
        QRectF r2( d->point, QSize( 1, 1 ) );
        r2.setBottomRight( d->controlPoint2 );
        rect = rect.unite( r2 );
    }
    return d->shape->shapeToDocument( rect );
}

void KoPathPoint::reverse()
{
    qSwap( d->controlPoint1, d->controlPoint2 );
    KoPointProperties newProps = Normal;
    if( d->properties & CanHaveControlPoint1 )
        newProps |= CanHaveControlPoint2;
    if( d->properties & CanHaveControlPoint2 )
        newProps |= CanHaveControlPoint1;
    if( d->properties & HasControlPoint1 )
        newProps |= HasControlPoint2;
    if( d->properties & HasControlPoint2 )
        newProps |= HasControlPoint1;
    newProps |= d->properties & IsSmooth;
    newProps |= d->properties & IsSymmetric;
    newProps |= d->properties & StartSubpath;
    newProps |= d->properties & CloseSubpath;
    d->properties = newProps;
}

void KoPathPoint::removeFromGroup()
{ 
    if ( d->pointGroup ) 
        d->pointGroup->remove( this ); 
    d->pointGroup = 0; 
}

void KoPathPoint::addToGroup( KoPointGroup *pointGroup ) 
{ 
    if ( d->pointGroup && d->pointGroup != pointGroup )
    {
        //TODO error message as this should not happen
        removeFromGroup();
    }
    d->pointGroup = pointGroup; 
}

KoPathPoint::KoPointProperties KoPathPoint::properties() const {
    return d->properties;
}

QPointF KoPathPoint::point() const {
    return d->point;
}

QPointF KoPathPoint::controlPoint1() const {
    return d->controlPoint1;
}

QPointF KoPathPoint::controlPoint2() const {
    return d->controlPoint2;
}

KoPathShape * KoPathPoint::parent() const {
    return d->shape;
}

KoPointGroup * KoPathPoint::group() {
    return d->pointGroup;
}
