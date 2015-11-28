/* This file is part of the KDE project
 * Copyright (C) 2009 Elvis Stansvik <elvstone@gmail.org>
 * Copyright (C) 2011 C. Boemann <cbo@kogmbh.com>
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

#include "KoTextLayoutTableArea.h"

#include "KoTextLayoutCellHelper.h"
#include "TableIterator.h"
#include "KoPointedAt.h"

#include <KoTableColumnAndRowStyleManager.h>
#include <KoTableColumnStyle.h>
#include <KoTableRowStyle.h>
#include <KoTableCellStyle.h>
#include <KoTableStyle.h>
#include <KoStyleManager.h>
#include <KoTextTableTemplate.h>

#include <QTextTable>
#include <QTextTableFormat>
#include <QPainter>
#include <QRectF>
#include <QPair>

#include "FrameIterator.h"

class Q_DECL_HIDDEN KoTextLayoutTableArea::Private
{
public:
    Private()
     : startOfArea(0)
    {
    }
    QVector<QVector<KoTextLayoutArea *> > cellAreas;
    TableIterator *startOfArea;
    TableIterator *endOfArea;
    bool lastRowHasSomething;
    QTextTable *table;
    int headerRows;
    qreal headerOffsetX;
    qreal headerOffsetY;
    KoTableColumnAndRowStyleManager carsManager;
    qreal tableWidth;
    QVector<qreal> headerRowPositions; // we will only fill those that this area covers
    QVector<qreal> rowPositions; // we will only fill those that this area covers
    QVector<qreal> columnWidths;
    QVector<qreal> columnPositions;
    bool collapsing;
    bool totalMisFit;
    KoTextDocumentLayout *documentLayout;

    KoTableCellStyle effectiveCellStyle(const QTextTableCell &tableCell);
};

KoTableCellStyle KoTextLayoutTableArea::Private::effectiveCellStyle(const QTextTableCell &tableCell)
{
    QTextTableFormat tableFormat = table->format();
    KoTableCellStyle cellStyle(tableCell.format().toTableCellFormat());
    if (documentLayout->styleManager() && table->format().hasProperty(KoTableStyle::TableTemplate)) {
        if (KoTextTableTemplate *tableTemplate = documentLayout->styleManager()->tableTemplate(table->format().intProperty(KoTableStyle::TableTemplate))) {
            //priorities according to ODF 1.2, 16.18 - table:table-template
            if (tableCell.column() == 0 && tableTemplate->firstColumn()
                    && tableFormat.boolProperty(KoTableStyle::UseFirstColumnStyles)) {
                cellStyle = *(documentLayout->styleManager()->tableCellStyle(tableTemplate->firstColumn()));
                return cellStyle;
            }

            if (tableCell.column() == (table->columns() - 1) && tableTemplate->lastColumn()
                    && tableFormat.boolProperty(KoTableStyle::UseLastColumnStyles)) {
                cellStyle = *(documentLayout->styleManager()->tableCellStyle(tableTemplate->lastColumn()));
                return cellStyle;
            }

            if (tableCell.row() == 0 && tableTemplate->firstRow()
                    && tableFormat.boolProperty(KoTableStyle::UseFirstRowStyles)) {
                cellStyle = *(documentLayout->styleManager()->tableCellStyle(tableTemplate->firstRow()));
                return cellStyle;
            }

            if (tableCell.row() == (table->rows() - 1) && tableTemplate->lastRow()
                    && tableFormat.boolProperty(KoTableStyle::UseLastRowStyles)) {
                cellStyle = *(documentLayout->styleManager()->tableCellStyle(tableTemplate->lastRow()));
                return cellStyle;
            }

            if (((tableCell.row() + 1) % 2) == 0 && tableTemplate->evenRows()
                    && tableFormat.boolProperty(KoTableStyle::UseBandingRowStyles)) {
                cellStyle = *(documentLayout->styleManager()->tableCellStyle(tableTemplate->evenRows()));
                return cellStyle;
            }

            if (((tableCell.row() + 1) % 2) != 0 && tableTemplate->oddRows()
                    && tableFormat.boolProperty(KoTableStyle::UseBandingRowStyles)) {
                cellStyle = *(documentLayout->styleManager()->tableCellStyle(tableTemplate->oddRows()));
                return cellStyle;
            }

            if (((tableCell.column() + 1) % 2) == 0 && tableTemplate->evenColumns()
                    && tableFormat.boolProperty(KoTableStyle::UseBandingColumnStyles)) {
                cellStyle = *(documentLayout->styleManager()->tableCellStyle(tableTemplate->evenColumns()));
                return cellStyle;
            }

            if (((tableCell.column() + 1) % 2) != 0 && tableTemplate->oddColumns()
                    && tableFormat.boolProperty(KoTableStyle::UseBandingColumnStyles)) {
                cellStyle = *(documentLayout->styleManager()->tableCellStyle(tableTemplate->oddColumns()));
                return cellStyle;
            }

            if (tableTemplate->body()) {
                cellStyle = *(documentLayout->styleManager()->tableCellStyle(tableTemplate->body()));
            }
        }
    }

    return cellStyle;
}


KoTextLayoutTableArea::KoTextLayoutTableArea(QTextTable *table, KoTextLayoutArea *parent, KoTextDocumentLayout *documentLayout)
  : KoTextLayoutArea(parent, documentLayout)
  , d(new Private)
{
    Q_ASSERT(table);
    Q_ASSERT(parent);

    d->table = table;
    d->documentLayout = documentLayout;
    d->carsManager = KoTableColumnAndRowStyleManager::getManager(table);

    // Resize geometry vectors for the table.
    d->rowPositions.resize(table->rows() + 1);
    d->headerRowPositions.resize(table->rows() + 1);
    d->cellAreas.resize(table->rows());
    for (int row = 0; row < table->rows(); ++row) {
        d->cellAreas[row].resize(table->columns());
    }
    KoTableStyle tableStyle(d->table->format());
    d->collapsing = tableStyle.collapsingBorderModel();
}

KoTextLayoutTableArea::~KoTextLayoutTableArea()
{
    for (int row = d->startOfArea->row; row < d->cellAreas.size(); ++row) {
        for (int col = 0; col < d->cellAreas[row].size(); ++col) {
            delete d->cellAreas[row][col];
        }
    }
    delete d->startOfArea;
    delete d->endOfArea;
    delete d;
}

KoPointedAt KoTextLayoutTableArea::hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const
{
    int firstRow = qMax(d->startOfArea->row, d->headerRows);
    int lastRow = d->endOfArea->row;

    if (d->lastRowHasSomething == false) {
        --lastRow;
    }

    if (lastRow <  d->startOfArea->row) {
        return KoPointedAt(); // empty
    }

    // Test normal cells.
    if (point.y() > d->rowPositions[firstRow] - 3.0 && point.y() < d->rowPositions[lastRow + 1] + 3.0) {
        QVector<qreal>::const_iterator start = d->rowPositions.constBegin() + firstRow;
        QVector<qreal>::const_iterator end = d->rowPositions.constBegin() + lastRow + 1;
        int row = qLowerBound(start, end, point.y()) - d->rowPositions.constBegin() - 1;
        int column = qLowerBound(d->columnPositions, point.x()) - d->columnPositions.constBegin() - 1;
        if (point.y() < d->rowPositions[firstRow]) {
            ++row;
        }
        column = qBound(0, column, d->table->columns() - 1);
        KoPointedAt pointedAt;
        if (qAbs(d->columnPositions[column] - point.x()) < 3.0) {
            pointedAt.tableHit = KoPointedAt::ColumnDivider;
        } else if (qAbs(d->columnPositions[column+1] - point.x()) < 3.0) {
            pointedAt.tableHit = KoPointedAt::ColumnDivider;
            ++column;
        } else if (d->columnPositions[0] < point.x()
                            && point.x() < d->columnPositions[d->table->columns()]
                            && qAbs(d->rowPositions[row] - point.y()) < 3.0) {
            pointedAt.tableHit = KoPointedAt::RowDivider;
        } else if (d->columnPositions[0] < point.x()
                            && point.x() < d->columnPositions[d->table->columns()]
                            && qAbs(d->rowPositions[row+1] - point.y()) < 3.0) {
            pointedAt.tableHit = KoPointedAt::RowDivider;
            ++row;
        } else {
            QTextTableCell cell = d->table->cellAt(row, column);
            pointedAt = d->cellAreas[cell.row()][cell.column()]->hitTest(point, accuracy);
        }

        if (pointedAt.tableHit == KoPointedAt::ColumnDivider) {
            if (column > 0) {
                pointedAt.tableLeadSize = d->columnPositions[column] - d->columnPositions[column-1];
            }
            if (column < d->table->columns()) {
                pointedAt.tableTrailSize = d->columnPositions[column+1] - d->columnPositions[column];
            }
        } else if (pointedAt.tableHit == KoPointedAt::RowDivider) {
            if (row > 0) {
                pointedAt.tableLeadSize = d->rowPositions[row] - d->rowPositions[row-1];
            }
            if (row < d->table->rows()) {
                pointedAt.tableTrailSize = d->rowPositions[row+1] - d->rowPositions[row];
            }
        }
        pointedAt.table = d->table;
        pointedAt.tableRowDivider = row;
        pointedAt.tableColumnDivider = column;
        pointedAt.tableDividerPos = QPointF(d->columnPositions[column],d->rowPositions[row]);
        return pointedAt;
    }

    // Test header row cells.
    QPointF headerPoint = point - QPointF(d->headerOffsetX, d->headerOffsetY);
    if (headerPoint.y() > d->headerRowPositions.first() && headerPoint.y() < d->headerRowPositions[d->headerRows]) {
        QVector<qreal>::const_iterator start = d->headerRowPositions.constBegin();
        QVector<qreal>::const_iterator end = d->headerRowPositions.constBegin() + d->headerRows;
        int row = qLowerBound(start, end, headerPoint.y()) - d->headerRowPositions.constBegin() - 1;
        int column = qLowerBound(d->columnPositions, headerPoint.x()) - d->columnPositions.constBegin() - 1;
        column = qBound(0, column, d->table->columns() - 1);
        KoPointedAt pointedAt;
        if (qAbs(d->columnPositions[column] - headerPoint.x()) < 3.0) {
            pointedAt.tableHit = KoPointedAt::ColumnDivider;
        } else if (qAbs(d->columnPositions[column+1] - headerPoint.x()) < 3.0) {
            pointedAt.tableHit = KoPointedAt::ColumnDivider;
            ++column;
        } else {
            QTextTableCell cell = d->table->cellAt(row, column);
            pointedAt = d->cellAreas[cell.row()][cell.column()]->hitTest(headerPoint, accuracy);
        }
        if (pointedAt.tableHit == KoPointedAt::ColumnDivider) {
            if (column > 0) {
                pointedAt.tableLeadSize = d->columnPositions[column] - d->columnPositions[column-1];
            }
            if (column < d->table->columns()) {
                pointedAt.tableTrailSize = d->columnPositions[column+1] - d->columnPositions[column];
            }
        }
        pointedAt.table = d->table;
        pointedAt.tableRowDivider = row;
        pointedAt.tableColumnDivider = column;
        pointedAt.tableDividerPos = QPointF(d->columnPositions[column],d->rowPositions[row]);
        return pointedAt;
    }

    return KoPointedAt();
}

QRectF KoTextLayoutTableArea::selectionBoundingBox(QTextCursor &cursor) const
{
    int lastRow = d->endOfArea->row;
    if (d->lastRowHasSomething == false) {
        --lastRow;
    }
    if (lastRow <  d->startOfArea->row) {
        return QRectF(); // empty
    }

    int firstRow = qMax(d->startOfArea->row, d->headerRows);
    QTextTableCell startTableCell = d->table->cellAt(cursor.selectionStart());
    QTextTableCell endTableCell = d->table->cellAt(cursor.selectionEnd());

    if (startTableCell == endTableCell) {
        if (startTableCell.row() < d->startOfArea->row || startTableCell.row() > lastRow) {
            return QRectF(); // cell is not in this area
        }
        KoTextLayoutArea *area = d->cellAreas[startTableCell.row()][startTableCell.column()];
        Q_ASSERT(area);
        return area->selectionBoundingBox(cursor);
    } else {
        int selectionRow;
        int selectionColumn;
        int selectionRowSpan;
        int selectionColumnSpan;
        cursor.selectedTableCells(&selectionRow, &selectionRowSpan, &selectionColumn, &selectionColumnSpan);

        qreal top, bottom;

        if (selectionRow < d->headerRows) {
            top = d->headerRowPositions[selectionRow] + d->headerOffsetY;
        } else {
            top = d->rowPositions[qMin(qMax(firstRow, selectionRow), lastRow)];
        }

        if (selectionRow + selectionRowSpan < d->headerRows) {
            bottom = d->headerRowPositions[selectionRow + selectionRowSpan] + d->headerOffsetY;
        } else {
            bottom = d->rowPositions[d->headerRows] + d->headerOffsetY;
            if (selectionRow + selectionRowSpan >= firstRow) {
                bottom = d->rowPositions[qMin(selectionRow + selectionRowSpan, lastRow + 1)];
            }
        }

        return QRectF(d->columnPositions[selectionColumn], top,
                      d->columnPositions[selectionColumn + selectionColumnSpan] - d->columnPositions[selectionColumn], bottom - top);
    }
}

bool KoTextLayoutTableArea::layoutTable(TableIterator *cursor)
{
    d->startOfArea = new TableIterator(cursor);
    d->headerRows = cursor->headerRows;
    d->totalMisFit = false;

    // If table is done we create an empty area and return true
    if (cursor->row == d->table->rows()) {
        setBottom(top());
        d->endOfArea = new TableIterator(cursor);
        return true;
    }
    layoutColumns();

    bool first = cursor->row == 0 && (d->cellAreas[0][0] == 0);
    if (first) { // are we at the beginning of the table
        cursor->row = 0;
        d->rowPositions[0] = top() + d->table->format().topMargin();
        d->headerOffsetX = 0;
        d->headerOffsetY = 0;
    } else {
        for (int row = 0; row < d->headerRows; ++row) {
            // Copy header rows
            d->headerRowPositions[row] = cursor->headerRowPositions[row];
            for (int col = 0; col < d->table->columns(); ++col) {
                d->cellAreas[row][col] = cursor->headerCellAreas[row][col];
            }
        }

        if (d->headerRows) {
            // Also set the position of the border below headers
            d->headerRowPositions[d->headerRows] = cursor->headerRowPositions[d->headerRows];
        }

        // If headerRows == 0 then the following reduces to: d->rowPositions[cursor->row] = top()
        d->headerOffsetY = top() - d->headerRowPositions[0];
        d->rowPositions[cursor->row] = d->headerRowPositions[d->headerRows] + d->headerOffsetY;

        // headerOffsetX should also be set
        d->headerOffsetX = d->columnPositions[0] - cursor->headerPositionX;
    }

    bool complete = first;
    qreal topBorderWidth = 0;
    qreal bottomBorderWidth = 0;
    qreal dummyWidth = 0;

    collectBorderThicknesss(cursor->row - 1, dummyWidth, topBorderWidth);
    collectBorderThicknesss(cursor->row, topBorderWidth, bottomBorderWidth);
    do {
        qreal nextBottomBorderWidth = 0;
        collectBorderThicknesss(cursor->row+1, bottomBorderWidth, nextBottomBorderWidth);

        d->lastRowHasSomething = false;

        complete = layoutRow(cursor, topBorderWidth, bottomBorderWidth);

        setBottom(d->rowPositions[cursor->row + 1] + bottomBorderWidth);
        topBorderWidth = bottomBorderWidth;
        bottomBorderWidth = nextBottomBorderWidth;


        if (complete) {
            setVirginPage(false);
            cursor->row++;
        }
    } while (complete && cursor->row < d->table->rows());

    if (cursor->row == d->table->rows()) {
        d->lastRowHasSomething = false;
    }


    if (first) { // were we at the beginning of the table
        for (int row = 0; row < d->headerRows; ++row) {
            // Copy header rows
            cursor->headerRowPositions[row] = d->rowPositions[row];
            d->headerRowPositions[row] = d->rowPositions[row];
            for (int col = 0; col < d->table->columns(); ++col) {
                cursor->headerCellAreas[row][col] = d->cellAreas[row][col];
            }
        }
        if (d->headerRows) {
            // Also set the position of the border below headers
            cursor->headerRowPositions[d->headerRows] = d->rowPositions[d->headerRows];
            d->headerRowPositions[d->headerRows] = d->rowPositions[d->headerRows];
        }
        cursor->headerPositionX = d->columnPositions[0];

        if (!virginPage() && d->totalMisFit) {
            //if we couldn't fit the header rows plus some then don't even try
            cursor->row = 0;
            nukeRow(cursor);
        }
    }

    d->endOfArea = new TableIterator(cursor);

    return complete;
}


void KoTextLayoutTableArea::layoutColumns()
{
    QTextTableFormat tableFormat = d->table->format();

    d->columnPositions.resize(d->table->columns() + 1);
    d->columnWidths.resize(d->table->columns() + 1);

    // Table width.
    d->tableWidth = 0;
    qreal parentWidth = right() - left();
    if (tableFormat.width().rawValue() == 0 || tableFormat.alignment() == Qt::AlignJustify) {
        // We got a zero width value or alignment is justify, so use 100% of parent.
        d->tableWidth = parentWidth - tableFormat.leftMargin() - tableFormat.rightMargin();
    } else {
        if (tableFormat.width().type() == QTextLength::FixedLength) {
            // Fixed length value, so use the raw value directly.
            d->tableWidth = tableFormat.width().rawValue();
        } else if (tableFormat.width().type() == QTextLength::PercentageLength) {
            // Percentage length value, so use a percentage of parent width.
            d->tableWidth = tableFormat.width().rawValue() * (parentWidth / 100)
                - tableFormat.leftMargin() - tableFormat.rightMargin();
        } else {
            // Unknown length type, so use 100% of parent.
            d->tableWidth = parentWidth - tableFormat.leftMargin() - tableFormat.rightMargin();
        }
    }

    // Column widths.
    qreal availableWidth = d->tableWidth; // Width available for columns.
    QList<int> fixedWidthColumns; // List of fixed width columns.
    QList<int> relativeWidthColumns; // List of relative width columns.
    qreal relativeWidthSum = 0; // Sum of relative column width values.
    int numNonStyleColumns = 0;
    for (int col = 0; col < d->table->columns(); ++col) {
        KoTableColumnStyle columnStyle = d->carsManager.columnStyle(col);

        if (columnStyle.hasProperty(KoTableColumnStyle::RelativeColumnWidth)) {
            // Relative width specified. Will be handled in the next loop.
            d->columnWidths[col] = 0.0;
            relativeWidthColumns.append(col);
            relativeWidthSum += columnStyle.relativeColumnWidth();
        } else if (columnStyle.hasProperty(KoTableColumnStyle::ColumnWidth)) {
            // Only width specified, so use it.
            d->columnWidths[col] = columnStyle.columnWidth();
            fixedWidthColumns.append(col);
            availableWidth -= columnStyle.columnWidth();
        } else {
            // Neither width nor relative width specified.
            d->columnWidths[col] = 0.0;
            relativeWidthColumns.append(col); // handle it as a relative width column without asking for anything
            ++numNonStyleColumns;
        }
    }

    // Handle the case that the fixed size columns are larger then the defined table width
    if (availableWidth < 0.0) {
        if (tableFormat.width().rawValue() == 0 && fixedWidthColumns.count() > 0) {
            // If not table width was defined then we need to scale down the fixed size columns so they match
            // into the width of the table.
            qreal diff = (-availableWidth) / qreal(fixedWidthColumns.count());
            Q_FOREACH (int col, fixedWidthColumns) {
                d->columnWidths[col] = qMax(qreal(0.0), d->columnWidths[col] - diff);
            }
        }
        availableWidth = 0.0;
    }

    // Calculate width to those columns that don't actually request it
    qreal widthForNonWidthColumn = ((1.0 - qMin<qreal>(relativeWidthSum, 1.0)) * availableWidth);
    availableWidth -= widthForNonWidthColumn; // might as well do this calc before dividing by numNonStyleColumns
    if (numNonStyleColumns > 0 && widthForNonWidthColumn > 0.0) {
        widthForNonWidthColumn /= numNonStyleColumns;
    }

    // Relative column widths have now been summed up and can be distributed.
    foreach (int col, relativeWidthColumns) {
        KoTableColumnStyle columnStyle = d->carsManager.columnStyle(col);
        if (columnStyle.hasProperty(KoTableColumnStyle::RelativeColumnWidth) || columnStyle.hasProperty(KoTableColumnStyle::ColumnWidth)) {
            d->columnWidths[col] = qMax<qreal>(columnStyle.relativeColumnWidth() * availableWidth / relativeWidthSum, 0.0);
        } else {
            d->columnWidths[col] = widthForNonWidthColumn;
        }
    }

    // Column positions.
    qreal columnPosition = left();
    qreal columnOffset = tableFormat.leftMargin();
    if (tableFormat.alignment() == Qt::AlignRight) {
        // Table is right-aligned, so add all of the remaining space.
        columnOffset += parentWidth - d->tableWidth;
    }
    if (tableFormat.alignment() == Qt::AlignHCenter) {
        // Table is centered, so add half of the remaining space.
        columnOffset += (parentWidth - d->tableWidth) / 2;
    }
    for (int col = 0; col < d->columnPositions.size(); ++col) {
        d->columnPositions[col] = columnPosition + columnOffset;
        // Increment by this column's width.
        columnPosition += d->columnWidths[col];
    }

    // Borders can be outside of the cell (outer-borders) in which case it's need
    // to take them into account to not cut content off.
    qreal leftBorder = 0.0;
    qreal rightBorder = 0.0;
    for (int row = 0; row < d->table->rows(); ++row) {
        QTextTableCell leftCell = d->table->cellAt(row, 0);
        KoTableCellStyle leftCellStyle = d->effectiveCellStyle(leftCell);
        leftBorder = qMax(leftBorder, leftCellStyle.leftOuterBorderWidth());

        QTextTableCell rightCell = d->table->cellAt(row, d->table->columns() - 1);
        KoTableCellStyle rightCellStyle = d->effectiveCellStyle(rightCell);
        rightBorder = qMax(rightBorder, rightCellStyle.rightOuterBorderWidth());
    }

    expandBoundingLeft(d->columnPositions[0] - leftBorder);
    expandBoundingRight(d->columnPositions[d->table->columns()] + rightBorder + leftBorder);
}

void KoTextLayoutTableArea::collectBorderThicknesss(int row, qreal &topBorderWidth, qreal &bottomBorderWidth)
{
    int col = 0;

    if (d->collapsing && row >= 0 && row < d->table->rows()) {
        // let's collect the border info
        while (col < d->table->columns()) {
            QTextTableCell cell = d->table->cellAt(row, col);

            if (row == cell.row() + cell.rowSpan() - 1) {
                /*
                * This cell ends vertically in this row, and hence should
                * contribute to the bottom border.
                */
                KoTableCellStyle cellStyle = d->effectiveCellStyle(cell);

                topBorderWidth = qMax(cellStyle.topBorderWidth(), topBorderWidth);
                bottomBorderWidth = qMax(cellStyle.bottomBorderWidth(), bottomBorderWidth);
            }
            col += cell.columnSpan(); // Skip across column spans.
        }
    }
}

