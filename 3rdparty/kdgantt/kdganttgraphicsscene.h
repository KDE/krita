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
#ifndef KDGANTTGRAPHICSSCENE_H
#define KDGANTTGRAPHICSSCENE_H

#include <QDateTime>
#include <QList>
#include <QGraphicsScene>
#include <QModelIndex>

#include "kdganttglobal.h"

class QAbstractProxyModel;
class QItemSelectionModel;
class QStyleOptionViewItem;

namespace KDGantt {
    class AbstractGrid;
    class AbstractRowController;
    class GraphicsItem;
    class Constraint;
    class ConstraintModel;
    class ConstraintGraphicsItem;
    class ItemDelegate;
    class GraphicsView;
    
    class GraphicsScene : public QGraphicsScene {
        Q_OBJECT
        KDGANTT_DECLARE_PRIVATE_BASE_POLYMORPHIC( GraphicsScene )
    public:
        explicit GraphicsScene( QObject* parent=0 );
        virtual ~GraphicsScene();

        //qreal dateTimeToSceneX( const QDateTime& dt ) const;
        //QDateTime sceneXtoDateTime( qreal x ) const;

        static QModelIndex mainIndex( const QModelIndex& idx );
        static QModelIndex dataIndex( const QModelIndex& idx );

        QAbstractItemModel* model() const;
        QAbstractProxyModel* summaryHandlingModel() const;
        QModelIndex rootIndex() const;
        ConstraintModel* constraintModel() const;
        QItemSelectionModel* selectionModel() const;

        void insertItem( const QPersistentModelIndex&, GraphicsItem* );
        void removeItem( const QModelIndex& );
        GraphicsItem* findItem( const QModelIndex& ) const;
        GraphicsItem* findItem( const QPersistentModelIndex& ) const;

        void updateItems();
        void clearItems();
        void deleteSubtree( const QModelIndex& );

        ConstraintGraphicsItem* findConstraintItem( const Constraint& ) const;
        QList<ConstraintGraphicsItem*> findConstraintItems( const QModelIndex& idx ) const;
        void clearConstraintItems();

        void setItemDelegate( ItemDelegate* );
        ItemDelegate* itemDelegate() const;

        void setRowController( AbstractRowController* rc );
        AbstractRowController* rowController() const;

        void setGrid( AbstractGrid* grid );
        AbstractGrid* grid() const;

        bool isReadOnly() const;

        void updateRow( const QModelIndex& idx );
        GraphicsItem* createItem( ItemType type ) const;

        /* used by GraphicsItem */
        void itemEntered( const QModelIndex& );
        void itemPressed( const QModelIndex& );
        void itemClicked( const QModelIndex& );
        void itemDoubleClicked( const QModelIndex& );
        void setDragSource( GraphicsItem* item );
        GraphicsItem* dragSource() const;

        /* Printing */
        void print( QPainter* painter, const QRectF& target = QRectF(), const QRectF& source = QRectF(), bool drawRowLabels=true, GraphicsView *view=0 );
        QRectF printRect(bool drawRowLabels, GraphicsView *view );
        
    Q_SIGNALS:
        void gridChanged();

        void clicked( const QModelIndex & index );
        void doubleClicked( const QModelIndex & index );
        void entered( const QModelIndex & index );
        void pressed( const QModelIndex & index );

    protected:
        /*reimp*/ void helpEvent( QGraphicsSceneHelpEvent *helpEvent );
        /*reimp*/ void drawBackground( QPainter* painter, const QRectF& rect );

        void drawTreeIndication( QPainter *painter, const QModelIndex &idx, const QRect &rect, int indent, bool drawRoot=false );
        int level( const QModelIndex &idx ) const;

    public Q_SLOTS:
        void setModel( QAbstractItemModel* );
        void setSummaryHandlingModel( QAbstractProxyModel* );
        void setConstraintModel( ConstraintModel* );
        void setRootIndex( const QModelIndex& idx );
        void setSelectionModel( QItemSelectionModel* selectionmodel );
        void setReadOnly( bool );

    private Q_SLOTS:
        /* slots for ConstraintModel */
        void slotConstraintAdded( const Constraint& );
        void slotConstraintRemoved( const Constraint& );
        void slotGridChanged();
    };
}

#endif /* KDGANTTGRAPHICSSCENE_H */
