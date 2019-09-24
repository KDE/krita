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

#include "KoTextLayoutArea.h"
#include "KoTextLayoutArea_p.h"

#include "TableIterator.h"
#include "ListItemsHelper.h"
#include "RunAroundHelper.h"
#include "KoTextDocumentLayout.h"
#include "FrameIterator.h"
#include "KoPointedAt.h"

#include <KoTextDocument.h>
#include <KoParagraphStyle.h>
#include <KoCharacterStyle.h>
#include <KoListStyle.h>
#include <KoTableStyle.h>
#include <KoStyleManager.h>
#include <KoTextBlockData.h>
#include <KoText.h>
#include <KoChangeTracker.h>
#include <KoChangeTrackerElement.h>
#include <KoInlineNote.h>
#include <KoTextSoftPageBreak.h>
#include <KoInlineTextObjectManager.h>

#include <TextLayoutDebug.h>

#include <QTextFrame>
#include <QTextTable>
#include <QTextList>
#include <QStyle>
#include <QFontMetrics>
#include <QTextFragment>
#include <QTextLayout>
#include <QTextCursor>

extern int qt_defaultDpiY();
Q_DECLARE_METATYPE(QTextDocument *)

#define DropCapsAdditionalFormattingId 25602902
#define PresenterFontStretch 1.2

KoTextLayoutArea::KoTextLayoutArea(KoTextLayoutArea *p, KoTextDocumentLayout *documentLayout)
 : d (new Private)
{
    d->parent = p;
    d->documentLayout = documentLayout;
}

KoTextLayoutArea::~KoTextLayoutArea()
{
    qDeleteAll(d->tableAreas);
    qDeleteAll(d->footNoteAreas);
    qDeleteAll(d->preregisteredFootNoteAreas);
    delete d->startOfArea;
    delete d->endOfArea;
    delete d;
}


KoPointedAt KoTextLayoutArea::hitTest(const QPointF &p, Qt::HitTestAccuracy accuracy) const
{
    QPointF point = p - QPointF(0, d->verticalAlignOffset);

    if (d->startOfArea == 0) // We have not been layouted yet
        return KoPointedAt();

    KoPointedAt pointedAt;
    bool basicallyFound = false;

    QTextFrame::iterator it = d->startOfArea->it;
    QTextFrame::iterator stop = d->endOfArea->it;
    if (!stop.atEnd()) {
        if(!stop.currentBlock().isValid() || d->endOfArea->lineTextStart >= 0) {
            // Last thing we contain is a frame (table) or first part of a paragraph split in two
            // The stop point should be the object after that
            // However if stop is already atEnd we shouldn't increment further
            ++stop;
        }
    }
    int tableAreaIndex = 0;
    int tocIndex = 0;
    int footNoteIndex = 0;
    for (; it != stop && !it.atEnd(); ++it) {
        QTextBlock block = it.currentBlock();
        QTextTable *table = qobject_cast<QTextTable*>(it.currentFrame());
        QTextFrame *subFrame = it.currentFrame();
        QTextBlockFormat format = block.blockFormat();

        if (table) {
            if (tableAreaIndex >= d->tableAreas.size()) {
                continue;
            }
            if (point.y() > d->tableAreas[tableAreaIndex]->top()
                    && point.y() < d->tableAreas[tableAreaIndex]->bottom()) {
                return d->tableAreas[tableAreaIndex]->hitTest(point, accuracy);
            }
            ++tableAreaIndex;
            continue;
        } else if (subFrame) {
            if (it.currentFrame()->format().intProperty(KoText::SubFrameType) == KoText::AuxillaryFrameType) {
                if (point.y() > d->endNotesArea->top()
                        && point.y() < d->endNotesArea->bottom()) {
                    pointedAt = d->endNotesArea->hitTest(point, accuracy);
                    return pointedAt;
                }
            }
            break;
        } else {
            if (!block.isValid())
                continue;
        }
        if (block.blockFormat().hasProperty(KoParagraphStyle::GeneratedDocument)) {
            // check if p is over table of content
            if (point.y() > d->generatedDocAreas[tocIndex]->top()
                    && point.y() < d->generatedDocAreas[tocIndex]->bottom()) {
                pointedAt = d->generatedDocAreas[tocIndex]->hitTest(point, accuracy);
                pointedAt.position = block.position();
                return pointedAt;
            }
            ++tocIndex;
            continue;
        }
        if (basicallyFound) // a subsequent table or lines have now had their chance
            return pointedAt;

        QTextLayout *layout = block.layout();
        QTextFrame::iterator next = it;
        ++next;
        if (next != stop && next.currentFrame() == 0 && point.y() > layout->boundingRect().bottom()) {
            // just skip this block.
            continue;
        }

        for (int i = 0; i < layout->lineCount(); i++) {
            QTextLine line = layout->lineAt(i);
            if (block == d->startOfArea->it.currentBlock() && line.textStart() < d->startOfArea->lineTextStart) {
                continue; // this line is part of a previous layoutArea
            }
            QRectF lineRect = line.naturalTextRect();
            if (point.y() > line.y() + line.height()) {
                pointedAt.position = block.position() + line.textStart() + line.textLength();
                if (block == d->endOfArea->it.currentBlock() && line.textStart() + line.textLength() >= d->endOfArea->lineTextStart) {
                    pointedAt.position = block.position() + line.xToCursor(point.x());
                    break; // this and following lines are part of a next layoutArea
                }
                continue;
            }
            if (accuracy == Qt::ExactHit && point.y() < line.y()) { // between lines
                return KoPointedAt();
            }
            if (accuracy == Qt::ExactHit && // left or right of line
                    (point.x() < line.naturalTextRect().left() || point.x() > line.naturalTextRect().right())) {
                return KoPointedAt();
            }
            if (point.x() > lineRect.x() + lineRect.width() && layout->textOption().textDirection() == Qt::RightToLeft) {
                // totally right of RTL text means the position is the start of the text.
                //TODO how about the other side?
                pointedAt.position = block.position() + line.textStart();
                return pointedAt;
            }
            if (basicallyFound && point.y() < lineRect.y()) {
                // This was not same baseline so basicallyFound was correct
                return pointedAt;
            }
            if (point.x() > lineRect.x() + lineRect.width()) {
                // right of line
                basicallyFound = true;
                pointedAt.position = block.position() + line.textStart() + line.textLength();
                continue; // don't break as next line may be on same baseline
            }
            pointedAt.position = block.position() + line.xToCursor(point.x());
            QTextCursor tmpCursor(block);
            tmpCursor.setPosition(block.position() + line.xToCursor(point.x(), QTextLine::CursorOnCharacter) + 1);
            pointedAt.fillInLinks(tmpCursor, d->documentLayout->inlineTextObjectManager(), d->documentLayout->textRangeManager());
            return pointedAt;
        }
    }

    //and finally test the footnotes
    point -= QPointF(0, bottom() - d->footNotesHeight);
    while (footNoteIndex < d->footNoteAreas.length()) {
        // check if p is over foot notes area
        if (point.y() > 0 && point.y() < d->footNoteAreas[footNoteIndex]->bottom()
                                                    - d->footNoteAreas[footNoteIndex]->top()) {
            pointedAt = d->footNoteAreas[footNoteIndex]->hitTest(point, accuracy);
            return pointedAt;
        }
        point -= QPointF(0, d->footNoteAreas[footNoteIndex]->bottom() - d->footNoteAreas[footNoteIndex]->top());
        ++footNoteIndex;
    }
    return pointedAt;
}

QRectF KoTextLayoutArea::selectionBoundingBox(QTextCursor &cursor) const
{
    QRectF retval(-5E6, top(), 105E6, 0);

    if (d->startOfArea == 0) // We have not been layouted yet
        return QRectF();
    if (d->endOfArea == 0) // no end area yet
        return QRectF();

    QTextFrame::iterator it = d->startOfArea->it;
    QTextFrame::iterator stop = d->endOfArea->it;
    if (!stop.atEnd()) {
        if(!stop.currentBlock().isValid() || d->endOfArea->lineTextStart >= 0) {
            // Last thing we show is a frame (table) or first part of a paragraph split in two
            // The stop point should be the object after that
            // However if stop is already atEnd we shouldn't increment further
            ++stop;
        }
    }

    QTextFrame *subFrame;
    int footNoteIndex = 0;
    qreal offset = bottom() - d->footNotesHeight;
    while (footNoteIndex < d->footNoteAreas.length()) {
        subFrame = d->footNoteFrames[footNoteIndex];
        if (cursor.selectionStart() >= subFrame->firstPosition() && cursor.selectionEnd() <= subFrame->lastPosition()) {
            return d->footNoteAreas[footNoteIndex]->selectionBoundingBox(cursor).translated(0, offset) ;
        }
        offset += d->footNoteAreas[footNoteIndex]->bottom() - d->footNoteAreas[footNoteIndex]->top();
        ++footNoteIndex;
    }

    int tableAreaIndex = 0;
    int tocIndex = 0;

    for (; it != stop && !it.atEnd(); ++it) {
        QTextBlock block = it.currentBlock();
        QTextTable *table = qobject_cast<QTextTable*>(it.currentFrame());
        QTextFrame *subFrame = it.currentFrame();
        QTextBlockFormat format = block.blockFormat();

        if (table) {
            if (tableAreaIndex >= d->tableAreas.size()) {
                continue;
            }
            if (cursor.selectionEnd() < table->firstPosition()) {
                return retval.translated(0, d->verticalAlignOffset);
            }
            if (cursor.selectionStart() > table->lastPosition()) {
                ++tableAreaIndex;
                continue;
            }
            if (cursor.selectionStart() >= table->firstPosition() && cursor.selectionEnd() <= table->lastPosition()) {
                return d->tableAreas[tableAreaIndex]->selectionBoundingBox(cursor).translated(0, d->verticalAlignOffset);
            }
            if (cursor.selectionStart() >= table->firstPosition()) {
                retval = d->tableAreas[tableAreaIndex]->boundingRect();
            } else {
                retval |= d->tableAreas[tableAreaIndex]->boundingRect();
            }
            ++tableAreaIndex;
            continue;
        } else if (subFrame) {
            if (it.currentFrame()->format().intProperty(KoText::SubFrameType) == KoText::AuxillaryFrameType) {
                if (cursor.selectionEnd() < subFrame->firstPosition()) {
                    return retval.translated(0, d->verticalAlignOffset);
                }
                if (cursor.selectionStart() > subFrame->lastPosition()) {
                    break;
                }
                if (cursor.selectionStart() >= subFrame->firstPosition() && cursor.selectionEnd() <= subFrame->lastPosition()) {
                    return d->endNotesArea->selectionBoundingBox(cursor).translated(0, d->verticalAlignOffset);
                }
                break;
            }
        } else {
            if (!block.isValid())
                continue;
        }
        if (block.blockFormat().hasProperty(KoParagraphStyle::GeneratedDocument)) {
            if (cursor.selectionStart()  <= block.position()
                && cursor.selectionEnd() >= block.position()) {
                retval |= d->generatedDocAreas[tocIndex]->boundingRect();
            }
            ++tocIndex;
            continue;
        }

        if(cursor.selectionEnd() < block.position()) {
            return retval.translated(0, d->verticalAlignOffset);
        }
        if(cursor.selectionStart() >= block.position()
            && cursor.selectionStart() < block.position() + block.length()) {
            QTextLine line = block.layout()->lineForTextPosition(cursor.selectionStart() - block.position());
            if (line.isValid()) {
                retval.setTop(line.y());
                retval.setBottom(line.y());
            }
        }
        if(cursor.selectionEnd() >= block.position()
            && cursor.selectionEnd() < block.position() + block.length()) {
            QTextLine line = block.layout()->lineForTextPosition(cursor.selectionEnd() - block.position());
            if (line.isValid()) {
                retval.setBottom(line.y() + line.height());
                if (line.ascent()==0) {
                    // Block is empty from any visible content and has as such no height
                    // but in that case the block font defines line height
                    retval.setBottom(line.y() + 24);
                }

                if (cursor.selectionStart() == cursor.selectionEnd()) {
                    // We only have a caret so let's set the rect a bit more narrow
                    retval.setX(line.cursorToX(cursor.position() - block.position()));
                    retval.setWidth(1);
                }
            }
        }
        // if the full paragraph is selected to add it to the rect. This makes sure we get a rect for the case
        // where the end of the selection lies is a different area.
        if (cursor.selectionEnd() >= block.position() + block.length() && cursor.selectionStart() <= block.position()) {
            QTextLine line = block.layout()->lineForTextPosition(block.length()-1);
            if (line.isValid()) {
                retval.setBottom(line.y() + line.height());
                if (line.ascent()==0) {
                    // Block is empty from any visible content and has as such no height
                    // but in that case the block font defines line height
                    retval.setBottom(line.y() + 24);
                }
            }
        }
    }
    return retval.translated(0, d->verticalAlignOffset);
}


