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

TableLayout::TableLayout(QTextTable *table, TableData *tableData) : m_dirty(true)
{
    setTable(table);
    setTableData(tableData);
}

TableLayout::TableLayout() : m_table(0), m_tableData(0), m_dirty(true)
{
}

void TableLayout::setTable(QTextTable *table)
{
    Q_ASSERT(table);
    m_table = table;

    m_dirty = true;
}

QTextTable *TableLayout::table() const
{
    return m_table;
}

void TableLayout::setTableData(TableData *tableData)
{
    Q_ASSERT(tableData);
    m_tableData = tableData;

    m_dirty = true;
}

TableData *TableLayout::tableData() const
{
    return m_tableData;
}


// TODO: Lots and lots.
void TableLayout::layout()
{
    Q_ASSERT(m_table);
    Q_ASSERT(m_tableData);

    // No table or table data set, return immediately.
    if (!m_table || !m_tableData) {
        return;
    }

    QTextTableFormat tableFormat = m_table->format();
    QTextFrame *parentFrame = m_table->parentFrame();
    Q_ASSERT(parentFrame);

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

bool TableLayout::isDirty() const
{
    return m_dirty;
}