void KoTextLayoutTableArea::nukeRow(TableIterator *cursor)
{
    for (int column = 0; column < d->table->columns(); ++column) {
        delete d->cellAreas[cursor->row][column];
        d->cellAreas[cursor->row][column] = 0;
        delete cursor->frameIterators[column];
        cursor->frameIterators[column] = 0;
    }
    d->lastRowHasSomething = false;
}

bool KoTextLayoutTableArea::layoutRow(TableIterator *cursor, qreal topBorderWidth, qreal bottomBorderWidth)
{
    int row = cursor->row;

    Q_ASSERT(row >= 0);
    Q_ASSERT(row < d->table->rows());

    QTextTableFormat tableFormat = d->table->format();

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

    KoTableRowStyle rowStyle = d->carsManager.rowStyle(row);
    qreal rowHeight = rowStyle.rowHeight();
    bool rowHasExactHeight = rowStyle.hasProperty(KoTableRowStyle::RowHeight);
    qreal rowBottom;
    if (rowHasExactHeight) {
        rowBottom = d->rowPositions[row] + rowHeight;
    } else {
        rowBottom = d->rowPositions[row] + rowStyle.minimumRowHeight();
    }

    if (rowBottom > maximumAllowedBottom()) {
        d->rowPositions[row+1] = d->rowPositions[row];
        if (cursor->row > d->startOfArea->row) {
            cursor->row--;
            layoutMergedCellsNotEnding(cursor, topBorderWidth, bottomBorderWidth, rowBottom);
            cursor->row++;
        }
        return false; // we can't honour minimum or fixed height so don't even try
    }

    bool allCellsFullyDone = true;
    bool anyCellTried = false;
    bool noCellsFitted = true;
    int col = 0;
    while (col < d->table->columns()) {
        // Get the cell format.
        QTextTableCell cell = d->table->cellAt(row, col);

        if (row == cell.row() + cell.rowSpan() - 1) {
            /*
             * This cell ends vertically in this row, and hence should
             * contribute to the row height.
             */
            bool ignoreMisFittingCell = false;
            KoTableCellStyle cellStyle = d->effectiveCellStyle(cell);
            anyCellTried = true;

            qreal maxBottom = maximumAllowedBottom();

            qreal requiredRowHeight = cellStyle.bottomPadding() + cellStyle.bottomPadding();

            if (rowHasExactHeight) {
                maxBottom = qMin(d->rowPositions[row] + rowHeight, maxBottom);
            }
            maxBottom -= cellStyle.bottomPadding();

            qreal areaTop = d->rowPositions[qMax(cell.row(), d->startOfArea->row)] + cellStyle.topPadding();

            if (d->collapsing) {
                areaTop += topBorderWidth;
                maxBottom -= bottomBorderWidth;
                requiredRowHeight += bottomBorderWidth + topBorderWidth;
            } else {
                areaTop += cellStyle.topBorderWidth();
                maxBottom -= cellStyle.bottomBorderWidth();
                requiredRowHeight += cellStyle.bottomBorderWidth() + cellStyle.topBorderWidth();
            }

            if (rowHasExactHeight && (rowHeight < requiredRowHeight))
            {
                ignoreMisFittingCell = true;
            }

            if (maxBottom < areaTop && !ignoreMisFittingCell) {
                d->rowPositions[row+1] = d->rowPositions[row];
                nukeRow(cursor);
                if (cursor->row > d->startOfArea->row) {
                    cursor->row--;
                    layoutMergedCellsNotEnding(cursor, topBorderWidth, bottomBorderWidth, rowBottom);
                    cursor->row++;
                }
                return false; // we can't honour the borders so give up doing row
            }

            KoTextLayoutArea *cellArea = new KoTextLayoutArea(this, documentLayout());
            d->cellAreas[cell.row()][cell.column()] = cellArea;

            qreal left = d->columnPositions[col] + cellStyle.leftPadding() + cellStyle.leftInnerBorderWidth();
            qreal right = qMax(left, d->columnPositions[col+cell.columnSpan()] - cellStyle.rightPadding() - cellStyle.rightInnerBorderWidth());

            cellArea->setReferenceRect(
                    left,
                    right,
                    areaTop,
                    maxBottom);

            cellArea->setVirginPage(virginPage());
            cellArea->setLayoutEnvironmentResctictions(true, true);

            FrameIterator *cellCursor = cursor->frameIterator(col);

            bool cellFully = cellArea->layout(cellCursor);
            allCellsFullyDone = allCellsFullyDone && (cellFully || rowHasExactHeight);

            noCellsFitted = noCellsFitted && (cellArea->top() >= cellArea->bottom()) && !ignoreMisFittingCell;

            if (!rowHasExactHeight) {
                /*
                 * Now we know how much height this cell contributes to the row,
                 * and can determine wheather the row height will grow.
                 */
                if (d->collapsing) {
                    rowBottom = qMax(cellArea->bottom() + cellStyle.bottomPadding(), rowBottom);
                } else {
                    rowBottom = qMax(cellArea->bottom() + cellStyle.bottomPadding() + cellStyle.bottomBorderWidth(), rowBottom);
                }
                rowBottom = qMax(rowBottom, documentLayout()->maxYOfAnchoredObstructions(cell.firstCursorPosition().position(), cell.lastCursorPosition().position()));
            }


            d->lastRowHasSomething = true; // last row contains something (even if empty)
        }

        col += cell.columnSpan(); // Skip across column spans.

    }

    if (allCellsFullyDone) {
        for (col = 0; col < d->table->columns(); ++col) {
            QTextTableCell cell = d->table->cellAt(row, col);

            if (row == cell.row() + cell.rowSpan() - 1) {
                delete cursor->frameIterators[col];
                cursor->frameIterators[col] = 0;
            }
        }
    }

    if (noCellsFitted && row <= d->headerRows) {
        d->totalMisFit = true;
    }

    if (anyCellTried && noCellsFitted && !rowHasExactHeight && !allCellsFullyDone) {
        d->rowPositions[row+1] = d->rowPositions[row];
        nukeRow(cursor);
        if (cursor->row > d->startOfArea->row) {
            cursor->row--;
            layoutMergedCellsNotEnding(cursor, topBorderWidth, bottomBorderWidth, rowBottom);
            cursor->row++;
        }
        return false; // we can't honour the anything inside so give up doing row
    }

    if (!allCellsFullyDone) {
        layoutMergedCellsNotEnding(cursor, topBorderWidth, bottomBorderWidth, rowBottom);
    } else {
        // Cells all ended naturally, so we can now do vertical alignment
        // Stop! Other odf implementors also only do it if all cells are fully done
        col = 0;
        while (col < d->table->columns()) {
            QTextTableCell cell = d->table->cellAt(row, col);

            if (row == cell.row() + cell.rowSpan() - 1) {
                // cell ended in this row
                KoTextLayoutArea *cellArea = d->cellAreas[cell.row()][cell.column()];
                KoTableCellStyle cellStyle = d->effectiveCellStyle(cell);

                if (cellStyle.alignment() & Qt::AlignBottom) {
                    if (true /*FIXME test no page based shapes interfering*/) {
                        cellArea->setVerticalAlignOffset(rowBottom - cellArea->bottom());
                    }
                }
                if (cellStyle.alignment() & Qt::AlignVCenter) {
                    if (true /*FIXME test no page based shapes interfering*/) {
                        cellArea->setVerticalAlignOffset((rowBottom - cellArea->bottom()) / 2);
                    }
                }
            }
            col += cell.columnSpan(); // Skip across column spans.
        }
    }

    // Adjust Y position of NEXT row.
    // This is nice since the outside layout routine relies on the next row having a correct y position
    // the first row y position is set in layout()
    d->rowPositions[row+1] = rowBottom;

    return allCellsFullyDone;
}

