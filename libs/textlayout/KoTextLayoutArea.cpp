/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008,2011 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2008 Roopesh Chander <roop@forwardbias.in>
 * Copyright (C) 2007-2008 Pierre Ducroquet <pinaraf@pinaraf.info>
 * Copyright (C) 2009-2011 KO GmbH <cbo@kogmbh.com>
 * Copyright (C) 2009-2011 Casper Boemann <cbo@boemann.dk>
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

#include "KoTextLayoutEndNotesArea.h"
#include "KoTextLayoutTableArea.h"
#include "KoTextLayoutNoteArea.h"
#include "TableIterator.h"
#include "ListItemsHelper.h"
#include "RunAroundHelper.h"
#include "KoTextDocumentLayout.h"
#include "KoTextLayoutObstruction.h"
#include "FrameIterator.h"
#include "ToCGenerator.h"
#include "BibliographyGenerator.h"
#include "KoPointedAt.h"

#include <KoParagraphStyle.h>
#include <KoCharacterStyle.h>
#include <KoListStyle.h>
#include <KoTableStyle.h>
#include <KoStyleManager.h>
#include <KoTextBlockData.h>
#include <KoTextBlockBorderData.h>
#include <KoTextBlockPaintStrategyBase.h>
#include <KoText.h>
#include <KoChangeTracker.h>
#include <KoChangeTrackerElement.h>
#include <KoImageData.h>
#include <KoImageCollection.h>
#include <KoInlineNote.h>
#include <KoInlineNote.h>
#include <KoTextSoftPageBreak.h>
#include <KoInlineTextObjectManager.h>
#include <KoTableOfContentsGeneratorInfo.h>
#include <KoBibliographyInfo.h>

#include <KDebug>

#include <QTextFrame>
#include <QTextTable>
#include <QTextList>
#include <QStyle>
#include <QFontMetrics>
#include <QTextFragment>
#include <QTextLayout>
#include <QTextCursor>
#include <QMessageBox>

extern int qt_defaultDpiY();

#define DropCapsAdditionalFormattingId 25602902
#define PresenterFontStretch 1.2

KoTextLayoutArea::KoTextLayoutArea(KoTextLayoutArea *p, KoTextDocumentLayout *documentLayout)
 : m_parent(p)
 , m_documentLayout(documentLayout)
 , m_left(0.0)
 , m_right(0.0)
 , m_top(0.0)
 , m_bottom(0.0)
 , m_maximalAllowedBottom(0.0)
 , m_maximumAllowedWidth(0.0)
 , m_isLayoutEnvironment(false)
 , m_actsHorizontally(false)
 , m_dropCapsWidth(0)
 , m_startOfArea(0)
 , m_endOfArea(0)
 , m_acceptsPageBreak(false)
 , m_virginPage(true)
 , m_verticalAlignOffset(0)
 , m_preregisteredFootNotesHeight(0)
 , m_footNotesHeight(0)
 , m_footNoteAutoCount(0)
 , m_extraTextIndent(0)
 , m_endNotesArea(0)
{
}

KoTextLayoutArea::~KoTextLayoutArea()
{
    qDeleteAll(m_tableAreas);
    qDeleteAll(m_footNoteAreas);
    qDeleteAll(m_preregisteredFootNoteAreas);
    delete m_startOfArea;
    delete m_endOfArea;
}


