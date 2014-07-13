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

#include "KoDeletedRowColumnDataStore.h"

#include "KoDeletedRowData.h"
#include "KoDeletedColumnData.h"

KoDeletedRowColumnDataStore::KoDeletedRowColumnDataStore()
{
}

KoDeletedRowColumnDataStore::~KoDeletedRowColumnDataStore()
{
}

KoDeletedRowData *KoDeletedRowColumnDataStore::addDeletedRow(QTextTable *table, int rowNumber, int changeId)
{
    KoDeletedRowData *deletedRowData = new KoDeletedRowData(rowNumber);
    deletedRowDataMap.insert(changeId, deletedRowData);
    QVector<int> *tableChangeIds = tableChangeIdsMap.value(table, NULL);
    if (!tableChangeIds) {
        tableChangeIds = new QVector<int>();
        tableChangeIdsMap.insert(table, tableChangeIds);
    }
    tableChangeIds->push_back(changeId);
    return deletedRowData;    
}

KoDeletedColumnData *KoDeletedRowColumnDataStore::addDeletedColumn(QTextTable *table, int columnNumber, int changeId)
{
    KoDeletedColumnData *deletedColumnData = new KoDeletedColumnData(columnNumber);
    deletedColumnDataMap.insert(changeId, deletedColumnData);
    QVector<int> *tableChangeIds = tableChangeIdsMap.value(table, NULL);
    if (!tableChangeIds) {
        tableChangeIds = new QVector<int>();
        tableChangeIdsMap.insert(table, tableChangeIds);
    }
    tableChangeIds->push_back(changeId);
    return deletedColumnData;
}

const QVector<int> *KoDeletedRowColumnDataStore::deletedRowColumnChangeIds(QTextTable *table)
{
    return tableChangeIdsMap.value(table, NULL);
}

KoDeletedRowColumnDataStore::DeleteType KoDeletedRowColumnDataStore::deleteType(int changeId)
{
    KoDeletedRowColumnDataStore::DeleteType retValue;
    if (deletedRowDataMap.value(changeId, NULL)) {
        retValue = KoDeletedRowColumnDataStore::eDeletedRow;
    } else if(deletedColumnDataMap.value(changeId, NULL)) {
        retValue = KoDeletedRowColumnDataStore::eDeletedColumn;
    } else {
        retValue = KoDeletedRowColumnDataStore::eUnknownDeleteType;
    }
    
    return retValue;
}

KoDeletedRowData *KoDeletedRowColumnDataStore::deletedRowData(int changeId)
{
    return deletedRowDataMap.value(changeId, NULL);
}

KoDeletedColumnData *KoDeletedRowColumnDataStore::deletedColumnData(int changeId)
{
    return deletedColumnDataMap.value(changeId, NULL);
}