bool KoTextLayoutArea::isStartingAt(FrameIterator *cursor) const
{
    if (d->startOfArea) {
        return *d->startOfArea == *cursor;
    }

    return false;
}

QTextFrame::iterator KoTextLayoutArea::startTextFrameIterator() const
{
    return d->startOfArea->it;
}

QTextFrame::iterator KoTextLayoutArea::endTextFrameIterator() const
{
    return d->endOfArea->it;
}

void KoTextLayoutArea::backtrackKeepWithNext(FrameIterator *cursor)
{
    QTextFrame::iterator it = cursor->it;

    while (!(it == d->startOfArea->it)) {
        --it;
        QTextBlock block = it.currentBlock();
        QTextTable *table = qobject_cast<QTextTable*>(it.currentFrame());
        QTextFrame *subFrame = it.currentFrame();
        bool keepWithNext = false;
        if (table) {
            keepWithNext = table->format().boolProperty(KoTableStyle::KeepWithNext);
            //setBottom(tableArea->bottom() + d->footNotesHeight);
        } else if (subFrame) {
            Q_ASSERT(false); // there should never be an aux frame before normal layouted stuff
        } else if (block.isValid()) {
            keepWithNext = block.blockFormat().boolProperty(KoParagraphStyle::KeepWithNext);
            //setBottom(d->blockRects.last()->bottom() + d->footNotesHeight);
        }
        if (!keepWithNext) {
            cursor->it = ++it;
            break;
        }
    }
}

bool KoTextLayoutArea::layout(FrameIterator *cursor)
{
    qDeleteAll(d->tableAreas);
    d->tableAreas.clear();
    qDeleteAll(d->footNoteAreas);
    d->footNoteAreas.clear();
    qDeleteAll(d->preregisteredFootNoteAreas);
    d->preregisteredFootNoteAreas.clear();
    d->footNoteFrames.clear();
    d->preregisteredFootNoteFrames.clear();
    qDeleteAll(d->generatedDocAreas);
    d->generatedDocAreas.clear();
    d->blockRects.clear();
    delete d->endNotesArea;
    d->endNotesArea=0;
    if (d->endOfArea) {
        delete d->copyEndOfArea;
        d->copyEndOfArea = new FrameIterator(d->endOfArea);
    }
    delete d->startOfArea;
    delete d->endOfArea;
    d->dropCapsWidth = 0;
    d->dropCapsDistance = 0;

    d->startOfArea = new FrameIterator(cursor);
    d->endOfArea = 0;
    d->y = top();
    d->neededWidth = 0;
    setBottom(top());
    d->bottomSpacing = 0;
    d->footNoteAutoCount = 0;
    d->footNotesHeight = 0;
    d->preregisteredFootNotesHeight = 0;
    d->prevBorder = 0;
    d->prevBorderPadding = 0;

    if (d->footNoteCursorFromPrevious) {
        KoTextLayoutNoteArea *footNoteArea = new KoTextLayoutNoteArea(d->continuedNoteFromPrevious, this, d->documentLayout);
        d->footNoteFrames.append(d->continuedNoteFromPrevious->textFrame());
        footNoteArea->setReferenceRect(left(), right(), 0, maximumAllowedBottom());
        footNoteArea->setAsContinuedArea(true);
        footNoteArea->layout(d->footNoteCursorFromPrevious);
        d->footNotesHeight += footNoteArea->bottom() - footNoteArea->top();
        d->footNoteAreas.append(footNoteArea);
    }
    while (!cursor->it.atEnd()) {
        QTextBlock block = cursor->it.currentBlock();
        QTextTable *table = qobject_cast<QTextTable*>(cursor->it.currentFrame());
        QTextFrame *subFrame = cursor->it.currentFrame();
        if (table) {
            QString masterPageName = table->frameFormat().property(KoTableStyle::MasterPageName).toString();
            bool masterPageNameChanged = !masterPageName.isEmpty();
            if (masterPageNameChanged) {
                cursor->masterPageName = masterPageName;
            }

            if (!virginPage()) {
                int breaktype = table->frameFormat().intProperty(KoTableStyle::BreakBefore);
                if ((acceptsPageBreak() && (masterPageNameChanged || (breaktype == KoText::PageBreak)))
                    || (acceptsColumnBreak() && (breaktype == KoText::ColumnBreak))) {
                    d->endOfArea = new FrameIterator(cursor);
                    setBottom(d->y + d->footNotesHeight);
                    if (!d->blockRects.isEmpty()) {
                        d->blockRects.last().setBottom(d->y);
                    }
                    return false;
                }
            }

            // Let's create KoTextLayoutTableArea and let that handle the table
            KoTextLayoutTableArea *tableArea = new KoTextLayoutTableArea(table, this, d->documentLayout);
            d->tableAreas.append(tableArea);
            d->y += d->bottomSpacing;
            if (!d->blockRects.isEmpty()) {
                d->blockRects.last().setBottom(d->y);
            }
            tableArea->setVirginPage(virginPage());
            tableArea->setReferenceRect(left(), right(), d->y, maximumAllowedBottom());
            if (tableArea->layoutTable(cursor->tableIterator(table)) == false) {
                d->endOfArea = new FrameIterator(cursor);
                d->y = tableArea->bottom();
                setBottom(d->y + d->footNotesHeight);
                // Expand bounding rect so if we have content outside we show it
                expandBoundingLeft(tableArea->boundingRect().left());
                expandBoundingRight(tableArea->boundingRect().right());

                return false;
            }
            setVirginPage(false);
            // Expand bounding rect so if we have content outside we show it
            expandBoundingLeft(tableArea->boundingRect().left());
            expandBoundingRight(tableArea->boundingRect().right());
            d->bottomSpacing = 0;
            d->y = tableArea->bottom();
            delete cursor->currentTableIterator;
            cursor->currentTableIterator = 0;
        } else if (subFrame) {
            if (subFrame->format().intProperty(KoText::SubFrameType) == KoText::AuxillaryFrameType) {
                Q_ASSERT(d->endNotesArea == 0);
                d->endNotesArea = new KoTextLayoutEndNotesArea(this, d->documentLayout);
                d->y += d->bottomSpacing;
                if (!d->blockRects.isEmpty()) {
                    d->blockRects.last().setBottom(d->y);
                }
                d->endNotesArea->setVirginPage(virginPage());
                d->endNotesArea->setReferenceRect(left(), right(), d->y, maximumAllowedBottom());
                if (d->endNotesArea->layout(cursor->subFrameIterator(subFrame)) == false) {
                    d->endOfArea = new FrameIterator(cursor);
                    d->y = d->endNotesArea->bottom();
                    setBottom(d->y + d->footNotesHeight);
                    // Expand bounding rect so if we have content outside we show it
                    expandBoundingLeft(d->endNotesArea->boundingRect().left());
                    expandBoundingRight(d->endNotesArea->boundingRect().right());
                    return false;
                }
                setVirginPage(false);
                // Expand bounding rect so if we have content outside we show it
                expandBoundingLeft(d->endNotesArea->boundingRect().left());
                expandBoundingRight(d->endNotesArea->boundingRect().right());
                d->bottomSpacing = 0;
                d->y = d->endNotesArea->bottom();
                delete cursor->currentSubFrameIterator;
                cursor->currentSubFrameIterator = 0;

                // we have layouted till the end of the document except for a blank block
                // which we should ignore
                ++(cursor->it);
                ++(cursor->it);
                break;
            }
        } else if (block.isValid()) {
            if (block.blockFormat().hasProperty(KoParagraphStyle::GeneratedDocument)) {
                QVariant data = block.blockFormat().property(KoParagraphStyle::GeneratedDocument);
                QTextDocument *generatedDocument = data.value<QTextDocument *>();

                // Let's create KoTextLayoutArea and let it handle the generated document
                KoTextLayoutArea *area = new KoTextLayoutArea(this, documentLayout());
                d->generatedDocAreas.append(area);
                d->y += d->bottomSpacing;
                if (!d->blockRects.isEmpty()) {
                    d->blockRects.last().setBottom(d->y);
                }
                area->setVirginPage(virginPage());
                area->setAcceptsPageBreak(acceptsPageBreak());
                area->setAcceptsColumnBreak(acceptsColumnBreak());
                area->setReferenceRect(left(), right(), d->y, maximumAllowedBottom());
                QTextLayout *blayout = block.layout();
                blayout->beginLayout();
                QTextLine line = blayout->createLine();
                line.setNumColumns(0);
                line.setPosition(QPointF(left(), d->y));
                blayout->endLayout();

                if (area->layout(cursor->subFrameIterator(generatedDocument->rootFrame())) == false) {
                    cursor->lineTextStart = 1; // fake we are not done
                    d->endOfArea = new FrameIterator(cursor);
                    d->y = area->bottom();
                    setBottom(d->y + d->footNotesHeight);
                    // Expand bounding rect so if we have content outside we show it
                    expandBoundingLeft(area->boundingRect().left());
                    expandBoundingRight(area->boundingRect().right());
                    return false;
                }
                setVirginPage(false);
                // Expand bounding rect so if we have content outside we show it
                expandBoundingLeft(area->boundingRect().left());
                expandBoundingRight(area->boundingRect().right());
                d->bottomSpacing = 0;
                d->y = area->bottom();
                delete cursor->currentSubFrameIterator;
                cursor->lineTextStart = -1; // fake we are done
                cursor->currentSubFrameIterator = 0;
            } else {
                // FIXME this doesn't work for cells inside tables. We probably should make it more
                // generic to handle such cases too.
                QString masterPageName = block.blockFormat().property(KoParagraphStyle::MasterPageName).toString();
                bool masterPageNameChanged = !masterPageName.isEmpty();
                if (masterPageNameChanged) {
                    cursor->masterPageName = masterPageName;
                }

                if (!virginPage()) {
                    int breaktype = block.blockFormat().intProperty(KoParagraphStyle::BreakBefore);
                    if ((acceptsPageBreak() && (masterPageNameChanged || (breaktype == KoText::PageBreak)))
                        ||(acceptsColumnBreak() && (breaktype == KoText::ColumnBreak))) {
                        d->endOfArea = new FrameIterator(cursor);
                        setBottom(d->y + d->footNotesHeight);
                        if (!d->blockRects.isEmpty()) {
                            d->blockRects.last().setBottom(d->y);
                        }
                        return false;
                    }
                }

                if (layoutBlock(cursor) == false) {
                    if (cursor->lineTextStart == -1) {
                        //Nothing was added so lets backtrack keep-with-next
                        backtrackKeepWithNext(cursor);
                    }
                    d->endOfArea = new FrameIterator(cursor);
                    setBottom(d->y + d->footNotesHeight);
                    d->blockRects.last().setBottom(d->y);
                    return false;
                }
                d->extraTextIndent = 0;

                int breaktype = block.blockFormat().intProperty(KoParagraphStyle::BreakAfter);
                if ((acceptsPageBreak() && (breaktype & KoText::PageBreak))
                    || (acceptsColumnBreak() && (breaktype & KoText::ColumnBreak))) {
                    Q_ASSERT(!cursor->it.atEnd());
                    QTextFrame::iterator nextIt = cursor->it;
                    ++nextIt;
                    bool wasIncremented = !nextIt.currentFrame();
                    if (wasIncremented)
                        cursor->it = nextIt;
                    d->endOfArea = new FrameIterator(cursor);
                    if (!wasIncremented)
                        ++(cursor->it);
                    setBottom(d->y + d->footNotesHeight);
                    d->blockRects.last().setBottom(d->y);
                    return false;
                }
            }
        }
        bool atEnd = cursor->it.atEnd();
        if (!atEnd) {
            ++(cursor->it);
        }
    }
    d->endOfArea = new FrameIterator(cursor);
    d->y = qMin(maximumAllowedBottom(), d->y + d->bottomSpacing);
    setBottom(d->y + d->footNotesHeight);
    if (!d->blockRects.isEmpty()) {
        d->blockRects.last().setBottom(d->y);
    }
    if (d->maximumAllowedWidth>0) {
        d->right += d->neededWidth - d->width;
        d->maximumAllowedWidth = 0;
        setVirginPage(true);
        KoTextLayoutArea::layout(new FrameIterator(d->startOfArea));
    }
    return true; // we have layouted till the end of the frame
}


