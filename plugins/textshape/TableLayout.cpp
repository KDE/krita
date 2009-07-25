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
#include "TableData.h"

#include <KoTableStyle.h>
#include <KoTableCellStyle.h>
#include <KoTableColumnStyle.h>
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
    setPosition(QPointF(0, 0));
}

TableLayout::TableLayout() :
    m_parentLayout(0),
    m_table(0),
    m_tableData(0),
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
    switch (tableFormat.width().type()) {
        case QTextLength::VariableLength:
        case QTextLength::FixedLength:
            m_tableData->m_width = tableFormat.width().rawValue();
            break;
        case QTextLength::PercentageLength: {
            qreal parentWidth = m_parentLayout->shape->size().width();
            m_tableData->m_width = tableFormat.width().rawValue() * (parentWidth / 100)
                - tableFormat.leftMargin() - tableFormat.rightMargin();
            break;
        }
        default:
            kWarning(32600) << "Unknown table width type";
            m_tableData->m_width = 0;
    }

    // Column widths and positions.
    if (tableFormat.hasProperty(KoTableStyle::ColumnAndRowStyleManager)) {
        // Get the column and row style manager.
        KoTableColumnAndRowStyleManager *carsManager =
            reinterpret_cast<KoTableColumnAndRowStyleManager *>(
                    tableFormat.property(KoTableStyle::ColumnAndRowStyleManager).value<void *>());
        Q_ASSERT(carsManager);

        // Column widths.
        qreal availableWidth = m_tableData->m_width; // Width available for columns.
        QList<int> relativeWidthColumns; // List of relative width columns.
        qreal relativeWidthSum; // Sum of relative column width values.
        for (int col = 0; col < m_tableData->m_columnPositions.size(); ++col) {
            KoTableColumnStyle *columnStyle = carsManager->columnStyle(col);
            Q_ASSERT(columnStyle);
            if (columnStyle->hasProperty(KoTableColumnStyle::RelativeColumnWidth)) {
                // Relative width specified. Will be handled in the next loop.
                relativeWidthColumns.append(col);
                relativeWidthSum += columnStyle->relativeColumnWidth();
            } else if (columnStyle->hasProperty(KoTableColumnStyle::ColumnWidth)) {
                // Only width specified, so use it.
                m_tableData->m_columnWidths[col] = columnStyle->columnWidth();
                availableWidth -= columnStyle->columnWidth();
            } else {
                // Neither width nor relative width specified. Can this happen?
                kWarning(32600) << "Neither column-width nor rel-column-width specified";
                m_tableData->m_columnWidths[col] = 0.0;
            }
        }
        // Relative column widths have now been summed up and can be distributed.
        foreach (qreal col, relativeWidthColumns) {
            KoTableColumnStyle *columnStyle = carsManager->columnStyle(col);
            Q_ASSERT(columnStyle);
            m_tableData->m_columnWidths[col] =
                qMax(columnStyle->relativeColumnWidth() * availableWidth / relativeWidthSum, 0.0);
        }

        // Column positions.
        qreal columnPosition = 0.0;
        for (int col = 0; col < m_tableData->m_columnPositions.size(); ++col) {
            m_tableData->m_columnPositions[col] = columnPosition + tableFormat.leftMargin();
            columnPosition += m_tableData->m_columnWidths[col];
        }
    } else {
        // FIXME: Unit tests need to set up a KoTableColumnAndRowStyleManager.
        //        Keeping this code for them to keep passing for now.
        kWarning(32600) << "No KoTableColumnAndRowStyleManager!";
        qreal columnWidth = m_tableData->m_width / m_table->columns();
        m_tableData->m_columnWidths.fill(columnWidth);
        for (int col = 0; col < m_tableData->m_columnPositions.size(); ++col) {
            m_tableData->m_columnPositions[col] = (col * columnWidth) + tableFormat.leftMargin();
        }
    }

    // table alignment.
    for (int col = 0; col < m_tableData->m_columnPositions.size(); ++col) {
        switch (tableFormat.alignment()) {
            case Qt::AlignRight:
                // Table is right-aligned, so add all of the remaining space.
                m_tableData->m_columnPositions[col] +=
                    m_parentLayout->shape->size().width() - m_tableData->m_width;
                break;
            case Qt::AlignHCenter:
                // Table is centered, so add half of the remaining space.
                m_tableData->m_columnPositions[col] +=
                    (m_parentLayout->shape->size().width() - m_tableData->m_width) / 2;
                break;
            default:
                break;
        }
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

    m_tableData->m_height = m_tableData->m_rowPositions[fromRow] - tableFormat.topMargin();

    /*
     * Note that the cells that span more than one row or column are
     * shared, so that QTextTable::cellAt() returns the combined cell, and
     * that this combined cell has row()/column() corresponding to
     * the top left corner of the span. We rely on this in this code.
     */
    for (int row = fromRow; row < m_table->rows(); ++row) {
        // adjust row height to the largest cell height in the row.
        qreal rowHeight = 0;
        int col = 0;
        while (col < m_table->columns()) {
            // get the cell border data.
            QTextTableCell cell = m_table->cellAt(row, col);
            QTextTableCellFormat cellFormat = cell.format().toTableCellFormat();
            if (row == cell.row() + cell.rowSpan() - 1) {
                /*
                 * we are only in cells that vertically ends in this row.
                 * we only want these cells because cells that don't end in this
                 * row should not affect the row height.
                 */
                KoTableCellStyle cellStyle(cellFormat);

                // passing content rect to boundingRect() (we're only interested in height).
                QRectF heightRect(1, 1, 1, m_tableData->m_contentHeights.at(cell.row()).at(cell.column()));

                /*
                 * we need to calculate the height which this cell contributes to this
                 * row, so once we have the height of the entire cell, including borders
                 * and padding, we can now subtract the height of the rows above.
                 */
                qreal cellHeight = cellStyle.boundingRect(heightRect).height();
                for (int r = row - 1; r >= cell.row(); --r) {
                    cellHeight -= m_tableData->m_rowHeights[r];
                }
                rowHeight = qMax(rowHeight, cellHeight);
            }
            col += cell.columnSpan();
        }
        m_tableData->m_rowHeights[row] = rowHeight;

        // adjust row Y position.
        if (row > 0) {
            m_tableData->m_rowPositions[row] =
                m_tableData->m_rowPositions[row - 1] + // position of previous row.
                m_tableData->m_rowHeights[row - 1];    // height of previous row.
        } else {
            // this is the first row, so just use table top margin.
            m_tableData->m_rowPositions[row] = tableFormat.topMargin();
        }

        // adjust table height.
        m_tableData->m_height += m_tableData->m_rowHeights[row];
    }
}

