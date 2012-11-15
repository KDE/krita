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
#ifndef KDGANTTSUMMARYHANDLINGPROXYMODEL_P_H
#define KDGANTTSUMMARYHANDLINGPROXYMODEL_P_H

#include "kdganttsummaryhandlingproxymodel.h"

#include <QDateTime>
#include <QHash>
#include <QPair>
#include <QPersistentModelIndex>

namespace KDGantt {
    class SummaryHandlingProxyModel::Private {
    public:
        bool cacheLookup( const QModelIndex& idx,
                          QPair<QDateTime,QDateTime>* result ) const;
        void insertInCache( const SummaryHandlingProxyModel* model, const QModelIndex& idx ) const;
        void removeFromCache( const QModelIndex& idx ) const;
        void clearCache() const;
		
		inline bool isSummary( const QModelIndex& idx ) const {
			int typ = idx.data( ItemTypeRole ).toInt();
			return (typ==TypeSummary) || (typ==TypeMulti);
		}

        mutable QHash<QModelIndex, QPair<QDateTime, QDateTime> > cached_summary_items;
    };
}

#endif /* KDGANTTSUMMARYHANDLINGPROXYMODEL_P_H */