QTextLine KoTextLayoutArea::Private::restartLayout(QTextBlock &block, int lineTextStartOfLastKeep)
{
    QTextLayout *layout = block.layout();
    KoTextBlockData blockData(block);
    QPointF stashedCounterPosition = blockData.counterPosition();
    QList<LineKeeper> stashedLines;
    QTextLine line;
    for(int i = 0; i < layout->lineCount(); i++) {
        QTextLine l = layout->lineAt(i);
        if (l.textStart() >= lineTextStartOfLastKeep) {
            break;
        }
        LineKeeper lk;
        lk.lineWidth = l.width();
        lk.columns = l.textLength();
        lk.position = l.position();
        stashedLines.append(lk);
    }
    layout->clearLayout();
    layout->beginLayout();
    line = layout->createLine();

    return recreatePartialLayout(block, stashedLines, stashedCounterPosition, line);
}

void KoTextLayoutArea::Private::stashRemainingLayout(QTextBlock &block, int lineTextStartOfFirstKeep, QList<LineKeeper> &stashedLines, QPointF &stashedCounterPosition)
{
    QTextLayout *layout = block.layout();
    KoTextBlockData blockData(block);
    stashedCounterPosition = blockData.counterPosition();
    QTextLine line;
    for(int i = 0; i < layout->lineCount(); i++) {
        QTextLine l = layout->lineAt(i);
        if (l.textStart() < lineTextStartOfFirstKeep) {
            continue;
        }
        LineKeeper lk;
        lk.lineWidth = l.width();
        lk.columns = l.textLength();
        lk.position = l.position();
        stashedLines.append(lk);
    }
}

QTextLine KoTextLayoutArea::Private::recreatePartialLayout(QTextBlock &block, QList<LineKeeper> stashedLines, QPointF &stashedCounterPosition, QTextLine &line)
{
    QTextLayout *layout = block.layout();
    KoTextBlockData blockData(block);
    documentLayout->allowPositionInlineObject(false);
    if (layout->lineCount() == 1) {
        blockData.setCounterPosition(stashedCounterPosition);
    }
    Q_FOREACH (const LineKeeper &lk, stashedLines) {
        line.setLineWidth(lk.lineWidth);
        if (lk.columns != line.textLength()) {
            // As setNumColumns might break differently we only use it if setLineWidth doesn't give
            // the same textLength as we had before
            line.setNumColumns(lk.columns, lk.lineWidth);
        }
        line.setPosition(lk.position);

        line = layout->createLine();
        if (!line.isValid())
            break;
    }
    documentLayout->allowPositionInlineObject(true);
    return line;
}

static bool compareTab(const QTextOption::Tab &tab1, const QTextOption::Tab &tab2)
{
    return tab1.position < tab2.position;
}

