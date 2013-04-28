/****************************************************************************
 ** Copyright (C) 2001-2006 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Gantt library.
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
#ifndef KDGANTTCONSTRAINTGRAPHICSITEM_H
#define KDGANTTCONSTRAINTGRAPHICSITEM_H

#include <QGraphicsItem>

#include "kdganttconstraint.h"

namespace KDGantt {
    class GraphicsScene;

    class ConstraintGraphicsItem : public QGraphicsItem {
    public:
        enum { Type = UserType + 43 };

        explicit ConstraintGraphicsItem( const Constraint& c,
                                         QGraphicsItem* parent = 0, GraphicsScene* scene = 0 );
        virtual ~ConstraintGraphicsItem();

        /*reimp*/ int type() const;
        /*reimp (non virtual)*/GraphicsScene* scene() const;

        /*reimp*/ QString ganttToolTip() const;

        /*reimp*/ QRectF boundingRect() const;
        /*reimp*/ void paint( QPainter* painter, const QStyleOptionGraphicsItem* option,
                              QWidget* widget = 0 );

        inline const Constraint& constraint() const { return m_constraint; }
        Constraint proxyConstraint() const;

        void setStart( const QPointF& start );
        inline QPointF start() const { return m_start; }
        void setEnd( const QPointF& end );
        inline QPointF end() const { return m_end; }

        void updateItem( const QPointF& start,const QPointF& end );
    private:
        Constraint m_constraint;
        QPointF m_start;
        QPointF m_end;
    };
}

#endif /* KDGANTTCONSTRAINTGRAPHICSITEM_H */

