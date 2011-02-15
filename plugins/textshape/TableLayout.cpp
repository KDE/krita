/* This file is part of the KDE project
 * Copyright (C) 2009 Elvis Stansvik <elvstone@gmail.org>
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
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

#include "TableLayout.h"
#include "TableLayoutData.h"

#include <KoTableStyle.h>
#include <KoTableBorderStyle.h>
#include <KoTableCellStyle.h>
#include <KoTableColumnStyle.h>
#include <KoTableRowStyle.h>
#include <KoTableColumnAndRowStyleManager.h>
#include <KoTextDocumentLayout.h>
#include <KoShape.h>

#include <KoChangeTracker.h>
#include <KoChangeTrackerElement.h>
#include <KoGenChange.h>

#include <QTextDocument>
#include <QTextTable>
#include <QTextLine>
#include <QPainter>
#include <QRectF>
#include <QPointF>

#include <kdebug.h>

TableLayout::TableLayout(QTextTable *table)
{
    setTable(table);
}

TableLayout::TableLayout() :
    m_table(0),
    m_tableLayoutData(0)
{
}

void TableLayout::setTable(QTextTable *table)
{
    Q_ASSERT(table);

    if (table != m_table) {
        // We are not already set
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
    }

    // Resize geometry vectors for the table.
    m_tableLayoutData->m_rowPositions.resize(m_table->rows());
    m_tableLayoutData->m_rowHeights.resize(m_table->rows());
    m_tableLayoutData->m_contentHeights.resize(m_table->rows());
    for (int row = 0; row < m_table->rows(); ++row) {
        m_tableLayoutData->m_contentHeights[row].resize(m_table->columns());
    }
}

QTextTable *TableLayout::table() const
{
    return m_table;
}

void TableLayout::startNewTableRect(QPointF position, qreal parentWidth, int fromRow)
{
    Q_ASSERT(isValid());

    if (!isValid()) {
        return;
    }

    // First remove the following (by now invalid) table rects
    while (!m_tableLayoutData->m_tableRects.isEmpty() && m_tableLayoutData->m_tableRects.back().fromRow > fromRow) {
        m_tableLayoutData->m_tableRects.removeLast();
    }

    // Also remove the back tableRect if we are about to add one similar.
    // TODO when breaking inside rows this needs to be made smarter
    if (!m_tableLayoutData->m_tableRects.isEmpty() && m_tableLayoutData->m_tableRects.back().fromRow == fromRow) {
        m_tableLayoutData->m_tableRects.removeLast();
    }

    QTextTableFormat tableFormat = m_table->format();

    // Table width.
    qreal tableWidth = 0;
    if (tableFormat.width().rawValue() == 0 || tableFormat.alignment() == Qt::AlignJustify) {
        // We got a zero width value or alignment is justify, so use 100% of parent.
        tableWidth = parentWidth;
    } else {
        if (tableFormat.width().type() == QTextLength::FixedLength) {
            // Fixed length value, so use the raw value directly.
            tableWidth = tableFormat.width().rawValue();
        } else if (tableFormat.width().type() == QTextLength::PercentageLength) {
            // Percentage length value, so use a percentage of parent width.
            tableWidth = tableFormat.width().rawValue() * (parentWidth / 100)
                - tableFormat.leftMargin() - tableFormat.rightMargin();
        } else {
            // Unknown length type, so use 100% of parent.
            kWarning(32600) << "Unknown table width type";
            tableWidth = parentWidth;
        }
    }

    TableRect tableRect;
    tableRect.fromRow = fromRow;
    tableRect.rect = QRectF(position.x() + tableFormat.leftMargin(),
                                            position.y() + (fromRow==0 ? tableFormat.topMargin() : 0.0),
                                            tableWidth, 1); // the 1 is to make sure it's valid
    tableRect.columnPositions.resize(m_table->columns());
    tableRect.columnWidths.resize(m_table->columns());

    // Get the column and row style manager.
    KoTableColumnAndRowStyleManager carsManager = KoTableColumnAndRowStyleManager::getManager(m_table);

    // Column widths.
    qreal availableWidth = tableWidth; // Width available for columns.
    QList<int> relativeWidthColumns; // List of relative width columns.
    qreal relativeWidthSum = 0; // Sum of relative column width values.
    int numNonStyleColumns = 0;
    for (int col = 0; col < tableRect.columnPositions.size(); ++col) {
        KoTableColumnStyle columnStyle = carsManager.columnStyle(col);
        if (columnStyle.hasProperty(KoTableColumnStyle::RelativeColumnWidth)) {
            // Relative width specified. Will be handled in the next loop.
            relativeWidthColumns.append(col);
            relativeWidthSum += columnStyle.relativeColumnWidth();
        } else if (columnStyle.hasProperty(KoTableColumnStyle::ColumnWidth)) {
            // Only width specified, so use it.
            tableRect.columnWidths[col] = columnStyle.columnWidth();
            availableWidth -= columnStyle.columnWidth();
        } else {
            // Neither width nor relative width specified.
            tableRect.columnWidths[col] = 0.0;
            relativeWidthColumns.append(col); // handle it as a relative width column without asking for anything
            ++numNonStyleColumns;
        }
    }

    // Calculate width to those columns that don't actually request it
    qreal widthForNonWidthColumn = ((1.0 - qMin<qreal>(relativeWidthSum, 1.0)) * availableWidth);
    availableWidth -= widthForNonWidthColumn; // might as well do this calc before dividing by numNonStyleColumns
    if (numNonStyleColumns)
        widthForNonWidthColumn /= numNonStyleColumns;

    // Relative column widths have now been summed up and can be distributed.
    foreach (int col, relativeWidthColumns) {
        KoTableColumnStyle columnStyle = carsManager.columnStyle(col);
        if (columnStyle.hasProperty(KoTableColumnStyle::RelativeColumnWidth) || columnStyle.hasProperty(KoTableColumnStyle::ColumnWidth)) {
            tableRect.columnWidths[col] =
                qMax<qreal>(columnStyle.relativeColumnWidth() * availableWidth / relativeWidthSum, 0.0);
        } else {
            tableRect.columnWidths[col] = widthForNonWidthColumn;
        }
    }

    // Column positions.
    qreal columnPosition = position.x();
    qreal columnOffset = tableFormat.leftMargin();
    if (tableFormat.alignment() == Qt::AlignRight) {
        // Table is right-aligned, so add all of the remaining space.
        columnOffset += parentWidth - tableWidth;
    }
    if (tableFormat.alignment() == Qt::AlignHCenter) {
        // Table is centered, so add half of the remaining space.
        columnOffset += (parentWidth - tableWidth) / 2;
    }
    for (int col = 0; col < tableRect.columnPositions.size(); ++col) {
        tableRect.columnPositions[col] = columnPosition + columnOffset;
        // Increment by this column's width.
        columnPosition += tableRect.columnWidths[col];
    }

    m_tableLayoutData->m_minX = qMin(m_tableLayoutData->m_minX, tableRect.columnPositions[0]);
    m_tableLayoutData->m_maxX = qMax(m_tableLayoutData->m_maxX, columnPosition);
    m_tableLayoutData->m_tableRects.append(tableRect);
    m_tableLayoutData->m_rowPositions[fromRow] = tableRect.rect.top(); //Initialize the position of first row of tableRect
}

void TableLayout::layoutRow(int row)
{
    Q_ASSERT(isValid());
    Q_ASSERT(row >= 0);
    Q_ASSERT(row < m_table->rows());

    if (!isValid()) {
        return;
    }
    if (row < 0 || row >= m_table->rows()) {
        return;
    }

    // First remove the following (by now invalid) table rects
    while (m_tableLayoutData->m_tableRects.back().fromRow > row) {
        m_tableLayoutData->m_tableRects.removeLast();
    }
    // Now the following calls to m_tableRects.back() are hitting the right table rect

    QTextTableFormat tableFormat = m_table->format();

    // Get the column and row style manager.
    KoTableColumnAndRowStyleManager carsManager = KoTableColumnAndRowStyleManager::getManager(m_table);

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

    KoTableRowStyle rowStyle = carsManager.rowStyle(row);

    // Adjust row height.
    qreal minimumRowHeight = rowStyle.minimumRowHeight();
    qreal rowHeight = rowStyle.rowHeight();
    bool rowHasExactHeight = rowStyle.hasProperty(KoTableRowStyle::RowHeight);

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

            // if the row does not have exact height defined
            if (!rowHasExactHeight) {
                /*
                 * Now we know how much height this cell contributes to the row,
                 * and can determine wheather the row height will grow.
                 */
                rowHeight = qMax(qMax(minimumRowHeight, cellHeight), rowHeight);
            }
        }
        col += cell.columnSpan(); // Skip across column spans.
    }
    m_tableLayoutData->m_rowHeights[row] = rowHeight;

    // Adjust Y position of NEXT row.
    // This is nice since the outside layout routine relies on the next row having a correct y position
    // the first row y position is set in createFirstLayoutRect()
    if (row+1 <  m_table->rows()) {
        m_tableLayoutData->m_rowPositions[row+1] =
            m_tableLayoutData->m_rowPositions[row ] + // Position of this row.
            m_tableLayoutData->m_rowHeights[row];    // Height of this row.
    } else
        m_tableLayoutData->m_dirty = false; //we have reached the end and the table should now be fully laied out

    // Adjust table rect height for new height.
    m_tableLayoutData->m_tableRects.last().rect.setHeight(m_tableLayoutData->m_rowPositions[row]
        + m_tableLayoutData->m_rowHeights[row]
        - m_tableLayoutData->m_rowPositions[m_tableLayoutData->m_tableRects.last().fromRow]);//FIXME review when breaking inside a row
}