// layoutBlock() method is structured like this:
//
// 1) Setup various helper values
//   a) related to or influenced by lists
//   b) related to or influenced by dropcaps
//   c) related to or influenced by margins
//   d) related to or influenced by tabs
//   e) related to or influenced by borders
//   f) related to or influenced by list counters
// 2)layout each line (possibly restarting where we stopped earlier)
//   a) fit line into sub lines with as needed for text runaround
//   b) break if we encounter softbreak
//   c) make sure we keep above maximumAllowedBottom
//   d) calls addLine()
//   e) update dropcaps related variables
bool KoTextLayoutArea::layoutBlock(FrameIterator *cursor)
{
    QTextBlock block(cursor->it.currentBlock());
    KoTextBlockData blockData(block);
    KoParagraphStyle pStyle(block.blockFormat(), block.charFormat());

    int dropCapsAffectsNMoreLines = 0;
    qreal dropCapsPositionAdjust = 0.0;
    bool lastOfPreviousRun = (d->copyEndOfArea && d->copyEndOfArea->it.currentBlock() == block);

    KoText::Direction dir = pStyle.textProgressionDirection();
    if (dir == KoText::InheritDirection)
        dir = parentTextDirection();
    if (dir == KoText::AutoDirection)
        d->isRtl = block.text().isRightToLeft();
    else
        d->isRtl =  dir == KoText::RightLeftTopBottom;

    // initialize list item stuff for this parag.
    QTextList *textList = block.textList();
    QTextListFormat listFormat;
    QTextCharFormat labelFormat;
    if (textList) {
        listFormat = textList->format();

        if (block.text().size() == 0 || d->documentLayout->wordprocessingMode()) {
            labelFormat = block.charFormat();
        } else {
            labelFormat = block.begin().fragment().charFormat();
        }

        if (d->documentLayout->styleManager()) {
            const int id = listFormat.intProperty(KoListStyle::CharacterStyleId);
            KoCharacterStyle *cs = d->documentLayout->styleManager()->characterStyle(id);
            if (cs) {
                cs->applyStyle(labelFormat);
                cs->ensureMinimalProperties(labelFormat);
            }
        }

        // fetch the text-properties of the label
        if (listFormat.hasProperty(KoListStyle::CharacterProperties)) {
            QVariant v = listFormat.property(KoListStyle::CharacterProperties);
            QSharedPointer<KoCharacterStyle> textPropertiesCharStyle = v.value< QSharedPointer<KoCharacterStyle> >();
            if (!textPropertiesCharStyle.isNull()) {
                textPropertiesCharStyle->applyStyle(labelFormat);
                textPropertiesCharStyle->ensureMinimalProperties(labelFormat);
            }
        }

        // Calculate the correct font point size taking into account the current
        // block format and the relative font size percent if the size is not absolute
        if (listFormat.hasProperty(KoListStyle::RelativeBulletSize)) {
            qreal percent = listFormat.property(KoListStyle::RelativeBulletSize).toDouble();
            labelFormat.setFontPointSize((percent*labelFormat.fontPointSize())/100.00);
        }

        QFont font(labelFormat.font(), d->documentLayout->paintDevice());

        if (!blockData.hasCounterData()) {
            ListItemsHelper lih(textList, font);
            lih.recalculateBlock(block);
        }
        blockData.setLabelFormat(labelFormat);
    } else { // make sure it is empty
        blockData.clearCounter();
    }

    QTextLayout *layout = block.layout();
    QTextOption option = layout->textOption();
    option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

    option.setAlignment(QStyle::visualAlignment(d->isRtl ? Qt::RightToLeft : Qt::LeftToRight, pStyle.alignment()));
    if (d->isRtl) {
        option.setTextDirection(Qt::RightToLeft);
        // For right-to-left we need to make sure that trailing spaces are included into the QTextLine naturalTextWidth
        // and naturalTextRect calculation so they are proper handled in the RunAroundHelper. For left-to-right we do
        // not like to include trailing spaces in the calculations cause else justified text would not look proper
        // justified. Seems for right-to-left we have to accept that justified text will not look proper justified then.
        // only set it for justified text as otherwise we will cut of text at the beginning of the line
        if (pStyle.alignment() == Qt::AlignJustify) {
            option.setFlags(QTextOption::IncludeTrailingSpaces);
        }

    } else {
        option.setFlags(0);
        option.setTextDirection(Qt::LeftToRight);
    }

    option.setUseDesignMetrics(true);

    //==========
    // Drop caps
    //==========

    d->dropCapsNChars = 0;
    if (cursor->lineTextStart == -1) {
        // first remove any drop-caps related formatting that's already there in the layout.
        // we'll do it all afresh now.
        QVector<QTextLayout::FormatRange> formatRanges = layout->formats();
        for (QVector< QTextLayout::FormatRange >::Iterator iter = formatRanges.begin();
                iter != formatRanges.end(); ) {
            if (iter->format.boolProperty(DropCapsAdditionalFormattingId)) {
                iter = formatRanges.erase(iter);
            } else {
                ++iter;
            }
        }
        if (formatRanges.count() != layout->formats().count())
            layout->setFormats(formatRanges);
        bool dropCaps = pStyle.dropCaps();
        int dropCapsLength = pStyle.dropCapsLength();
        int dropCapsLines = pStyle.dropCapsLines();

        if (dropCaps && dropCapsLines > 1 && block.length() > 1) {
            QString blockText = block.text();
            d->dropCapsDistance = pStyle.dropCapsDistance();

            if (dropCapsLength == 0) { // means whole word is to be dropped
                int firstNonSpace = blockText.indexOf(QRegExp("[^ ]"));
                dropCapsLength = blockText.indexOf(QRegExp("\\W"), firstNonSpace);
            } else {
                // LibreOffice skips softbreaks but not spaces. We will do the same
                QTextCursor c1(block);
                c1.setPosition(block.position());
                c1.setPosition(c1.position() + 1, QTextCursor::KeepAnchor);

                KoTextSoftPageBreak *softPageBreak = dynamic_cast<KoTextSoftPageBreak*>(d->documentLayout->inlineTextObjectManager()->inlineTextObject(c1));
                if (softPageBreak) {
                    dropCapsLength++;
                }
            }
            dropCapsLength = qMin(dropCapsLength, blockText.length() - 1);

            if (dropCapsLength > 0) {
                // increase the size of the dropped chars
                QTextCursor blockStart(block);
                QTextLayout::FormatRange dropCapsFormatRange;
                dropCapsFormatRange.format = blockStart.charFormat();

                // find out lineHeight for this block.
                QTextBlock::iterator it = block.begin();
                QTextFragment lineRepresentative = it.fragment();
                qreal lineHeight = pStyle.lineHeightAbsolute();
                qreal dropCapsHeight = 0;
                if (lineHeight == 0) {
                    lineHeight = lineRepresentative.charFormat().fontPointSize();
                    qreal linespacing = pStyle.lineSpacing();
                    if (linespacing == 0) { // unset
                        qreal percent = pStyle.lineHeightPercent();
                        if (percent != 0)
                            linespacing = lineHeight * ((percent - 100) / 100.0);
                        else if (linespacing == 0)
                            linespacing = lineHeight * 0.2; // default
                    }
                    dropCapsHeight = linespacing * (dropCapsLines-1);
                }
                const qreal minimum = pStyle.minimumLineHeight();
                if (minimum > 0.0) {
                    lineHeight = qMax(lineHeight, minimum);
                }

                dropCapsHeight += lineHeight * dropCapsLines;

                int dropCapsStyleId = pStyle.dropCapsTextStyleId();
                KoCharacterStyle *dropCapsCharStyle = 0;
                if (dropCapsStyleId > 0 && d->documentLayout->styleManager()) {
                    dropCapsCharStyle = d->documentLayout->styleManager()->characterStyle(dropCapsStyleId);
                    dropCapsCharStyle->applyStyle(dropCapsFormatRange.format);
                }

                QFont f(dropCapsFormatRange.format.font(), d->documentLayout->paintDevice());
                QString dropCapsText(block.text().left(dropCapsLength));
                f.setPointSizeF(dropCapsHeight);
                for (int i=0; i < 5; ++i) {
                    QTextLayout tmplayout(dropCapsText, f);
                    tmplayout.setTextOption(option);
                    tmplayout.beginLayout();
                    QTextLine tmpline = tmplayout.createLine();
                    tmplayout.endLayout();
                    d->dropCapsWidth = tmpline.naturalTextWidth();

                    QFontMetricsF fm(f, documentLayout()->paintDevice());
                    QRectF rect = fm.tightBoundingRect(dropCapsText);
                    const qreal diff = dropCapsHeight - rect.height();
                    dropCapsPositionAdjust = rect.top() + fm.ascent();
                    if (qAbs(diff) < 0.5) // good enough
                        break;

                    const qreal adjustment = diff * (f.pointSizeF() / rect.height());
                    // warnTextLayout << "adjusting with" << adjustment;
                    f.setPointSizeF(f.pointSizeF() + adjustment);
                }

                dropCapsFormatRange.format.setFontPointSize(f.pointSizeF());
                dropCapsFormatRange.format.setProperty(DropCapsAdditionalFormattingId,
                        (QVariant) true);
                dropCapsFormatRange.start = 0;
                dropCapsFormatRange.length = dropCapsLength;
                formatRanges.append(dropCapsFormatRange);
                layout->setFormats(formatRanges);

                d->dropCapsNChars = dropCapsLength;
                dropCapsAffectsNMoreLines = (d->dropCapsNChars > 0) ? dropCapsLines : 0;
            }
        }
    }

    //========
    // Margins
    //========
    qreal startMargin = block.blockFormat().leftMargin();
    qreal endMargin = block.blockFormat().rightMargin();
    if (d->isRtl) {
        std::swap(startMargin, endMargin);
    }
    d->indent = textIndent(block, textList, pStyle) + d->extraTextIndent;

    qreal labelBoxWidth = 0;
    qreal labelBoxIndent = 0;
    if (textList) {
        if (listFormat.boolProperty(KoListStyle::AlignmentMode)) {
            // according to odf 1.2 17.20 list margin should be used when paragraph margin is
            // not specified by the auto style (additionally LO/OO uses 0 as condition so we do too)
            int id = pStyle.styleId();
            bool set = false;
            if (id && d->documentLayout->styleManager()) {
                KoParagraphStyle *originalParagraphStyle = d->documentLayout->styleManager()->paragraphStyle(id);
                if (originalParagraphStyle->leftMargin() != startMargin) {
                    set = (startMargin != 0);
                }
            } else {
                set = (startMargin != 0);
            }
            if (! set) {
                startMargin = listFormat.doubleProperty(KoListStyle::Margin);
            }

            labelBoxWidth = blockData.counterWidth();
            Qt::Alignment align = static_cast<Qt::Alignment>(listFormat.intProperty(KoListStyle::Alignment));
            if (align == 0) {
                align = Qt::AlignLeft;
            }
            if (align & Qt::AlignLeft) {
                d->indent += labelBoxWidth;
            } else if (align & Qt::AlignHCenter) {
                d->indent += labelBoxWidth/2;
            }
            labelBoxIndent = d->indent - labelBoxWidth;
        } else {
            labelBoxWidth = blockData.counterSpacing() + blockData.counterWidth();
        }
    }

    d->width = right() - left();
    d->width -= startMargin + endMargin;
    d->x = left() + (d->isRtl ? 0.0 : startMargin);

    d->documentLayout->clearInlineObjectRegistry(block);

    //========
    // Tabs
    //========
    QList<KoText::Tab> tabs = pStyle.tabPositions();

    // Handle tabs relative to startMargin
    qreal tabOffset = -d->indent;
    if (!d->documentLayout->relativeTabs(block)) {
        tabOffset -= startMargin;
    }

    // Make a list of tabs that Qt can use
    QList<QTextOption::Tab> qTabs;
    // Note: Converting to Qt tabs is needed as long as we use Qt for layout, but we
    // loose the possibility to do leader chars.
    foreach (const KoText::Tab &kTab, tabs) {
        qreal value = kTab.position;
        if (value == MaximumTabPos) { // MaximumTabPos is used in index generators
            // note: we subtract right margin as this is where the tab should be
            // note: we subtract indent so tab is not relative to it
            // note: we subtract left margin so tab is not relative to it
            // if rtl the above left/right reasons swap but formula stays the same
            // -tabOfset is just to cancel that we add it next
            // -2 is to avoid wrap at right edge to the next line
            value = right() - left() - startMargin - endMargin - d->indent - tabOffset - 2;
        }

        // conversion here is required because Qt thinks in device units and we don't
        value *= qt_defaultDpiY() / 72.0;

        value += tabOffset * qt_defaultDpiY() / 72.0;

        QTextOption::Tab tab;
        tab.position = value;
        tab.type = kTab.type;
        tab.delimiter = kTab.delimiter;
        qTabs.append(tab);
    }

    qreal presentationListTabValue(0.0); // for use in presentationListTabWorkaround

    // For some lists we need to add a special list tab according to odf 1.2 19.830
    if (textList && listFormat.intProperty(KoListStyle::LabelFollowedBy) == KoListStyle::ListTab) {
        qreal listTab = 0;
        if (listFormat.hasProperty(KoListStyle::TabStopPosition)) {
            listTab = listFormat.doubleProperty(KoListStyle::TabStopPosition);
            if (!d->documentLayout->relativeTabs(block)) {
                // How list tab is defined if fixed tabs:
                //        listTab
                //|>-------------------------|
                //           d->indent
                //         |---------<|
                //     LABEL                 TEXT STARTS HERE AND GOES ON
                //                    TO THE NEXT LINE
                //|>------------------|
                //     startMargin
                listTab -= startMargin;
            } else {
                // How list tab is defined if relative tabs:
                // It's relative to startMargin - list.startMargin
                //              listTab
                //       |>-------------------|
                //             d->indent
                //           |---------<|
                //       LABEL                 TEXT STARTS HERE AND GOES ON
                //                      TO THE NEXT LINE
                //|>--------------------|
                //     startMargin       |
                //       |>-------------|
                //          list.margin
                listTab -= listFormat.doubleProperty(KoListStyle::Margin);
            }
        }
        // How list tab is defined now:
        //                    listTab
        //                    |>-----|
        //           d->indent
        //         |---------<|
        //     LABEL                 TEXT STARTS HERE AND GOES ON
        //                    TO THE NEXT LINE
        //|>------------------|
        //     startMargin
        presentationListTabValue = listTab;
        listTab -= d->indent;

        // And now listTab is like this:
        //         x()
        //         |     listTab
        //         |>---------------|
        //           d->indent
        //         |---------<|
        //     LABEL                 TEXT STARTS HERE AND GOES ON
        //                    TO THE NEXT LINE
        //|>------------------|
        //     startMargin

        // conversion here is required because Qt thinks in device units and we don't
        listTab *= qt_defaultDpiY() / 72.0;

        QTextOption::Tab tab;
        tab.position = listTab;
        tab.type = d->isRtl ? QTextOption::RightTab : QTextOption::LeftTab;
        qTabs.append(tab);
    }

    // We need to sort as the MaximumTabPos may be converted to a value that really
    // should be in the middle, and listtab needs to be sorted in too
    std::sort(qTabs.begin(), qTabs.end(), compareTab);

    // Regular interval tabs. Since Qt doesn't handle regular interval tabs offset
    // by a fixed number we need to create the regular tabs ourselves.
    qreal tabStopDistance = pStyle.tabStopDistance() * qt_defaultDpiY() / 72.0;
    if (tabStopDistance <= 0) {
        tabStopDistance = d->documentLayout->defaultTabSpacing() * qt_defaultDpiY() / 72.0;
    }

    qreal regularSpacedTabPos = -d->indent * qt_defaultDpiY() / 72.0 -0.1; // first possible position
    if (!qTabs.isEmpty()) {
        regularSpacedTabPos = qTabs.last().position;
    }

    regularSpacedTabPos -= tabOffset * qt_defaultDpiY() / 72.0;
    if (regularSpacedTabPos < 0) {
        regularSpacedTabPos = -int(-regularSpacedTabPos / tabStopDistance) * tabStopDistance;
    } else {
        regularSpacedTabPos = (int(regularSpacedTabPos / tabStopDistance) + 1) * tabStopDistance;
    }
    regularSpacedTabPos += tabOffset * qt_defaultDpiY() / 72.0;

    while (regularSpacedTabPos < MaximumTabPos) {
        QTextOption::Tab tab;
        tab.position = regularSpacedTabPos;
        qTabs.append(tab);
        regularSpacedTabPos += tabStopDistance;
    }

    option.setTabs(qTabs);

    // conversion here is required because Qt thinks in device units and we don't
    option.setTabStop(tabStopDistance * qt_defaultDpiY() / 72.);

    layout->setTextOption(option);


    // ==============
    // Possibly store the old layout of lines in case we end up splitting the paragraph at the same position
    // ==============
    QList<LineKeeper> stashedLines;
    QPointF stashedCounterPosition;
    if (lastOfPreviousRun) {
        // we have been layouted before, and the block ended on the following page so better
        // stash the layout for later
        d->stashRemainingLayout(block, d->copyEndOfArea->lineTextStart, stashedLines, stashedCounterPosition);
    }

    // ==============
    // Setup line and possibly restart paragraph continuing from previous other area
    // ==============
    QTextLine line;
    if (cursor->lineTextStart == -1) {
        layout->beginLayout();
        line = layout->createLine();
        cursor->fragmentIterator = block.begin();
    } else {
        line = d->restartLayout(block, cursor->lineTextStart);
        d->indent = d->extraTextIndent;
    }

    if (block.blockFormat().boolProperty(KoParagraphStyle::UnnumberedListItem)) {
        // Unnumbered list items act like "following lines" in a numbered block
        d->indent = 0;
    }

    // ==============
    // List label/counter positioning
    // ==============
    if (textList && block.layout()->lineCount() == 1
        && ! block.blockFormat().boolProperty(KoParagraphStyle::UnnumberedListItem)) {
        // If first line in a list then set the counterposition. Following lines in the same
        // list-item have nothing to do with the counter.
        if (listFormat.boolProperty(KoListStyle::AlignmentMode) == false) {
            qreal minLabelWidth = listFormat.doubleProperty(KoListStyle::MinimumWidth);
            if (!d->isRtl) {
                d->x += listFormat.doubleProperty(KoListStyle::Indent) + minLabelWidth;
            }
            d->width -= listFormat.doubleProperty(KoListStyle::Indent) + minLabelWidth;
            d->indent +=  labelBoxWidth - minLabelWidth;
            blockData.setCounterPosition(QPointF(d->x + d->indent - labelBoxWidth, d->y));
        } else if (labelBoxWidth > 0.0 || blockData.counterText().length() > 0) {
            // Alignmentmode and there is a label (double check needed to account for both
            // picture bullets and non width chars)
            blockData.setCounterPosition(QPointF(d->x + labelBoxIndent, d->y));
            if (listFormat.intProperty(KoListStyle::LabelFollowedBy) == KoListStyle::ListTab
                && !presentationListTabWorkaround(textIndent(block, textList, pStyle), labelBoxWidth, presentationListTabValue)) {
                Q_FOREACH (QTextOption::Tab tab, qTabs) {
                    qreal position = tab.position  * 72. / qt_defaultDpiY();
                    if (position > 0.0) {
                        d->indent += position;
                        break;
                    }
                }

                //And finally it's like this:
                //                          x()
                //                    d->indent
                //                    |>-----|
                //     LABEL                 TEXT STARTS HERE AND GOES ON
                //                    TO THE NEXT LINE
                //|>------------------|
                //     startMargin
            } else if (listFormat.intProperty(KoListStyle::LabelFollowedBy) == KoListStyle::Space) {
                 QFontMetrics fm(labelFormat.font(), d->documentLayout->paintDevice());
                 d->indent += fm.width(' ');
            }
            // default needs to be no space so presentationListTabWorkaround above makes us go here
        }
    }

    // Whenever we relayout the markup layout becomes invalid
    blockData.setMarkupsLayoutValidity(KoTextBlockData::Misspell, false);
    blockData.setMarkupsLayoutValidity(KoTextBlockData::Grammar, false);

    // ==============
    // Now once we know the physical context we can work on the borders of the paragraph
    // ==============
    if (block.blockFormat().hasProperty(KoParagraphStyle::HiddenByTable)) {
        if (!d->blockRects.isEmpty()) {
            d->blockRects.last().setBottom(d->y);
        }
        d->y += d->bottomSpacing;
        d->bottomSpacing = 0;
        d->blockRects.append(QRectF(d->x, d->y, d->width, 10.0));
    } else {
        handleBordersAndSpacing(blockData, &block);
    }

    // Expand bounding rect so if we have content outside we show it
    expandBoundingLeft(d->blockRects.last().x());
    expandBoundingRight(d->blockRects.last().right());

    // ==============
    // Create the lines of this paragraph
    // ==============
    RunAroundHelper runAroundHelper;
    runAroundHelper.setObstructions(documentLayout()->currentObstructions());
    qreal maxLineHeight = 0;
    qreal y_justBelowDropCaps = 0;
    bool anyLineAdded = false;
    int numBaselineShifts = 0;

    while (line.isValid()) {
        runAroundHelper.setLine(this, line);
        runAroundHelper.setObstructions(documentLayout()->currentObstructions());
        QRectF anchoringRect = d->blockRects.last();
        anchoringRect.setTop(d->anchoringParagraphContentTop);
        documentLayout()->setAnchoringParagraphContentRect(anchoringRect);
        anchoringRect.setLeft(left());
        anchoringRect.setWidth(right() - left());
        anchoringRect.setTop(d->anchoringParagraphTop);
        documentLayout()->setAnchoringParagraphRect(anchoringRect);
        documentLayout()->setAnchoringLayoutEnvironmentRect(layoutEnvironmentRect());
        runAroundHelper.fit( /* resetHorizontalPosition */ false, /* rightToLeft */ d->isRtl, QPointF(x(), d->y));

        documentLayout()->positionAnchorTextRanges(block.position()+line.textStart(), line.textLength(), block.document());
        qreal bottomOfText = line.y() + line.height();

        bool softBreak = false;
        bool moreInMiddle = d->y > maximumAllowedBottom() - 150;
        if (acceptsPageBreak() && !pStyle.nonBreakableLines() && moreInMiddle) {
            int softBreakPos = -1;
            QString text = block.text();
            int pos = text.indexOf(QChar::ObjectReplacementCharacter, line.textStart());

            while (pos >= 0 && pos <= line.textStart() + line.textLength()) {
                QTextCursor c1(block);
                c1.setPosition(block.position() + pos);
                c1.setPosition(c1.position() + 1, QTextCursor::KeepAnchor);

                KoTextSoftPageBreak *softPageBreak = dynamic_cast<KoTextSoftPageBreak*>(d->documentLayout->inlineTextObjectManager()->inlineTextObject(c1));
                if (softPageBreak) {
                    softBreakPos = pos;
                    break;
                }

                pos = text.indexOf(QChar::ObjectReplacementCharacter, pos + 1);
            }

            if (softBreakPos >= 0 && softBreakPos < line.textStart() + line.textLength()) {
                line.setNumColumns(softBreakPos - line.textStart() + 1, line.width());
                softBreak = true;
                // if the softBreakPos is at the start of the line stop here so
                // we don't add a line here. That fixes the problem that e.g. the counter is before
                // the page break and the text is after the page break
                if (!virginPage() && softBreakPos == 0) {
                    d->recreatePartialLayout(block, stashedLines, stashedCounterPosition, line);
                    layout->endLayout();
                    return false;
                }
            }
        }

        if (documentLayout()->anchoringSoftBreak() <= block.position() + line.textStart() + line.textLength()) {
            //don't add an anchor that has been moved away
            line.setNumColumns(documentLayout()->anchoringSoftBreak() - block.position() - line.textStart(), line.width());
            softBreak = true;
            // if the softBreakPos is at the start of the block stop here so
            // we don't add a line here. That fixes the problem that e.g. the counter is before
            // the page break and the text is after the page break
            if (!virginPage() && documentLayout()->anchoringSoftBreak() == block.position()) {
                d->recreatePartialLayout(block, stashedLines, stashedCounterPosition, line);
                layout->endLayout();
                return false;
            }
        }

        findFootNotes(block, line, bottomOfText);
        if (bottomOfText > maximumAllowedBottom()) {
            // We can not fit line within our allowed space
            // in case we resume layout on next page the line is reused later
            // but if not then we need to make sure the line becomes invisible
            // we use d->maximalAllowedBottom because we want to be below
            // footnotes too.
            if (!virginPage() && pStyle.nonBreakableLines()) {
                line.setPosition(QPointF(x(), d->maximalAllowedBottom));
                cursor->lineTextStart = -1;
                d->recreatePartialLayout(block, stashedLines, stashedCounterPosition, line);
                layout->endLayout();
                clearPreregisteredFootNotes();
                return false; //to indicate block was not done!
            }
            if (!virginPage() && pStyle.orphanThreshold() != 0
                              && pStyle.orphanThreshold() > numBaselineShifts) {
                line.setPosition(QPointF(x(), d->maximalAllowedBottom));
                cursor->lineTextStart = -1;
                d->recreatePartialLayout(block, stashedLines, stashedCounterPosition, line);
                layout->endLayout();
                clearPreregisteredFootNotes();
                return false; //to indicate block was not done!
            }
            if (!virginPage() || anyLineAdded) {
                line.setPosition(QPointF(x(), d->maximalAllowedBottom));
                d->recreatePartialLayout(block, stashedLines, stashedCounterPosition, line);
                layout->endLayout();
                clearPreregisteredFootNotes();
                return false; //to indicate block was not done!
            }
        }
        confirmFootNotes();
        anyLineAdded = true;
        maxLineHeight = qMax(maxLineHeight, addLine(line, cursor, blockData));

        d->neededWidth = qMax(d->neededWidth, line.naturalTextWidth() + d->indent);

        if (!runAroundHelper.stayOnBaseline() && !(block.blockFormat().hasProperty(KoParagraphStyle::HiddenByTable)
         && block.length() <= 1)) {
            d->y += maxLineHeight;
            maxLineHeight = 0;
            d->indent = 0;
            d->extraTextIndent = 0;
            ++numBaselineShifts;
        }

        // drop caps
        if (d->dropCapsNChars > 0) { // we just laid out the dropped chars
            y_justBelowDropCaps = d->y; // save the y position just below the dropped characters
            d->y = line.y();              // keep the same y for the next line
            line.setPosition(line.position() - QPointF(0, dropCapsPositionAdjust));
            d->dropCapsNChars -= line.textLength();
        } else if (dropCapsAffectsNMoreLines > 0) { // we just laid out a drop-cap-affected line
            dropCapsAffectsNMoreLines--;
            if (dropCapsAffectsNMoreLines == 0) {   // no more drop-cap-affected lines
                if (d->y < y_justBelowDropCaps)
                    d->y = y_justBelowDropCaps; // make sure d->y is below the dropped characters
                y_justBelowDropCaps = 0;
                d->dropCapsWidth = 0;
                d->dropCapsDistance = 0;
            }
        }
        documentLayout()->positionAnchoredObstructions();

        // line fitted so try and do the next one
        line = layout->createLine();
        if (!line.isValid()) {
            break; // no more line means our job is done
        }
        cursor->lineTextStart = line.textStart();

        if (softBreak) {
            d->recreatePartialLayout(block, stashedLines, stashedCounterPosition, line);
            layout->endLayout();
            return false; // page-break means we need to start again on the next page
        }
    }

    d->bottomSpacing = pStyle.bottomMargin();

    layout->endLayout();
    setVirginPage(false);
    cursor->lineTextStart = -1; //set lineTextStart to -1 and returning true indicate new block
    block.setLineCount(layout->lineCount());
    return true;
}

