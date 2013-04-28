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
#ifndef KDGANTTFORWARDINGPROXYMODEL_H
#define KDGANTTFORWARDINGPROXYMODEL_H

#include <QAbstractProxyModel>

#include "kdganttglobal.h"

namespace KDGantt {
    class KDGANTT_EXPORT ForwardingProxyModel : public QAbstractProxyModel {
        Q_OBJECT
        Q_DISABLE_COPY(ForwardingProxyModel)
    public:
        explicit ForwardingProxyModel( QObject* parent=0 );
        virtual ~ForwardingProxyModel();

        /*reimp*/ QModelIndex mapFromSource ( const QModelIndex & sourceIndex ) const;
        /*reimp*/ QModelIndex mapToSource ( const QModelIndex & proxyIndex ) const;

        /*reimp*/ void setSourceModel( QAbstractItemModel* model );

        /*reimp*/ QModelIndex index( int row, int column, const QModelIndex& parent = QModelIndex() ) const;
        /*reimp*/ QModelIndex parent( const QModelIndex& idx ) const;

        /*reimp*/ int rowCount( const QModelIndex& idx = QModelIndex() ) const;
        /*reimp*/ int columnCount( const QModelIndex& idx = QModelIndex() ) const;

        /*reimp*/ bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );

    protected Q_SLOTS:
        virtual void sourceModelAboutToBeReset();
        virtual void sourceModelReset();
        virtual void sourceLayoutAboutToBeChanged();
        virtual void sourceLayoutChanged();
        virtual void sourceDataChanged( const QModelIndex& from, const QModelIndex& to );
        virtual void sourceColumnsAboutToBeInserted( const QModelIndex& idx, int start, int end );
        virtual void sourceColumnsInserted( const QModelIndex& idx, int start, int end );
        virtual void sourceColumnsAboutToBeRemoved( const QModelIndex& idx, int start, int end );
        virtual void sourceColumnsRemoved( const QModelIndex& idx, int start, int end );
        virtual void sourceRowsAboutToBeInserted( const QModelIndex& idx, int start, int end );
        virtual void sourceRowsInserted( const QModelIndex& idx, int start, int end );
        virtual void sourceRowsAboutToBeRemoved( const QModelIndex&, int start, int end );
        virtual void sourceRowsRemoved( const QModelIndex&, int start, int end );
    };
}

#endif /* KDGANTTFORWARDINGPROXYMODEL_H */