void TableLayout::drawBackground(QPainter *painter, const KoTextDocumentLayout::PaintContext &context) const
{
    if (m_tableLayoutData->m_tableRects.isEmpty()) {
        return;
    }
    painter->save();
    // Draw table background and row backgrounds
    KoTableColumnAndRowStyleManager carsManager = KoTableColumnAndRowStyleManager::getManager(m_table);
    foreach (TableRect tRect, m_tableLayoutData->m_tableRects) {
        painter->fillRect(tRect.rect, m_table->format().background());
        QRectF tableRect = tRect.rect;
        for (int row = tRect.fromRow; row < m_table->rows() && m_tableLayoutData->m_rowPositions[row] < tableRect.bottom(); ++row) {
            QRectF rowRect(tableRect.x(), m_tableLayoutData->m_rowPositions[row], tableRect.width(), m_tableLayoutData->m_rowHeights[row]);
            KoTableRowStyle rowStyle = carsManager.rowStyle(row);
            painter->fillRect(rowRect, rowStyle.background());
        }
    }
    // Draw cell backgrounds using their styles.
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
                QTextTableCellFormat tfm(tableCell.format().toTableCellFormat());
                QRectF bRect(cellBoundingRect(tableCell,tfm));
                QVariant background(tfm.property(KoTableBorderStyle::CellBackgroundBrush));
                if (!background.isNull()) {
                    painter->fillRect(bRect, qvariant_cast<QBrush>(background));
                }
                // possibly draw the selection of the entire cell
                foreach(const QAbstractTextDocumentLayout::Selection & selection,   context.textContext.selections) {
                    if (selection.cursor.hasComplexSelection()) {
                        int selectionRow;
                        int selectionColumn;
                        int selectionRowSpan;
                        int selectionColumnSpan;
                        selection.cursor.selectedTableCells(&selectionRow, &selectionRowSpan, &selectionColumn, &selectionColumnSpan);
                        if (row >= selectionRow && column>=selectionColumn
                            && row < selectionRow + selectionRowSpan
                            && column < selectionColumn + selectionColumnSpan) {
                            painter->fillRect(bRect, selection.format.background());
                        }
                    } else if (selection.cursor.selectionStart()  < m_table->firstPosition()
                        && selection.cursor.selectionEnd() > m_table->lastPosition()) {
                        painter->fillRect(bRect, selection.format.background());
                    }
                }
            }
        }
    }

    // Draw a background to indicate a change-type
    KoChangeTracker *changeTracker = KoTextDocument(m_table->document()).changeTracker();
    if (changeTracker && changeTracker->displayChanges()) {
        for (int row = 0; row < m_table->rows(); ++row) {
            for (int column = 0; column < m_table->columns(); ++column) {
                QTextTableCell tableCell = m_table->cellAt(row, column);
                KoChangeTrackerElement *changeElement = changeTracker->elementById(tableCell.format().property(KoCharacterStyle::ChangeTrackerId).toInt());
                if (!changeElement) {
                    //Check For table changes
                    changeElement = changeTracker->elementById(m_table->format().property(KoCharacterStyle::ChangeTrackerId).toInt());
                }

                if (changeElement && changeElement->isEnabled()) {
                    switch(changeElement->getChangeType()) {
                        case KoGenChange::InsertChange:
                            painter->fillRect(cellBoundingRect(tableCell), changeTracker->getInsertionBgColor());
                        break;
                        case KoGenChange::FormatChange:
                            painter->fillRect(cellBoundingRect(tableCell), changeTracker->getFormatChangeBgColor());
                        break;
                        case KoGenChange::DeleteChange:
                            painter->fillRect(cellBoundingRect(tableCell), changeTracker->getDeletionBgColor());
                        break;
                    }
                }
            }
        }   
    }

    painter->restore();
}

