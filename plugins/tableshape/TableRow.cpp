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
#include "TableRow.h"

#include <QList>

#include "TableCell.h"

class TableRow::Private
{
public:

    Private()
        {
            softPageBreak = false;
            repeated = 1;
            visibility = Visible;
        }

    QList<TableCell*> cells;
    bool softPageBreak;
    int repeated;
    Visibility visibility;
};

TableRow::TableRow( int columns )
    : d( new Private() )
{
    for (int i = 0; i < columns; ++i) {
        TableCell * cell = new TableCell();
        d->cells.append(cell);
    }
}


TableRow::~TableRow()
{
    delete d;
}

TableRow::TableRow(const TableRow & rhs)
    : QObject()
    , d( new Private() )
{
    Q_UNUSED(rhs);
}

void TableRow::setSoftPageBreak(bool on)
{
    d->softPageBreak = on;
}

bool TableRow::softPageBreak() const
{
    return d->softPageBreak;
}

void TableRow::setRepeat(int repeat)
{
    d->repeated = repeat;
}

int TableRow::repeat() const
{
    return d->repeated;
}

void TableRow::setVisibility(const QString & visibility)
{
    if (visibility == "collapse") {
        d->visibility = Collapse;
    }
    else if (visibility == "filter") {
        d->visibility = Filter;
    }
    else {
        d->visibility = Visible;
    }
}

void TableRow::setVisibility(Visibility visibility)
{
    d->visibility = visibility;
}

TableRow::Visibility TableRow::visibility() const
{
    return d->visibility;
}

#include "TableRow.moc"