KoPointedAt KoTextLayoutArea::hitTest(const QPointF &p, Qt::HitTestAccuracy accuracy) const
{
    QPointF point = p - QPointF(0, m_verticalAlignOffset);

    if (m_startOfArea == 0) // We have not been layouted yet
        return KoPointedAt();

    KoPointedAt pointedAt;
    bool basicallyFound = false;

    QTextFrame::iterator it = m_startOfArea->it;
    QTextFrame::iterator stop = m_endOfArea->it;
    if(!stop.currentBlock().isValid() || m_endOfArea->lineTextStart >= 0) {
        ++stop;
    }
    int tableAreaIndex = 0;
    int tocIndex = 0;
    int footNoteIndex = 0;
    bool atEnd = false;
    for (; it != stop && !atEnd; ++it) {
        atEnd = it.atEnd();
        QTextBlock block = it.currentBlock();
        QTextTable *table = qobject_cast<QTextTable*>(it.currentFrame());
        QTextFrame *subFrame = it.currentFrame();
        QTextBlockFormat format = block.blockFormat();

        if (table) {
            if (tableAreaIndex >= m_tableAreas.size()) {
                continue;
            }
            if (point.y() > m_tableAreas[tableAreaIndex]->top()
                    && point.y() < m_tableAreas[tableAreaIndex]->bottom()) {
                return m_tableAreas[tableAreaIndex]->hitTest(point, accuracy);
            }
            ++tableAreaIndex;
            continue;
        } else if (subFrame) {
            if (it.currentFrame()->format().intProperty(KoText::SubFrameType) == KoText::AuxillaryFrameType) {
                if (point.y() > m_endNotesArea->top()
                        && point.y() < m_endNotesArea->bottom()) {
                    pointedAt = m_endNotesArea->hitTest(point, accuracy);
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
            if (point.y() > m_generatedDocAreas[tocIndex]->top()
                    && point.y() < m_generatedDocAreas[tocIndex]->bottom()) {
                pointedAt = m_generatedDocAreas[tocIndex]->hitTest(point, accuracy);
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
            QRectF lineRect = line.naturalTextRect();
            if (point.y() > line.y() + line.height()) {
                pointedAt.position = block.position() + line.textStart() + line.textLength();
                pointedAt.fillInBookmark(QTextCursor(block), m_documentLayout->inlineTextObjectManager());
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
                pointedAt.fillInBookmark(QTextCursor(block), m_documentLayout->inlineTextObjectManager());
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
                pointedAt.fillInBookmark(QTextCursor(block), m_documentLayout->inlineTextObjectManager());
                continue; // don't break as next line may be on same baseline
            }
            pointedAt.position = block.position() + line.xToCursor(point.x());
            pointedAt.fillInBookmark(QTextCursor(block), m_documentLayout->inlineTextObjectManager());
            return pointedAt;
        }
    }
    point -= QPointF(0, bottom() - m_footNotesHeight);
    while (footNoteIndex<m_footNoteAreas.length()) {
        // check if p is over foot notes area
        if (point.y() > m_footNoteAreas[footNoteIndex]->top()
                && point.y() < m_footNoteAreas[footNoteIndex]->bottom()) {
            pointedAt = m_footNoteAreas[footNoteIndex]->hitTest(point, accuracy);
            return pointedAt;
        }
        point -= QPointF(0,m_footNoteAreas[footNoteIndex]->bottom());
        ++footNoteIndex;
    }
    return pointedAt;
}

QRectF KoTextLayoutArea::selectionBoundingBox(QTextCursor &cursor) const
{
    QRectF retval(-5E6, top(), 105E6, 0);

    if (m_startOfArea == 0) // We have not been layouted yet
        return QRectF();
    if (m_endOfArea == 0) // no end area yet
        return QRectF();

    QTextFrame::iterator it = m_startOfArea->it;
    QTextFrame::iterator stop = m_endOfArea->it;
    if(!stop.currentBlock().isValid() || m_endOfArea->lineTextStart >= 0) {
        ++stop;
    }

    QTextFrame *subFrame;
    int footNoteIndex = 0;
    qreal offset = bottom() - m_footNotesHeight;
    while (footNoteIndex < m_footNoteAreas.length()) {
        subFrame = m_footNoteFrames[footNoteIndex];
        if (cursor.selectionStart() >= subFrame->firstPosition() && cursor.selectionEnd() <= subFrame->lastPosition()) {
            return m_footNoteAreas[footNoteIndex]->selectionBoundingBox(cursor).translated(0, offset) ;
        }
        offset += m_footNoteAreas[footNoteIndex]->bottom();
        ++footNoteIndex;
    }

    int tableAreaIndex = 0;
    int tocIndex = 0;

    bool atEnd = false;
    for (; it != stop && !atEnd; ++it) {
        atEnd = it.atEnd();
        QTextBlock block = it.currentBlock();
        QTextTable *table = qobject_cast<QTextTable*>(it.currentFrame());
        QTextFrame *subFrame = it.currentFrame();
        QTextBlockFormat format = block.blockFormat();

        if (table) {
            if (tableAreaIndex >= m_tableAreas.size()) {
                continue;
            }
            if (cursor.selectionEnd() < table->firstPosition()) {
                return retval.translated(0, m_verticalAlignOffset);
            }
            if (cursor.selectionStart() > table->lastPosition()) {
                ++tableAreaIndex;
                continue;
            }
            if (cursor.selectionStart() >= table->firstPosition() && cursor.selectionEnd() <= table->lastPosition()) {
                return m_tableAreas[tableAreaIndex]->selectionBoundingBox(cursor).translated(0, m_verticalAlignOffset);
            }
            if (cursor.selectionStart() >= table->firstPosition()) {
                retval = m_tableAreas[tableAreaIndex]->boundingRect();
            } else {
                retval |= m_tableAreas[tableAreaIndex]->boundingRect();
            }
            ++tableAreaIndex;
            continue;
        } else if (subFrame) {
            if (it.currentFrame()->format().intProperty(KoText::SubFrameType) == KoText::AuxillaryFrameType) {
                if (cursor.selectionEnd() < subFrame->firstPosition()) {
                    return retval.translated(0, m_verticalAlignOffset);
                }
                if (cursor.selectionStart() > subFrame->lastPosition()) {
                    break;
                }
                if (cursor.selectionStart() >= subFrame->firstPosition() && cursor.selectionEnd() <= subFrame->lastPosition()) {
                    return m_endNotesArea->selectionBoundingBox(cursor).translated(0, m_verticalAlignOffset);
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
                retval |= m_generatedDocAreas[tocIndex]->boundingRect();
            }
            ++tocIndex;
            continue;
        }

        if(cursor.selectionEnd() < block.position()) {
            return retval.translated(0, m_verticalAlignOffset);
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
    return retval.translated(0, m_verticalAlignOffset);
}


bool KoTextLayoutArea::isStartingAt(FrameIterator *cursor) const
{
    if (m_startOfArea) {
        return *m_startOfArea == *cursor;
    }

    return false;
}

QTextFrame::iterator KoTextLayoutArea::startTextFrameIterator() const
{
    return m_startOfArea->it;
}

QTextFrame::iterator KoTextLayoutArea::endTextFrameIterator() const
{
    return m_endOfArea->it;
}

void KoTextLayoutArea::backtrackKeepWithNext(FrameIterator *cursor)
{
    QTextFrame::iterator it = cursor->it;

    while (!(it == m_startOfArea->it)) {
        --it;
        QTextBlock block = it.currentBlock();
        QTextTable *table = qobject_cast<QTextTable*>(it.currentFrame());
        QTextFrame *subFrame = it.currentFrame();
        bool keepWithNext = false;
        if (table) {
            keepWithNext = table->format().boolProperty(KoTableStyle::KeepWithNext);
            //setBottom(tableArea->bottom() + m_footNotesHeight);
        } else if (subFrame) {
            Q_ASSERT(false); // there should never be an aux frame before normal layouted stuff
        } else if (block.isValid()) {
            keepWithNext = block.blockFormat().boolProperty(KoParagraphStyle::KeepWithNext);
            //setBottom(m_blockRects.last()->bottom() + m_footNotesHeight);
        }
        if (!keepWithNext) {
            break;
        }
        --(cursor->it);
    }
}

bool KoTextLayoutArea::layout(FrameIterator *cursor)
{
    qDeleteAll(m_tableAreas);
    m_tableAreas.clear();
    qDeleteAll(m_footNoteAreas);
    m_footNoteAreas.clear();
    qDeleteAll(m_preregisteredFootNoteAreas);
    m_preregisteredFootNoteAreas.clear();
    m_footNoteFrames.clear();
    m_preregisteredFootNoteFrames.clear();
    qDeleteAll(m_generatedDocAreas);
    m_generatedDocAreas.clear();
    m_blockRects.clear();
    delete m_endNotesArea;
    m_endNotesArea=0;
    delete m_startOfArea;
    delete m_endOfArea;
    m_dropCapsWidth = 0;

    m_startOfArea = new FrameIterator(cursor);
    m_endOfArea = 0;
    m_y = top();
    setBottom(top());
    m_bottomSpacing = 0;
    m_footNoteAutoCount = 0;
    m_footNotesHeight = 0;
    m_preregisteredFootNotesHeight = 0;
    m_prevBorder = 0;
    m_prevBorderPadding = 0;

    while (!cursor->it.atEnd()) {
        QTextBlock block = cursor->it.currentBlock();
        QTextTable *table = qobject_cast<QTextTable*>(cursor->it.currentFrame());
        QTextFrame *subFrame = cursor->it.currentFrame();
        if (table) {
            bool masterPageNameChanged = false;
            QString masterPageName = table->frameFormat().property(KoTableStyle::MasterPageName).toString();
            if (!masterPageName.isEmpty() && cursor->masterPageName != masterPageName) {
                masterPageNameChanged = true;
                cursor->masterPageName = masterPageName;
            }

            if (!virginPage() && acceptsPageBreak() && (masterPageNameChanged ||
                   ((table->frameFormat().intProperty(KoTableStyle::BreakBefore) & KoText::PageBreak)))) {
                m_endOfArea = new FrameIterator(cursor);
                setBottom(m_y + m_footNotesHeight);
                if (!m_blockRects.isEmpty()) {
                    m_blockRects.last().setBottom(m_y);
                }
                return false;
            }

            // Let's create KoTextLayoutTableArea and let that handle the table
            KoTextLayoutTableArea *tableArea = new KoTextLayoutTableArea(table, this, m_documentLayout);
            m_tableAreas.append(tableArea);
            m_y += m_bottomSpacing;
            if (!m_blockRects.isEmpty()) {
                m_blockRects.last().setBottom(m_y);
            }
            tableArea->setVirginPage(virginPage());
            tableArea->setReferenceRect(left(), right(), m_y, maximumAllowedBottom());
            if (tableArea->layoutTable(cursor->tableIterator(table)) == false) {
                m_endOfArea = new FrameIterator(cursor);
                m_y = tableArea->bottom();
                setBottom(m_y + m_footNotesHeight);
                // Expand bounding rect so if we have content outside we show it
                expandBoundingLeft(tableArea->boundingRect().left());
                expandBoundingRight(tableArea->boundingRect().right());

                return false;
            }
            setVirginPage(false);
            // Expand bounding rect so if we have content outside we show it
            expandBoundingLeft(tableArea->boundingRect().left());
            expandBoundingRight(tableArea->boundingRect().right());
            m_bottomSpacing = 0;
            m_y = tableArea->bottom();
            delete cursor->currentTableIterator;
            cursor->currentTableIterator = 0;
        } else if (subFrame) {
            if (subFrame->format().intProperty(KoText::SubFrameType) == KoText::AuxillaryFrameType) {
                Q_ASSERT(m_endNotesArea == 0);
                m_endNotesArea = new KoTextLayoutEndNotesArea(this, m_documentLayout);
                m_y += m_bottomSpacing;
                if (!m_blockRects.isEmpty()) {
                    m_blockRects.last().setBottom(m_y);
                }
                m_endNotesArea->setVirginPage(virginPage());
                m_endNotesArea->setReferenceRect(left(), right(), m_y, maximumAllowedBottom());
                if (m_endNotesArea->layout(cursor->subFrameIterator(subFrame)) == false) {
                    m_endOfArea = new FrameIterator(cursor);
                    m_y = m_endNotesArea->bottom();
                    setBottom(m_y + m_footNotesHeight);
                    // Expand bounding rect so if we have content outside we show it
                    expandBoundingLeft(m_endNotesArea->boundingRect().left());
                    expandBoundingRight(m_endNotesArea->boundingRect().right());
                    return false;
                }
                setVirginPage(false);
                // Expand bounding rect so if we have content outside we show it
                expandBoundingLeft(m_endNotesArea->boundingRect().left());
                expandBoundingRight(m_endNotesArea->boundingRect().right());
                m_bottomSpacing = 0;
                m_y = m_endNotesArea->bottom();
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
                m_generatedDocAreas.append(area);
                m_y += m_bottomSpacing;
                if (!m_blockRects.isEmpty()) {
                    m_blockRects.last().setBottom(m_y);
                }
                area->setVirginPage(virginPage());
                area->setAcceptsPageBreak(acceptsPageBreak());
                area->setReferenceRect(left(), right(), m_y, maximumAllowedBottom());
                QTextLayout *blayout = block.layout();
                blayout->beginLayout();
                QTextLine line = blayout->createLine();
                line.setNumColumns(0);
                line.setPosition(QPointF(left(), m_y));
                blayout->endLayout();

                if (area->layout(cursor->subFrameIterator(generatedDocument->rootFrame())) == false) {
                    cursor->lineTextStart = 1; // fake we are not done
                    m_endOfArea = new FrameIterator(cursor);
                    m_y = area->bottom();
                    setBottom(m_y + m_footNotesHeight);
                    // Expand bounding rect so if we have content outside we show it
                    expandBoundingLeft(area->boundingRect().left());
                    expandBoundingRight(area->boundingRect().right());
                    return false;
                }
                setVirginPage(false);
                // Expand bounding rect so if we have content outside we show it
                expandBoundingLeft(area->boundingRect().left());
                expandBoundingRight(area->boundingRect().right());
                m_bottomSpacing = 0;
                m_y = area->bottom();
                delete cursor->currentSubFrameIterator;
                cursor->lineTextStart = -1; // fake we are done
                cursor->currentSubFrameIterator = 0;
            } else {
                // FIXME this doesn't work for cells inside tables. We probably should make it more
                // generic to handle such cases too.
                bool masterPageNameChanged = false;
                QString masterPageName = block.blockFormat().property(KoParagraphStyle::MasterPageName).toString();
                if (!masterPageName.isEmpty() && cursor->masterPageName != masterPageName) {
                    masterPageNameChanged = true;
                    cursor->masterPageName = masterPageName;
                }

                if (!virginPage() && acceptsPageBreak() &&
                    (masterPageNameChanged ||
                        (block.blockFormat().intProperty(KoParagraphStyle::BreakBefore) & KoText::PageBreak))) {
                    m_endOfArea = new FrameIterator(cursor);
                    setBottom(m_y + m_footNotesHeight);
                    if (!m_blockRects.isEmpty()) {
                        m_blockRects.last().setBottom(m_y);
                    }
                    return false;
                }

                if (layoutBlock(cursor) == false) {
                    if (cursor->lineTextStart == -1) {
                        //Nothing was added so lets backtrack keep-with-next
                        backtrackKeepWithNext(cursor);
                    }
                    m_endOfArea = new FrameIterator(cursor);
                    setBottom(m_y + m_footNotesHeight);
                    m_blockRects.last().setBottom(m_y);
                    return false;
                }
                m_extraTextIndent = 0;

                if (acceptsPageBreak()
                    && (block.blockFormat().intProperty(KoParagraphStyle::BreakAfter) & KoText::PageBreak)) {
                    Q_ASSERT(!cursor->it.atEnd());
                    QTextFrame::iterator nextIt = cursor->it;
                    ++nextIt;
                    bool wasIncremented = !nextIt.currentFrame();
                    if (wasIncremented)
                        cursor->it = nextIt;
                    m_endOfArea = new FrameIterator(cursor);
                    if (!wasIncremented)
                        ++(cursor->it);
                    setBottom(m_y + m_footNotesHeight);
                    m_blockRects.last().setBottom(m_y);
                    return false;
                }
            }
        }
        bool atEnd = cursor->it.atEnd();
        if (!atEnd) {
            ++(cursor->it);
        }
    }
    m_endOfArea = new FrameIterator(cursor);
    m_y = qMin(maximumAllowedBottom(), m_y + m_bottomSpacing);
    setBottom(m_y + m_footNotesHeight);
    if (!m_blockRects.isEmpty()) {
        m_blockRects.last().setBottom(m_y);
    }
    if (m_maximumAllowedWidth>0) {
        m_left = m_boundingRect.left();
        m_right = m_boundingRect.right();
        m_maximumAllowedWidth = 0;

        KoTextLayoutArea::layout(new FrameIterator(m_startOfArea));
    }
    return true; // we have layouted till the end of the frame
}


//local type for temporary use in restartLayout
struct LineKeeper
{
    int columns;
    qreal lineWidth;
    QPointF position;
};

QTextLine KoTextLayoutArea::restartLayout(QTextLayout *layout, int lineTextStartOfLastKeep)
{
    QList<LineKeeper> lineKeeps;
    QTextLine line;
    for(int i = 0; i < layout->lineCount(); i++) {
        QTextLine l = layout->lineAt(i);
        if (l.textStart() > lineTextStartOfLastKeep) {
            break;
        }
        LineKeeper lk;
        lk.lineWidth = l.width();
        lk.columns = l.textLength();
        lk.position = l.position();
        lineKeeps.append(lk);
    }
    layout->clearLayout();
    documentLayout()->allowPositionInlineObject(false);
    layout->beginLayout();
    foreach(const LineKeeper &lk, lineKeeps) {
        line = layout->createLine();
        if (!line.isValid())
            break;
        line.setNumColumns(lk.columns, lk.lineWidth);
        line.setPosition(lk.position);
    }
    documentLayout()->allowPositionInlineObject(true);
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
    KoTextBlockData *blockData = dynamic_cast<KoTextBlockData *>(block.userData());
    KoParagraphStyle pStyle(block.blockFormat(), block.charFormat());

    int dropCapsAffectsNMoreLines = 0;
    qreal dropCapsPositionAdjust = 0.0;

    KoText::Direction dir = pStyle.textProgressionDirection();
    if (dir == KoText::InheritDirection)
        dir = parentTextDirection();
    if (dir == KoText::AutoDirection)
        m_isRtl = block.text().isRightToLeft();
    else
        m_isRtl =  dir == KoText::RightLeftTopBottom;

    // initialize list item stuff for this parag.
    QTextList *textList = block.textList();
    QTextListFormat listFormat;
    QTextCharFormat labelFormat;
    if (textList) {
        listFormat = textList->format();

        if (block.text().size() == 0 || m_documentLayout->wordprocessingMode()) {
            labelFormat = block.charFormat();
        } else {
            labelFormat = block.begin().fragment().charFormat();
        }

        if (m_documentLayout->styleManager()) {
            const int id = listFormat.intProperty(KoListStyle::CharacterStyleId);
            KoCharacterStyle *cs = m_documentLayout->styleManager()->characterStyle(id);
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

        QFont font(labelFormat.font(), m_documentLayout->paintDevice());

        if (!(blockData && blockData->hasCounterData())) {
            ListItemsHelper lih(textList, font);
            lih.recalculateBlock(block);
            blockData = dynamic_cast<KoTextBlockData*>(block.userData());
        }
        if (blockData) {
            blockData->setLabelFormat(labelFormat);
        }
    } else if (blockData) { // make sure it is empty
        blockData->clearCounter();
    }
    if (blockData == 0) {
        blockData = new KoTextBlockData();
        block.setUserData(blockData);
    }

    QTextLayout *layout = block.layout();
    QTextOption option = layout->textOption();
    option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

    option.setAlignment(QStyle::visualAlignment(m_isRtl ? Qt::RightToLeft : Qt::LeftToRight, pStyle.alignment()));
    if (m_isRtl) {
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

    m_dropCapsNChars = 0;
    if (cursor->lineTextStart == -1) {
        // first remove any drop-caps related formatting that's already there in the layout.
        // we'll do it all afresh now.
        QList<QTextLayout::FormatRange> formatRanges = layout->additionalFormats();
        for (QList< QTextLayout::FormatRange >::Iterator iter = formatRanges.begin();
                iter != formatRanges.end(); ) {
            if (iter->format.boolProperty(DropCapsAdditionalFormattingId)) {
                iter = formatRanges.erase(iter);
            } else {
                ++iter;
            }
        }
        if (formatRanges.count() != layout->additionalFormats().count())
            layout->setAdditionalFormats(formatRanges);
        bool dropCaps = pStyle.dropCaps();
        int dropCapsLength = pStyle.dropCapsLength();
        int dropCapsLines = pStyle.dropCapsLines();

        if (dropCaps && dropCapsLength != 0 && dropCapsLines > 1 && block.length() > 1) {
            QString blockText = block.text();

            if (dropCapsLength < 0) { // means whole word is to be dropped
                int firstNonSpace = blockText.indexOf(QRegExp("[^ ]"));
                dropCapsLength = blockText.indexOf(QRegExp("\\W"), firstNonSpace);
            } else {
                // LibreOffice skips softbreaks but not spaces. We will do the same
                QTextCursor c1(block);
                c1.setPosition(block.position());
                c1.setPosition(c1.position() + 1, QTextCursor::KeepAnchor);

                KoTextSoftPageBreak *softPageBreak = dynamic_cast<KoTextSoftPageBreak*>(m_documentLayout->inlineTextObjectManager()->inlineTextObject(c1));
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
                        int percent = pStyle.lineHeightPercent();
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
                if (dropCapsStyleId > 0 && m_documentLayout->styleManager()) {
                    dropCapsCharStyle = m_documentLayout->styleManager()->characterStyle(dropCapsStyleId);
                    dropCapsCharStyle->applyStyle(dropCapsFormatRange.format);
                }

                QFont f(dropCapsFormatRange.format.font(), m_documentLayout->paintDevice());
                QString dropCapsText(block.text().left(dropCapsLength));
                f.setPointSizeF(dropCapsHeight);
                for (int i=0; i < 5; ++i) {
                    QTextLayout tmplayout(dropCapsText, f);
                    tmplayout.setTextOption(option);
                    tmplayout.beginLayout();
                    QTextLine tmpline = tmplayout.createLine();
                    tmplayout.endLayout();
                    m_dropCapsWidth = tmpline.naturalTextWidth();

                    QFontMetricsF fm(f, documentLayout()->paintDevice());
                    QRectF rect = fm.tightBoundingRect(dropCapsText);
                    const qreal diff = dropCapsHeight - rect.height();
                    dropCapsPositionAdjust = rect.top() + fm.ascent();
                    if (qAbs(diff < 0.5)) // good enough
                        break;

                    const qreal adjustment = diff * (f.pointSizeF() / rect.height());
                    // kDebug() << "adjusting with" << adjustment;
                    f.setPointSizeF(f.pointSizeF() + adjustment);
                }

                dropCapsFormatRange.format.setFontPointSize(f.pointSizeF());
                dropCapsFormatRange.format.setProperty(DropCapsAdditionalFormattingId,
                        (QVariant) true);
                dropCapsFormatRange.start = 0;
                dropCapsFormatRange.length = dropCapsLength;
                formatRanges.append(dropCapsFormatRange);
                layout->setAdditionalFormats(formatRanges);

                m_dropCapsNChars = dropCapsLength;
                dropCapsAffectsNMoreLines = (m_dropCapsNChars > 0) ? dropCapsLines : 0;
            }
        }
    }

    //========
    // Margins
    //========
    qreal startMargin = block.blockFormat().leftMargin();
    qreal endMargin = block.blockFormat().rightMargin();
    if (m_isRtl) {
        qSwap(startMargin, endMargin);
    }
    m_indent = textIndent(block, textList, pStyle);

    qreal labelBoxWidth = 0;
    qreal labelBoxIndent = 0;
    if (textList) {
        if (listFormat.boolProperty(KoListStyle::AlignmentMode)) {
            // according to odf 1.2 17.20 list margin should be used when paragraph margin is
            // not specified by the auto style (additionally LO/OO uses 0 as condition so we do too)
            int id = pStyle.styleId();
            bool set = false;
            if (id && m_documentLayout->styleManager()) {
                KoParagraphStyle *originalParagraphStyle = m_documentLayout->styleManager()->paragraphStyle(id);
                if (originalParagraphStyle->leftMargin() != startMargin) {
                    set = (startMargin != 0);
                }
            } else {
                set = (startMargin != 0);
            }
            if (! set) {
                startMargin = listFormat.doubleProperty(KoListStyle::Margin);
            }

            labelBoxWidth = blockData->counterWidth();
            Qt::Alignment align = static_cast<Qt::Alignment>(listFormat.intProperty(KoListStyle::Alignment));
            if (align == 0) {
                align = Qt::AlignLeft;
            }
            if (align & Qt::AlignLeft) {
                m_indent += labelBoxWidth;
            } else if (align & Qt::AlignHCenter) {
                m_indent += labelBoxWidth/2;
            }
            labelBoxIndent = m_indent - labelBoxWidth;
        } else {
            labelBoxWidth = blockData->counterSpacing() + blockData->counterWidth();
        }
    }

    m_width = right() - left();
    m_width -= startMargin + endMargin;
    m_x = left() + (m_isRtl ? 0.0 : startMargin);

    m_documentLayout->clearInlineObjectRegistry(block);

    //========
    // Tabs
    //========
    QList<KoText::Tab> tabs = pStyle.tabPositions();

    // Handle tabs relative to startMargin
    qreal tabOffset = -m_indent;
    if (!m_documentLayout->relativeTabs(block)) {
        tabOffset -= startMargin;
    }

    // Make a list of tabs that Qt can use
    QList<QTextOption::Tab> qTabs;
    // Note: Converting to Qt tabs is needed as long as we use Qt for layout, but we
    // loose the posibility to do leader chars.
    foreach (KoText::Tab kTab, tabs) {
        qreal value = kTab.position;
        if (value == MaximumTabPos) { // MaximumTabPos is used in index generators
            // note: we subtract right margin as this is where the tab should be
            // note: we subtract indent so tab is not relative to it
            // note: we subtract left margin so tab is not relative to it
            // if rtl the above left/right reasons swap but formula stays the same
            // -tabOfset is just to cancel that we add it next
            // -2 is to avoid wrap at right edge to the next line
            value = right() - left() - startMargin - endMargin - m_indent - tabOffset - 2;
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
            if (!m_documentLayout->relativeTabs(block)) {
                // How list tab is defined if fixed tabs:
                //        listTab
                //|>-------------------------|
                //           m_indent
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
                //             m_indent
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
        //           m_indent
        //         |---------<|
        //     LABEL                 TEXT STARTS HERE AND GOES ON
        //                    TO THE NEXT LINE
        //|>------------------|
        //     startMargin
        presentationListTabValue = listTab;
        listTab -= m_indent;

        // And now listTab is like this:
        //         x()
        //         |     listTab
        //         |>---------------|
        //           m_indent
        //         |---------<|
        //     LABEL                 TEXT STARTS HERE AND GOES ON
        //                    TO THE NEXT LINE
        //|>------------------|
        //     startMargin

        // conversion here is required because Qt thinks in device units and we don't
        listTab *= qt_defaultDpiY() / 72.0;

        QTextOption::Tab tab;
        tab.position = listTab;
        tab.type = m_isRtl ? QTextOption::RightTab : QTextOption::LeftTab;
        qTabs.append(tab);
    }

    // We need to sort as the MaximumTabPos may be converted to a value that really
    // should be in the middle, and listtab needs to be sorted in too
    qSort(qTabs.begin(), qTabs.end(), compareTab);

    // Regular interval tabs. Since Qt doesn't handle regular interval tabs offset
    // by a fixed number we need to create the regular tabs ourselves.
    qreal tabStopDistance = pStyle.tabStopDistance() * qt_defaultDpiY() / 72.0;
    if (tabStopDistance <= 0) {
        tabStopDistance = m_documentLayout->defaultTabSpacing() * qt_defaultDpiY() / 72.0;
    }

    qreal regularSpacedTabPos = -m_indent * qt_defaultDpiY() / 72.0 -0.1; // first possible position
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
    // Setup line and possibly restart paragraph continuing from previous other area
    // ==============
    QTextLine line;
    if (cursor->lineTextStart == -1) {
        layout->beginLayout();
        line = layout->createLine();
        cursor->fragmentIterator = block.begin();
    } else {
        line = restartLayout(layout, cursor->lineTextStart);
        m_indent = 0;
    }

    if (block.blockFormat().boolProperty(KoParagraphStyle::UnnumberedListItem)) {
        // Unnumbered list items act like "following lines" in a numbered block
        m_indent = 0;
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
            if (!m_isRtl) {
                m_x += listFormat.doubleProperty(KoListStyle::Indent) + minLabelWidth;
            }
            m_width -= listFormat.doubleProperty(KoListStyle::Indent) + minLabelWidth;
            m_indent +=  labelBoxWidth - minLabelWidth;
            blockData->setCounterPosition(QPointF(m_x + m_indent - labelBoxWidth, m_y));
        } else if (labelBoxWidth > 0.0 || blockData->counterText().length() > 0) {
            // Alignmentmode and there is a label (double check needed to acount for both
            // picture bullets and non width chars)
            blockData->setCounterPosition(QPointF(m_x + labelBoxIndent, m_y));
            if (listFormat.intProperty(KoListStyle::LabelFollowedBy) == KoListStyle::ListTab
                && !presentationListTabWorkaround(textIndent(block, textList, pStyle), labelBoxWidth, presentationListTabValue)) {
                foreach(QTextOption::Tab tab, qTabs) {
                    qreal position = tab.position  * 72. / qt_defaultDpiY();
                    if (position > 0.0) {
                        m_indent += position;
                        break;
                    }
                }

                //And finally it's like this:
                //                          x()
                //                    m_indent
                //                    |>-----|
                //     LABEL                 TEXT STARTS HERE AND GOES ON
                //                    TO THE NEXT LINE
                //|>------------------|
                //     startMargin
            } else if (listFormat.intProperty(KoListStyle::LabelFollowedBy) == KoListStyle::Space) {
                 QFontMetrics fm(labelFormat.font(), m_documentLayout->paintDevice());
                 m_indent += fm.width(' ');
            }
            // default needs to be no space so presentationListTabWorkaround above makes us go here
        }
    }

    // ==============
    // Now once we know the physical context we can work on the borders of the paragraph
    // ==============
    if (block.blockFormat().hasProperty(KoParagraphStyle::HiddenByTable)) {
        if (!m_blockRects.isEmpty()) {
            m_blockRects.last().setBottom(m_y);
        }
        m_y += m_bottomSpacing;
        m_bottomSpacing = 0;
        m_blockRects.append(QRectF(m_x, m_y, m_width, 10.0));
    } else {
        handleBordersAndSpacing(blockData, &block);
    }

    // Expand bounding rect so if we have content outside we show it
    expandBoundingLeft(m_blockRects.last().x());
    expandBoundingRight(m_blockRects.last().right());

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
        QRectF anchoringRect = m_blockRects.last();
//        qDebug() << anchoringRect.top() << m_anchoringParagraphTop;
        anchoringRect.setTop(m_anchoringParagraphTop);
        documentLayout()->setAnchoringParagraphRect(anchoringRect);
        documentLayout()->setAnchoringLayoutEnvironmentRect(layoutEnvironmentRect());
        runAroundHelper.fit( /* resetHorizontalPosition */ false, /* rightToLeft */ m_isRtl, QPointF(x(), m_y));
        qreal bottomOfText = line.y() + line.height();

        bool softBreak = false;
        bool moreInMiddle = m_y > maximumAllowedBottom() - 150;
        if (acceptsPageBreak() && !pStyle.nonBreakableLines() && moreInMiddle) {
            int softBreakPos = -1;
            QString text = block.text();
            int pos = text.indexOf(QChar::ObjectReplacementCharacter, line.textStart());

            while (pos >= 0 && pos <= line.textStart() + line.textLength()) {
                QTextCursor c1(block);
                c1.setPosition(block.position() + pos);
                c1.setPosition(c1.position() + 1, QTextCursor::KeepAnchor);

                KoTextSoftPageBreak *softPageBreak = dynamic_cast<KoTextSoftPageBreak*>(m_documentLayout->inlineTextObjectManager()->inlineTextObject(c1));
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
                layout->endLayout();
                return false;
            }
        }

        findFootNotes(block, line);
        if (bottomOfText > maximumAllowedBottom()) {
            // We can not fit line within our allowed space
            // in case we resume layout on next page the line is reused later
            // but if not then we need to make sure the line becomes invisible
            // we use m_maximalAllowedBottom because we want to be below
            // footnotes too.
            if (!virginPage() && pStyle.nonBreakableLines()) {
                line.setPosition(QPointF(x(), m_maximalAllowedBottom));
                cursor->lineTextStart = -1;
                layout->endLayout();
                clearPreregisteredFootNotes();
                return false; //to indicate block was not done!
            }
            if (!virginPage() && pStyle.orphanThreshold() != 0
                              && pStyle.orphanThreshold() > numBaselineShifts) {
                line.setPosition(QPointF(x(), m_maximalAllowedBottom));
                cursor->lineTextStart = -1;
                layout->endLayout();
                clearPreregisteredFootNotes();
                return false; //to indicate block was not done!
            }
            if (!virginPage() || anyLineAdded) {
                line.setPosition(QPointF(x(), m_maximalAllowedBottom));
                layout->endLayout();
                clearPreregisteredFootNotes();
                return false; //to indicate block was not done!
            }
        }
        confirmFootNotes();
        anyLineAdded = true;
        maxLineHeight = qMax(maxLineHeight, addLine(line, cursor, blockData));

        if (!runAroundHelper.stayOnBaseline() && !(block.blockFormat().hasProperty(KoParagraphStyle::HiddenByTable)
         && block.length() <= 1)) {
            m_y += maxLineHeight;
            maxLineHeight = 0;
            m_indent = 0;
            ++numBaselineShifts;
        }

        // drop caps
        if (m_dropCapsNChars > 0) { // we just laid out the dropped chars
            y_justBelowDropCaps = m_y; // save the y position just below the dropped characters
            m_y = line.y();              // keep the same y for the next line
            line.setPosition(line.position() - QPointF(0, dropCapsPositionAdjust));
            m_dropCapsNChars -= line.textLength();
        } else if (dropCapsAffectsNMoreLines > 0) { // we just laid out a drop-cap-affected line
            dropCapsAffectsNMoreLines--;
            if (dropCapsAffectsNMoreLines == 0) {   // no more drop-cap-affected lines
                if (m_y < y_justBelowDropCaps)
                    m_y = y_justBelowDropCaps; // make sure m_y is below the dropped characters
                y_justBelowDropCaps = 0;
                m_dropCapsWidth = 0;
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
            layout->endLayout();
            return false; // page-break means we need to start again on the next page
        }
    }

    m_bottomSpacing = pStyle.bottomMargin();

    layout->endLayout();
    setVirginPage(false);
    cursor->lineTextStart = -1; //set lineTextStart to -1 and returning true indicate new block
    return true;
}

bool KoTextLayoutArea::presentationListTabWorkaround(qreal indent, qreal labelBoxWidth, qreal presentationListTabValue)
{
    if (!m_documentLayout->wordprocessingMode() && indent < 0.0) {
        // Impress / Powerpoint expects the label to be before the text
        if (indent + labelBoxWidth >= presentationListTabValue) {
            // but here is an unforseen overlap with normal text
            return true;
        }
    }
    return false;
}

qreal KoTextLayoutArea::textIndent(QTextBlock block, QTextList *textList, const KoParagraphStyle &pStyle) const
{
    if (pStyle.autoTextIndent()) {
        // if auto-text-indent is set,
        // return an indent approximately 3-characters wide as per current font
        QTextCursor blockCursor(block);
        qreal guessGlyphWidth = QFontMetricsF(blockCursor.charFormat().font()).width('x');
        return guessGlyphWidth * 3 + m_extraTextIndent;
    }

    qreal blockTextIndent = block.blockFormat().textIndent();

    if (textList && textList->format().boolProperty(KoListStyle::AlignmentMode)) {
        // according to odf 1.2 17.20 list text indent should be used when paragraph text indent is
        // not specified (additionally LO/OO uses 0 as condition so we do too)
        int id = pStyle.styleId();
        bool set = false;
        if (id && m_documentLayout->styleManager()) {
            KoParagraphStyle *originalParagraphStyle = m_documentLayout->styleManager()->paragraphStyle(id);
            if (originalParagraphStyle->textIndent() != blockTextIndent) {
                set = (blockTextIndent != 0);
            }
        } else {
            set = (blockTextIndent != 0);
        }
        if (! set) {
            return textList->format().doubleProperty(KoListStyle::TextIndent) + m_extraTextIndent;
        }
    }
    return blockTextIndent + m_extraTextIndent;
}

void KoTextLayoutArea::setExtraTextIndent(qreal extraTextIndent)
{
    m_extraTextIndent = extraTextIndent;
}

qreal KoTextLayoutArea::x() const
{
    if (m_isRtl) {
        return m_x;
    } else {
        return m_x + m_indent + (m_dropCapsNChars > 0 ? 0.0 : m_dropCapsWidth);
    }
}

qreal KoTextLayoutArea::width() const
{
    if (m_dropCapsNChars > 0) {
        return m_dropCapsWidth;
    }
    qreal width = m_width;
    if (m_maximumAllowedWidth > 0) {
        // lets use that instead but remember all the indent stuff we have calculated
        width = m_width - (m_right - m_left) + m_maximumAllowedWidth;
    }
    return width - m_indent - m_dropCapsWidth;
}

void KoTextLayoutArea::setAcceptsPageBreak(bool accept)
{
    m_acceptsPageBreak = accept;
}

bool KoTextLayoutArea::acceptsPageBreak() const
{
    return m_acceptsPageBreak;
}

void KoTextLayoutArea::setVirginPage(bool virgin)
{
    m_virginPage = virgin;
}

bool KoTextLayoutArea::virginPage() const
{
    return m_virginPage;
}

void KoTextLayoutArea::setVerticalAlignOffset(qreal offset)
{
    m_boundingRect.setTop(m_top + qMin(qreal(0.0), offset));
    m_boundingRect.setBottom(m_bottom + qMax(qreal(0.0), offset));
    Q_ASSERT_X(m_boundingRect.top() <= m_boundingRect.bottom(), __FUNCTION__, "Bounding-rect is not normalized");
    m_verticalAlignOffset = offset;
}

qreal KoTextLayoutArea::verticalAlignOffset() const
{
    return m_verticalAlignOffset;
}

qreal KoTextLayoutArea::addLine(QTextLine &line, FrameIterator *cursor, KoTextBlockData *blockData)
{
    QTextBlock block = cursor->it.currentBlock();
    QTextBlockFormat format = block.blockFormat();
    KoParagraphStyle style(format, block.charFormat());

    if (blockData && block.textList() && block.layout()->lineCount() == 1) {
        Qt::Alignment alignment = format.alignment();
        if (m_isRtl && (alignment & Qt::AlignAbsolute) == 0) {
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
            newX = blockData->counterPosition().x() + (m_isRtl ? -padding : padding);
        } else if (alignment & Qt::AlignRight) {
            const qreal padding = line.width() - line.naturalTextWidth();
            newX = blockData->counterPosition().x() + (m_isRtl ? -padding : padding);
        } else {
            newX = blockData->counterPosition().x();
        }
        if (m_isRtl) {
            newX = line.x() + line.naturalTextWidth() + line.x() + m_indent - newX;
        }

        blockData->setCounterPosition(QPointF(newX, blockData->counterPosition().y()));
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

        KoInlineObjectExtent pos = m_documentLayout->inlineObjectExtent(fragment);
        ascent = qMax(ascent, pos.m_ascent);
        descent = qMax(descent, pos.m_descent);

        bool lineBreak = false;
        int lastCharPos = block.position() + line.textStart() + line.textLength() - 1;
        if (block.text().at(line.textStart() + line.textLength() - 1) == QChar(0x2028)) {
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
            if (!m_documentLayout->changeTracker()
                || !m_documentLayout->changeTracker()->displayChanges()
                || !m_documentLayout->changeTracker()->containsInlineChanges(fragment.charFormat())
                || !m_documentLayout->changeTracker()->elementById(fragment.charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt())
                || !m_documentLayout->changeTracker()->elementById(fragment.charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt())->isEnabled()
                || (m_documentLayout->changeTracker()->elementById(fragment.charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt())->getChangeType() != KoGenChange::DeleteChange)
                || m_documentLayout->changeTracker()->displayChanges()) {
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

                KoInlineObjectExtent pos = m_documentLayout->inlineObjectExtent(fragment);
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

                    KoInlineObjectExtent pos = m_documentLayout->inlineObjectExtent(fragment);
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
    qreal fixedLineHeight = format.doubleProperty(KoParagraphStyle::FixedLineHeight);
    if (fixedLineHeight != 0.0) {
        qreal prevHeight = height;
        height = fixedLineHeight;
        lineAdjust += height - prevHeight;
    } else {
        qreal lineSpacing = format.doubleProperty(KoParagraphStyle::LineSpacing);
        if (lineSpacing == 0.0) { // unset
            int percent = format.intProperty(KoParagraphStyle::PercentLineHeight);
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

    //rounding problems due to Qt-scribe internally using ints.
    //also used when line was moved down because of intersections with other shapes
    if (qAbs(m_y - line.y()) >= 0.126) {
        m_y = line.y();
    }

    if (lineAdjust) {
        // Adjust the position of the line itself.
        line.setPosition(QPointF(line.x(), line.y() + lineAdjust));

        // Adjust the position of the block-rect for this line which is used later
        // to proper clip the line while drawing. If we would not adjust it here
        // then we could end with text-lines being partly cutoff.
        if (lineAdjust < 0.0) {
            m_blockRects.last().moveTop(m_blockRects.last().top() + lineAdjust);
        }

        if (blockData && block.textList() && block.layout()->lineCount() == 1) {
            // If this is the first line in a list (aka the first line after the list-
            // item) then we also need to adjust the counter to match to the line again.
            blockData->setCounterPosition(QPointF(blockData->counterPosition().x(), blockData->counterPosition().y() + lineAdjust));
        }
    }

    return height;
}

void KoTextLayoutArea::setLayoutEnvironmentResctictions(bool isLayoutEnvironment, bool actsHorizontally)
{
    m_isLayoutEnvironment = isLayoutEnvironment;
    m_actsHorizontally = actsHorizontally;
}

QRectF KoTextLayoutArea::layoutEnvironmentRect() const
{
    QRectF rect(-5e10, -5e10, 10e10, 10e20); // large values that never really restrict anything

    if (m_parent) {
        rect = m_parent->layoutEnvironmentRect();
    }

    if (m_isLayoutEnvironment) {
        if (m_actsHorizontally) {
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
    return m_boundingRect;
}

qreal KoTextLayoutArea::maximumAllowedBottom() const
{
    return m_maximalAllowedBottom - m_footNotesHeight
                    - m_preregisteredFootNotesHeight;
}

void KoTextLayoutArea::setNoWrap(qreal maximumAllowedWidth)
{
    m_maximumAllowedWidth = maximumAllowedWidth;
}

KoText::Direction KoTextLayoutArea::parentTextDirection() const
{
    Q_ASSERT(m_parent); //Root areas should overload this method
    return m_parent->parentTextDirection();
}

KoTextLayoutArea *KoTextLayoutArea::parent() const
{
    return m_parent;
}

KoTextDocumentLayout *KoTextLayoutArea::documentLayout() const
{
    return m_documentLayout;
}

void KoTextLayoutArea::setReferenceRect(qreal left, qreal right, qreal top, qreal maximumAllowedBottom)
{
    m_left = left;
    m_right = right;
    m_top = top;
    m_boundingRect = QRectF(left, top, right - left, 0.0);
    Q_ASSERT_X(m_boundingRect.top() <= m_boundingRect.bottom() && m_boundingRect.left() <= m_boundingRect.right(), __FUNCTION__, "Bounding-rect is not normalized");
    m_maximalAllowedBottom = maximumAllowedBottom;
}

QRectF KoTextLayoutArea::referenceRect() const
{
    return QRectF(m_left, m_top, m_right - m_left, m_bottom - m_top);
}

qreal KoTextLayoutArea::left() const
{
    return m_left;
}

qreal KoTextLayoutArea::right() const
{
    return m_right;
}

qreal KoTextLayoutArea::top() const
{
    return m_top;
}

qreal KoTextLayoutArea::bottom() const
{
    return m_bottom;
}

void KoTextLayoutArea::setBottom(qreal bottom)
{
    m_boundingRect.setBottom(bottom + qMax(qreal(0.0), m_verticalAlignOffset));
    Q_ASSERT_X(m_boundingRect.top() <= m_boundingRect.bottom(), __FUNCTION__, "Bounding-rect is not normalized");
    m_bottom = bottom;
}

void KoTextLayoutArea::findFootNotes(QTextBlock block, const QTextLine &line)
{
    if (m_documentLayout->inlineTextObjectManager() == 0) {
        return;
    }
    QString text = block.text();
    int pos = text.indexOf(QChar::ObjectReplacementCharacter, line.textStart());

    while (pos >= 0 && pos <= line.textStart() + line.textLength()) {
        QTextCursor c1(block);
        c1.setPosition(block.position() + pos);
        c1.setPosition(c1.position() + 1, QTextCursor::KeepAnchor);

        KoInlineNote *note = dynamic_cast<KoInlineNote*>(m_documentLayout->inlineTextObjectManager()->inlineTextObject(c1));
        if (note && note->type() == KoInlineNote::Footnote) {
            preregisterFootNote(note);
        }

        pos = text.indexOf(QChar::ObjectReplacementCharacter, pos + 1);
    }
}

qreal KoTextLayoutArea::preregisterFootNote(KoInlineNote *note)
{
    if (m_parent == 0) {
        // TODO to support footnotes at end of document this is
        // where we need to add some extra condition
        if(note->autoNumbering())
            note->setAutoNumber(m_footNoteAutoCount++);

        QTextFrame *subFrame = note->textFrame();
        FrameIterator iter(subFrame);
        KoTextLayoutNoteArea *footNoteArea = new KoTextLayoutNoteArea(note, this, m_documentLayout);

        m_preregisteredFootNoteFrames.append(subFrame);
        footNoteArea->setReferenceRect(left(), right(), 0, maximumAllowedBottom() - bottom());
        footNoteArea->layout(&iter);

        m_preregisteredFootNotesHeight += footNoteArea->bottom();
        m_preregisteredFootNoteAreas.append(footNoteArea);
        return footNoteArea->bottom();
    }
    qreal h = m_parent->preregisterFootNote(note);
    m_preregisteredFootNotesHeight += h;
    return h;
}

void KoTextLayoutArea::confirmFootNotes()
{
    m_footNotesHeight += m_preregisteredFootNotesHeight;
    m_footNoteAreas.append(m_preregisteredFootNoteAreas);
    m_footNoteFrames.append(m_preregisteredFootNoteFrames);
    m_preregisteredFootNotesHeight = 0;
    m_preregisteredFootNoteAreas.clear();
    m_preregisteredFootNoteFrames.clear();
    if (m_parent) {
        m_parent->confirmFootNotes();
    }
}

void KoTextLayoutArea::expandBoundingLeft(qreal x)
{
    m_boundingRect.setLeft(qMin(x, m_boundingRect.x()));
}

void KoTextLayoutArea::expandBoundingRight(qreal x)
{
    m_boundingRect.setRight(qMax(x, m_boundingRect.right()));
}

void KoTextLayoutArea::clearPreregisteredFootNotes()
{
    m_preregisteredFootNotesHeight = 0;
    m_preregisteredFootNoteAreas.clear();
    m_preregisteredFootNoteFrames.clear();
    if (m_parent) {
        m_parent->clearPreregisteredFootNotes();
    }
}

void KoTextLayoutArea::handleBordersAndSpacing(KoTextBlockData *blockData, QTextBlock *block)
{
    QTextBlockFormat format = block->blockFormat();
    KoParagraphStyle formatStyle(format, block->charFormat());

    // The AddParaTableSpacingAtStart config-item is used to be able to optionally prevent that
    // defined fo:margin-top are applied to the first paragraph. If true then the fo:margin-top
    // is applied to all except the first paragraph. If false fo:margin-top is applied to all
    // paragraphs.
    bool paraTableSpacingAtStart = KoTextDocument(m_documentLayout->document()).paraTableSpacingAtStart();
    bool paddingExpandsBorders = false;//KoTextDocument(m_documentLayout->document()).paddingExpandsBorders();

    qreal topMargin = 0;
    if (paraTableSpacingAtStart || block->previous().isValid()) {
        topMargin = formatStyle.topMargin();
    }
    qreal spacing = qMax(m_bottomSpacing, topMargin);
    qreal divider = m_y;
    if (spacing) {
        divider += spacing * m_bottomSpacing / (m_bottomSpacing + topMargin);
    }
    qreal dx = 0.0;
    qreal x = m_x;
    qreal width = m_width;
    if (m_indent < 0) {
        x += m_indent;
        width -= m_indent;
    }
    if (blockData && blockData->hasCounterData() && blockData->counterPosition().x() < x) {
       width += x - blockData->counterPosition().x();
       x = blockData->counterPosition().x();
    }

    KoTextBlockBorderData border(QRectF(x, m_y, width, 1));
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

    if (border.hasBorders()) {
        if (blockData == 0) {
            blockData = new KoTextBlockData();
            block->setUserData(blockData);
        }

        // check if we can merge with the previous parags border.
        if (m_prevBorder && m_prevBorder->equals(border)) {
            blockData->setBorder(m_prevBorder);
            // Merged mean we don't have inserts inbetween the blocks
            if (!m_blockRects.isEmpty()) {
                m_blockRects.last().setBottom(divider);
            }
            m_y += spacing;
            m_blockRects.append(QRectF(x, divider, width, 1.0));
        } else {
            // can't merge; then these are our new borders.
            KoTextBlockBorderData *newBorder = new KoTextBlockBorderData(border);
            blockData->setBorder(newBorder);
            if (m_prevBorder) {
                m_y += m_prevBorderPadding;
                m_y += m_prevBorder->inset(KoTextBlockBorderData::Bottom);
            }
            if (!m_blockRects.isEmpty()) {
                m_blockRects.last().setBottom(m_y);
            }
            m_y += spacing;
            if (paddingExpandsBorders) {
                m_blockRects.append(QRectF(x - format.doubleProperty(KoParagraphStyle::LeftPadding), m_y,
                    width + format.doubleProperty(KoParagraphStyle::LeftPadding) + format.doubleProperty(KoParagraphStyle::RightPadding), 1.0));
            } else {
                m_blockRects.append(QRectF(x, m_y, width, 1.0));
            }
            m_y += newBorder->inset(KoTextBlockBorderData::Top);
            m_y += format.doubleProperty(KoParagraphStyle::TopPadding);
        }

        // finally, horizontal components of the borders
        dx = border.inset(KoTextBlockBorderData::Left);
        m_x += dx;
        m_width -= border.inset(KoTextBlockBorderData::Left);
        m_width -= border.inset(KoTextBlockBorderData::Right);
    } else { // this parag has no border.
        if (m_prevBorder) {
            m_y += m_prevBorderPadding;
            m_y += m_prevBorder->inset(KoTextBlockBorderData::Bottom);
        }
        if (blockData)
            blockData->setBorder(0); // remove an old one, if there was one.
        if (!m_blockRects.isEmpty()) {
            m_blockRects.last().setBottom(m_y);
        }
        m_y += spacing;
        m_blockRects.append(QRectF(x, m_y, width, 1.0));
    }
    if (!paddingExpandsBorders) {
        // add padding inside the border
        dx += format.doubleProperty(KoParagraphStyle::LeftPadding);
        m_x += format.doubleProperty(KoParagraphStyle::LeftPadding);
        m_width -= format.doubleProperty(KoParagraphStyle::LeftPadding);
        m_width -= format.doubleProperty(KoParagraphStyle::RightPadding);
    }
    if (block->layout()->lineCount() == 1 && blockData && blockData->hasCounterData()) {
        blockData->setCounterPosition(QPointF(blockData->counterPosition().x() + dx, m_y));
    }
    m_prevBorder = blockData->border();
    m_prevBorderPadding = format.doubleProperty(KoParagraphStyle::BottomPadding);
    m_anchoringParagraphTop = divider;
}
