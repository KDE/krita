/* This file is part of the KDE project
 * Copyright (C) 2009 Elvis Stansvik <elvstone@gmail.org>
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

#ifndef TABLELAYOUT_H
#define TABLELAYOUT_H

#include <QObject>
#include <QPointF>
#include <QMap>

class TableData;
class QTextTable;
class QTextTableCell;
class QPainter;
class QRectF;

/*
 * Anatomy of a Table
 * ------------------
 *
 *   +--1. Table Bounding Rect----------------------------------------- -- -
 *   |
 *   |   +--2. Table Border Rect--------------------------------------- -- -
 *   |   |
 *   |   |   +--3. Cell Border Rect--------------+---+----------------- -- -
 *   |   |   |                                   |   |
 *   |   |   |   +--4. Cell Content Rect-----+   |   |   +------------- -- -
 *   |   |   |   |                           |   |   |   |
 *   |   |   |   |                           |   |   |   |
 *   |   |   |   |                           |   |   |   |
 *   |   |   |   |          Cell 1           |   |   |   |          Cell 2
 *   |   |   |   |                           |   |   |   |
 *   |   |   |   |                           |   |   |   |
 *   |   |   |   |                           |   |   |   |
 *   |   |   |   +---------------------------+   |   |   +------------- -- -
 *   |   |   |                                   |   |
 *   |   |   +-----------------------------------+   +----------------- -- -
 *   |   |   |                              5. Cell Spacing
 *   |   |   +-----------------------------------+   +----------------- -- -
 *   |   |   |                                   |   |
 *   |   |   |   +---------------------------+   |   |   +------------- -- -
 *   |   |   |   |                           |   |   |   |
 *   |   |   |   |                           |   |   |   |
 *   |   |   |   |                           |   |   |   |
 *   |   |   |   |          Cell 3           |   |   |   |          Cell 4
 *   |   |   |   |                           |   |   |   |
 *   |   |   |   |                           |   |   |   |
 *   |   |   |   |                           |   |   |   |
 *   |   |   |   +---------------------------+   |   |   +------------- -- -
 *   |   |   |   6. Cell Padding                 |   |
 *   |   |   +-----------------------------------+---+----------------- -- -
 *   |   |   7. Table Padding
 *   |   +------------------------------------------------------------- -- -
 *   |   8. Table Margin
 *   +----------------------------------------------------------------- -- -
 *
 *  #    API (Most still to be written)
 * -------------------------------------------------------------------------
 * (1)   TableLayout::boundingRect()
 * (2)   TableLayout::borderRect()
 * (3)   TableLayout::cellBorderRect(cell)
 * (4)   TableLayout::cellContentRect(cell)
 * (5)   QTextTableFormat::cellSpacing()
 * (6)   QTextTableCellFormat::left-/right-/top-/bottomPadding() [*]
 * (7)   QTextTableFormat::padding()
 * (8)   QTextTableFormat::margin()
 * -------------------------------------------------------------------------
 *
 * [*] Or QTextTableFormat::cellPadding() if not specified per cell.
 */

/**
 * @brief Table layout class.
 *
 * This class is responsible for laying out and drawing QTextTable instances.
 *
 * A full layout is triggered with the layout() function, or from a specific row
 * with the layoutFromRow() function.
 *
 * The table layout has a dirty state which indicates whether a layout is needed
 * This state may be queried using the isDirty() function.
 *
 * It also has a valid state which indicates if the layout is valid or not.
 * This state may be queried using the isValid() function.
 *
 * The table layout may be drawn using a given QPainter with the draw() function.
 *
 * Instances of this class may be reused and the current table can be set using
 * the setTable() function. The table layout caches the internal data objects
 * used for laying out tables.
 *
 * The table layout does not take ownership of the table and it is up to the user
 * of this class to delete the table appropriately.
 *
 * Note that this class does not lay out the content of the table. It just lays out
 * and draws the cell borders et.c. and provides an API to query, and to a certain
 * extent influence, the geometry of the table and its cells. It is up to the user
 * of this class to lay out the content into the cells of the table.
 *
 * \sa TableData, Layout
 */
