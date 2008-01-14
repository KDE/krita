/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
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

#include "Table.h"

#include "DDEData.h"
#include "TableRow.h"
#include "TableColumn.h"
#include "TableShape.h"
#include "TableCell.h"

class Table::Private
{
public:
    
    QList<TableColumn*> columns;
    QList<TableRow*> rows;
    QList<DDEData*> ddeConnections;
    TableShape * tableShape;
    
};

Table::Table( QTextDocument * document, TableShape * tableShape )
    : QTextTable( document )
    , d (new Private() )
{
    d->tableShape = tableShape;
}


Table::~Table()
{
    delete d;
}

Table::Table(const Table & rhs)
    : QTextTable( rhs.document() )
    , d( new Private() )
{
}


void Table::addDDEConnection(DDEData * data)
{
    d->ddeConnections.append(data);
}

QList<DDEData*> Table::ddeConnections() const
{
    return d->ddeConnections;
}

QList<TableColumn*> Table::columns() const
{
    return d->columns;
}

TableColumn * Table::createColumn(int pos)
{
    QTextTable::insertColumns(pos - 1, 1);
    
    TableColumn * column = new TableColumn();
    if (pos > d->columns.size() || pos < 0) pos = d->columns.size();
    d->columns.insert(pos, column);
    if (d->rows.size() > 0) {
        foreach (TableRow * row, d->rows) {
            TableCell * cell = row->createCell(pos);
            KoShape * shape = cell->createShape("TextShapeID");
            shape->setParent(d->tableShape);
        }
    }
    
    return column;
}

void Table::removeColumn(int pos)
{
    if (pos > d->columns.size() || pos < 0) return;
    if (d->rows.size() > 0) {
        foreach(TableRow * row, d->rows) {
            row->removeCell(pos);
        }
    }
    // and qt takes care of the document
    QTextTable::removeColumns(pos - 1, 1);
}

QList<TableRow*> Table::rows() const
{
    return d->rows;
}

TableRow * Table::createRow(int pos)
{
    if (pos > d->rows.size() || pos < 0 ) pos = d->rows.size();
    
    QTextTable::insertRows(pos -1, 1);
    TableRow * row = new TableRow(d->columns.size());
    d->rows.insert(pos, row);

    return row;
}

void Table::removeRow(int pos)
{
    if (pos > d->rows.size() || pos < 0) return;
    TableRow * row = d->rows.takeAt(pos);
    delete row;
    QTextTable::removeRows(pos - 1, 1);
    
}

TableCell * Table::cellAt(int row, int col) const
{
    if (row < 0 || row >= d->rows.size()) return 0;
    if (col < 0 || col >= d->columns.size()) return 0;

    return d->rows[row]->cellAt(col);
}

#include "Table.moc"
