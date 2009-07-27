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

#include "KoTextDocumentLayout.h"

#include <QObject>
#include <QPointF>
#include <QMap>
#include <QTextFormat>

class TableLayoutData;

class QTextTable;
class QTextTableCell;
class QPainter;
class QRectF;

/*
 * Anatomy of a Table
 * ------------------
 *
 *   +-----1. Table Position
 *   |
 *   v
 *   X-------------------------2. Table Bounding Rect--------------------------+
 *   |                                                                         |
 *   |   +--3. Cell Bounding Rect---------+--------------------------------+   |
 *   |   |                                |                                |   |
 *   |   |   +--4. Cell Content Rect--+   |   +------------------------+   |   |
 *   |   |   |                        |   |   |                        |   |   |
 *   |   |   |                        |   |   |                        |   |   |
 *   |   |   |                        |   |   |                        |   |   |
 *   |   |   |          Cell 1        |   |   |          Cell 2        |   |   |
 *   |   |   |                        |   |   |                        |   |   |
 *   |   |   |                        |   |   |                        |   |   |
 *   |   |   |                        |   |   |                        |   |   |
 *   |   |   +------------------------+   |   +------------------------+   |   |
 *   |   |        5. Cell Padding         |                                |   |
 *   |   +--------------------------------+--------------------------------+   |
 *   |   |                                |                                |   |
 *   |   |   +------------------------+   |   +------------------------+   |   |
 *   |   |   |                        |   |   |                        |   |   |
 *   |   |   |                        |   |   |                        |   |   |
 *   |   |   |                        |   |   |                        |   |   |
 *   |   |   |          Cell 3        |   |   |          Cell 4        |   |   |
 *   |   |   |                        |   |   |                        |   |   |
 *   |   |   |                        |   |   |                        |   |   |
 *   |   |   |                        |   |   |                        |   |   |
 *   |   |   +------------------------+   |   +------------------------+   |   |
 *   |   |                                |                                |   |
 *   |   +--------------------------------+--------------------------------+   |
 *   |                            6. Table Margin                              |
 *   +-------------------------------------------------------------------------+
 *
 *  #    API                                       Relative To
 * --------------------------------------------------------------------
 * (1)   TableLayout::position()/setPosition()     Document
 * (2)   TableLayout::boundingRect()               Table Position (1)
 * (3)   TableLayout::cellBoundingRect()           Table Position (1)
 * (4)   TableLayout::cellContentRect()            Table Position (1)
 * (5)   QTextTableCellFormat::leftPadding()       (N/A)
 *       QTextTableCellFormat::rightPadding()      (N/A)
 *       QTextTableCellFormat::topPadding()        (N/A)
 *       QTextTableCellFormat::bottomPadding()     (N/A)
 * (6)   QTextTableFormat::leftMargin()            (N/A)
 *       QTextTableFormat::rightMargin()           (N/A)
 *       QTextTableFormat::topMargin()             (N/A)
 *       QTextTableFormat::bottomMargin()          (N/A)
 * --------------------------------------------------------------------
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
 * \sa TableLayoutData, Layout
 */
class TableLayout : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor.
     *
     * Create a new table layout and set it up with the given parent layout
     * and table.
     *
     * @param parentLayout the parent layout.
     * @param table a pointer to the table.
     */
    TableLayout(KoTextDocumentLayout::LayoutState *parentLayout, QTextTable *table);

    /**
     * @brief Constructor.
     *
     * Create a new, empty table layout.
     *
     * The table layout will be in an invalid state until a parent layout and
     * table has been set using setTable() and setParentLayout(). Calls to
     * layout() will return immediately until this has been done.
     * 
     * \sa setTable(), setParentLayout()
     *
     * @param parentLayout a pointer to the parent layout.
     */
    TableLayout();

    /**
     * Set the parent layout.
     * 
     * It is the caller's responsibility to delete the table object that is
     * passed in.
     *
     * @param parentLayout a pointer the parentLayout.
     *
     * \sa parentLayout()
     */
    void setParentLayout(KoTextDocumentLayout::LayoutState *parentLayout);

    /**
     * Get the current parent layout.
     *
     * @return a pointer to the parent layout.
     *
     * \sa setTable()
     */
    KoTextDocumentLayout::LayoutState *parentLayout() const;

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
     * Get the list of rectangles that the table occupies.
     *
     * The table should have one rectangle for each shape it is in after
     * layout.
     *
     * @return the list of rectangles.
     */
    QList<QRectF> tableRects() const;

    /**
     * Append a rectangle to the list of rectangles for the table.
     *
     * @param tableRect the rectangle to append.
     */
    void appendTableRect(QRectF tableRect);

    /**
     * Clear the list of rectangles for the table.
     */
    void clearTableRects();

    /**
     * Get the bounding rectangle of a given cell.
     *
     * The rectangle returned is relative to the table layout position.
     *
     * @param cell the cell.
     * @return the bounding rectangle of the cell.
     */
    QRectF cellBoundingRect(const QTextTableCell &cell) const;

    /**
     * Get the content rectangle of a given cell.
     *
     * The rectangle returned is relative to the table layout position.
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
     * Get the width of the current table.
     *
     * @return the width of the current table.
     *
     * \sa height()
     */
    qreal width() const;

    /**
     * Get the height of the current table.
     *
     * @return the height of the current table.
     *
     * \sa width()
     */
    qreal height() const;

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
     * the internal table layout data map cache.
     *
     * @param object pointer to the object that was destroyed.
     */
    void tableDestroyed(QObject *object);

private:
    friend class TestTableLayout; // To allow direct testing.

    KoTextDocumentLayout::LayoutState *m_parentLayout; /**< Parent layout. */

    QTextTable *m_table;                /**< The current table. */
    TableLayoutData *m_tableLayoutData; /**< The current table layout data. */

    QMap<QTextTable *, TableLayoutData *> m_tableLayoutDataMap;  /**< The map of table layout data objects. */

    bool m_dirty;        /**< Table layout is dirty. */
};

#endif // TABLELAYOUT_H