class TableLayout : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor.
     *
     * Create a new table layout and set it up with the given table.
     *
     * @param table a pointer to the table.
     */
    TableLayout(QTextTable *table);

    /**
     * @brief Constructor.
     *
     * Create a new, empty table layout.
     *
     * The table layout will be in an invalid state until a table has been set
     * using setTable() and calls to layout() will return immediately.
     * 
     * \sa setTable()
     */
    TableLayout();

    /**
     * Set the table to be laid out.
     *
     * It is the caller's responsibility to delete the table object that is
     * passed in.
     *
     * @param table a pointer the table.
     *
     * \sa table()
     */
    void setTable(QTextTable *table);

    /**
     * Get the current table.
     *
     * @return a pointer to the current table or 0 if none has been set.
     *
     * \sa setTable()
     */
    QTextTable *table() const;

    /**
     * Trigger a full layout of the table.
     */
    void layout();

    /**
     * Trigger a layout of the table, starting at row \fromRow.
     *
     * @param from the row from which to start calculating.
     */
    void layoutFromRow(int fromRow);

    /**
     * Draw the table using the given QPainter.
     *
     * @param painter a pointer to the QPainter to draw the table with.
     */
    void draw(QPainter *painter) const;

    /**
     * Get the bounding rectangle of the table.
     *
     * @return the bounding rectangle of the table.
     */
    QRectF boundingRect() const;

    /**
     * Get the content rectangle of a given cell.
     *
     * The rectangle returned is relative to the table position.
     *
     * @param cell the cell.
     * @return the rectangle of the cell.
     *
     * \sa position(), setPosition()
     */
    QRectF cellContentRect(const QTextTableCell &cell) const;

    /**
     * Calculate the content height of a given cell.
     *
     * Call this when the content of the cell has changed.
     *
     * @note Make sure that all blocks in the cell have been laid out before calling
     * this function.
     *
     * @param cell the cell.
     */
    void calculateCellContentHeight(const QTextTableCell &cell);

    /**
     * Get the cell that contains the character at the given position in the document.

     * @param position the character position in the document.
     * @return the table cell.
     */
    QTextTableCell cellAt(int position) const;

    /**
     * Get the position of the table layout.
     *
     * @return the position of the table layout.
     *
     * \sa setPosition()
     */
    QPointF position() const;

    /**
     * Set the position of the table layout.
     *
     * @param position the position of the table layout.
     *
     * \sa position()
     */
    void setPosition(QPointF position);

    /**
     * Check the dirty state.
     * @return true if the table needs to be laid out, otherwise false.
     */
    bool isDirty() const;

    /**
     * Check if the table layout is valid.
     * @return true if the table layout is valid, otherwise false.
     */
    bool isValid() const;

private slots:
    /**
     * Table data map cleanup slot.
     *
     * This slot is called when a table is destroyed and takes care of cleaning up
     * the internal table data map cache.
     *
     * @param object pointer to the object that was destroyed.
     */
    void tableDestroyed(QObject *object);

private:
    /**
     * Get the content rectangle of a cell at a given table coordinate.
     *
     * The rectangle returned is relative to the table position.
     *
     * @param row the row of the cell.
     * @param column the column of the cell.
     * @return the rectangle of the cell.
     *
     * \sa position(), setPosition()
     */
    QRectF cellContentRect(int row, int column) const;

    /**
     * Calculate column widths and X positions, starting at the given column.
     *
     * This function calculates the column widths and X positions, starting at the
     * given column \a fromColumn.
     *
     * @param from the column from which to start calculating.
     */
    void calculateColumns(int fromColumn);

private:
    friend class TestTableLayout; // To allow direct testing.

    QTextTable *m_table;     /**< The current table. */
    TableData *m_tableData;  /**< The current table data. */

    QMap<QTextTable *, TableData *> m_tableDataMap;  /**< The map of table data objects. */

    QPointF m_position;  /**< The global position of the table layout. */
    bool m_dirty;        /**< Table layout is dirty. */
};

#endif // TABLELAYOUT_H
