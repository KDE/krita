/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <mkruisselbrink@kde.org>
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
#include "PartsListModel.h"
#include "../core/Sheet.h"
#include "../core/Part.h"

using namespace MusicCore;

PartsListModel::PartsListModel(Sheet* sheet)
    : m_sheet(sheet)
{
    connect(m_sheet, SIGNAL(partAdded(int,MusicCore::Part*)), this, SLOT(partAdded(int,MusicCore::Part*)));
    connect(m_sheet, SIGNAL(partRemoved(int,MusicCore::Part*)), this, SLOT(partRemoved(int,MusicCore::Part*)));
}

int PartsListModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        return m_sheet->partCount();
    } else {
        return 0;
    }
}

QVariant PartsListModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole) {
        int row = index.row();
        if (row < 0 || row >= m_sheet->partCount()) return QString("invalid");
        return m_sheet->part(row)->name();
    }
    return QVariant();
}

void PartsListModel::partAdded(int index, Part* part)
{
    Q_UNUSED( part );
    
    beginInsertRows(QModelIndex(), index, index);
    endInsertRows();
}

void PartsListModel::partRemoved(int index, Part* part)
{
    Q_UNUSED( part );
    
    beginRemoveRows(QModelIndex(), index, index);
    endRemoveRows();
}
