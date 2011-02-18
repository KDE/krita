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

#include <QTextTableCellFormat>
#include <QTextDocumentFragment>

#include "KoDeletedRowData.h"
#include "KoDeletedCellData.h"

#include <styles/KoTableRowStyle.h>

#include <QTextTable>

KoDeletedRowData::KoDeletedRowData(int rowNumber)
{
    this->row_number = rowNumber;
}

KoDeletedRowData::~KoDeletedRowData()
{
}

int KoDeletedRowData::rowNumber()
{
    return row_number;
}

void KoDeletedRowData::setRowStyle(KoTableRowStyle *rowStyle)
{
    this->row_style = rowStyle;
}

KoTableRowStyle *KoDeletedRowData::rowStyle()
{
    return row_style;
}

const QVector<KoDeletedCellData *>& KoDeletedRowData::deletedCells()
{
    return deleted_cells;
}

void KoDeletedRowData::storeDeletedCells(QTextTable *table)
{
}

