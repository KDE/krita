/* This file is part of the KDE project
 * Copyright (C) 2009 Elvis Stansvik <elvstone@gmail.org>
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
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
#include "TableLayoutData.h"

#include <KoTableStyle.h>
#include <KoTableCellStyle.h>
#include <KoTableColumnStyle.h>
#include <KoTableRowStyle.h>
#include <KoTableColumnAndRowStyleManager.h>
#include <KoTextDocumentLayout.h>
#include <KoShape.h>

#include <QTextDocument>
#include <QTextTable>
#include <QTextLine>
#include <QPainter>
#include <QDebug>
#include <QRectF>
#include <QPointF>

#include <kdebug.h>

TableLayout::TableLayout(KoTextDocumentLayout::LayoutState *parentLayout, QTextTable *table) : m_dirty(true)
{
    setParentLayout(parentLayout);
    setTable(table);
}

TableLayout::TableLayout() :
    m_parentLayout(0),
    m_table(0),
    m_tableLayoutData(0),
    m_dirty(true)
{
}

void TableLayout::setParentLayout(KoTextDocumentLayout::LayoutState *parentLayout)
{
    Q_ASSERT(parentLayout);

    m_parentLayout = parentLayout;
}

KoTextDocumentLayout::LayoutState *TableLayout::parentLayout() const
{
    return m_parentLayout;
}

void TableLayout::setTable(QTextTable *table)
{
    Q_ASSERT(table);
    TableLayoutData *tableLayoutData;

    if (!m_tableLayoutDataMap.contains(table)) {
        // Set up new table layout data.
        tableLayoutData = new TableLayoutData();
        m_tableLayoutDataMap.insert(table, tableLayoutData);
        connect(table, SIGNAL(destroyed(QObject *)), this, SLOT(tableDestroyed(QObject *)));
    } else {
        // Table layout data already in map.
        tableLayoutData = m_tableLayoutDataMap.value(table);
    }

    m_table = table;
    m_tableLayoutData = tableLayoutData;

    // Resize geometry vectors for the table.
    m_tableLayoutData->m_rowPositions.resize(m_table->rows());
    m_tableLayoutData->m_rowHeights.resize(m_table->rows());
    m_tableLayoutData->m_columnPositions.resize(m_table->columns());
    m_tableLayoutData->m_columnWidths.resize(m_table->columns());
    m_tableLayoutData->m_contentHeights.resize(m_table->rows());
    for (int row = 0; row < m_table->rows(); ++row) {
        m_tableLayoutData->m_contentHeights[row].resize(m_table->columns());
    }

    m_dirty = true;
}

QTextTable *TableLayout::table() const
{
    return m_table;
}

void TableLayout::layout()
{
    Q_ASSERT(isValid());

    if (!isValid()) {
        return;
    }

    QTextTableFormat tableFormat = m_table->format();

    // Table width.
    switch (tableFormat.width().type()) {
        case QTextLength::VariableLength: // Unused.
        case QTextLength::FixedLength:
            m_tableLayoutData->m_width = tableFormat.width().rawValue();
            break;
        case QTextLength::PercentageLength: {
            // FIXME: Nested tables?
            qreal parentWidth = m_parentLayout->shape->size().width();
            m_tableLayoutData->m_width = tableFormat.width().rawValue() * (parentWidth / 100)
                - tableFormat.leftMargin() - tableFormat.rightMargin();
            break;
        }
        default:
            kWarning(32600) << "Unknown table width type";
            m_tableLayoutData->m_width = 0;
            break;
    }

    // Get the column and row style manager.
    Q_ASSERT(tableFormat.hasProperty(KoTableStyle::ColumnAndRowStyleManager));
    KoTableColumnAndRowStyleManager *carsManager =
    reinterpret_cast<KoTableColumnAndRowStyleManager *>(
            tableFormat.property(KoTableStyle::ColumnAndRowStyleManager).value<void *>());
    Q_ASSERT(carsManager);

    // Column widths.
    qreal availableWidth = m_tableLayoutData->m_width; // Width available for columns.
    QList<int> relativeWidthColumns; // List of relative width columns.
    qreal relativeWidthSum; // Sum of relative column width values.
    for (int col = 0; col < m_tableLayoutData->m_columnPositions.size(); ++col) {
        KoTableColumnStyle *columnStyle = carsManager->columnStyle(col);
        Q_ASSERT(columnStyle); // Can there be no style?
        if (columnStyle->hasProperty(KoTableColumnStyle::RelativeColumnWidth)) {
            // Relative width specified. Will be handled in the next loop.
            relativeWidthColumns.append(col);
            relativeWidthSum += columnStyle->relativeColumnWidth();
        } else if (columnStyle->hasProperty(KoTableColumnStyle::ColumnWidth)) {
            // Only width specified, so use it.
            m_tableLayoutData->m_columnWidths[col] = columnStyle->columnWidth();
            availableWidth -= columnStyle->columnWidth();
        } else {
            // Neither width nor relative width specified. Can this happen?
            kWarning(32600) << "Neither column-width nor rel-column-width specified";
            m_tableLayoutData->m_columnWidths[col] = 0.0;
        }
    }
    // Relative column widths have now been summed up and can be distributed.
    foreach (qreal col, relativeWidthColumns) {
        KoTableColumnStyle *columnStyle = carsManager->columnStyle(col);
        Q_ASSERT(columnStyle);
        m_tableLayoutData->m_columnWidths[col] =
            qMax(columnStyle->relativeColumnWidth() * availableWidth / relativeWidthSum, 0.0);
    }

    // Column positions.
    qreal columnPosition = 0.0;
    qreal columnOffset = tableFormat.leftMargin();
    if (tableFormat.alignment() == Qt::AlignRight) {
        // Table is right-aligned, so add all of the remaining space.
        columnOffset += m_parentLayout->shape->size().width() - m_tableLayoutData->m_width;
    }
    if (tableFormat.alignment() == Qt::AlignHCenter) {
        // Table is centered, so add half of the remaining space.
        columnOffset += (m_parentLayout->shape->size().width() - m_tableLayoutData->m_width) / 2;
    }
    for (int col = 0; col < m_tableLayoutData->m_columnPositions.size(); ++col) {
        m_tableLayoutData->m_columnPositions[col] = columnPosition + columnOffset;
        // Increment by this column's width.
        columnPosition += m_tableLayoutData->m_columnWidths[col];
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

    QTextTableFormat tableFormat = m_table->format();

    // Get the column and row style manager.
    Q_ASSERT(tableFormat.hasProperty(KoTableStyle::ColumnAndRowStyleManager));
    KoTableColumnAndRowStyleManager *carsManager =
    reinterpret_cast<KoTableColumnAndRowStyleManager *>(
            tableFormat.property(KoTableStyle::ColumnAndRowStyleManager).value<void *>());
    Q_ASSERT(carsManager);

    /*
     * Implementation Note:
     *
     * An undocumented behavior of QTextTable::cellAt is that requesting a
     * cell that is covered by a spanning cell will return the cell that
     * spans over the requested cell. Example:
     *
     * +------------+------------+
     * |            |            |
     * |            |            |
     * |            +------------+
     * |            |            |
     * |            |            |
     * +------------+------------+
     *
     * table.cellAt(1, 0).row() // Will return 0.
     *
     * In the code below, we rely on this behavior to determine wheather
     * a cell "vertically" ends in the current row, as those are the only
     * cells that should contribute to the row height.
     */

    // Table height is at least as high as the rows that are above the
    // the row we're starting on.
    m_tableLayoutData->m_height = m_tableLayoutData->m_rowPositions[fromRow] - tableFormat.topMargin();

    for (int row = fromRow; row < m_table->rows(); ++row) {
        KoTableRowStyle *rowStyle = carsManager->rowStyle(row);

        // Adjust row height.
        qreal rowHeight = rowStyle ? rowStyle->minimumRowHeight() : 0.0;
        int col = 0;
        while (col < m_table->columns()) {
            // Get the cell format.
            QTextTableCell cell = m_table->cellAt(row, col);
            QTextTableCellFormat cellFormat = cell.format().toTableCellFormat();

            if (row == cell.row() + cell.rowSpan() - 1) {
                /*
                 * This cell ends vertically in this row, and hence should
                 * contribute to the row height. So we get the height of the
                 * entire cell, including borders and padding. This is done
                 * by calling KoTableCellStyle::boundingRect() with a
                 * rectangle as high as the cell content.
                 */
                KoTableCellStyle cellStyle(cellFormat);
                QRectF heightRect(1, 1, 1, m_tableLayoutData->m_contentHeights.at(cell.row()).at(cell.column()));
                qreal cellHeight = cellStyle.boundingRect(heightRect).height();

                // Subtract the height of the rows above.
                for (int rowAbove = row - 1; rowAbove >= cell.row(); --rowAbove) {
                    cellHeight -= m_tableLayoutData->m_rowHeights[rowAbove];
                }

                /*
                 * Now we know how much height this cell contributes to the row,
                 * and can determine wheather the row height will grow.
                 */
                rowHeight = qMax(rowHeight, cellHeight);
            }
            col += cell.columnSpan(); // Skip across column spans.
        }
        m_tableLayoutData->m_rowHeights[row] = rowHeight;

        // Adjust row Y position.
        if (row > 0) {
            m_tableLayoutData->m_rowPositions[row] =
                m_tableLayoutData->m_rowPositions[row - 1] + // Position of previous row.
                m_tableLayoutData->m_rowHeights[row - 1];    // Height of previous row.
        } else {
            // This is the first row, so just use table top margin.
            m_tableLayoutData->m_rowPositions[row] = tableFormat.topMargin();
        }

        // Add row height to table height.
        m_tableLayoutData->m_height += m_tableLayoutData->m_rowHeights[row];
    }
}

