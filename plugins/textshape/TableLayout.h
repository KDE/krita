/* This file is part of the KDE project
 * Copyright (C) 2009 Elvis Stansvik <elvstone@gmail.org>
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
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
class QPainterPath;
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
     * Create a new table layout and set it up with the given
     * table.
     *
     * @param table a pointer to the table.
     */
    TableLayout(QTextTable *table);

    /**
     * @brief Constructor.
     *
     * Create a new, empty table layout.
     *
     * The table layout will be in an invalid state until a table has
     * been set using setTable(). Calls to layout will return
     * immediately until this has been done.
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
     * Create a new tableRect starting at a specific row
     * All the following tableRects are removed as the are no longer valid
     * The caller should now layout from this row onward
     * @param position The position where layout can happen. Table margins will be subtracted
     * @param width The width available from the surrounding shape or table or what not
     * @param fromRow The first row in this rect
     */
    void startNewTableRect(QPointF position, qreal parentWidth, int fromRow);

    /**
     * Layout the row \row.
     * You need to have layed out the contents of this row before calling
     * This method only updates the row position and size
     *
     * @param row the row that is to be layed out.
     */
    void layoutRow(int row);

    /**
     * Draw the table background using the given QPainter.
     *
     * @param painter a pointer to the QPainter to draw the table with.
     */
    void drawBackground(QPainter *painter) const;

    /**
     * Draw the table borders using the given QPainter.
     *
     * @param painter a pointer to the QPainter to draw the table with.
     * @param accuBlankBorders a painterpath that should accumulate blank borders.
     */
    void drawBorders(QPainter *painter, QPainterPath *accuBlankBorders) const;

    /**
     * Figures outDraw the table borders using the given QPainter.
     *
     * @param point the point that needs to be within the returned cell
     *
     * @return the cell that is hit (can be invalid if no hit).
     */
    QTextTableCell hitTestTable(const QPointF &point) const;

    /**
     * Get the list of rectangles that the table occupies.
     * This is the the effective areas so table margins have been subtracted,
     * and they are placed according to alignment etc
     *
     * The table should have one rectangle for each shape it is in after
     * layout.
     *
     * @return the list of rectangles.
     */
    QList<QRectF> tableRects() const;

    /**
     * Get the bounding rectangle of a given cell.
     *
     * @param cell the cell.
     * @return the bounding rectangle of the cell.
     */
    QRectF cellBoundingRect(const QTextTableCell &cell) const;

    /**
     * Get the content rectangle of a given cell.
     *
     * @param cell the cell.
     * @return the rectangle of the cell.
     *
     * \sa position(), setPosition()
     */
    QRectF cellContentRect(const QTextTableCell &cell) const;

    /**
     * Get the bounding rectangle of a given row.
     *
     * @param row the row.
     * @return the bounding rectangle of the row.
     */
    QRectF rowBoundingRect(int row) const;

    void setCellContentHeight(const QTextTableCell &cell, qreal bottom);

    /**
     * Get the cell that contains the character at the given position in the document.

     * @param position the character position in the document.
     * @return the table cell.
     */
    QTextTableCell cellAt(int position) const;

    /**
     * Get the y position after the table.
     *
     * Adds the table bottom margin too
     * @return the y position usable for layout after the table.
     */
    qreal yAfterTable() const;

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