bool KoTextLayoutTableArea::layoutMergedCellsNotEnding(TableIterator *cursor, qreal topBorderWidth, qreal bottomBorderWidth, qreal rowBottom)
{
    Q_UNUSED(topBorderWidth)
    Q_UNUSED(bottomBorderWidth)

    // Let's make sure all merged cells in this row, that don't end in this row get's a layout
    int row = cursor->row;
    int col = 0;
    while (col < d->table->columns()) {
        QTextTableCell cell = d->table->cellAt(row, col);
        if (row != cell.row() + cell.rowSpan() - 1) {
        // TODO do all of the following like in layoutRow()
            KoTableCellStyle cellStyle = d->effectiveCellStyle(cell);

            KoTextLayoutArea *cellArea = new KoTextLayoutArea(this, documentLayout());

            d->cellAreas[cell.row()][cell.column()] = cellArea;

            qreal left = d->columnPositions[col] + cellStyle.leftPadding() + cellStyle.leftInnerBorderWidth();
            qreal right = qMax(left, d->columnPositions[col+cell.columnSpan()] - cellStyle.rightPadding() - cellStyle.rightInnerBorderWidth());

            cellArea->setReferenceRect(
                    left,
                    right,
                    d->rowPositions[qMax(cell.row(), d->startOfArea->row)] + cellStyle.topPadding() + cellStyle.topBorderWidth(),
                    rowBottom - cellStyle.bottomPadding() - cellStyle.bottomBorderWidth());

            cellArea->setVirginPage(virginPage());
            cellArea->setLayoutEnvironmentResctictions(true, true);

            FrameIterator *cellCursor =  cursor->frameIterator(col);

            cellArea->layout(cellCursor);
            if (cellArea->top() < cellArea->bottom() && row == d->headerRows) {
                d->totalMisFit = false;
            }
        }
        col += cell.columnSpan(); // Skip across column spans.
    }
    return true;
}

