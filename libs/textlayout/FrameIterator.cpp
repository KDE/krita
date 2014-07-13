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

#include "FrameIterator.h"

#include "TableIterator.h"

#include <QTextFrame>
#include <QTextTableCell>

FrameIterator::FrameIterator(QTextFrame *frame)
{
    it = frame->begin();
    currentTableIterator = 0;
    currentSubFrameIterator = 0;
    lineTextStart = -1;
    endNoteIndex = 0;
}

FrameIterator::FrameIterator(const QTextTableCell &cell)
{
    Q_ASSERT(cell.isValid());
    it = cell.begin();
    currentTableIterator = 0;
    currentSubFrameIterator = 0;
    lineTextStart = -1;
    endNoteIndex = 0;
}

FrameIterator::FrameIterator(FrameIterator *other)
{
    it = other->it;
    masterPageName = other->masterPageName;
    lineTextStart = other->lineTextStart;
    fragmentIterator = other->fragmentIterator;
    endNoteIndex = other->endNoteIndex;
    if (other->currentTableIterator)
        currentTableIterator = new TableIterator(other->currentTableIterator);
    else
        currentTableIterator = 0;

    if (other->currentSubFrameIterator)
        currentSubFrameIterator = new FrameIterator(other->currentSubFrameIterator);
    else
        currentSubFrameIterator = 0;
}

FrameIterator::~FrameIterator()
{
    delete currentTableIterator;
    delete currentSubFrameIterator;
}

bool FrameIterator::operator ==(const FrameIterator &other) const
{
    if (it != other.it)
        return false;
    if (endNoteIndex != other.endNoteIndex)
        return false;
    if (currentTableIterator || other.currentTableIterator) {
        if (!currentTableIterator || !other.currentTableIterator)
            return false;
        return *currentTableIterator == *(other.currentTableIterator);
    } else if (currentSubFrameIterator || other.currentSubFrameIterator) {
        if (!currentSubFrameIterator || !other.currentSubFrameIterator)
            return false;
        return *currentSubFrameIterator == *(other.currentSubFrameIterator);
    } else {
        return lineTextStart == other.lineTextStart;
    }
}

TableIterator *FrameIterator::tableIterator(QTextTable *table)
{
    if(table == 0) {
        delete currentTableIterator;
        currentTableIterator = 0;
    } else if(currentTableIterator == 0) {
        currentTableIterator = new TableIterator(table);
        currentTableIterator->masterPageName = masterPageName;
    }
    return currentTableIterator;
}

FrameIterator *FrameIterator::subFrameIterator(QTextFrame *subFrame)
{
    if(subFrame == 0) {
        delete currentSubFrameIterator;
        currentSubFrameIterator = 0;
    } else if(currentSubFrameIterator == 0) {
        currentSubFrameIterator = new FrameIterator(subFrame);
        currentSubFrameIterator->masterPageName = masterPageName;
    }
    return currentSubFrameIterator;
}
