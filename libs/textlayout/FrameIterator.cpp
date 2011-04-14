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
#include "FrameIterator.h"

#include "TableIterator.h"

#include <KoParagraphStyle.h>

#include <QTextFrame>
#include <QTextTableCell>
#include <QTextLine>

FrameIterator::FrameIterator(QTextFrame *frame)
{
    it = frame->begin();
    currentTableIterator = 0;
    currentSubFrameIterator = 0;
}

FrameIterator::FrameIterator(QTextTableCell cell)
{
    it = cell.begin();
    currentTableIterator = 0;
    currentSubFrameIterator = 0;
}

FrameIterator::FrameIterator(FrameIterator *other)
{
    it = other->it;
    line = other->line;
    fragmentIterator = other->fragmentIterator;
    if (other->currentTableIterator)
        currentTableIterator = new TableIterator(other->currentTableIterator);
    else
        currentTableIterator = 0;

    if (other->currentSubFrameIterator)
        currentSubFrameIterator = new FrameIterator(other->currentSubFrameIterator);
    else
        currentSubFrameIterator = 0;
}

bool FrameIterator::operator ==(const FrameIterator &other)
{
    if (it != other.it)
        return false;

    if (currentTableIterator) {
        if (currentTableIterator != other.currentTableIterator)
            return false;
        return *currentTableIterator == *(other.currentTableIterator);
    } else {
        if (line.isValid() != other.line.isValid())
            return false;
        if (line.isValid() == false)
            return true; // both lines were invalid ie the same

        return line.textStart() == other.line.textStart();
    }
}

TableIterator *FrameIterator::tableIterator(QTextTable *table)
{
    if(table == 0) {
        delete currentTableIterator;
        currentTableIterator = 0;
    } else if(currentTableIterator == 0) {
        currentTableIterator = new TableIterator(table);
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
    }
    return currentSubFrameIterator;
}

QString FrameIterator::wantedMasterPage(const QString defaultName) const
{
    if (it.currentBlock().isValid()) {
        QVariant name = it.currentBlock().blockFormat().property(KoParagraphStyle::MasterPageName);
        if (name.isValid()) {
            return name.toString();
        }
    }

    return defaultName;
}
