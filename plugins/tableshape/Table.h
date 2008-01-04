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
#ifndef TABLE_H
#define TABLE_H

#include <QObject>
#include <QTextTable>

/**
 * A table can contain any number of rows. In a table, all cells in a columns
 * share the same left edge position. Cells can be merged with other cells,
 * which means that they overlap to the right and downwards empty cells.
 *
 * A table contains a single QTextDocument; all text and table shapes in the cells
 * share this QTextDocument.
 *
 * It is possible to place any shape in a table: this may be trick ODF-wise, though.
 */
class Table : public QObject, public QTextTable {

public:
    
    Table();

    virtual ~Table();

    Table(const Table & rhs);
    
};

#endif
