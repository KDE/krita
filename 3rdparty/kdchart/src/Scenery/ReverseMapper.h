/****************************************************************************
** Copyright (C) 2001-2010 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Chart library.
**
** Licensees holding valid commercial KD Chart licenses may use this file in
** accordance with the KD Chart Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#ifndef REVERSEMAPPER_H
#define REVERSEMAPPER_H

#include <QModelIndex>
#include <QHash>

class QRectF;
class QGraphicsScene;
class QPolygonF;

namespace KDChart {

    class AbstractDiagram;
    class ChartGraphicsItem;

    /**
      * @brief The ReverseMapper stores information about objects on a chart and their respective model indexes
      * \internal
      */
    class ReverseMapper
    {

    public:
        ReverseMapper();
        explicit ReverseMapper( AbstractDiagram* diagram );

        ~ReverseMapper();

        void setDiagram( AbstractDiagram* diagram );

        void clear();

        QModelIndexList indexesAt( const QPointF& point ) const;
        QModelIndexList indexesIn( const QRect& rect ) const;

        QPolygonF polygon( int row, int column ) const;
        QRectF boundingRect( int row, int column ) const;

        void addItem( ChartGraphicsItem* item );

        // convenience methods:
        void addPolygon( int row, int column, const QPolygonF& polygon );
        void addRect( int row, int column, const QRectF& rect );
        void addCircle( int row, int column, const QPointF& location, const QSizeF& diameter );
        void addLine( int row, int column, const QPointF& from, const QPointF& to );

    private:
        QGraphicsScene* m_scene;
        AbstractDiagram* m_diagram;
        QHash<QModelIndex, ChartGraphicsItem*> m_itemMap;
    };

}

#endif