bool KoTextLayoutArea::presentationListTabWorkaround(qreal indent, qreal labelBoxWidth, qreal presentationListTabValue)
{
    if (!d->documentLayout->wordprocessingMode() && indent < 0.0) {
        // Impress / Powerpoint expects the label to be before the text
        if (indent + labelBoxWidth >= presentationListTabValue) {
            // but here is an unforseen overlap with normal text
            return true;
        }
    }
    return false;
}

qreal KoTextLayoutArea::textIndent(const QTextBlock &block, QTextList *textList, const KoParagraphStyle &pStyle) const
{
    if (pStyle.autoTextIndent()) {
        // if auto-text-indent is set,
        // return an indent approximately 3-characters wide as per current font
        QTextCursor blockCursor(block);
        qreal guessGlyphWidth = QFontMetricsF(blockCursor.charFormat().font()).width('x');
        return guessGlyphWidth * 3;
    }

    qreal blockTextIndent = block.blockFormat().textIndent();

    if (textList && textList->format().boolProperty(KoListStyle::AlignmentMode)) {
        // according to odf 1.2 17.20 list text indent should be used when paragraph text indent is
        // not specified (additionally LO/OO uses 0 as condition so we do too)
        int id = pStyle.styleId();
        bool set = false;
        if (id && d->documentLayout->styleManager()) {
            KoParagraphStyle *originalParagraphStyle = d->documentLayout->styleManager()->paragraphStyle(id);
            if (originalParagraphStyle->textIndent() != blockTextIndent) {
                set = (blockTextIndent != 0);
            }
        } else {
            set = (blockTextIndent != 0);
        }
        if (! set) {
            return textList->format().doubleProperty(KoListStyle::TextIndent);
        }
    }
    return blockTextIndent;
}

