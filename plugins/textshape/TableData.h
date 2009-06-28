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

#ifndef TABLEDATA_H
#define TABLEDATA_H

#include <QtGlobal>
#include <QVector>
#include <QPointF>

class QRectF;
class QTextTableCell;

/**
 * @brief Table data class.
 *
 * This class holds layout helper data for tables. It has a set of geometry
 * vectors that describes the table, along with some convenience functions for
 * getting the current table/cell geometries.
 */
class TableData
{
public:
    /// Constructor.
    TableData();

    /**
     * Get the content rectangle of a given cell.
     * @param cell the cell.
     * @return the rectangle of the cell.
     */
    QRectF cellContentRect(const QTextTableCell &cell) const;

    /**
     * Get the content rectangle of a cell at a given table coordinate.
     * @param row the row of the cell.
     * @param column the column of the cell.
     * @return the rectangle of the cell.
     */
    QRectF cellContentRect(int row, int column) const;

private:
    // To allow direct testing.
    friend class TestTableData;
    friend class TestTableLayout;

    // To allow direct manipulation during layout.
    friend class TableLayout;

    QVector<qreal> m_columnWidths;     /**< Column widths. */
    QVector<qreal> m_columnPositions;  /**< Column positions along X axis. */
    QVector<qreal> m_rowHeights;       /**< Row heights. */
    QVector<qreal> m_rowPositions;     /**< Row positions along Y axis. */

    QPointF m_position;  /**< Table position. */
    qreal m_width;       /**< Table width. */
    qreal m_height;      /**< Table height. */
};

#endif // TABLEDATA_H