void TableLayout::draw(QPainter *painter) const
{
    painter->save();

    // Draw table background.
    QRectF tableRect = boundingRect();
    painter->fillRect(tableRect, m_table->format().background());

    // Draw cells using their styles.
    painter->translate(tableRect.topLeft());
    for (int row = 0; row < m_table->rows(); ++row) {
        for (int column = 0; column < m_table->columns(); ++column) {
            QTextTableCell tableCell = m_table->cellAt(row, column);
            /*
             * The following check relies on the fact that QTextTable::cellAt()
             * will return the cell that has the span when a covered cell is
             * requested.
             */
            if (row == tableCell.row() && column == tableCell.column()) {
                // This is an actual cell we want to draw, and not a covered one.
                KoTableCellStyle cellStyle(tableCell.format().toTableCellFormat());
                cellStyle.paint(*painter, cellBoundingRect(tableCell));
            }
        }
    }

    painter->restore();
}

QRectF TableLayout::boundingRect() const
{
    Q_ASSERT(isValid());

    qreal horizontalMargins = m_table->format().leftMargin() + m_table->format().rightMargin();
    qreal verticalMargins = m_table->format().topMargin() + m_table->format().bottomMargin();

    return QRectF(m_tableLayoutData->m_position.x(), m_tableLayoutData->m_position.y(),
            m_tableLayoutData->m_width + horizontalMargins, m_tableLayoutData->m_height + verticalMargins);
}