void KoTextLayoutTableArea::paint(QPainter *painter, const KoTextDocumentLayout::PaintContext &context)
{
    if (d->startOfArea == 0) // We have not been layouted yet
        return;

    int lastRow = d->endOfArea->row;
    if (d->lastRowHasSomething == false) {
        --lastRow;
    }
    if (lastRow <  d->startOfArea->row) {
        return; // empty
    }

    int firstRow = qMax(d->startOfArea->row, d->headerRows);

    // Draw table background
    qreal topY = d->headerRows ? d->rowPositions[0] : d->rowPositions[firstRow];
    QRectF tableRect(d->columnPositions[0], topY, d->tableWidth,
                     d->headerRowPositions[d->headerRows] - d->headerRowPositions[0]
                     + d->rowPositions[lastRow+1] - d->rowPositions[firstRow]);

    painter->fillRect(tableRect, d->table->format().background());

    KoTextDocumentLayout::PaintContext cellContext = context;
    QColor tableBackground = context.background;
    if (d->table->format().hasProperty(QTextFormat::BackgroundBrush)) {
        tableBackground = d->table->format().background().color();
    }

    // Draw header row backgrounds
    for (int row = 0; row < d->headerRows; ++row) {
        QRectF rowRect(d->columnPositions[0], d->headerRowPositions[row], d->tableWidth, d->headerRowPositions[row+1] - d->headerRowPositions[row]);

        KoTableRowStyle rowStyle = d->carsManager.rowStyle(row);

        rowRect.translate(0, d->headerOffsetY);

        painter->fillRect(rowRect, rowStyle.background());
    }

    // Draw plain row backgrounds
    for (int row = firstRow; row <= lastRow; ++row) {
        QRectF rowRect(d->columnPositions[0], d->rowPositions[row], d->tableWidth, d->rowPositions[row+1] - d->rowPositions[row]);

        KoTableRowStyle rowStyle = d->carsManager.rowStyle(row);

        painter->fillRect(rowRect, rowStyle.background());
    }

    QSet<QPair<int, int> > visitedCells;
    // Draw cell backgrounds and contents.
    for (int row = firstRow; row <= lastRow; ++row) {
        for (int column = 0; column < d->table->columns(); ++column) {
            QTextTableCell tableCell = d->table->cellAt(row, column);

            int testRow = (row == firstRow ? tableCell.row() : row);
            if (d->cellAreas[testRow][column] && !visitedCells.contains(QPair<int, int>(testRow, column))) {
                cellContext.background = tableBackground;
                QBrush bgBrush = d->effectiveCellStyle(tableCell).background();
                if (bgBrush != Qt::NoBrush) {
                    cellContext.background = bgBrush.color();
                }
                paintCell(painter, cellContext, tableCell, d->cellAreas[testRow][column]);
                visitedCells.insert(QPair<int, int>(testRow, column));
            }
        }
    }

    painter->translate(0, d->headerOffsetY);

    QVector<QLineF> accuBlankBorders;

    bool hasAntialiasing = painter->testRenderHint(QPainter::Antialiasing);

    // Draw header row cell backgrounds and contents.
    for (int row = 0; row < d->headerRows; ++row) {
        for (int column = 0; column < d->table->columns(); ++column) {
            QTextTableCell tableCell = d->table->cellAt(row, column);

            int testRow = row == firstRow ? tableCell.row() : row;
            if (d->cellAreas[testRow][column]) {
                cellContext.background = tableBackground;
                QBrush bgBrush = d->effectiveCellStyle(tableCell).background();
                if (bgBrush != Qt::NoBrush) {
                    cellContext.background = bgBrush.color();
                }
                paintCell(painter, cellContext, tableCell, d->cellAreas[testRow][column]);
            }
        }
    }
    // Draw header row cell borders.(need to be second step so nabour cells don't overwrite)
    for (int row = 0; row < d->headerRows; ++row) {
        for (int column = 0; column < d->table->columns(); ++column) {
            QTextTableCell tableCell = d->table->cellAt(row, column);

            int testRow = row == firstRow ? tableCell.row() : row;
            if (d->cellAreas[testRow][column]) {
                painter->setRenderHint(QPainter::Antialiasing, true);
                paintCellBorders(painter, context, tableCell, false, lastRow, &accuBlankBorders);
                painter->setRenderHint(QPainter::Antialiasing, hasAntialiasing);
            }
        }
    }
    for (int i = 0; i < accuBlankBorders.size(); ++i) {
        accuBlankBorders[i].translate(0, d->headerOffsetY);
    }

    painter->translate(0, -d->headerOffsetY);

    // Draw cell borders.
    bool topRow = !d->headerRows && firstRow != 0; // are we top row in this area

    painter->setRenderHint(QPainter::Antialiasing, true);
    visitedCells.clear();
    for (int row = firstRow; row <= lastRow; ++row) {
        for (int column = 0; column < d->table->columns(); ++column) {
            QTextTableCell tableCell = d->table->cellAt(row, column);

            int testRow = row == firstRow ? tableCell.row() : row;
            if (d->cellAreas[testRow][column] && !visitedCells.contains(QPair<int, int>(testRow, column))) {
                paintCellBorders(painter, context, tableCell, topRow, lastRow, &accuBlankBorders);
                visitedCells.insert(QPair<int, int>(testRow, column));
            }
        }
        topRow = false;
    }

    painter->setRenderHint(QPainter::Antialiasing, hasAntialiasing);

    if (context.showTableBorders) {
        QPen pen(painter->pen());
        painter->setPen(QPen(QColor(0,0,0,96)));
        painter->drawLines(accuBlankBorders);
        painter->setPen(pen);
    }
}

