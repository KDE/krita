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
#ifndef KDGANTTGRAPHICSVIEW_P_H
#define KDGANTTGRAPHICSVIEW_P_H

#include "kdganttgraphicsview.h"
#include "kdganttgraphicsscene.h"
#include "kdganttdatetimegrid.h"

#include <QPointer>

class QEvent;
class QFocusEvent;

namespace KDGantt {
    class Slider;

    class HeaderWidget : public QWidget {
        Q_OBJECT
    public:
        explicit HeaderWidget( GraphicsView* parent );
        virtual ~HeaderWidget();

        GraphicsView* view() const { return qobject_cast<GraphicsView*>( parent() );}

        void render( QPainter* painter, const QRectF &targetRect, const QRectF &sourceRect, Qt::AspectRatioMode aspectRatioMode );
        
    public Q_SLOTS:
        void scrollTo( int );
    protected:
        /*reimp*/ bool event( QEvent* ev );
        /*reimp*/ void paintEvent( QPaintEvent* ev );
        /*reimp*/ void contextMenuEvent( QContextMenuEvent* ev );
        /*reimp*/ void mouseMoveEvent( QMouseEvent* ev );

    private:
        qreal m_offset;
        Slider *m_zoomwidget;
    };

    class GraphicsView::Private {
        Q_DISABLE_COPY( Private )
    public:
        explicit Private(GraphicsView* _q);

        void updateHeaderGeometry();

        void slotGridChanged();
        void slotHorizontalScrollValueChanged( int val );

        /* slots for QAbstractItemModel signals */
        void slotColumnsInserted( const QModelIndex& parent,  int start, int end );
        void slotColumnsRemoved( const QModelIndex& parent,  int start, int end );
        void slotDataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight );
        void slotLayoutChanged();
        void slotModelReset();
        void slotRowsInserted( const QModelIndex& parent,  int start, int end );
        void slotRowsAboutToBeRemoved( const QModelIndex& parent,  int start, int end );
        void slotRowsRemoved( const QModelIndex& parent,  int start, int end );

        void slotItemClicked( const QModelIndex& idx );
        void slotItemDoubleClicked( const QModelIndex& idx );


        GraphicsView* q;
        AbstractRowController* rowcontroller;
        HeaderWidget headerwidget;
        GraphicsScene scene;
    };
}

#endif /* KDGANTTGRAPHICSVIEW_P_H */