QRectF TableLayout::contentRect() const
{
    Q_ASSERT(isValid());

    QTextTableFormat tableFormat = m_table->format();

    return QRectF(tableFormat.leftMargin(), tableFormat.topMargin(),
            m_tableLayoutData->m_width, m_tableLayoutData->m_height);
}

QRectF TableLayout::cellContentRect(const QTextTableCell &cell) const
{
    Q_ASSERT(isValid());
    Q_ASSERT(cell.isValid());
    Q_ASSERT(cell.row() < m_tableLayoutData->m_rowPositions.size());
    Q_ASSERT(cell.column() < m_tableLayoutData->m_columnPositions.size());

    /*
     * Get the cell style and return the bounding rect adjusted for
     * borders and paddings by calling KoTableCellStyle::contentRect().
     */
    KoTableCellStyle cellStyle(cell.format().toTableCellFormat());
    return cellStyle.contentRect(cellBoundingRect(cell));
}

QRectF TableLayout::cellBoundingRect(const QTextTableCell &cell) const
{
    Q_ASSERT(isValid());
    Q_ASSERT(cell.row() < m_tableLayoutData->m_rowPositions.size());
    Q_ASSERT(cell.column() < m_tableLayoutData->m_columnPositions.size());

    // Cell width.
    qreal width = 0;
    for (int col = 0; col < cell.columnSpan(); ++col) {
        width += m_tableLayoutData->m_columnWidths[cell.column() + col];
    }

    // Cell height.
    qreal height = 0;
    for (int r = 0; r < cell.rowSpan(); ++r) {
        height += m_tableLayoutData->m_rowHeights[cell.row() + r];
    }

    return QRectF(
            m_tableLayoutData->m_columnPositions[cell.column()], m_tableLayoutData->m_rowPositions[cell.row()],
            width, height);
}