void KoTextLayoutTableArea::paintCell(QPainter *painter, const KoTextDocumentLayout::PaintContext &context, const QTextTableCell &tableCell, KoTextLayoutArea *frameArea)
{
    int row = tableCell.row();
    int column = tableCell.column();

    // This is an actual cell we want to draw, and not a covered one.
    QRectF bRect(cellBoundingRect(tableCell));

    painter->save();
    painter->setClipRect(bRect, Qt::IntersectClip);

    // Possibly paint the background of the cell
    QBrush background(d->effectiveCellStyle(tableCell).background());
    if (background != Qt::NoBrush) {
        painter->fillRect(bRect, background);
    }

    // Possibly paint the selection of the entire cell
    if (context.showSelections) {
        Q_FOREACH (const QAbstractTextDocumentLayout::Selection & selection,   context.textContext.selections) {
            QTextTableCell startTableCell = d->table->cellAt(selection.cursor.selectionStart());
            QTextTableCell endTableCell = d->table->cellAt(selection.cursor.selectionEnd());

            if (startTableCell.isValid() && startTableCell != endTableCell) {
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
            } else if (selection.cursor.selectionStart()  < d->table->firstPosition()
                && selection.cursor.selectionEnd() > d->table->lastPosition()) {
                painter->fillRect(bRect, selection.format.background());
            }
        }
    }

    if (row < d->headerRows) {
        painter->translate(d->headerOffsetX, 0);
    }

    // Paint the content of the cellArea
    frameArea->paint(painter, context);

    painter->restore();
}

