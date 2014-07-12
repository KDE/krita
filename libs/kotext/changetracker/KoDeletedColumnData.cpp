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

#include <QTextCursor>
#include <QTextTable>

#include "KoDeletedCellData.h"

#include <styles/KoTableColumnStyle.h>

KoDeletedColumnData::KoDeletedColumnData(int columnNumber)
{
    this->column_number = columnNumber;
}

KoDeletedColumnData::~KoDeletedColumnData()
{
    KoDeletedCellData *cellData;
    foreach (cellData, deleted_cells) {
        delete cellData;
    }
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
    QTextCursor cursor(table->document());
    int rows = table->rows();
    
    for (int i=0; i < rows; i++) {
        KoDeletedCellData *cellData = new KoDeletedCellData(i, column_number);
        QTextTableCell cell = table->cellAt(i, column_number);
        cursor.setPosition(cell.firstCursorPosition().position());
        cursor.setPosition(cell.lastCursorPosition().position(), QTextCursor::KeepAnchor);
        cellData->setCellFormat(cell.format().toTableCellFormat());
        cellData->setCellContent(cursor.selection());
        deleted_cells.push_back(cellData);
    }
}

