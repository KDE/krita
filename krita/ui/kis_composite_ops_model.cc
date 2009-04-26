/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_composite_ops_model.h"
#include <KoCompositeOp.h>

#include <kcategorizedsortfilterproxymodel.h>

KisCompositeOpsModel::KisCompositeOpsModel(const QList<KoCompositeOp*>& list)
{
    foreach(KoCompositeOp* op, list)
    {
        if(op->userVisible())
        {
            m_list.push_back(op);
        }
    }
}

KisCompositeOpsModel::~KisCompositeOpsModel()
{
}

int KisCompositeOpsModel::rowCount( const QModelIndex & parent ) const
{
    return m_list.count();
}

QVariant KisCompositeOpsModel::data( const QModelIndex & index, int role ) const
{
    if(index.isValid())
    {
        switch( role )
        {
            case Qt::DisplayRole:
                return m_list[index.row()]->description();
            case KCategorizedSortFilterProxyModel::CategoryDisplayRole:
            case KCategorizedSortFilterProxyModel::CategorySortRole:
                return m_list[index.row()]->category();
        }
    }
    return QVariant();
}

KoCompositeOp* KisCompositeOpsModel::itemAt(qint32 idx) const
{
    if (idx > m_list.count() ) return 0;
    return m_list[idx];
}

QModelIndex KisCompositeOpsModel::indexOf( const KoCompositeOp* op) const
{
    return indexOf(op->id());
}

QModelIndex KisCompositeOpsModel::indexOf( const QString& id) const
{
    int index = 0;
    foreach(KoCompositeOp * op2, m_list) {
        if (id == op2->id())
            break;
        ++index;
    }
    if(index < m_list.count())
    {
        return createIndex(index, 0);
    } else {
        return QModelIndex();
    }

}