void TableLayout::calculateCellContentHeight(const QTextTableCell &cell)
{
    Q_ASSERT(isValid());
    Q_ASSERT(cell.isValid());

    if (!isValid() || !cell.isValid()) {
        return;
    }

    // Get the first line in the first block in the cell.
    QTextFrame::iterator cellIterator = cell.begin();
    Q_ASSERT(cellIterator.currentFrame() == 0); // TODO: Nested tables?
    QTextBlock firstBlock = cellIterator.currentBlock();
    Q_ASSERT(firstBlock.isValid());
    QTextLine topLine = firstBlock.layout()->lineAt(0);
    Q_ASSERT(topLine.isValid());

    // Get the last line in the last block in the cell.
    cellIterator = cell.end();
    cellIterator--;
    Q_ASSERT(cellIterator.currentFrame() == 0); // TODO: Nested tables?
    QTextBlock lastBlock = cellIterator.currentBlock();
    Q_ASSERT(lastBlock.isValid());
    QTextLine bottomLine = lastBlock.layout()->lineAt(lastBlock.layout()->lineCount() - 1);
    Q_ASSERT(bottomLine.isValid());

    // Content height is the difference between bottomLine and topLine.
    qreal contentHeight = (bottomLine.y() + bottomLine.height()) - topLine.y();
    Q_ASSERT(contentHeight >= 0); // Sanity check.
    contentHeight = qMax(contentHeight, 0.);

    // Update content height value of the cell.
    m_tableLayoutData->m_contentHeights[cell.row()][cell.column()] = contentHeight;

    // Re-layout from the row the cell is in.
    layoutFromRow(cell.row());
}

QTextTableCell TableLayout::cellAt(int position) const
{
    Q_ASSERT(isValid());

    if (!isValid()) {
        return QTextTableCell(); // Return invalid cell.
    }

    return m_table->cellAt(position);
}

QPointF TableLayout::position() const
{
    Q_ASSERT(isValid());

    return m_tableLayoutData->m_position;
}

void TableLayout::setPosition(QPointF position)
{
    Q_ASSERT(isValid());

    if (!isValid()) {
        return;
    }

    m_tableLayoutData->m_position = position;
}

bool TableLayout::isDirty() const
{
    return m_dirty;
}

bool TableLayout::isValid() const
{
    return (m_parentLayout && m_table && m_tableLayoutData);
}

void TableLayout::tableDestroyed(QObject *object)
{
    QTextTable *table = static_cast<QTextTable *>(object);
    Q_ASSERT(table);

    if (m_tableLayoutDataMap.contains(table)) {
        // Clean up table layout data for the table.
        delete m_tableLayoutDataMap.value(table);
        m_tableLayoutDataMap.remove(table);
    }

    if (m_table == table) {
        // It was the currently set table that was deleted.
        m_table = 0;
        m_tableLayoutData = 0;
    }
}

#include "TableLayout.moc"
