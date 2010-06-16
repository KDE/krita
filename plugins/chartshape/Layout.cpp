/* This file is part of the KDE project

   Copyright 2010 Johannes Simon <johannes.simon@gmail.com>

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

// Qt
#include <QSizeF>
#include <QPointF>

// KChart
#include "Layout.h"

// KOffice
#include <KoShapeContainer.h>

using namespace KChart;


// Static helper methods (defined at end of file)
static QPointF itemPosition( KoShape *shape );
static QSizeF itemSize( KoShape *shape );
static void setItemPosition( KoShape *shape, const QPointF& pos );

class Layout::LayoutData
{
public:
    Position pos;
    int weight;
    bool clipped;
    bool inheritsTransform;

    LayoutData( Position _pos, int _weight )
        : pos( _pos ),
          weight( _weight ),
          clipped( true ),
          inheritsTransform(true) {}
};

Layout::Layout()
    : m_doingLayout( false )
    , m_relayoutScheduled( false )
{
}

Layout::~Layout()
{
    foreach( LayoutData *data, m_layoutItems.values() )
        delete data;
}

void Layout::add( KoShape *shape )
{
    Q_ASSERT( !m_layoutItems.contains( shape ) );
    m_layoutItems.insert( shape, new LayoutData( FloatingPosition, 0 ) );
    scheduleRelayout();
}

void Layout::add( KoShape *shape, Position pos, int weight )
{
    Q_ASSERT( !m_layoutItems.contains( shape ) );
    m_layoutItems.insert( shape, new LayoutData( pos, weight ) );
    scheduleRelayout();
}

void Layout::remove( KoShape *shape )
{
    if ( m_layoutItems.contains( shape ) ) {
        // delete LayoutData
        delete m_layoutItems.value( shape );
        m_layoutItems.remove( shape );
        scheduleRelayout();
    }
}

void Layout::setClipped( const KoShape *shape, bool clipping )
{
    Q_ASSERT( m_layoutItems.contains( const_cast<KoShape*>(shape) ) );
    m_layoutItems.value( const_cast<KoShape*>(shape) )->clipped = clipping;
}

bool Layout::isClipped( const KoShape *shape ) const
{
    Q_ASSERT( m_layoutItems.contains( const_cast<KoShape*>(shape) ) );
    return m_layoutItems.value( const_cast<KoShape*>(shape) )->clipped;
}

void Layout::setInheritsTransform(const KoShape *shape, bool inherit)
{
    m_layoutItems.value(const_cast<KoShape*>(shape))->inheritsTransform = inherit;
}

bool Layout::inheritsTransform(const KoShape *shape) const
{
    return m_layoutItems.value(const_cast<KoShape*>(shape))->inheritsTransform;
}

int Layout::count() const
{
    return m_layoutItems.size();
}

QList<KoShape*> Layout::shapes() const
{
    return m_layoutItems.keys();
}

void Layout::containerChanged( KoShapeContainer *container, KoShape::ChangeType type )
{
    switch( type ) {
    case KoShape::SizeChanged:
        m_containerSize = container->size();
        scheduleRelayout();
        break;
    default:
        break;
    }
}

bool Layout::isChildLocked( const KoShape *shape ) const
{
    return shape->isGeometryProtected();
}

void Layout::setPosition( const KoShape *shape, Position pos, int weight )
{
    Q_ASSERT( m_layoutItems.contains( const_cast<KoShape*>(shape) ) );
    LayoutData *data = m_layoutItems.value( const_cast<KoShape*>(shape) );
    data->pos = pos;
    data->weight = weight;
    scheduleRelayout();
}

void Layout::childChanged( KoShape *shape, KoShape::ChangeType type )
{
    Q_UNUSED( shape );

    // Do not relayout again if we're currently in the process of a relayout.
    // Repositioning a layout item or resizing it will result in a cull of this method.
    if ( m_doingLayout )
        return;

    // This can be fine-tuned, but right now, simply everything will be re-layouted.
    switch ( type ) {
    case KoShape::PositionChanged:
    case KoShape::SizeChanged:
        scheduleRelayout();
    // FIXME: There's some cases that would require relayouting but that don't make sense
    // for chart items, e.g. ShearChanged. How should these changes be avoided or handled?
    default:
        break;
    }
}

void Layout::scheduleRelayout()
{
    m_relayoutScheduled = true;
}

void Layout::layout()
{
    Q_ASSERT( !m_doingLayout );

    if ( !m_relayoutScheduled )
        return;

    m_doingLayout = true;

    QMap<int, KoShape*> top, bottom, start, end;
    KoShape *topStart    = 0,
            *bottomStart = 0,
            *topEnd      = 0,
            *bottomEnd   = 0,
            *p      = 0;

    QMapIterator<KoShape*, LayoutData*> it( m_layoutItems );
    while ( it.hasNext() ) {
        it.next();
        KoShape *shape = it.key();
        if ( !shape->isVisible() )
            continue;
        LayoutData *data = it.value();
        switch ( data->pos ) {
        case TopPosition:
            top.insert( data->weight, shape );
            break;
        case BottomPosition:
            bottom.insert( data->weight, shape );
            break;
        case StartPosition:
            start.insert( data->weight, shape );
            break;
        case EndPosition:
            end.insert( data->weight, shape );
            break;
        case TopStartPosition:
            topStart = shape;
            break;
        case BottomStartPosition:
            bottomStart = shape;
            break;
        case TopEndPosition:
            topEnd = shape;
            break;
        case BottomEndPosition:
            bottomEnd = shape;
            break;
        case CenterPosition:
            p = shape;
            break;
        case FloatingPosition:
            // Nothing to do
            break;
        }
    }

    qreal topY = layoutTop( top );
    qreal bottomY = layoutBottom( bottom );
    qreal startX = layoutStart( start );
    qreal endX = layoutEnd( end );
    if ( p ) {
        setItemPosition( p, QPointF( startX, topY ) );
        p->setSize( QSizeF( endX - startX, bottomY - topY ) );
    }

    layoutTopStart( topStart );
    layoutBottomStart( bottomStart );
    layoutTopEnd( topEnd );
    layoutBottomEnd( bottomEnd );

    m_doingLayout = false;
    m_relayoutScheduled = false;
}



/// Private Methods



qreal Layout::layoutTop( const QMap<int, KoShape*>& shapes )
{
    qreal top = 0.0;
    qreal pX = m_containerSize.width() / 2.0;
    foreach( KoShape *shape, shapes ) {
        QSizeF size = itemSize( shape );
        setItemPosition( shape, QPointF( pX - size.width() / 2.0, top ) );
        top += size.height();
    }
    return top;
}

qreal Layout::layoutBottom( const QMap<int, KoShape*>& shapes )
{
    qreal bottom = m_containerSize.height();
    qreal pX = m_containerSize.width() / 2.0;
    foreach( KoShape *shape, shapes ) {
        QSizeF size = itemSize( shape );
        bottom -= size.height();
        setItemPosition( shape, QPointF( pX - size.width() / 2.0, bottom ) );
    }
    return bottom;
}

qreal Layout::layoutStart( const QMap<int, KoShape*>& shapes )
{
    qreal start = 0.0;
    qreal pY = m_containerSize.height() / 2.0;
    foreach( KoShape *shape, shapes ) {
        QSizeF size = itemSize( shape );
        setItemPosition( shape, QPointF( start, pY - size.height() / 2.0 ) );
        start += size.width();
    }
    return start;
}

qreal Layout::layoutEnd( const QMap<int, KoShape*>& shapes )
{
    qreal end = m_containerSize.width();
    qreal pY = m_containerSize.height() / 2.0;
    foreach( KoShape *shape, shapes ) {
        QSizeF size = itemSize( shape );
        end -= size.width();
        setItemPosition( shape, QPointF( end, pY - size.height() / 2.0 ) );
    }
    return end;
}

void Layout::layoutTopStart( KoShape *shape )
{
    if ( !shape )
        return;
    setItemPosition( shape, QPointF( 0, 0 ) );
}

void Layout::layoutBottomStart( KoShape *shape )
{
    if ( !shape )
        return;
    setItemPosition( shape, QPointF( 0, m_containerSize.height() - itemSize( shape ).height() ) );
}

void Layout::layoutTopEnd( KoShape *shape )
{
    if ( !shape )
        return;
    setItemPosition( shape, QPointF( m_containerSize.width() - itemSize( shape ).width(), 0 ) );
}

void Layout::layoutBottomEnd( KoShape *shape )
{
    if ( !shape )
        return;
    const QSizeF size = itemSize( shape );
    setItemPosition( shape, QPointF( m_containerSize.width()  - size.width(),
                                     m_containerSize.height() - size.height() ) );
}


/// Static Helper Methods

static QPointF itemPosition( KoShape *shape )
{
    const QRectF boundingRect = QRectF( QPointF( 0, 0 ), shape->size() );
    return shape->transformation().mapRect( boundingRect ).topLeft();
}

static QSizeF itemSize( KoShape *shape )
{
    const QRectF boundingRect = QRectF( QPointF( 0, 0 ), shape->size() );
    return shape->transformation().mapRect( boundingRect ).size();
}

static void setItemPosition( KoShape *shape, const QPointF& pos )
{
    const QPointF offset =  shape->position() - itemPosition( shape );
    shape->setPosition( pos + offset );
}