void TableLayout::drawBorders(QPainter *painter, QVector<QLineF> *accuBlankBorders) const
{
    if (m_tableLayoutData->m_tableRects.isEmpty()) {
        return;
    }

    painter->save();

    KoTableStyle tableStyle(m_table->format());
    bool collapsing = tableStyle.collapsingBorderModel();

    // Draw cell borders using their styles.
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
                QTextTableCellFormat tfm(tableCell.format().toTableCellFormat());
                KoTableBorderStyle cellStyle(tfm);

                QRectF bRect = cellBoundingRect(tableCell,tfm);
                if (collapsing) {

                    // First the horizontal borders
                    if (row == 0) {
                        cellStyle.drawTopHorizontalBorder(*painter, bRect.x(), bRect.y(), bRect.width(), accuBlankBorders);
                    }
                    if (row + tfm.tableCellRowSpan() == m_table->rows()) {
                        // we hit the bottom of the table so just draw the bottom border
                        cellStyle.drawBottomHorizontalBorder(*painter, bRect.x(), bRect.bottom(), bRect.width(), accuBlankBorders);
                    } else {
                        int c = column;
                        while (c < column + tfm.tableCellColumnSpan()) {
                            QTextTableCell tableCellBelow = m_table->cellAt(row + tfm.tableCellRowSpan(), c);
                            QTextTableCellFormat belowTfm(tableCellBelow.format().toTableCellFormat());
                            QRectF belowBRect = cellBoundingRect(tableCellBelow, belowTfm);
                            qreal x = qMax(bRect.x(), belowBRect.x());
                            qreal x2 = qMin(bRect.right(), belowBRect.right());
                            KoTableBorderStyle cellBelowStyle(belowTfm);
                            cellStyle.drawSharedHorizontalBorder(*painter, cellBelowStyle, x, bRect.bottom(), x2 - x, accuBlankBorders);
                            c = tableCellBelow.column() + belowTfm.tableCellColumnSpan();
                        }
                    }

                    // And then the same treatment for vertical borders
                    if (column == 0) {
                        cellStyle.drawLeftmostVerticalBorder(*painter, bRect.x(), bRect.y(), bRect.height(), accuBlankBorders);
                    }
                    if (column + tfm.tableCellColumnSpan() == m_table->columns()) {
                        // we hit the rightmost edge of the table so draw the rightmost border
                        cellStyle.drawRightmostVerticalBorder(*painter, bRect.right(), bRect.y(), bRect.height(), accuBlankBorders);
                    } else {
                        // we have cells to the right so draw sharedborders
                        int r = row;
                        while (r < row + tfm.tableCellRowSpan()) {
                            QTextTableCell tableCellRight = m_table->cellAt(r, column + tfm.tableCellColumnSpan());
                            QTextTableCellFormat rightTfm(tableCellRight.format().toTableCellFormat());
                            QRectF rightBRect = cellBoundingRect(tableCellRight, rightTfm);
                            qreal y = qMax(bRect.y(), rightBRect.y());
                            qreal y2 = qMin(bRect.bottom(), rightBRect.bottom());
                            KoTableBorderStyle cellBelowRight(rightTfm);
                            cellStyle.drawSharedVerticalBorder(*painter, cellBelowRight, bRect.right(), y, y2-y, accuBlankBorders);
                            r = tableCellRight.row() + rightTfm.tableCellRowSpan();
                        }
                    }
                    // Paint diagonal borders for current cell
                    cellStyle.paintDiagonalBorders(*painter, bRect);
                } else { // separating border model
                    cellStyle.paintBorders(*painter, bRect, accuBlankBorders);
                }
            }
        }
    }

    painter->restore();
}

