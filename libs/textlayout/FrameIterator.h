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
#ifndef FRAMEITERATOR_H
#define FRAMEITERATOR_H


#include <QTextFrame>
#include <QTextLine>
#include <QTextTableCell>

class TableIterator;
class QTextTable;
class KoTextLayoutTableArea;

class FrameIterator
{
public:
    FrameIterator(QTextFrame *frame);
    FrameIterator(QTextTableCell frame);
    FrameIterator(FrameIterator *other);

    bool operator ==(const FrameIterator &other);

    TableIterator *tableIterator(QTextTable *);
    FrameIterator *subFrameIterator(QTextFrame *);
    QString wantedMasterPage(const QString defaultName) const;

    QTextFrame::iterator it;
    // the following can be seen as the "sub cursor" of text blocks
    QTextLine line;   //useful if it is pointing to a table. invalid if about to start a new bloco
    QTextBlock::Iterator fragmentIterator;   //useful if it is pointing to a table
    TableIterator *currentTableIterator;  //useful if it is pointing to a table
    FrameIterator *currentSubFrameIterator;  //useful if it is pointing to a subFrame
};

#endif
