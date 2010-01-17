/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_meta_data_model.h"
#include <kis_meta_data_store.h>
#include <kis_meta_data_entry.h>
#include <kis_meta_data_value.h>

KisMetaDataModel::KisMetaDataModel(KisMetaData::Store* store) : m_store(store)
{

}

int KisMetaDataModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_store->keys().count();
}

int KisMetaDataModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

QVariant KisMetaDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    Q_ASSERT(index.row() < m_store->keys().count());
    switch (role) {
    case Qt::DisplayRole: {
        switch (index.column()) {
        case 0:
            return m_store->keys()[index.row()];
        case 1:
            return m_store->entries()[index.row()].value().asVariant();
        }
    }
    default:
        return QVariant();
    }
}
