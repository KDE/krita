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
#ifndef KDGANTTPROXYMODEL_H
#define KDGANTTPROXYMODEL_H

#include "kdganttforwardingproxymodel.h"

namespace KDGantt {
    class KDGANTT_EXPORT ProxyModel : public ForwardingProxyModel {
        Q_OBJECT
        Q_DISABLE_COPY(ProxyModel)
        KDGANTT_DECLARE_PRIVATE_BASE_POLYMORPHIC( ProxyModel )
    public:
        explicit ProxyModel( QObject* parent=0 );
        virtual ~ProxyModel();

        void setColumn( int ganttrole, int col );
        void setRole( int ganttrole, int role );

        int column( int ganttrole ) const;
        int role( int ganttrole ) const;

#if 0
        void setCalendarMode( bool enable );
        bool calendarMode() const;
#endif

        /*reimp*/ QModelIndex mapFromSource( const QModelIndex& idx) const;
        /*reimp*/ QModelIndex mapToSource( const QModelIndex& proxyIdx ) const;

        /*reimp*/ int rowCount( const QModelIndex& idx ) const;
        /*reimp*/ int columnCount( const QModelIndex& idx ) const;

        /*reimp*/ QVariant data( const QModelIndex& idx, int role = Qt::DisplayRole ) const;
        /*reimp*/ bool setData( const QModelIndex& idx, const QVariant& value, int role=Qt::EditRole );
    };
}

#endif /* KDGANTTPROXYMODEL_H */

