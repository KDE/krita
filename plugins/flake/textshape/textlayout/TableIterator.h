/* This file is part of the KDE project
 * Copyright (C) 2011 C. Boemann, KO GmbH <cbo@kogmbh.com>
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
#ifndef TABLEITERATOR_H
#define TABLEITERATOR_H

#include <QVector>
#include <QString>

class FrameIterator;
class KoTextLayoutArea;
class QTextTable;

/**
 * Convenience cursor class used during table layout.
 *
 * The class holds information about a table and the current row. Can also
 * provide a FrameIterator for a given column in the table.
 */
class TableIterator
{
public:
    /**
     * Constructs a new iterator for the given table.
     *
     * @param table table to use.
     */
    explicit TableIterator(QTextTable *table);

    /**
     * Constructs a new iterator initialized from another.
     *
     * @param other iterator to initialize the iterator from.
     */
    explicit TableIterator(TableIterator *other);

    /// Destructor.
    ~TableIterator();

    /// Compare this iterator to another.
    bool operator ==(const TableIterator &other) const;

    /**
     * Returns a frame iterator that iterates over the frames in a given column.
     *
     * @param column column for which and iterator should be returned.
     */
    FrameIterator *frameIterator(int column);

    /// Table being iterated over.
    QTextTable *table;
    /// Current row.
    int row;
    /// Number of header rows.
    int headerRows;
    /// X position of header.
    qreal headerPositionX;
    /// Frame iterators, one for each column.
    QVector<FrameIterator *> frameIterators;
    /// Header row positions.
    QVector<qreal> headerRowPositions; // we will only fill those of header rows
    /// Rows of header cell areas.
    QVector<QVector<KoTextLayoutArea *> > headerCellAreas;
    /// The name of the master-page that is used for this table.
    QString masterPageName;
};

#endif