QTextTableCell TableLayout::hitTestTable(const QPointF &point) const
{
    foreach (TableRect tableRect, m_tableLayoutData->m_tableRects) {
        if (tableRect.rect.contains(point)) {
            int col;
            for (col = 1; col < m_table->columns(); ++col) {
                if (point.x() < tableRect.columnPositions[col])
                    break;
            }
            --col;
            int row;
            for (row = tableRect.fromRow; row < m_table->rows(); ++row) {
                if (point.y() < m_tableLayoutData->m_rowPositions[row])
                    break;
            }
            --row;
            return m_table->cellAt(row, col);
        }
    }

    return QTextTableCell();
}

qreal TableLayout::tableMinX() const
{
    return m_tableLayoutData->m_minX-10; //-10 is just to add potential penwidths
}

qreal TableLayout::tableMaxX() const
{
    return m_tableLayoutData->m_maxX+10; //10 is just to add potential penwidths
}

qreal TableLayout::cellContentY(const QTextTableCell &cell) const
{
    Q_ASSERT(isValid());
    Q_ASSERT(cell.isValid());

    /*
     * Get the cell style and return the y pos adjusted for
     * borders and paddings.
     */
    KoTableCellStyle cellStyle(cell.format().toTableCellFormat());
    return m_tableLayoutData->m_rowPositions[cell.row()] + cellStyle.topPadding() + cellStyle.topBorderWidth();
}