void KoTextLayoutArea::setExtraTextIndent(qreal extraTextIndent)
{
    d->extraTextIndent = extraTextIndent;
}

qreal KoTextLayoutArea::x() const
{
    if (d->isRtl) {
        return d->x;
    } else {
        if (d->dropCapsNChars > 0 || d->dropCapsWidth == 0)
            return d->x + d->indent ;
        else
            return d->x + d->indent + d->dropCapsWidth + d->dropCapsDistance;
    }
}

qreal KoTextLayoutArea::width() const
{
    if (d->dropCapsNChars > 0) {
        return d->dropCapsWidth;
    }
    qreal width = d->width;
    if (d->maximumAllowedWidth > 0) {
        // lets use that instead but remember all the indent stuff we have calculated
        width = d->width - (d->right - d->left) + d->maximumAllowedWidth;
    }
    return width - d->indent - d->dropCapsWidth - d->dropCapsDistance;
}

void KoTextLayoutArea::setAcceptsPageBreak(bool accept)
{
    d->acceptsPageBreak = accept;
}

bool KoTextLayoutArea::acceptsPageBreak() const
{
    return d->acceptsPageBreak;
}

void KoTextLayoutArea::setAcceptsColumnBreak(bool accept)
{
    d->acceptsColumnBreak = accept;
}

bool KoTextLayoutArea::acceptsColumnBreak() const
{
    return d->acceptsColumnBreak;
}

void KoTextLayoutArea::setVirginPage(bool virgin)
{
    d->virginPage = virgin;
}

bool KoTextLayoutArea::virginPage() const
{
    return d->virginPage;
}

void KoTextLayoutArea::setVerticalAlignOffset(qreal offset)
{
    d->boundingRect.setTop(d->top + qMin(qreal(0.0), offset));
    d->boundingRect.setBottom(d->bottom + qMax(qreal(0.0), offset));
    Q_ASSERT_X(d->boundingRect.top() <= d->boundingRect.bottom(), __FUNCTION__, "Bounding-rect is not normalized");
    d->verticalAlignOffset = offset;
}

qreal KoTextLayoutArea::verticalAlignOffset() const
{
    return d->verticalAlignOffset;
}