void TableLayout::draw(QPainter *painter) const
{
    Q_UNUSED(painter);
    // TODO.
    QRectF tableRect = boundingRect();
    painter->save();
    painter->fillRect(tableRect,m_table->format().background());
    painter->translate(tableRect.topLeft());

    for (int row = 0; row < m_table->rows(); ++row) {
        for (int column = 0; column < m_table->columns(); ++column) {
            QTextTableCell tableCell = m_table->cellAt(row, column);
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

    return QRectF(m_position.x(), m_position.y(),
            m_tableData->m_width + horizontalMargins,
            m_tableData->m_height + verticalMargins);
}

QRectF TableLayout::contentRect() const
{
    Q_ASSERT(isValid());

    QTextTableFormat tableFormat = m_table->format();

    return QRectF(tableFormat.leftMargin(), tableFormat.topMargin(),
            m_tableData->m_width, m_tableData->m_height);
}

QRectF TableLayout::cellContentRect(const QTextTableCell &cell) const
{
    Q_ASSERT(isValid());
    Q_ASSERT(cell.isValid());
    Q_ASSERT(cell.row() < m_tableData->m_rowPositions.size());
    Q_ASSERT(cell.column() < m_tableData->m_columnPositions.size());

    // get the border data for the cell and return the adjusted rect.
    KoTableCellStyle cellStyle(cell.format().toTableCellFormat());

    return cellStyle.contentRect(cellBoundingRect(cell));
}

QRectF TableLayout::cellBoundingRect(const QTextTableCell &cell) const
{
    Q_ASSERT(isValid());
    Q_ASSERT(cell.row() < m_tableData->m_rowPositions.size());
    Q_ASSERT(cell.column() < m_tableData->m_columnPositions.size());

    qreal width = 0;
    qreal height = 0;

    for (int c = 0; c < cell.columnSpan(); ++c) {
        width += m_tableData->m_columnWidths[cell.column() + c];
    }

    for (int r = 0; r < cell.rowSpan(); ++r) {
        height += m_tableData->m_rowHeights[cell.row() + r];
    }

    return QRectF(
            m_tableData->m_columnPositions[cell.column()], m_tableData->m_rowPositions[cell.row()],
            width, height);
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
    QTextLine bottomLine = lastBlock.layout()->lineAt(lastBlock.layout()->lineCount() - 1);
    Q_ASSERT(bottomLine.isValid());

    qreal contentHeight = (bottomLine.y() + bottomLine.height()) - topLine.y();
    Q_ASSERT(contentHeight >= 0); // sanity check.
    contentHeight = qMax(contentHeight, 0.);

    // update content height value of the cell.
    m_tableData->m_contentHeights[cell.row()][cell.column()] = contentHeight;

    // re-layout from the row the cell is in.
    layoutFromRow(cell.row());
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
    return (m_parentLayout && m_table && m_tableData);
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
