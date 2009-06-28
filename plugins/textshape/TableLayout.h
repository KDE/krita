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

class TableData;
class QTextDocument;
class QTextTable;
class QPainter;
class QRectF;

/**
 * @brief Table layout class.
 *
 * Given a QTextTable and an accompanying TableData object, this class is able to
 * lay out and draw given table. After layout, the table data object that was passed
 * in may be queried for cell content geometry, border positions et.c.
 *
 * A layout is triggered with the layout() function and repeated calls to this
 * function can be used to trigger a re-layout, if, for instance, the table
 * data object has been changed by the layout process.
 *
 * The table layout has a dirty state which indicates whether a layout is needed
 * This state may be queried using the isDirty() function.
 *
 * The table layout may be drawn using a QPainter with the draw() function.
 *
 * Instances of this class may be reused and the table and table data object can
 * be changed with setTable() and setTableData(), respectively.
 *
 * The class does not take ownership of the table or the table data that is passed
 * in and it is up to the user of this class to delete those objects appropriately.
 *
 * Note that this class does not lay out the table contents. It just lays out
 * the cell rectangles, border positions et.c. and provides an API to query these.
 * It is up to the user of this class to lay out the content into these rectangles,
 * as well as updating the table data and issuing re-layouts of the table if needed.
 *
 * \sa TableData, Layout
 */
class TableLayout
{
public:
    /**
     * @brief Constructor.
     *
     * Create a new table layout from the given table and table data.
     *
     * @param table a pointer to the table.
     * @param tableData a pointer to the table data.
     */
    TableLayout(QTextTable *table, TableData *tableData);

    /**
     * @brief Constructor.
     *
     * Create a new, empty table layout. Calls to layout() will immediately
     * return and do nothing on table layouts initialized using this constructor,
     * unless both a table and table data has been set first using setTable()
     * and setTableData() respectively.
     * 
     * \sa setTable(), setTableData()
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
     * Set the table data object to be used.
     *
     * It is the caller's responsibility to delete the table data object that
     * is passed in.
     *
     * @param a pointer to the table data object.
     *
     * \sa tableData(), TableData
     */
    void setTableData(TableData *tableData);

    /**
     * Get the current table data.
     *
     * @return a pointer to the current table data or 0 if none has been set.
     *
     * \sa setTableData()
     */
    TableData *tableData() const;

    /**
     * Trigger a layout of the table.
     */
    void layout();

    /**
     * Draw the table using the given QPainter.
     * @param painter a pointer to the QPainter to draw the table with.
     */
    void draw(QPainter *painter) const;

    /**
     * Get the bounding rectangle of the table.
     * @return the bounding rectangle of the table.
     */
    QRectF boundingRect() const;

    /**
     * Check the dirty state.
     * @return true if the table needs to be laid out, otherwise false.
     */
    bool isDirty() const;

private:
    QTextTable *m_table;     /**< The current table. */
    TableData *m_tableData;  /**< The current table data. */

    bool m_dirty;            /**< Table layout is dirty. */
};

#endif // TABLELAYOUT_H
