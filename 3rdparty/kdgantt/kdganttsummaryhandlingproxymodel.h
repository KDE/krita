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
#ifndef KDGANTTSUMMARYHANDLINGPROXYMODEL_H
#define KDGANTTSUMMARYHANDLINGPROXYMODEL_H

#include "kdganttforwardingproxymodel.h"

namespace KDGantt {
    class KDGANTT_EXPORT SummaryHandlingProxyModel : public ForwardingProxyModel {
        Q_OBJECT
        KDGANTT_DECLARE_PRIVATE_BASE_POLYMORPHIC( SummaryHandlingProxyModel )
    public:
        explicit SummaryHandlingProxyModel( QObject* parent=0 );
        virtual ~SummaryHandlingProxyModel();

        /*reimp*/ void setSourceModel( QAbstractItemModel* model );

        /*reimp*/ QVariant data( const QModelIndex& proxyIndex, int role = Qt::DisplayRole) const;
        /*reimp*/ bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );

        /*reimp*/ Qt::ItemFlags flags( const QModelIndex& idx ) const;

    protected:
        /*reimp*/ void sourceModelReset();
        /*reimp*/ void sourceLayoutChanged();
        /*reimp*/ void sourceDataChanged( const QModelIndex& from, const QModelIndex& to );
        /*reimp*/ void sourceColumnsAboutToBeInserted( const QModelIndex& idx, int start, int end );
        /*reimp*/ void sourceColumnsAboutToBeRemoved( const QModelIndex& idx, int start, int end );
        /*reimp*/ void sourceRowsAboutToBeInserted( const QModelIndex& idx, int start, int end );
        /*reimp*/ void sourceRowsAboutToBeRemoved( const QModelIndex&, int start, int end );
    };
}

#endif /* KDGANTTSUMMARYHANDLINGPROXYMODEL_H */

