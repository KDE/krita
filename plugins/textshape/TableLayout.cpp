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
#include <QTextLine>
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

    m_tableData->m_rowPositions.resize(m_table->rows());
    m_tableData->m_rowHeights.resize(m_table->rows());
    m_tableData->m_columnPositions.resize(m_table->columns());
    m_tableData->m_columnWidths.resize(m_table->columns());
    m_tableData->m_contentHeights.resize(m_table->rows());

    for (int row = 0; row < m_table->rows(); ++row) {
        m_tableData->m_contentHeights[row].resize(m_table->columns());
    }

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

    // Table width.
    if (tableFormat.width().type() == QTextLength::FixedLength) {
        m_tableData->m_width = tableFormat.width().rawValue();
    } else {
        m_tableData->m_width = 0; // TODO: Variable / percentage.
    }

    // Column widths/positions. Only explicit fixed width for now.
    qreal columnWidth = m_tableData->m_width / m_table->columns();
    m_tableData->m_columnWidths.fill(columnWidth);
    for (int col = 0; col < m_tableData->m_columnPositions.size(); ++col) {
        m_tableData->m_columnPositions[col] = col * columnWidth;
    }

    layoutFromRow(0);
    m_dirty = false;
}

void TableLayout::layoutFromRow(int fromRow)
{
    Q_ASSERT(isValid());
    Q_ASSERT(fromRow >= 0);
    Q_ASSERT(fromRow < m_table->rows());

    if (!isValid()) {
        return;
    }

    if (fromRow < 0 || fromRow >= m_table->rows()) {
        return;
    }

    m_tableData->m_height = m_tableData->m_rowPositions[fromRow];

    for (int row = fromRow; row < m_table->rows(); ++row) {
        /*
         * 1. find max content height in row.
         * 2. adjust row/table height.
         * 3. adjust row Y position.
         */
        qreal contentHeight = 0;
        foreach (qreal height, m_tableData->m_contentHeights.at(row)) {
            contentHeight = qMax(contentHeight, height);
        }

        m_tableData->m_rowHeights[row] = contentHeight;
        m_tableData->m_height += m_tableData->m_rowHeights[row];

        if (row > 0) { // never adjust Y position of first row.
            m_tableData->m_rowPositions[row] = m_tableData->m_rowPositions[row - 1] +
                m_tableData->m_rowHeights[row - 1];
        }
    }
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

    return QRectF(m_position.x(), m_position.y(),
            m_tableData->m_width + horizontalMargins,
            m_tableData->m_height + verticalMargins);
}

QRectF TableLayout::cellContentRect(const QTextTableCell &cell) const
{
    Q_ASSERT(cell.isValid());

    return cellContentRect(cell.row(), cell.column());
}

QRectF TableLayout::cellContentRect(int row, int column) const
{
    Q_ASSERT(isValid());
    Q_ASSERT(row < m_tableData->m_rowPositions.size());
    Q_ASSERT(column < m_tableData->m_columnPositions.size());

    return QRectF(
            m_tableData->m_columnPositions[column], m_tableData->m_rowPositions[row],
            m_tableData->m_columnWidths[column], m_tableData->m_rowHeights[row]);
}

void TableLayout::calculateCellContentHeight(const QTextTableCell &cell)
{
    Q_ASSERT(isValid());
    Q_ASSERT(cell.isValid());

    if (!isValid() || !cell.isValid()) {
        return;
    }

    // get the first line in the first block in the cell.
    QTextFrame::iterator cellIterator = cell.begin();
    Q_ASSERT(cellIterator.currentFrame() == 0);
    QTextBlock firstBlock = cellIterator.currentBlock();
    Q_ASSERT(firstBlock.isValid());
    QTextLine topLine = firstBlock.layout()->lineAt(0);
    Q_ASSERT(topLine.isValid());

    // get the last line in the last block in the cell.
    cellIterator = cell.end();
    cellIterator--;
    Q_ASSERT(cellIterator.currentFrame() == 0);
    QTextBlock lastBlock = cellIterator.currentBlock();
    Q_ASSERT(lastBlock.isValid());
    QTextLine bottomLine = lastBlock.layout()->lineAt(lastBlock.lineCount() - 1);
    Q_ASSERT(bottomLine.isValid());

    qreal contentHeight = (bottomLine.y() + bottomLine.height()) - topLine.y();
    Q_ASSERT(contentHeight >= 0); // sanity check.
    contentHeight = qMax(contentHeight, 0.);

    // update content height value of the cell.
    m_tableData->m_contentHeights[cell.row()][cell.column()] = contentHeight;

    // re-layout from the row the cell is in.
    layoutFromRow(cell.row());
}

void TableLayout::calculateColumns(int fromColumn)
{
    Q_ASSERT(isValid());
    Q_ASSERT(fromColumn >= 0);
    Q_ASSERT(fromColumn < m_table->columns());

    if (!isValid()) {
        return;
    }

    if (fromColumn < 0 || fromColumn >= m_table->columns()) {
        return;
    }

    // TODO.
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
