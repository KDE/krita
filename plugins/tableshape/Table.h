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

#include <QTextTable>

struct DDEData;

class QTextDocument;
class TableShape;
class TableColumn;
class TableRow;
class TableCell;

/**
 * A table can contain any number of rows. In a table, all cells in a columns
 * share the same left edge position. Cells can be merged with other cells,
 * which means that they overlap to the right and downwards empty cells.
 *
 * A table contains a single QTextDocument; all text and table shapes in the cells
 * share this QTextDocument.
 *
 * It is possible to place any shape in a table: this may be trick ODF-wise, though.
 *
 * It is possible for TableShapes to share the same Table instance, for instance,
 * for pagination.
 *
 * Only for spreadsheet, apparently:
 *
 *  - table-source (pivot tables)
 *  - table-scenario (8.3.3)
 *  - office-forms (chapter 11)
 *
 * XXX: What about this?
 *  - table-shapes (8.3.4: The <table:shapes> element contains all graphic shapes with
 *                         an anchor on the table this element is a child of. It is a container
 *                         element and does not have any associated attributes.)
 */
class Table : public QTextTable {

    Q_OBJECT

public:
    
    Table( QTextDocument * document, TableShape * shape );

    virtual ~Table();

    Table(const Table & rhs);

    /**
     * For roundtrip purposes, don't forget the DDE data.
     */
    void addDDEConnection(DDEData * data);

    /**
     * Return all dde connections defined for this table.
     */
     QList<DDEData*> ddeConnections() const;

   /**
    * Return a vector with all columns in this table. Columns do not contain cells: rows
    * contain cells. Columns contain default styles for cells.
    */
    QList<TableColumn*> columns() const;

    /**
    * Create a new column at the specified point. If the table is already populated,
    * create empty cells in all rows at the specified point.
    */
    TableColumn * createColumn(int pos);

    /**
    * Remove the column at the specified position. The cells in all rows in this
    * table at pos will be removed. This may possibly uncover covered cells.
    */
    void removeColumn(int pos);

    /**
    * return a vector with all rows in this table. Rows contain cells.
    */
    QList<TableRow*> rows() const;

    /**
    * Create a new row at the specified point. This will cause a relayout of all
    * rows below the new row (athough that might be optimized by just shifting
    * the position of all shapes in those rows).
    */
    TableRow * createRow(int pos);

    /**
    * Remove the row at the specified point. All cells and shapes in this row
    * will be removed. All cells merges cells in this row covered will be uncovered.
    * All rows below this row will shift upwards.
    */
    void removeRow(int pos);

    /**
    * Return the cell at the specified row, col position, or 0 if the position is
    * out of range.
    */
    TableCell * cellAt(int row, int col) const;

      
      
private:
    
    class Private;
    Private * const d;
    
};

#endif