void KoTextLayoutTableArea::paintCellBorders(QPainter *painter, const KoTextDocumentLayout::PaintContext &context, const QTextTableCell &tableCell, bool topRow, int lastRow, QVector<QLineF> *accuBlankBorders)
{
    Q_UNUSED(context);

    int row = tableCell.row();
    int column = tableCell.column();

    // This is an actual cell we want to draw, and not a covered one.
    KoTableCellStyle cellStyle = d->effectiveCellStyle(tableCell);
    KoTextLayoutCellHelper cellStyleHelper(cellStyle);

    QRectF bRect = cellBoundingRect(tableCell);

    if (d->collapsing) {
        // First the horizontal borders
        if (row == 0) {
            cellStyleHelper.drawTopHorizontalBorder(*painter, bRect.x(), bRect.y(), bRect.width(), accuBlankBorders);
        }
        if (topRow && row != 0) {
            // in collapsing mode we need to also paint the top border of the area
            int c = column;
            while (c < column + tableCell.columnSpan()) {
                QTextTableCell tableCellAbove = d->table->cellAt(row - 1, c);
                QRectF aboveBRect = cellBoundingRect(tableCellAbove);
                qreal x = qMax(bRect.x(), aboveBRect.x());
                qreal x2 = qMin(bRect.right(), aboveBRect.right());
                KoTableCellStyle cellAboveStyle = d->effectiveCellStyle(tableCellAbove);
                KoTextLayoutCellHelper cellAboveStyleHelper(cellAboveStyle);
                cellAboveStyleHelper.drawSharedHorizontalBorder(*painter, cellStyle, x, bRect.y(), x2 - x, accuBlankBorders);
                c = tableCellAbove.column() + tableCellAbove.columnSpan();
            }
        }
        if (row + tableCell.rowSpan() == d->table->rows()) {
            // we hit the bottom of the table so just draw the bottom border
            cellStyleHelper.drawBottomHorizontalBorder(*painter, bRect.x(), bRect.bottom(), bRect.width(), accuBlankBorders);
        } else {
            int c = column;
            while (c < column + tableCell.columnSpan()) {
                QTextTableCell tableCellBelow = d->table->cellAt(row == d->headerRows - 1 ?
                            d->startOfArea->row : row + tableCell.rowSpan(), c);
                QRectF belowBRect = cellBoundingRect(tableCellBelow);
                qreal x = qMax(bRect.x(), belowBRect.x());
                qreal x2 = qMin(bRect.right(), belowBRect.right());
                KoTableCellStyle cellBelowStyle = d->effectiveCellStyle(tableCellBelow);
                cellStyleHelper.drawSharedHorizontalBorder(*painter, cellBelowStyle, x, bRect.bottom(), x2 - x, accuBlankBorders);
                c = tableCellBelow.column() + tableCellBelow.columnSpan();
            }
        }

        // And then the same treatment for vertical borders
        if (column == 0) {
            cellStyleHelper.drawLeftmostVerticalBorder(*painter, bRect.x(), bRect.y(), bRect.height() + cellStyle.bottomOuterBorderWidth(), accuBlankBorders);
        }
        if (column + tableCell.columnSpan() == d->table->columns()) {
            // we hit the rightmost edge of the table so draw the rightmost border
            cellStyleHelper.drawRightmostVerticalBorder(*painter, bRect.right(), bRect.y(), bRect.height() + cellStyle.bottomOuterBorderWidth(), accuBlankBorders);
        } else {
            // we have cells to the right so draw sharedborders
            int r = row;
            while (r < row + tableCell.rowSpan() && r <= lastRow) {
                QTextTableCell tableCellRight = d->table->cellAt(r, column + tableCell.columnSpan());
                KoTableCellStyle cellRightStyle = d->effectiveCellStyle(tableCellRight);
                QRectF rightBRect = cellBoundingRect(tableCellRight);
                qreal y = qMax(bRect.y(), rightBRect.y());
                qreal y2 = qMin(bRect.bottom() + cellStyle.bottomOuterBorderWidth(), rightBRect.bottom() + cellRightStyle.bottomOuterBorderWidth());
                cellStyleHelper.drawSharedVerticalBorder(*painter, cellRightStyle, bRect.right(), y, y2-y, accuBlankBorders);
                r = tableCellRight.row() + tableCellRight.rowSpan();
            }
        }

        // Paint diagonal borders for current cell
        cellStyleHelper.paintDiagonalBorders(*painter, bRect);
    } else { // separating border model
        cellStyleHelper.paintBorders(*painter, bRect, accuBlankBorders);
    }
}

QRectF KoTextLayoutTableArea::cellBoundingRect(const QTextTableCell &cell) const
{
    int row = cell.row();
    int rowSpan = cell.rowSpan();
    const int column = cell.column();
    const int columnSpan = cell.columnSpan();
    const qreal width = d->columnPositions[column + columnSpan] - d->columnPositions[column];

    if (row >= d->headerRows) {
        int lastRow = d->endOfArea->row;
        if (d->lastRowHasSomething == false) {
            --lastRow;
        }
        if (lastRow <  d->startOfArea->row) {
            return QRectF(); // empty
        }

        // Limit cell to within the area
        if (row < d->startOfArea->row) {
            rowSpan -= d->startOfArea->row - row;
            row += d->startOfArea->row - row;
        }
        if (row + rowSpan - 1 > lastRow) {
            rowSpan = lastRow - row + 1;
        }
        const qreal height = d->rowPositions[row + rowSpan] - d->rowPositions[row];
        return QRectF(d->columnPositions[column], d->rowPositions[row], width, height);
    } else {
        return QRectF(d->columnPositions[column], d->headerRowPositions[row], width, d->headerRowPositions[row + rowSpan] - d->headerRowPositions[row]);
    }
}
