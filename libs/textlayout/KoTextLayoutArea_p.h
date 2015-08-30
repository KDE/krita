/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008,2011 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2008 Roopesh Chander <roop@forwardbias.in>
 * Copyright (C) 2007-2008 Pierre Ducroquet <pinaraf@pinaraf.info>
 * Copyright (C) 2009-2011 KO GmbH <cbo@kogmbh.com>
 * Copyright (C) 2009-2011 C. Boemann <cbo@boemann.dk>
 * Copyright (C) 2010 Nandita Suri <suri.nandita@gmail.com>
 * Copyright (C) 2010 Ajay Pundhir <ajay.pratap@iiitb.net>
 * Copyright (C) 2011 Lukáš Tvrdý <lukas.tvrdy@ixonos.com>
 * Copyright (C) 2011 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
 * Copyright (C) 2011 Stuart Dickson <stuart@furkinfantasic.net>
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

#ifndef KOTEXTLAYOUTAREA_P_H
#define KOTEXTLAYOUTAREA_P_H

#include "KoTextLayoutTableArea.h"
#include "KoTextLayoutEndNotesArea.h"
#include "KoTextLayoutNoteArea.h"

#include <KoTextBlockBorderData.h>

//local type for temporary use in restartLayout
struct LineKeeper
{
    int columns;
    qreal lineWidth;
    QPointF position;
};


class Q_DECL_HIDDEN KoTextLayoutArea::Private
{
public:
    Private()
        : left(0.0)
        , right(0.0)
        , top(0.0)
        , bottom(0.0)
        , maximalAllowedBottom(0.0)
        , maximumAllowedWidth(0.0)
        , neededWidth(0.0)
        , isLayoutEnvironment(false)
        , actsHorizontally(false)
        , dropCapsWidth(0)
        , dropCapsDistance(0)
        , startOfArea(0)
        , endOfArea(0)
        , copyEndOfArea(0)
        , footNoteCursorToNext(0)
        , footNoteCursorFromPrevious(0)
        , continuedNoteToNext(0)
        , continuedNoteFromPrevious(0)
        , footNoteCountInDoc(0)
        , acceptsPageBreak(false)
        , acceptsColumnBreak(false)
        , virginPage(true)
        , verticalAlignOffset(0)
        , preregisteredFootNotesHeight(0)
        , footNotesHeight(0)
        , footNoteAutoCount(0)
        , extraTextIndent(0)
        , endNotesArea(0)
        {
        }

    KoTextLayoutArea *parent; //  A pointer to the parent

    KoTextDocumentLayout *documentLayout;

    qreal left; // reference area left
    qreal right; // reference area right
    qreal top; // reference area top
    qreal bottom; // reference area top
    qreal maximalAllowedBottom;
    qreal maximumAllowedWidth; // 0 indicates wrapping is allowed
    qreal neededWidth; // used in conjuntion with grow-text-width
    QRectF boundingRect;
    bool isLayoutEnvironment;
    bool actsHorizontally;
    KoTextBlockBorderData *prevBorder;
    qreal prevBorderPadding;

    qreal x; // text area starts here as defined by margins (so not == left)
    qreal y;
    qreal width; // of text area as defined by margins (so not == right - left)
    qreal indent;
    qreal dropCapsWidth;
    qreal dropCapsDistance;
    int dropCapsNChars;
    bool isRtl;
    qreal bottomSpacing;
    QList<KoTextLayoutTableArea *> tableAreas;
    FrameIterator *startOfArea;
    FrameIterator *endOfArea;
    FrameIterator *copyEndOfArea;
    FrameIterator *footNoteCursorToNext;
    FrameIterator *footNoteCursorFromPrevious;
    KoInlineNote *continuedNoteToNext;
    KoInlineNote *continuedNoteFromPrevious;
    int footNoteCountInDoc;

    bool acceptsPageBreak;
    bool acceptsColumnBreak;
    bool virginPage;
    qreal verticalAlignOffset;
    QList<QRectF> blockRects;
    qreal anchoringParagraphTop;
    qreal anchoringParagraphContentTop;

    qreal preregisteredFootNotesHeight;
    qreal footNotesHeight;
    int footNoteAutoCount;
    qreal extraTextIndent;
    QList<KoTextLayoutNoteArea *> preregisteredFootNoteAreas;
    QList<KoTextLayoutNoteArea *> footNoteAreas;
    QList<QTextFrame *> preregisteredFootNoteFrames;
    QList<QTextFrame *> footNoteFrames;
    KoTextLayoutEndNotesArea *endNotesArea;
    QList<KoTextLayoutArea *> generatedDocAreas;

    /// utility method to restart layout of a block
    QTextLine restartLayout(QTextBlock &block, int lineTextStartOfLastKeep);
    /// utility method to store remaining layout of a split block
    void stashRemainingLayout(QTextBlock &block, int lineTextStartOfFirstKeep, QList<LineKeeper> &stashedLines, QPointF &stashedCounterPosition);
    /// utility method to recreate partial layout of a split block
    QTextLine recreatePartialLayout(QTextBlock &block, QList<LineKeeper> stashedLines, QPointF &stashedCounterPosition, QTextLine &line);


};

#endif // KOTEXTLAYOUTAREA_P_H