qreal KoTextLayoutArea::addLine(QTextLine &line, FrameIterator *cursor, KoTextBlockData &blockData)
{
    QTextBlock block = cursor->it.currentBlock();
    QTextBlockFormat format = block.blockFormat();
    KoParagraphStyle style(format, block.charFormat());

    if (block.textList() && block.layout()->lineCount() == 1) {
        Qt::Alignment alignment = format.alignment();
        if (d->isRtl && (alignment & Qt::AlignAbsolute) == 0) {
            if (alignment & Qt::AlignLeft) {
                alignment = Qt::AlignRight;
            } else if (alignment & Qt::AlignRight) {
                alignment = Qt::AlignLeft;
            }
        }
        alignment &= Qt::AlignRight | Qt::AlignLeft | Qt::AlignHCenter;

        // First line, lets check where the line ended up and adjust the positioning of the counter.
        qreal newX;
        if (alignment & Qt::AlignHCenter) {
            const qreal padding = (line.width() - line.naturalTextWidth()) / 2;
            newX = blockData.counterPosition().x() + (d->isRtl ? -padding : padding);
        } else if (alignment & Qt::AlignRight) {
            const qreal padding = line.width() - line.naturalTextWidth();
            newX = blockData.counterPosition().x() + (d->isRtl ? -padding : padding);
        } else {
            newX = blockData.counterPosition().x();
        }
        if (d->isRtl) {
            newX = line.x() + line.naturalTextWidth() + line.x() + d->indent - newX;
        }

        blockData.setCounterPosition(QPointF(newX, blockData.counterPosition().y()));
    }

    qreal height = 0;
    qreal breakHeight = 0.0;
    qreal ascent = 0.0;
    qreal descent = 0.0;
    const bool useFontProperties = format.boolProperty(KoParagraphStyle::LineSpacingFromFont);

    if (cursor->fragmentIterator.atEnd()) {// no text in parag.
        qreal fontStretch = 1;
        QTextCharFormat charFormat = block.charFormat();
        if (block.blockFormat().hasProperty(KoParagraphStyle::EndCharStyle)) {
            QVariant v = block.blockFormat().property(KoParagraphStyle::EndCharStyle);
            QSharedPointer<KoCharacterStyle> endCharStyle = v.value< QSharedPointer<KoCharacterStyle> >();
            if (!endCharStyle.isNull()) {
                endCharStyle->applyStyle(charFormat);
                endCharStyle->ensureMinimalProperties(charFormat);
            }
        }

        if (useFontProperties) {
            //stretch line height to powerpoint size
            fontStretch = PresenterFontStretch;
        } else if (block.charFormat().hasProperty(KoCharacterStyle::FontYStretch)) {
            // stretch line height to ms-word size
            fontStretch = charFormat.property(KoCharacterStyle::FontYStretch).toDouble();
        }
        height = charFormat.fontPointSize() * fontStretch;
    } else {
        qreal fontStretch = 1;
        QTextFragment fragment = cursor->fragmentIterator.fragment();
        if (useFontProperties) {
            //stretch line height to powerpoint size
            fontStretch = PresenterFontStretch;
        } else if (fragment.charFormat().hasProperty(KoCharacterStyle::FontYStretch)) {
            // stretch line height to ms-word size
            fontStretch = fragment.charFormat().property(KoCharacterStyle::FontYStretch).toDouble();
        }
        // read max font height
        height = qMax(height, fragment.charFormat().fontPointSize() * fontStretch);

        KoInlineObjectExtent pos = d->documentLayout->inlineObjectExtent(fragment);
        ascent = qMax(ascent, pos.m_ascent);
        descent = qMax(descent, pos.m_descent);

        bool lineBreak = false;
        int lastCharPos = block.position() + line.textStart() + line.textLength() - 1;
        int blockLastCharWithoutPreedit = line.textStart() + line.textLength() - 1;
        if (block.layout()->preeditAreaPosition() >= block.position() + line.textStart() &&
                block.layout()->preeditAreaPosition() <= lastCharPos) {
            blockLastCharWithoutPreedit -= block.layout()->preeditAreaText().length();
        }
        if (block.text().at(blockLastCharWithoutPreedit) == QChar(0x2028)) {
            // Was a line with line-break
            if (line.textLength() != 1) { //unless empty line we should ignore the format of it
                --lastCharPos;
            }
                lineBreak = true;
        }
        while (!(fragment.contains(lastCharPos))) {
            cursor->fragmentIterator++;
            if (cursor->fragmentIterator.atEnd()) {
                break;
            }
            fragment = cursor->fragmentIterator.fragment();
            if (!d->documentLayout->changeTracker()
                || !d->documentLayout->changeTracker()->displayChanges()
                || !d->documentLayout->changeTracker()->containsInlineChanges(fragment.charFormat())
                || !d->documentLayout->changeTracker()->elementById(fragment.charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt())
                || !d->documentLayout->changeTracker()->elementById(fragment.charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt())->isEnabled()
                || (d->documentLayout->changeTracker()->elementById(fragment.charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt())->getChangeType() != KoGenChange::DeleteChange)) {
                qreal fontStretch = 1;
                if (useFontProperties) {
                    //stretch line height to powerpoint size
                    fontStretch = PresenterFontStretch;
                } else if (fragment.charFormat().hasProperty(KoCharacterStyle::FontYStretch)) {
                    // stretch line height to ms-word size
                    fontStretch = fragment.charFormat().property(KoCharacterStyle::FontYStretch).toDouble();
                }
                // read max font height
                height = qMax(height, fragment.charFormat().fontPointSize() * fontStretch);

                KoInlineObjectExtent pos = d->documentLayout->inlineObjectExtent(fragment);
                ascent = qMax(ascent, pos.m_ascent);
                descent = qMax(descent, pos.m_descent);
            }
        }

        if (lineBreak) {
            // Was a line with line-break - the format of the line-break should not be
            // considered for the next line either. So we may have to advance the fragmentIterator.
            while (!cursor->fragmentIterator.atEnd() && lastCharPos > fragment.position() + fragment.length()-1) {
                cursor->fragmentIterator++;
                fragment = cursor->fragmentIterator.fragment();
            }

            qreal breakAscent = ascent;
            qreal breakDescent = descent;
            breakHeight = height;

            int firstPos = block.position() + line.textStart() + line.textLength();

            // Was a line with line-break - the format of the line-break should not be
            // considered for the next line either. So we may have to advance the fragmentIterator.
            while (!cursor->fragmentIterator.atEnd() && firstPos > fragment.position() + fragment.length()-1) {
                cursor->fragmentIterator++;
                if (!cursor->fragmentIterator.atEnd()) {
                    fragment = cursor->fragmentIterator.fragment();

                    // read max font height
                    breakHeight = qMax(breakHeight, fragment.charFormat().fontPointSize() * fontStretch);

                    KoInlineObjectExtent pos = d->documentLayout->inlineObjectExtent(fragment);
                    breakAscent = qMax(breakAscent, pos.m_ascent);
                    breakDescent = qMax(breakDescent, pos.m_descent);
                }
            }
            breakHeight = qMax(breakHeight, breakAscent + breakDescent);
        }
    }

    height = qMax(height, ascent + descent);

    if (height < 0.01) {
        height = 12; // default size for uninitialized styles.
    }

    // Calculate adjustment to the height due to line height calculated by qt which shouldn't be
    // there in reality. We will just move the line
    qreal lineAdjust = 0.0;
    if (breakHeight > height) {
        lineAdjust = height - breakHeight;
    }

    // Adjust the line-height according to a probably defined fixed line height,
    // a proportional (percent) line-height and/or the line-spacing. Together
    // with the line-height we maybe also need to adjust the position of the
    // line. This is for example needed if the line needs to shrink in height
    // so the line-text stays on the baseline. If the line grows in height then
    // we don't need to do anything.
    if (d->dropCapsNChars <= 0) { // linespacing rules doesn't apply to drop caps
        qreal fixedLineHeight = format.doubleProperty(KoParagraphStyle::FixedLineHeight);
        if (fixedLineHeight != 0.0) {
            qreal prevHeight = height;
            height = fixedLineHeight;
            lineAdjust += height - prevHeight;
        } else {
            qreal lineSpacing = format.doubleProperty(KoParagraphStyle::LineSpacing);
            if (lineSpacing == 0.0) { // unset
                qreal percent = format.doubleProperty(KoParagraphStyle::PercentLineHeight);
                if (percent != 0) {
                    height *= percent / 100.0;
                } else {
                    height *= 1.2; // default
                }
            }
            height += lineSpacing;
        }

        qreal minimum = style.minimumLineHeight();
        if (minimum > 0.0) {
            height = qMax(height, minimum);
        }
    } else {
        // for drop caps we just work with a basic linespacing for the dropped characters
        height *= 1.2;
    }
    //rounding problems due to Qt-scribe internally using ints.
    //also used when line was moved down because of intersections with other shapes
    if (qAbs(d->y - line.y()) >= 0.126) {
        d->y = line.y();
    }

    if (lineAdjust) {
        // Adjust the position of the line itself.
        line.setPosition(QPointF(line.x(), line.y() + lineAdjust));

        // Adjust the position of the block-rect for this line which is used later
        // to proper clip the line while drawing. If we would not adjust it here
        // then we could end with text-lines being partly cutoff.
        if (lineAdjust < 0.0) {
            d->blockRects.last().moveTop(d->blockRects.last().top() + lineAdjust);
        }

        if (block.textList() && block.layout()->lineCount() == 1) {
            // If this is the first line in a list (aka the first line after the list-
            // item) then we also need to adjust the counter to match to the line again.
            blockData.setCounterPosition(QPointF(blockData.counterPosition().x(), blockData.counterPosition().y() + lineAdjust));
        }
    }

    return height;
}

void KoTextLayoutArea::setLayoutEnvironmentResctictions(bool isLayoutEnvironment, bool actsHorizontally)
{
    d->isLayoutEnvironment = isLayoutEnvironment;
    d->actsHorizontally = actsHorizontally;
}

QRectF KoTextLayoutArea::layoutEnvironmentRect() const
{
    QRectF rect(-5e10, -5e10, 10e10, 10e20); // large values that never really restrict anything

    if (d->parent) {
        rect = d->parent->layoutEnvironmentRect();
    }

    if (d->isLayoutEnvironment) {
        if (d->actsHorizontally) {
            rect.setLeft(left());
            rect.setRight(right());
        }
        rect.setTop(top());
        rect.setBottom(maximumAllowedBottom());
    }

    return rect;
}

QRectF KoTextLayoutArea::boundingRect() const
{
    return d->boundingRect;
}

qreal KoTextLayoutArea::maximumAllowedBottom() const
{
    return d->maximalAllowedBottom - d->footNotesHeight
                    - d->preregisteredFootNotesHeight;
}

FrameIterator *KoTextLayoutArea::footNoteCursorToNext() const
{
    return d->footNoteCursorToNext;
}

KoInlineNote *KoTextLayoutArea::continuedNoteToNext() const
{
    return d->continuedNoteToNext;
}

int KoTextLayoutArea::footNoteAutoCount() const
{
    return d->footNoteAutoCount;
}

void KoTextLayoutArea::setFootNoteCountInDoc(int count)
{
    d->footNoteCountInDoc = count;
}

void KoTextLayoutArea::setFootNoteFromPrevious(FrameIterator *footNoteCursor, KoInlineNote *note)
{
    d->footNoteCursorFromPrevious = footNoteCursor;
    d->continuedNoteFromPrevious = note;
}

void KoTextLayoutArea::setNoWrap(qreal maximumAllowedWidth)
{
    d->maximumAllowedWidth = maximumAllowedWidth;
}

KoText::Direction KoTextLayoutArea::parentTextDirection() const
{
    Q_ASSERT(d->parent); //Root areas should overload this method
    return d->parent->parentTextDirection();
}

KoTextLayoutArea *KoTextLayoutArea::parent() const
{
    return d->parent;
}

KoTextDocumentLayout *KoTextLayoutArea::documentLayout() const
{
    return d->documentLayout;
}

void KoTextLayoutArea::setReferenceRect(qreal left, qreal right, qreal top, qreal maximumAllowedBottom)
{
    d->left = left;
    d->right = right;
    d->top = top;
    d->boundingRect = QRectF(left, top, right - left, 0.0);
    Q_ASSERT_X(d->boundingRect.top() <= d->boundingRect.bottom() && d->boundingRect.left() <= d->boundingRect.right(), __FUNCTION__, "Bounding-rect is not normalized");
    d->maximalAllowedBottom = maximumAllowedBottom;
}

QRectF KoTextLayoutArea::referenceRect() const
{
    return QRectF(d->left, d->top, d->right - d->left, d->bottom - d->top);
}

qreal KoTextLayoutArea::left() const
{
    return d->left;
}

qreal KoTextLayoutArea::right() const
{
    return d->right;
}

qreal KoTextLayoutArea::top() const
{
    return d->top;
}

qreal KoTextLayoutArea::bottom() const
{
    return d->bottom;
}

void KoTextLayoutArea::setBottom(qreal bottom)
{
    d->boundingRect.setBottom(bottom + qMax(qreal(0.0), d->verticalAlignOffset));
    Q_ASSERT_X(d->boundingRect.top() <= d->boundingRect.bottom(), __FUNCTION__, "Bounding-rect is not normalized");
    d->bottom = bottom;
}

