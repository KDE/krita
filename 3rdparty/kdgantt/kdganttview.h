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
#ifndef KDGANTTVIEW_H
#define KDGANTTVIEW_H

#include <QWidget>
#include "kdganttglobal.h"

class QAbstractItemModel;
class QAbstractProxyModel;
class QAbstractItemView;
class QModelIndex;
class QItemSelectionModel;

namespace KDGantt {
    class ItemDelegate;
    class Constraint;
    class ConstraintModel;
    class AbstractGrid;
    class GraphicsView;
    class AbstractRowController;
    class GraphicsItem;
    
    class KDGANTT_EXPORT View : public QWidget {
        Q_OBJECT
        KDGANTT_DECLARE_PRIVATE_BASE_POLYMORPHIC_QWIDGET(View)
        Q_PRIVATE_SLOT( d, void slotCollapsed(const QModelIndex&) )
        Q_PRIVATE_SLOT( d, void slotExpanded(const QModelIndex&) )
        Q_PRIVATE_SLOT( d, void slotVerticalScrollValueChanged( int ) )
        Q_PRIVATE_SLOT( d, void slotLeftWidgetVerticalRangeChanged( int, int ) )
        Q_PRIVATE_SLOT( d, void slotGfxViewVerticalRangeChanged( int, int ) )
    public:
        explicit View(QWidget* parent=0);
        virtual ~View();

        QAbstractItemModel* model() const;
        QItemSelectionModel* selectionModel() const;
        ItemDelegate* itemDelegate() const;
        ConstraintModel* constraintModel() const;
        AbstractGrid* grid() const;
        QModelIndex rootIndex() const;

        QModelIndex indexAt( const QPoint& pos ) const;

        void setLeftView( QAbstractItemView* );
        const QAbstractItemView* leftView() const;
        QAbstractItemView* leftView();

        void setRowController( AbstractRowController* );
        AbstractRowController* rowController();
        const AbstractRowController* rowController() const;

        const GraphicsView* graphicsView() const;
        GraphicsView* graphicsView();
        const QAbstractProxyModel* ganttProxyModel() const;
        QAbstractProxyModel* ganttProxyModel();

        void print( QPainter* painter, const QRectF& target = QRectF(), const QRectF& source = QRectF(), bool drawRowLabels=true, bool drawHeader=true);
        QRectF printRect( bool drawRowLabels=true, bool drawHeader=true);

    public Q_SLOTS:
        void setModel(QAbstractItemModel* model);
        void setRootIndex( const QModelIndex& idx );
        void setSelectionModel( QItemSelectionModel* smodel );
        void setItemDelegate( ItemDelegate* );
        void setConstraintModel( ConstraintModel* );
        void setGrid( AbstractGrid* );

    protected:
        /*reimp*/ void resizeEvent(QResizeEvent*);
    };
}

#endif /* KDGANTTVIEW_H */