qreal TableLayout::cellContentX(const QTextTableCell &cell) const
{
    Q_ASSERT(isValid());
    Q_ASSERT(cell.isValid());
    Q_ASSERT(cell.row() < m_tableLayoutData->m_rowPositions.size());
    
    if (m_tableLayoutData->m_tableRects.isEmpty())
        return 0.0;
    
    TableRect tableRect = m_tableLayoutData->m_tableRects.last();
    int i = m_tableLayoutData->m_tableRects.size()-1;
    while (tableRect.fromRow > cell.row()) {
        --i;
        tableRect =  m_tableLayoutData->m_tableRects[i];
    }
    Q_ASSERT(cell.column() + cell.columnSpan() <=  tableRect.columnPositions.size());

    /*
     * Get the cell style and return the bounding rect adjusted for
     * borders and paddings by calling KoTableCellStyle::contentRect().
     */
    KoTableCellStyle cellStyle(cell.format().toTableCellFormat());
    return tableRect.columnPositions[cell.column()] + cellStyle.topPadding() + cellStyle.topBorderWidth();
}

QRectF TableLayout::cellContentRect(const QTextTableCell &cell) const
{
    Q_ASSERT(isValid());
    Q_ASSERT(cell.isValid());

    /*
     * Get the cell style and return the bounding rect adjusted for
     * borders and paddings by calling KoTableCellStyle::contentRect().
     */
    KoTableCellStyle cellStyle(cell.format().toTableCellFormat());
    return cellStyle.contentRect(cellBoundingRect(cell));
}

