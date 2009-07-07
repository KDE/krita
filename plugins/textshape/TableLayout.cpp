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

#include "TableLayout.h"
#include "TableData.h"

#include <QTextDocument>
#include <QTextTable>
#include <QPainter>
#include <QDebug>
#include <QRectF>
#include <QPointF>

TableLayout::TableLayout(QTextTable *table) : m_dirty(true)
{
    setTable(table);
    setPosition(QPointF(0, 0));
}

TableLayout::TableLayout() : m_table(0), m_tableData(0), m_dirty(true)
{
}

void TableLayout::setTable(QTextTable *table)
{
    Q_ASSERT(table);
    TableData *tableData;

    if (!m_tableDataMap.contains(table)) {
        // set up new table data.
        tableData = new TableData();
        m_tableDataMap.insert(table, tableData);
        connect(table, SIGNAL(destroyed(QObject *)), this, SLOT(tableDestroyed(QObject *)));
    } else {
        // table data already in map.
        tableData = m_tableDataMap.value(table);
    }

    m_table = table;
    m_tableData = tableData;
    m_dirty = true;
}

QTextTable *TableLayout::table() const
{
    return m_table;
}

// TODO: Lots and lots.
void TableLayout::layout()
{
    Q_ASSERT(isValid());

    if (!isValid()) {
        return;
    }

    QTextTableFormat tableFormat = m_table->format();

    m_tableData->m_rowPositions.resize(m_table->rows());
    m_tableData->m_rowHeights.resize(m_table->rows());
    m_tableData->m_columnPositions.resize(m_table->columns());
    m_tableData->m_columnWidths.resize(m_table->columns());

    // TODO: Table position. Find out where we are, or does this have to be passed in?

    // Table width. Only explicit fixed width for now.
    m_tableData->m_width = tableFormat.width().rawValue();

    // Column widths/positions. Only explicit fixed width for now.
    qreal columnWidth = m_tableData->m_width / m_table->columns();
    m_tableData->m_columnWidths.fill(columnWidth);
    for (int col = 0; col < m_tableData->m_columnPositions.size(); ++col) {
        m_tableData->m_columnPositions[col] = col * columnWidth; // TODO: + table x pos
    }

    // Table height. Only explicit fixed height for now.
    m_tableData->m_height = tableFormat.height().rawValue();

    // Row heights/positions. Only explicit fixed width for now.
    qreal rowHeight = m_tableData->m_height / m_table->rows();
    m_tableData->m_rowHeights.fill(rowHeight);
    for (int row = 0; row < m_tableData->m_rowPositions.size(); ++row) {
        m_tableData->m_rowPositions[row] = row * rowHeight; // TODO: + table y pos
    }

    m_dirty = false;
}

void TableLayout::draw(QPainter *painter) const
{
    Q_UNUSED(painter);
    // TODO.
}

QRectF TableLayout::boundingRect() const
{
    Q_ASSERT(isValid());

    qreal horizontalMargins = m_table->format().leftMargin() + m_table->format().rightMargin();
    qreal verticalMargins = m_table->format().topMargin() + m_table->format().bottomMargin();

    return QRectF(m_tableData->m_position.x(), m_tableData->m_position.y(), // x, y
            m_tableData->m_width + horizontalMargins, // width
            m_tableData->m_height + verticalMargins); // height
}

QRectF TableLayout::cellContentRect(const QTextTableCell &cell) const
{
    return cellContentRect(cell.row(), cell.column());
}

QRectF TableLayout::cellContentRect(int row, int column) const
{
    Q_ASSERT(isValid());
    Q_ASSERT(row < m_tableData->m_rowPositions.size());
    Q_ASSERT(column < m_tableData->m_columnPositions.size());

    // TODO: Take borders, padding et.c. into consideration.

    return QRectF(
            m_tableData->m_columnPositions[column], m_tableData->m_rowPositions[row], // x, y
            m_tableData->m_columnWidths[column], m_tableData->m_rowHeights[row]);     // width, height
}

QTextTableCell TableLayout::cellAt(int position) const
{
    Q_ASSERT(isValid());

    if (!isValid()) {
        return QTextTableCell();
    }

    return m_table->cellAt(position);
}

QPointF TableLayout::position() const
{
    return m_position;
}

void TableLayout::setPosition(QPointF position)
{
    m_position = position;
}

bool TableLayout::isDirty() const
{
    return m_dirty;
}

bool TableLayout::isValid() const
{
    return (m_table && m_tableData);
}

void TableLayout::tableDestroyed(QObject *object)
{
    QTextTable *table = static_cast<QTextTable *>(object);
    Q_ASSERT(table);

    if (m_tableDataMap.contains(table)) {
        delete m_tableDataMap.value(table);
        m_tableDataMap.remove(table);
    }

    if (m_table == table) {
        m_table = 0;
        m_tableData = 0;
    }
}

#include "TableLayout.moc"
