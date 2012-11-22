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
#ifndef KDGANTTGRAPHICSSCENE_P_H
#define KDGANTTGRAPHICSSCENE_P_H

#include <QPersistentModelIndex>
#include <QHash>
#include <QPointer>
#include <QItemSelectionModel>
#include <QAbstractProxyModel>

#include "kdganttgraphicsscene.h"
#include "kdganttconstraintmodel.h"
#include "kdganttdatetimegrid.h"

namespace KDGantt {
    class GraphicsScene::Private {
    public:
        explicit Private(GraphicsScene*);

        void resetConstraintItems();
        void createConstraintItem( const Constraint& c );
        void deleteConstraintItem( ConstraintGraphicsItem* citem );
        void deleteConstraintItem( const Constraint& c );
        ConstraintGraphicsItem* findConstraintItem( const Constraint& c ) const;

        GraphicsScene* q;

        QHash<QPersistentModelIndex,GraphicsItem*> items;
        GraphicsItem* dragSource;

        QPointer<ItemDelegate> itemDelegate;
        AbstractRowController* rowController;
        DateTimeGrid           default_grid;
        QPointer<AbstractGrid> grid;
        bool readOnly;

        QPointer<QAbstractProxyModel> summaryHandlingModel;

        QPointer<ConstraintModel> constraintModel;

        QPointer<QItemSelectionModel> selectionModel;
    };

    GraphicsScene::GraphicsScene( GraphicsScene::Private* d ) : _d( d )
    {
        init();
    }
}

#endif /* KDGANTTGRAPHICSSCENE_P_H */

