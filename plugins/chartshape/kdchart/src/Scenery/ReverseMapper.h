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
