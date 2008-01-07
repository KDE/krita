/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
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
#ifndef TABLEROW_H
#define TABLEROW_H

#include <QObject>

class TableCell;

/**
 * A table row contains a row of cells. A row has a certain height,
 * which is the greatest hight of those cells in a row that are not merged
 * with cells below the current row.
 */
class TableRow : public QObject {

Q_OBJECT

public:

    enum Visibility {
        Visible,
        Collapse,
        Filter
    };

    /**
     * create a new table row.
     *
     * @param columns the number of columns this row will have
     */
    TableRow( int columns = 4 );

    virtual ~TableRow();

    TableRow(const TableRow & rhs);

    /**
     * create an empty cell in the specified position. The cell does
     * not have a shape and no contents. If no position is specified,
     * the cell is appended.
     */
    TableCell * createCell( int pos = -1);

    /**
     * Get a pointer to the cell at the specified position. If the position
     * is outside the range or the row is empty, return 0.
     */
    TableCell * cellAt( int pos );

    /**
     * remove the cell at the specified position. If the position
     * is outside the range or the row is empty, do nothing.
     */
    void removeCell( int pos );

    /// Set a soft page break -- if true, a page break is desired
    /// before the current row. (2.3.1, text:soft-page-break)
    void setSoftPageBreak(bool on);
    bool softPageBreak() const;

    /// Set the number of times this row must be displayed. See the
    /// odf spec, 8.1.2, number-rows-repeated. The default is 1,
    /// which means no repeat.
    void setRepeat(int repeat);
    int repeat() const;

    /// Set the visibility of this row as per 8.1.2, table:visibility
    void setVisibility(const QString & visibility);
    void setVisibility(Visibility visibility);
    Visibility visibility() const;

signals:

    void rowHeightChanged(float height);

private:

    class Private;
    Private * const d;
};

#endif
