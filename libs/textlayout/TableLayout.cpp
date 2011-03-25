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

#include <QTextDocument>
#include <QTextTable>
#include <QTextLine>
#include <QPainter>
#include <QRectF>
#include <QPointF>

#include <kdebug.h>


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


QRectF TableLayout::cellBoundingRect(const QTextTableCell &cell, const QTextCharFormat &fmt) const
{
    Q_ASSERT(isValid());

    const int row = cell.row();
    const int column = cell.column();
    const int rowSpan = fmt.tableCellRowSpan();
    int columnSpan = fmt.tableCellColumnSpan();

    Q_ASSERT(row < m_tableLayoutData->m_rowPositions.size());
    TableRect tableRect = m_tableLayoutData->m_tableRects.last();
    int i = m_tableLayoutData->m_tableRects.size()-1;
    while (tableRect.fromRow > row) {
        --i;
        tableRect =  m_tableLayoutData->m_tableRects[i];
    }

    if (column + columnSpan > tableRect.columnPositions.size())
        columnSpan = tableRect.columnPositions.size() - column;

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

QTextTableCell TableLayout::cellAt(int position) const
{
    Q_ASSERT(isValid());

    if (!isValid())
        return QTextTableCell(); // Return invalid cell.

    return m_table->cellAt(position);
}


#include <TableLayout.moc>