void KoTextLayoutArea::findFootNotes(const QTextBlock &block, const QTextLine &line, qreal bottomOfText)
{
    if (d->documentLayout->inlineTextObjectManager() == 0) {
        return;
    }
    QString text = block.text();
    int pos = text.indexOf(QChar::ObjectReplacementCharacter, line.textStart());

    while (pos >= 0 && pos <= line.textStart() + line.textLength()) {
        QTextCursor c1(block);
        c1.setPosition(block.position() + pos);
        c1.setPosition(c1.position() + 1, QTextCursor::KeepAnchor);

        KoInlineNote *note = dynamic_cast<KoInlineNote*>(d->documentLayout->inlineTextObjectManager()->inlineTextObject(c1));
        if (note && note->type() == KoInlineNote::Footnote) {
            preregisterFootNote(note, bottomOfText);
        }

        pos = text.indexOf(QChar::ObjectReplacementCharacter, pos + 1);
    }
}

qreal KoTextLayoutArea::preregisterFootNote(KoInlineNote *note, qreal bottomOfText)
{
    if (d->parent == 0) {
        // TODO to support footnotes at end of document this is
        // where we need to add some extra condition
        if (note->autoNumbering()) {
            KoOdfNotesConfiguration *notesConfig = d->documentLayout->styleManager()->notesConfiguration(KoOdfNotesConfiguration::Footnote);
            if (notesConfig->numberingScheme() == KoOdfNotesConfiguration::BeginAtDocument) {
                note->setAutoNumber(d->footNoteCountInDoc + (d->footNoteAutoCount++));
            } else if (notesConfig->numberingScheme() == KoOdfNotesConfiguration::BeginAtPage) {
                note->setAutoNumber(d->footNoteAutoCount++);
            }
        }

        if (maximumAllowedBottom() - bottomOfText > 0) {
            QTextFrame *subFrame = note->textFrame();
            d->footNoteCursorToNext = new FrameIterator(subFrame);
            KoTextLayoutNoteArea *footNoteArea = new KoTextLayoutNoteArea(note, this, d->documentLayout);

            d->preregisteredFootNoteFrames.append(subFrame);
            footNoteArea->setReferenceRect(left(), right(), 0, maximumAllowedBottom() - bottomOfText);
            bool contNotNeeded = footNoteArea->layout(d->footNoteCursorToNext);
            if (contNotNeeded) {
                delete d->footNoteCursorToNext;
                d->footNoteCursorToNext = 0;
                d->continuedNoteToNext = 0;
            } else {
                d->continuedNoteToNext = note;
                //layout again now it has set up a continuationObstruction
                delete d->footNoteCursorToNext;
                d->footNoteCursorToNext = new FrameIterator(subFrame);
                footNoteArea->setReferenceRect(left(), right(), 0, maximumAllowedBottom() - bottomOfText);
                footNoteArea->layout(d->footNoteCursorToNext);
                documentLayout()->setContinuationObstruction(0); // remove it again
            }
            d->preregisteredFootNotesHeight += footNoteArea->bottom() - footNoteArea->top();
            d->preregisteredFootNoteAreas.append(footNoteArea);
            return footNoteArea->bottom() - footNoteArea->top();
        }
        return 0.0;
    }
    qreal h = d->parent->preregisterFootNote(note, bottomOfText);
    d->preregisteredFootNotesHeight += h;
    return h;
}

void KoTextLayoutArea::confirmFootNotes()
{
    d->footNotesHeight += d->preregisteredFootNotesHeight;
    d->footNoteAreas.append(d->preregisteredFootNoteAreas);
    d->footNoteFrames.append(d->preregisteredFootNoteFrames);
    d->preregisteredFootNotesHeight = 0;
    d->preregisteredFootNoteAreas.clear();
    d->preregisteredFootNoteFrames.clear();
    if (d->parent) {
        d->parent->confirmFootNotes();
    }
}

void KoTextLayoutArea::expandBoundingLeft(qreal x)
{
    d->boundingRect.setLeft(qMin(x, d->boundingRect.x()));
}

void KoTextLayoutArea::expandBoundingRight(qreal x)
{
    d->boundingRect.setRight(qMax(x, d->boundingRect.right()));
}

void KoTextLayoutArea::clearPreregisteredFootNotes()
{
    d->preregisteredFootNotesHeight = 0;
    d->preregisteredFootNoteAreas.clear();
    d->preregisteredFootNoteFrames.clear();
    if (d->parent) {
        d->parent->clearPreregisteredFootNotes();
    }
}

void KoTextLayoutArea::handleBordersAndSpacing(KoTextBlockData &blockData, QTextBlock *block)
{
    QTextBlockFormat format = block->blockFormat();
    KoParagraphStyle formatStyle(format, block->charFormat());

    // The AddParaTableSpacingAtStart config-item is used to be able to optionally prevent that
    // defined fo:margin-top are applied to the first paragraph. If true then the fo:margin-top
    // is applied to all except the first paragraph. If false fo:margin-top is applied to all
    // paragraphs.
    bool paraTableSpacingAtStart = KoTextDocument(d->documentLayout->document()).paraTableSpacingAtStart();
    bool paddingExpandsBorders = false;//KoTextDocument(d->documentLayout->document()).paddingExpandsBorders();

    qreal topMargin = 0;
    if (paraTableSpacingAtStart || block->previous().isValid()) {
        topMargin = formatStyle.topMargin();
    }
    qreal spacing = qMax(d->bottomSpacing, topMargin);
    qreal dx = 0.0;
    qreal x = d->x;
    qreal width = d->width;
    if (d->indent < 0) {
        x += d->indent;
        width -= d->indent;
    }
    if (blockData.hasCounterData() && blockData.counterPosition().x() < x) {
       width += x - blockData.counterPosition().x();
       x = blockData.counterPosition().x();
    }

    KoTextBlockBorderData border(QRectF(x, d->y, width, 1));
    border.setEdge(border.Left, format, KoParagraphStyle::LeftBorderStyle,
                   KoParagraphStyle::LeftBorderWidth, KoParagraphStyle::LeftBorderColor,
                   KoParagraphStyle::LeftBorderSpacing, KoParagraphStyle::LeftInnerBorderWidth);
    border.setEdge(border.Right, format, KoParagraphStyle::RightBorderStyle,
                   KoParagraphStyle::RightBorderWidth, KoParagraphStyle::RightBorderColor,
                   KoParagraphStyle::RightBorderSpacing, KoParagraphStyle::RightInnerBorderWidth);
    border.setEdge(border.Top, format, KoParagraphStyle::TopBorderStyle,
                   KoParagraphStyle::TopBorderWidth, KoParagraphStyle::TopBorderColor,
                   KoParagraphStyle::TopBorderSpacing, KoParagraphStyle::TopInnerBorderWidth);
    border.setEdge(border.Bottom, format, KoParagraphStyle::BottomBorderStyle,
                   KoParagraphStyle::BottomBorderWidth, KoParagraphStyle::BottomBorderColor,
                   KoParagraphStyle::BottomBorderSpacing, KoParagraphStyle::BottomInnerBorderWidth);
    border.setMergeWithNext(formatStyle.joinBorder());

    if (border.hasBorders()) {
        // check if we can merge with the previous parags border.
        if (d->prevBorder && d->prevBorder->equals(border)) {
            blockData.setBorder(d->prevBorder);
            // Merged mean we don't have inserts inbetween the blocks
            d->anchoringParagraphTop = d->y;
            if (d->bottomSpacing + topMargin) {
                d->anchoringParagraphTop += spacing * d->bottomSpacing / (d->bottomSpacing + topMargin);
            }
            if (!d->blockRects.isEmpty()) {
                d->blockRects.last().setBottom(d->anchoringParagraphTop);
            }
            d->anchoringParagraphTop = d->y;
            d->y += spacing;
            d->blockRects.append(QRectF(x, d->anchoringParagraphTop, width, 1.0));
        } else {
            // can't merge; then these are our new borders.
            KoTextBlockBorderData *newBorder = new KoTextBlockBorderData(border);
            blockData.setBorder(newBorder);
            if (d->prevBorder) {
                d->y += d->prevBorderPadding;
                d->y += d->prevBorder->inset(KoTextBlockBorderData::Bottom);
            }
            if (!d->blockRects.isEmpty()) {
                d->blockRects.last().setBottom(d->y);
            }
            d->anchoringParagraphTop = d->y;
            if (d->bottomSpacing + topMargin) {
                d->anchoringParagraphTop += spacing * d->bottomSpacing / (d->bottomSpacing + topMargin);
            }
            d->y += spacing;
            if (paddingExpandsBorders) {
                d->blockRects.append(QRectF(x - format.doubleProperty(KoParagraphStyle::LeftPadding), d->y,
                    width + format.doubleProperty(KoParagraphStyle::LeftPadding) + format.doubleProperty(KoParagraphStyle::RightPadding), 1.0));
            } else {
                d->blockRects.append(QRectF(x, d->y, width, 1.0));
            }
            d->y += newBorder->inset(KoTextBlockBorderData::Top);
            d->y += format.doubleProperty(KoParagraphStyle::TopPadding);
        }

        // finally, horizontal components of the borders
        dx = border.inset(KoTextBlockBorderData::Left);
        d->x += dx;
        d->width -= border.inset(KoTextBlockBorderData::Left);
        d->width -= border.inset(KoTextBlockBorderData::Right);
    } else { // this parag has no border.
        if (d->prevBorder) {
            d->y += d->prevBorderPadding;
            d->y += d->prevBorder->inset(KoTextBlockBorderData::Bottom);
        }
        blockData.setBorder(0); // remove an old one, if there was one.
        if (!d->blockRects.isEmpty()) {
            d->blockRects.last().setBottom(d->y);
        }
        d->anchoringParagraphTop = d->y;
        if (d->bottomSpacing + topMargin) {
            d->anchoringParagraphTop += spacing * d->bottomSpacing / (d->bottomSpacing + topMargin);
        }
        d->y += spacing;
        d->blockRects.append(QRectF(x, d->y, width, 1.0));
    }
    if (!paddingExpandsBorders) {
        // add padding inside the border
        dx += format.doubleProperty(KoParagraphStyle::LeftPadding);
        d->x += format.doubleProperty(KoParagraphStyle::LeftPadding);
        d->width -= format.doubleProperty(KoParagraphStyle::LeftPadding);
        d->width -= format.doubleProperty(KoParagraphStyle::RightPadding);
    }
    if (block->layout()->lineCount() == 1 && blockData.hasCounterData()) {
        blockData.setCounterPosition(QPointF(blockData.counterPosition().x() + dx, d->y));
    }
    d->prevBorder = blockData.border();
    d->prevBorderPadding = format.doubleProperty(KoParagraphStyle::BottomPadding);
    d->anchoringParagraphContentTop = d->y;
}
