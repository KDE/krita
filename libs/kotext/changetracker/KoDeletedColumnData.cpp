/* This file is part of the KDE project
 * Copyright (C) 2011 Ganesh Paramasivam <ganesh@crystalfab.com>
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

#include "KoDeletedColumnData.h"
#include "KoDeletedCellData.h"

#include <styles/KoTableColumnStyle.h>

#include <QTextTable>

KoDeletedColumnData::KoDeletedColumnData(int columnNumber)
{
    this->column_number = columnNumber;
}

KoDeletedColumnData::~KoDeletedColumnData()
{
}

int KoDeletedColumnData::columnNumber()
{
    return column_number;
}

void KoDeletedColumnData::setColumnStyle(KoTableColumnStyle *columnStyle)
{
    this->column_style = columnStyle;
}

KoTableColumnStyle *KoDeletedColumnData::columnStyle()
{
    return column_style;
}

const QVector<KoDeletedCellData *>& KoDeletedColumnData::deletedCells()
{
    return deleted_cells;
}

void KoDeletedColumnData::storeDeletedCells(QTextTable *table)
{
}

