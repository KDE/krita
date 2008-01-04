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
#ifndef TABLEROW_H
#define TABLEROW_H

#include <QObject>

/**
 * A table row contains a row of cells. A row has a certain height,
 * which is the greatest hight of those cells in a row that are not merged
 * with cells below the current row.
 */
class TableRow : public QObject {

Q_OBJECT

public:
    TableRow();

    virtual ~TableRow();

    const TableRow(const TableRow & rhs);

signals:

    void rowHeightChanged(float height);

private:


};

#endif
