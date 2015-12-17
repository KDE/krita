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
#ifndef FRAMEITERATOR_H
#define FRAMEITERATOR_H

#include <QTextFrame>

class TableIterator;
class QTextTableCell;
class QTextTable;

class FrameIterator
{
public:
    explicit FrameIterator(QTextFrame *frame);
    explicit FrameIterator(const QTextTableCell &frame);
    explicit FrameIterator(FrameIterator *other);
    ~FrameIterator();

    bool operator ==(const FrameIterator &other) const;

    TableIterator *tableIterator(QTextTable *);
    FrameIterator *subFrameIterator(QTextFrame *);

    QTextFrame::iterator it;

    QString masterPageName;

    // lineTextStart and fragmentIterator can be seen as the "sub cursor" of text blocks
    int lineTextStart; // a value of -1 indicate block not processed yet
    QTextBlock::Iterator fragmentIterator;

    TableIterator *currentTableIterator;  //useful if it is pointing to a table

    FrameIterator *currentSubFrameIterator;  //useful if it is pointing to a subFrame

    int endNoteIndex;
};

#endif
