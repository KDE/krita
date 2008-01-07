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

class Table::Private
{
public:
    
    QVector<TableColumn*> columns;
    QVector<TableRow*> rows;
    QVector<DDEData*> ddeConnections;
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

QVector<DDEData*> Table::ddeConnections() const
{
    return d->ddeConnections;
}

#include "Table.moc"
