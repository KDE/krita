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
#ifndef TABLECELL_H
#define TABLECELL_H

#include <QObject>
#include <QTextTableCell>

class KoShape;

/**
 * A TableCell represents a single cell in a TableShape table.
 * It contains a shape (of any kind) that represents content. A
 * cell has a fixed width but a variable height. If the height
 * of a cell changes because of content changes, the owning table
 * is notified that a re-layout of this row is needed.
 */
class TableCell : public QObject, public QTextTableCell {

    Q_OBJECT
    
public:

    TableCell();

    ~TableCell();

    TableCell(const TableCell & rhs);

    /**
     * @return the shape that contains the contents for this cell.
     */
    KoShape * shape() const;

    /**
     * Asks the cell to create a shape of a particular type and returns
     * it.
     *
     * @param shapeId the ID of the shape to get the factory from the factory
     *                registry
     * @return the newly created shape, or 0 if no shape could be created.
     */
    KoShape * createShape( const QString & shapeId );

signals:

    /**
     * Emitted whenever the height of this cell changes due to re-layouting
     * after content changes.
     */
    void heightChanged(TableCell *);

private:

    class Private;
    Private * const d;

};

#endif
