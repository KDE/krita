/* -*- Mode: C++ -*-
   KDChart - a multi-platform charting engine
   */

/****************************************************************************
 ** Copyright (C) 2005-2007 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Chart library.
 **
 ** This file may be used under the terms of the GNU General Public
 ** License versions 2.0 or 3.0 as published by the Free Software
 ** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
 ** included in the packaging of this file.  Alternatively you may (at
 ** your option) use any later version of the GNU General Public
 ** License if such license has been publicly approved by
 ** Klarälvdalens Datakonsult AB (or its successors, if any).
 ** 
 ** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
 ** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE. Klarälvdalens Datakonsult AB reserves all rights
 ** not expressly granted herein.
 ** 
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 **********************************************************************/

#include "ReverseMapper.h"

#include <math.h>

#include <QRect>
#include <QtDebug>
#include <QPolygonF>
#include <QPainterPath>
#include <QGraphicsScene>

#include "../KDChartAbstractDiagram.h"
#include "ChartGraphicsItem.h"

using namespace KDChart;

ReverseMapper::ReverseMapper()
    : m_scene( 0 )
    , m_diagram( 0 )
{
}

ReverseMapper::ReverseMapper( AbstractDiagram* diagram )
    : m_scene( 0 )
    , m_diagram( diagram )
{
}

ReverseMapper::~ReverseMapper()
{
    delete m_scene; m_scene = 0;
}

void ReverseMapper::setDiagram( AbstractDiagram* diagram )
{

    m_diagram = diagram;
}

void ReverseMapper::clear()
{
    delete m_scene;
    m_scene = new QGraphicsScene();
}

QModelIndexList ReverseMapper::indexesIn( const QRect& rect ) const
{
    Q_ASSERT( m_diagram );
    if ( m_scene && m_scene->sceneRect().intersects( rect ) ) {
        QList<QGraphicsItem *> items = m_scene->items( rect );
        QModelIndexList indexes;
        Q_FOREACH( QGraphicsItem* item, items ) {
            ChartGraphicsItem* i = qgraphicsitem_cast<ChartGraphicsItem*>( item );
            if ( i ) {
                QModelIndex index ( m_diagram->model()->index( i->row(), i->column(), m_diagram->rootIndex() ) );
                indexes << index;
            }
        }
        return indexes;
    } else {
        return QModelIndexList();
    }
}

QModelIndexList ReverseMapper::indexesAt( const QPointF& point ) const
{
    Q_ASSERT( m_diagram );
    if ( m_scene && m_scene->sceneRect().contains( point ) ) {
        QList<QGraphicsItem *> items = m_scene->items( point );
        QModelIndexList indexes;
        Q_FOREACH( QGraphicsItem* item, items ) {
            ChartGraphicsItem* i = qgraphicsitem_cast<ChartGraphicsItem*>( item );
            if ( i ) {
                QModelIndex index ( m_diagram->model()->index( i->row(), i->column(), m_diagram->rootIndex() ) );
                if( !indexes.contains(index) )
                    indexes << index;
            }
        }
        return indexes;
    } else {
        return QModelIndexList();
    }
}

QPolygonF ReverseMapper::polygon( int row, int column ) const
{
    const QModelIndex index = m_diagram->model()->index( row, column, m_diagram->rootIndex() );
    return m_itemMap.contains( index ) ? m_itemMap[ index ]->polygon() : QPolygon();
}

QRectF ReverseMapper::boundingRect( int row, int column ) const
{
    const QModelIndex index = m_diagram->model()->index( row, column, m_diagram->rootIndex() );
    return m_itemMap.contains( index ) ? m_itemMap[ index ]->polygon().boundingRect() : QRectF();
}

void ReverseMapper::addItem( ChartGraphicsItem* item )
{
    Q_ASSERT( m_scene );
    m_scene->addItem( item );
    m_itemMap.insert( m_diagram->model()->index( item->row(), item->column(), m_diagram->rootIndex() ), item );
}

void ReverseMapper::addRect( int row, int column, const QRectF& rect )
{
    addPolygon( row, column, QPolygonF( rect ) );
}

void ReverseMapper::addPolygon( int row, int column, const QPolygonF& polygon )
{
    ChartGraphicsItem* item = new ChartGraphicsItem( row, column );
    item->setPolygon( polygon );
    addItem( item );
}

void ReverseMapper::addCircle( int row, int column, const QPointF& location, const QSizeF& diameter )
{
    QPainterPath path;
    QPointF ossfet( -0.5*diameter.width(), -0.5*diameter.height() );
    path.addEllipse( QRectF( location + ossfet, diameter ) );
    addPolygon( row, column, QPolygonF( path.toFillPolygon() ) );
}

void ReverseMapper::addLine( int row, int column, const QPointF& from, const QPointF& to )
{
    // that's no line, dude... make a small circle around that point, instead
    if( from == to )
    {
        addCircle( row, column, from, QSizeF( 1.5, 1.5 ) );
        return;
    }
    // lines do not make good polygons to click on. we calculate a 2
    // pixel wide rectangle, where the original line is excatly
    // centered in.
    // make a 3 pixel wide polygon from the line:
    static const QPointF pixel( 1.0, 1.0 );
    QPointF left, right;
    if ( from.x() < to.x() ) {
        left = from;
        right = to;
    } else {
        right = from;
        left = to;
    }
    const QPointF lineVector( right - left );
    const qreal lineVectorLength = sqrt( lineVector.x() * lineVector.x() + lineVector.y() * lineVector.y() );
    const QPointF lineVectorUnit( lineVector / lineVectorLength );
    const QPointF normOfLineVectorUnit( -lineVectorUnit.y(), lineVectorUnit.x() );
    // now the four polygon end points:
    const QPointF one( left - lineVectorUnit + normOfLineVectorUnit );
    const QPointF two( left - lineVectorUnit - normOfLineVectorUnit );
    const QPointF three( right + lineVectorUnit - normOfLineVectorUnit );
    const QPointF four( right + lineVectorUnit + normOfLineVectorUnit );
    addPolygon( row, column, QPolygonF() << one << two << three << four );
}