QRectF TableLayout::cellBoundingRect(const QTextTableCell &cell) const
{
    return cellBoundingRect(cell, cell.format());
}

QRectF TableLayout::cellBoundingRect(const QTextTableCell &cell, const QTextCharFormat &fmt) const
{
    Q_ASSERT(isValid());

    const int row = cell.row();
    const int column = cell.column();
    const int rowSpan = fmt.tableCellRowSpan();
    const int columnSpan = fmt.tableCellColumnSpan();

    Q_ASSERT(row < m_tableLayoutData->m_rowPositions.size());
    TableRect tableRect = m_tableLayoutData->m_tableRects.last();
    int i = m_tableLayoutData->m_tableRects.size()-1;
    while (tableRect.fromRow > row) {
        --i;
        tableRect =  m_tableLayoutData->m_tableRects[i];
    }
    Q_ASSERT(column + columnSpan <=  tableRect.columnPositions.size());

    // Cell width.
    qreal width = 0;
    for (int col = 0; col < columnSpan; ++col) {
        width += tableRect.columnWidths[column + col];
    }

    // Cell height.
    qreal height = 0;
    for (int r = 0; r < rowSpan; ++r) {
        height += m_tableLayoutData->m_rowHeights[row + r];
        // TODO  when breaking within a row the tableRect needs to be switched at some point
    }

    return QRectF(tableRect.columnPositions[column], m_tableLayoutData->m_rowPositions[row],
            width, height);
}

QRectF TableLayout::rowBoundingRect(int row) const
{
    Q_ASSERT(row < m_tableLayoutData->m_rowPositions.size());
    TableRect tableRect = m_tableLayoutData->m_tableRects.last();
    int i = m_tableLayoutData->m_tableRects.size()-1;
    while (tableRect.fromRow > row) {
        --i;
        tableRect =  m_tableLayoutData->m_tableRects[i];
    }
    return QRectF(tableRect.rect.left(), m_tableLayoutData->m_rowPositions[row],
                tableRect.rect.width(),  m_tableLayoutData->m_rowHeights[row]);
}

void TableLayout::setCellContentHeight(const QTextTableCell &cell, qreal bottom)
{
    Q_ASSERT(isValid());
    Q_ASSERT(cell.isValid());

    if (!isValid() || !cell.isValid())
        return;

    KoTableCellStyle cellStyle(cell.format().toTableCellFormat());
    qreal top = m_tableLayoutData->m_rowPositions[cell.row()]
    + cellStyle.topPadding() + cellStyle.topBorderWidth();
    qreal contentHeight = bottom - top;
    if (contentHeight < (qreal)0.126) // rounding problems due to Qt-scribe internally using ints.
        contentHeight = (qreal)0.0;

    Q_ASSERT(contentHeight >= 0);

    // Update content height value of the cell.
    m_tableLayoutData->m_contentHeights[cell.row()][cell.column()] = contentHeight;
}

QTextTableCell TableLayout::cellAt(int position) const
{
    Q_ASSERT(isValid());

    if (!isValid())
        return QTextTableCell(); // Return invalid cell.

    return m_table->cellAt(position);
}

qreal TableLayout::yAfterTable() const
{
    Q_ASSERT(isValid());

    QTextTableFormat tableFormat = m_table->format();

    return m_tableLayoutData->m_tableRects.last().rect.bottom() + tableFormat.bottomMargin();
}

bool TableLayout::isDirty() const
{
    return isValid() && m_tableLayoutData->m_dirty;
}

bool TableLayout::isValid() const
{
    return (m_table && m_tableLayoutData);
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

#include <TableLayout.moc>
