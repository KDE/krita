/* This file is part of the KDE project
 * Copyright (C) 2011 Casper Boemann, KO GmbH <cbo@kogmbh.com>
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


#include <QTextTable>
#include <QVector>

class FrameIterator;
class KoTextLayoutArea;

class TableIterator
{
public:
    TableIterator(QTextTable *table);
    TableIterator(TableIterator *other);
    ~TableIterator();

    bool operator ==(const TableIterator &other);

    FrameIterator *frameIterator(int column);

    QTextTable *table;
    int row;
    int headerRows;
    qreal headerPositionX;
    QVector<FrameIterator *> frameIterators;
    QVector<qreal> headerRowPositions; // we will only fill those of header rows
    QVector<QVector<KoTextLayoutArea *> > headerCellAreas;
};

#endif
