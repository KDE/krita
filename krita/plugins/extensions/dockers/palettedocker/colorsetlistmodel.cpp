/*
 *  Copyright (c) 2013 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "colorsetlistmodel.h"

ColorSetListModel::ColorSetListModel(KoResourceServerAdapter<KoColorSet>* adapter, QObject* parent): QAbstractListModel(parent), m_serverAdapter(adapter)
{
}

QVariant ColorSetListModel::data(const QModelIndex& index, int role) const
{
    switch (role) {
        case Qt::DisplayRole: {
            int i = index.row();
            return m_serverAdapter->resources().at(i)->name();
        }
        break;
    }
    return QVariant();
}

int ColorSetListModel::rowCount(const QModelIndex& parent) const
{
    return m_serverAdapter->resources().count();
}

