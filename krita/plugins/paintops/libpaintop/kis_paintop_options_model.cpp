/* This file is part of the KDE project
 * Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_paintop_options_model.h"

#include <kcategorizedsortfilterproxymodel.h>
#include "kis_paintop_option.h"

KisPaintOpOptionsModel::KisPaintOpOptionsModel()
{
}

KisPaintOpOptionsModel::~KisPaintOpOptionsModel()
{
}

int KisPaintOpOptionsModel::rowCount(const QModelIndex & /*parent*/ ) const
{
    return m_list.count();
}

bool KisPaintOpOptionsModel::setData(const QModelIndex &index, const QVariant &value, int role )
{
    if (index.isValid())
    {
        switch (role) {
        case Qt::CheckStateRole: {
           if(m_list[index.row()]->isCheckable()) {
                m_list[index.row()]->setChecked( value.toInt() == Qt::Checked);
                return true;
            }
            break;
        }
        }
    }
    return false;
}

QVariant KisPaintOpOptionsModel::data(const QModelIndex & index, int role ) const
{
    if (index.isValid()) {
        switch (role) {
        case Qt::DisplayRole: {
            return m_list[index.row()]->label();
        }
        case Qt::CheckStateRole: {
           if(m_list[index.row()]->isCheckable()) {
                return m_list[index.row()]->isChecked() ? Qt::Checked : Qt::Unchecked;
            }
            break;
        }
        case SortingRole:
            return index.row();
        case WidgetIndexRole:
            return m_widgetIndex[index.row()];
        case KCategorizedSortFilterProxyModel::CategoryDisplayRole:
        case KCategorizedSortFilterProxyModel::CategorySortRole:
            return m_list[index.row()]->category();
        }
    }
    return QVariant();
}

Qt::ItemFlags KisPaintOpOptionsModel::flags(const QModelIndex & index) const
{
    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if(m_list[index.row()]->isCheckable())
    {
        flags |= Qt::ItemIsUserCheckable;
    }
    return flags;
}

void KisPaintOpOptionsModel::addPaintOpOption(KisPaintOpOption * option, int widgetIndex)
{
    beginResetModel();
    m_list.append(option);
    m_widgetIndex.append(widgetIndex);
    endResetModel();
}
