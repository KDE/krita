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
 , m_dropCapsWidth(0)
 , m_startOfArea(0)
 , m_endOfArea(0)
 , m_acceptsPageBreak(false)
 , m_virginPage(true)
 , m_verticalAlignOffset(0)
 , m_preregisteredFootNotesHeight(0)
 , m_footNotesHeight(0)
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
            continue;
        } else {
            if (!block.isValid())
                continue;
        }
        if (block.blockFormat().hasProperty(KoParagraphStyle::TableOfContentsDocument)) {
            // check if p is over table of content
            if (point.y() > m_tableOfContentsAreas[tocIndex]->top()
                    && point.y() < m_tableOfContentsAreas[tocIndex]->bottom()) {
                pointedAt = m_tableOfContentsAreas[tocIndex]->hitTest(point, accuracy);
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
        if (next != stop && point.y() > layout->boundingRect().bottom()) {
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
            if (point.x() > lineRect.x() + lineRect.width()) {
                // right of line
                basicallyFound = true;
                pointedAt.position = block.position() + line.textStart() + line.textLength();
                pointedAt.fillInBookmark(QTextCursor(block), m_documentLayout->inlineTextObjectManager());
                continue;
            }
            pointedAt.position = block.position() + line.xToCursor(point.x());
            pointedAt.fillInBookmark(QTextCursor(block), m_documentLayout->inlineTextObjectManager());
            return pointedAt;
        }
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
            continue;
        } else {
            if (!block.isValid())
                continue;
        }
        if (block.blockFormat().hasProperty(KoParagraphStyle::TableOfContentsDocument)) {
            if (cursor.selectionStart()  <= block.position()
                && cursor.selectionEnd() >= block.position()) {
                retval |= m_tableOfContentsAreas[tocIndex]->boundingRect();
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

bool KoTextLayoutArea::layout(FrameIterator *cursor)
{
    qDeleteAll(m_tableAreas);
    m_tableAreas.clear();
    qDeleteAll(m_footNoteAreas);
    m_footNoteAreas.clear();
    qDeleteAll(m_preregisteredFootNoteAreas);
    m_preregisteredFootNoteAreas.clear();
    qDeleteAll(m_tableOfContentsAreas);
    m_tableOfContentsAreas.clear();
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
    m_footNotesHeight = 0;
    m_preregisteredFootNotesHeight = 0;
    m_prevBorder = 0;
    m_prevBorderPadding = 0;

    while (!cursor->it.atEnd()) {
        QTextBlock block = cursor->it.currentBlock();
        QTextTable *table = qobject_cast<QTextTable*>(cursor->it.currentFrame());
        QTextFrame *subFrame = cursor->it.currentFrame();
        if (table) {
            if (acceptsPageBreak() && !virginPage()
                   && (table->frameFormat().intProperty(KoTableStyle::BreakBefore) & KoText::PageBreak)) {
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
            if (subFrame->format().intProperty(KoText::SubFrameType) == KoText::EndNotesFrameType) {
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
            }
        } else if (block.isValid()) {
            if (block.blockFormat().hasProperty(KoParagraphStyle::TableOfContentsDocument)) {
                QVariant data = block.blockFormat().property(KoParagraphStyle::TableOfContentsDocument);
                QTextDocument *tocDocument = data.value<QTextDocument *>();

                data = block.blockFormat().property(KoParagraphStyle::TableOfContentsData);
                KoTableOfContentsGeneratorInfo *tocInfo = data.value<KoTableOfContentsGeneratorInfo *>();

                if (!tocInfo->generator()) {
                    // The generator attaches itself to the tocInfo
                    new ToCGenerator(tocDocument, tocInfo);
                }
                tocInfo->generator()->setMaxTabPosition(right() - left());

                if (!cursor->currentSubFrameIterator) {
                    // Let the generator know which QTextBlock it needs to ask for a relayout once the toc got generated.
                    tocInfo->generator()->setBlock(block);
                }

                // Let's create KoTextLayoutArea and let to handle the ToC
                KoTextLayoutArea *tocArea = new KoTextLayoutArea(this, documentLayout());
                m_tableOfContentsAreas.append(tocArea);
                m_y += m_bottomSpacing;
                if (!m_blockRects.isEmpty()) {
                    m_blockRects.last().setBottom(m_y);
                }
                tocArea->setVirginPage(virginPage());
                tocArea->setReferenceRect(left(), right(), m_y, maximumAllowedBottom());
                QTextLayout *blayout = block.layout();
                blayout->beginLayout();
                QTextLine line = blayout->createLine();
                line.setNumColumns(0);
                line.setPosition(QPointF(left(), m_y));
                blayout->endLayout();

                if (tocArea->layout(cursor->subFrameIterator(tocDocument->rootFrame())) == false) {
                    cursor->lineTextStart = 1; // fake we are not done
                    m_endOfArea = new FrameIterator(cursor);
                    m_y = tocArea->bottom();
                    setBottom(m_y + m_footNotesHeight);
                    // Expand bounding rect so if we have content outside we show it
                    expandBoundingLeft(tocArea->boundingRect().left());
                    expandBoundingRight(tocArea->boundingRect().right());
                    return false;
                }
                setVirginPage(false);
                // Expand bounding rect so if we have content outside we show it
                expandBoundingLeft(tocArea->boundingRect().left());
                expandBoundingRight(tocArea->boundingRect().right());
                m_bottomSpacing = 0;
                m_y = tocArea->bottom();
                delete cursor->currentSubFrameIterator;
                cursor->lineTextStart = -1; // fake we are done
                cursor->currentSubFrameIterator = 0;
            } else if (block.blockFormat().hasProperty(KoParagraphStyle::BibliographyDocument)) {

                QVariant data = block.blockFormat().property(KoParagraphStyle::BibliographyDocument);
                QTextDocument *bibDocument = data.value<QTextDocument *>();

                data = block.blockFormat().property(KoParagraphStyle::BibliographyData);
                KoBibliographyInfo *bibInfo = data.value<KoBibliographyInfo *>();

                if (!bibInfo->generator()) {
                    // The generator attaches itself to the bibInfo
                    new BibliographyGenerator(bibDocument, block, bibInfo, cursor->it.currentBlock().document());
                }

                // Let's create KoTextLayoutArea and let to handle the Bibliography
                KoTextLayoutArea *bibArea = new KoTextLayoutArea(this, documentLayout());
                m_bibliographyAreas.append(bibArea);
                m_y += m_bottomSpacing;
                if (!m_blockRects.isEmpty()) {
                    m_blockRects.last().setBottom(m_y);
                }
                bibArea->setVirginPage(virginPage());
                bibArea->setReferenceRect(left(), right(), m_y, maximumAllowedBottom());
                QTextLayout *blayout = block.layout();
                blayout->beginLayout();
                QTextLine line = blayout->createLine();
                line.setNumColumns(0);
                line.setPosition(QPointF(left(), m_y));
                blayout->endLayout();

                if (bibArea->layout(cursor->subFrameIterator(bibDocument->rootFrame())) == false) {
                    cursor->lineTextStart = 1; // fake we are not done
                    m_endOfArea = new FrameIterator(cursor);
                    m_y = bibArea->bottom();
                    setBottom(m_y + m_footNotesHeight);
                    // Expand bounding rect so if we have content outside we show it
                    expandBoundingLeft(bibArea->boundingRect().left());
                    expandBoundingRight(bibArea->boundingRect().right());
                    return false;
                }
                setVirginPage(false);
                // Expand bounding rect so if we have content outside we show it
                expandBoundingLeft(bibArea->boundingRect().left());
                expandBoundingRight(bibArea->boundingRect().right());
                m_bottomSpacing = 0;
                m_y = bibArea->bottom();
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

                if (!virginPage() &&
                    (masterPageNameChanged ||
                        (acceptsPageBreak() &&
                        (block.blockFormat().intProperty(KoParagraphStyle::BreakBefore) & KoText::PageBreak)))) {
                    m_endOfArea = new FrameIterator(cursor);
                    setBottom(m_y + m_footNotesHeight);
                    if (!m_blockRects.isEmpty()) {
                        m_blockRects.last().setBottom(m_y);
                    }
                    return false;
                }

                if (layoutBlock(cursor) == false) {
                    m_endOfArea = new FrameIterator(cursor);
                    setBottom(m_y + m_footNotesHeight);
                    m_blockRects.last().setBottom(m_y);
                    return false;
                }

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
            atEnd = cursor->it.atEnd();
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

QTextLine restartLayout(QTextLayout *layout, int lineTextStartOfLastKeep)
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
    layout->beginLayout();
    foreach(const LineKeeper &lk, lineKeeps) {
        line = layout->createLine();
        if (!line.isValid())
            break;
        line.setNumColumns(lk.columns, lk.lineWidth);
        line.setPosition(lk.position);
    }
    return line;
}

// layoutBlock() method is structured like this:
//
// 1) Setup various helper values
//   a) related to or influenced by lists
//   b) related to or influenced by dropcaps
//   c) related to or influenced by tabs
// 2)layout each line (possibly restarting where we stopped earlier)
//   a) fit line into sub lines with as needed for text runaround
//   b) make sure we keep above maximumAllowedBottom
//   c) calls addLine()
//   d) update dropcaps related variables
bool KoTextLayoutArea::layoutBlock(FrameIterator *cursor)
{
    QTextBlock block(cursor->it.currentBlock());
    KoTextBlockData *blockData = dynamic_cast<KoTextBlockData *>(block.userData());
    KoParagraphStyle format(block.blockFormat(), block.charFormat());

    int dropCapsAffectsNMoreLines = 0;
    qreal dropCapsPositionAdjust = 0.0;

    KoText::Direction dir = format.textProgressionDirection();
    if (dir == KoText::InheritDirection)
        dir = parentTextDirection();
    if (dir == KoText::AutoDirection)
        m_isRtl = block.text().isRightToLeft();
    else
        m_isRtl =  dir == KoText::RightLeftTopBottom || dir == KoText::PerhapsRightLeftTopBottom;

    // initialize list item stuff for this parag.
    QTextList *textList = block.textList();
    if (textList) {
        QTextListFormat listFormat = textList->format();

        KoCharacterStyle *cs = 0;
        if (m_documentLayout->styleManager()) {
            const int id = listFormat.intProperty(KoListStyle::CharacterStyleId);
            cs = m_documentLayout->styleManager()->characterStyle(id);
            if (!cs) {
                KoParagraphStyle *ps = m_documentLayout->styleManager()->paragraphStyle(
                                       block.blockFormat().intProperty(KoParagraphStyle::StyleId));
                if (ps && !ps->hasDefaults()) {
                    cs = ps->characterStyle();
                }
            }
        }

        // use format from the actual block of the list item
        QTextCharFormat labelFormat;
        if ( cs && cs->hasProperty(QTextFormat::FontPointSize) ) {
                cs->applyStyle(labelFormat);
        } else {
            if (block.text().size() == 0) {
                labelFormat = block.charFormat();
            } else {
                labelFormat = block.begin().fragment().charFormat();
            }
        }

        // fetch the text properties of the list-level-style-bullet
        if (listFormat.hasProperty(KoListStyle::MarkCharacterStyleId)) {
            QVariant v = listFormat.property(KoListStyle::MarkCharacterStyleId);
            QSharedPointer<KoCharacterStyle> textPropertiesCharStyle = v.value< QSharedPointer<KoCharacterStyle> >();
            if (!textPropertiesCharStyle.isNull()) {
                //calculate the correct font point size taking into account the current
                // block format and the relative font size percent if the size is not absolute
                if (!textPropertiesCharStyle->hasProperty(QTextFormat::FontPointSize)) {
                    qreal percent = 100.0;
                    if (listFormat.hasProperty(KoListStyle::RelativeBulletSize)) {
                        percent = listFormat.property(KoListStyle::RelativeBulletSize).toDouble();
                    } else {
                        listFormat.setProperty(KoListStyle::RelativeBulletSize, percent);
                    }
                    textPropertiesCharStyle->setFontPointSize((percent*labelFormat.fontPointSize())/100.00);
                }
                textPropertiesCharStyle->applyStyle(labelFormat);
            }
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

    option.setAlignment(QStyle::visualAlignment(m_isRtl ? Qt::RightToLeft : Qt::LeftToRight, format.alignment()));
    if (m_isRtl) {
        option.setTextDirection(Qt::RightToLeft);
        // For right-to-left we need to make sure that trailing spaces are included into the QTextLine naturalTextWidth
        // and naturalTextRect calculation so they are proper handled in the RunAroundHelper. For left-to-right we do
        // not like to include trailing spaces in the calculations cause else justified text would not look proper
        // justified. Seems for right-to-left we have to accept that justified text will not look proper justified then.
        option.setFlags(QTextOption::IncludeTrailingSpaces);
    } else {
        option.setTextDirection(Qt::LeftToRight);
    }

    option.setUseDesignMetrics(true);
    // Drop caps
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
    bool dropCaps = format.dropCaps();
    int dropCapsLength = format.dropCapsLength();
    int dropCapsLines = format.dropCapsLines();

    if (dropCaps && dropCapsLength != 0 && dropCapsLines > 1
            && dropCapsAffectsNMoreLines == 0 // first line of this para is not affected by a previous drop-cap
            && block.length() > 1) {
        QString blockText = block.text();
        // ok, now we can drop caps for this block
        int firstNonSpace = blockText.indexOf(QRegExp("[^ ]")); // since QTextLine::setNumColumns()
        // skips blankspaces, we will too
        if (dropCapsLength < 0) // means whole word is to be dropped
            dropCapsLength = blockText.indexOf(QRegExp("\\W"), firstNonSpace);
        // increase the size of the dropped chars
        QTextCursor blockStart(block);
        QTextLayout::FormatRange dropCapsFormatRange;
        dropCapsFormatRange.format = blockStart.charFormat();

        // find out lineHeight for this block.
        QTextBlock::iterator it = block.begin();
        QTextFragment lineRepresentative = it.fragment();
        qreal lineHeight = format.lineHeightAbsolute();
        qreal dropCapsHeight = 0;
        if (lineHeight == 0) {
            lineHeight = lineRepresentative.charFormat().fontPointSize();
            qreal linespacing = format.lineSpacing();
            if (linespacing == 0) { // unset
                int percent = format.lineHeightPercent();
                if (percent != 0)
                    linespacing = lineHeight * ((percent - 100) / 100.0);
                else if (linespacing == 0)
                    linespacing = lineHeight * 0.2; // default
            }
            dropCapsHeight = linespacing * (dropCapsLines-1);
        }
        const qreal minimum = format.minimumLineHeight();
        if (minimum > 0.0) {
            lineHeight = qMax(lineHeight, minimum);
        }

        dropCapsHeight += lineHeight * dropCapsLines;

        int dropCapsStyleId = format.dropCapsTextStyleId();
        KoCharacterStyle *dropCapsCharStyle = 0;
        if (dropCapsStyleId > 0 && m_documentLayout->styleManager()) {
            dropCapsCharStyle = m_documentLayout->styleManager()->characterStyle(dropCapsStyleId);
            dropCapsCharStyle->applyStyle(dropCapsFormatRange.format);
        }
        QFont f(dropCapsFormatRange.format.font(), m_documentLayout->paintDevice());
        QString dropCapsText(block.text().left(dropCapsLength));
        f.setPointSizeF(dropCapsHeight);
        QChar lastChar = dropCapsText.at(dropCapsText.length()-1);
        for (int i=0; i < 5; ++i) {
            QFontMetricsF fm(f);
            QRectF rect = fm.tightBoundingRect(dropCapsText);
            m_dropCapsWidth = rect.width();
            m_dropCapsWidth += fm.rightBearing(lastChar);
            const qreal diff = dropCapsHeight - rect.height();
            dropCapsPositionAdjust = rect.top() + fm.ascent();
            if (qAbs(diff < 0.5)) // good enough
                break;

            const qreal adjustment = diff * ((f.pointSizeF() / diff) / (rect.height() / diff));
            // kDebug() << "adjusting with" << adjustment;
            f.setPointSizeF(f.pointSizeF() + adjustment);
        }
        dropCapsFormatRange.format.setFontPointSize(f.pointSizeF());
        dropCapsFormatRange.format.setProperty(DropCapsAdditionalFormattingId,
                (QVariant) true);
        dropCapsFormatRange.start = 0;
        dropCapsFormatRange.length = dropCapsLength + firstNonSpace;
        formatRanges.append(dropCapsFormatRange);
        layout->setAdditionalFormats(formatRanges);

        m_dropCapsNChars = dropCapsLength + firstNonSpace;
        dropCapsAffectsNMoreLines = (m_dropCapsNChars > 0) ? dropCapsLines : 0;
    } else {
        m_dropCapsNChars = 0;
    }

    qreal leftMargin = format.leftMargin();
    qreal rightMargin = format.rightMargin();

    m_listIndent = 0;
    qreal listLabelIndent = 0;
    if (textList) {
        if (!m_isRtl) {
            listLabelIndent = blockData->counterSpacing() + blockData->counterWidth();
        }
        if (textList->format().boolProperty(KoListStyle::AlignmentMode) == false) {
            m_listIndent = textList->format().doubleProperty(KoListStyle::Indent) + listLabelIndent;
        } else {
            if (!format.hasProperty(KoParagraphStyle::ListLevel)) {
                leftMargin = textList->format().doubleProperty(KoListStyle::Margin);
                m_listIndent = textList->format().doubleProperty(KoListStyle::Indent);
            }
        }
    }

    m_width = right() - left();
    m_width -= leftMargin + rightMargin;
    m_x = left() + (m_isRtl ? rightMargin : leftMargin);

    m_documentLayout->clearInlineObjectRegistry(block);
    m_indent = 0;
    QTextLine line;
    bool anyLineAdded = false;
    if (cursor->lineTextStart == -1) {
        layout->beginLayout();
        m_indent = textIndent(block, textList);

        line = layout->createLine();
        cursor->fragmentIterator = block.begin();
    } else {
        line = restartLayout(layout, cursor->lineTextStart);
    }

    // Tabs
    qreal tabStopDistance = format.tabStopDistance();

    if (tabStopDistance <= 0) {
        tabStopDistance = m_documentLayout->defaultTabSpacing();
    }

    QList<KoText::Tab> tabs = format.tabPositions();
    qreal tabOffset = - left();

    if (m_documentLayout->relativeTabs()) {
        tabOffset -= (m_isRtl ? 0.0 : (m_indent));
    } else {
        tabOffset -= (m_isRtl ? rightMargin : (leftMargin + m_indent));
    }

    // Set up a var to keep track of where last added tab is. Conversion of tabOffset is required because Qt thinks in device units and we don't
    qreal position = tabOffset;

    if (!tabs.isEmpty()) {
        position = tabs.last().position;
    }

    // Since we might have tabs relative to first indent we need to always specify a lot of
    // regular interval tabs (relative to the indent naturally)
    // So first figure out where the first regular interval tab should be.
    position = tabOffset;
    position = (int(position / tabStopDistance) + 1) * tabStopDistance + tabOffset;
    for(int i=0 ; i<16; ++i) { // let's just add 16 but we really should limit to pagewidth
        KoText::Tab tab;

        // conversion here is required because Qt thinks in device units and we don't
        tab.position = position;
        tabs.append(tab);
        position += tabStopDistance;
    }

    QList<QTextOption::Tab> qTabs;
    ///@TODO: don't do this kind of conversion, we lose data for layout.
    foreach (KoText::Tab kTab, tabs) {
#if QT_VERSION >= 0x040700
        qTabs.append(QTextOption::Tab((kTab.position + tabOffset) * qt_defaultDpiY() / 72. -1, kTab.type, kTab.delimiter));
#else
        QTextOption::Tab tab;
        tab.position = (kTab.position + tabOffset) * qt_defaultDpiY() / 72. -1;
        tab.type = kTab.type;
        tab.delimiter = kTab.delimiter;
        qTabs.append(tab);
#endif
    }
    option.setTabs(qTabs);
    option.setTabStop(tabStopDistance * qt_defaultDpiY() / 72.);

    //Now once we know the physical context we can work on the borders of the paragraph
    handleBordersAndSpacing(blockData, &block);
    m_blockRects.last().setLeft(m_blockRects.last().left() + qMin(m_indent, qreal(0.0)));

    if (textList && block.layout()->lineCount() == 1) {
        // If first line in a list then set the counterposition. Following lines in the same
        // list-item have nothing to do with the counter. Do this after borders so we can
        // account for them.
        if (m_isRtl) {
            m_width -= blockData->counterWidth() + blockData->counterSpacing() + m_listIndent;
            if (textList->format().boolProperty(KoListStyle::AlignmentMode) == false) {
                blockData->setCounterPosition(QPointF(right() -
                                                      blockData->counterWidth() - leftMargin, m_y));
            } else {
                blockData->setCounterPosition(QPointF(right() - leftMargin, m_y));
            }
        } else {
            if (textList->format().boolProperty(KoListStyle::AlignmentMode) == false) {
                m_x += m_listIndent;
                m_width -= m_listIndent;
                blockData->setCounterPosition(QPointF(x() - listLabelIndent, m_y));
            } else {
                blockData->setCounterPosition(QPointF(x(), m_y));
            }
        }

        if (textList->format().boolProperty(KoListStyle::AlignmentMode)) {
            if (block.blockFormat().intProperty(KoListStyle::LabelFollowedBy) == KoListStyle::ListTab) {
                if (textList->format().hasProperty(KoListStyle::TabStopPosition)) {
                    qreal listTab = textList->format().doubleProperty(KoListStyle::TabStopPosition);
                    if (!m_documentLayout->relativeTabs()) {
                        listTab += leftMargin + m_indent;
                    } else {
                        listTab -= leftMargin + m_indent; // express it relatively like other tabs
                    }

                    foreach(KoText::Tab tab, tabs) {
                        qreal position = tab.position  * 72. / qt_defaultDpiY();
                        if (position > listLabelIndent) {
                            // found the relevant normal tab
                            if (position > listTab && listTab > listLabelIndent) {
                                // But special tab is more relevant
                                position = listTab;
                            }
                            m_indent += position;
                            break;
                        }
                    }
                } else {
                    m_indent = 0;
                }
            }
        }
    }

    layout->setTextOption(option);
    // So now is the time to create the lines of this paragraph
    RunAroundHelper runAroundHelper;
    runAroundHelper.setObstructions(documentLayout()->currentObstructions());
    qreal maxLineHeight = 0;
    qreal y_justBelowDropCaps = 0;

    while (line.isValid()) {
        runAroundHelper.setLine(this, line);
        runAroundHelper.setObstructions(documentLayout()->currentObstructions());
        documentLayout()->setAnchoringParagraphRect(m_blockRects.last());
        runAroundHelper.fit( /* resetHorizontalPosition */ false, /* rightToLeft */ m_isRtl, QPointF(x(), m_y));
        qreal bottomOfText = line.y() + line.height();

        bool softBreak = false;
        if (acceptsPageBreak() && !format.nonBreakableLines() && bottomOfText > maximumAllowedBottom() - 150) {
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
                    return false;
                }
            }
        }
        findFootNotes(block, line);
        if (bottomOfText > maximumAllowedBottom()) {
            // We can not fit line within our allowed space
            // in case we resume layout on next page the line is reused later
            // but if not then we need to make sure the line becomes invisible
            // we use m_maximalAllowedBottom because we want to be below
            // footnotes too.
            if (!virginPage() && format.nonBreakableLines()) {
                line.setPosition(QPointF(x(), m_maximalAllowedBottom));
                cursor->lineTextStart = -1;
                layout->endLayout();
                clearPreregisteredFootNotes();
                return false; //to indicate block was not done!
            }
            if (!virginPage() || anyLineAdded) {
                line.setPosition(QPointF(x(), m_maximalAllowedBottom));
                clearPreregisteredFootNotes();
                return false; //to indicate block was not done!
            }
        }
        confirmFootNotes();
        anyLineAdded = true;
        maxLineHeight = qMax(maxLineHeight, addLine(line, cursor, blockData));

        if (!runAroundHelper.stayOnBaseline()) {
            m_y += maxLineHeight;
            maxLineHeight = 0;
            m_indent = 0;
        }
        // drop caps
        if (m_dropCapsNChars > 0) { // we just laid out the dropped chars
            y_justBelowDropCaps = m_y; // save the y position just below the dropped characters
            m_y = line.y();              // keep the same y for the next line
            line.setPosition(line.position() - QPointF(0, dropCapsPositionAdjust));
            m_dropCapsNChars = 0;
        } else if (dropCapsAffectsNMoreLines > 0) { // we just laid out a drop-cap-affected line
            dropCapsAffectsNMoreLines--;
            if (dropCapsAffectsNMoreLines == 0) {   // no more drop-cap-affected lines
                if (m_y < y_justBelowDropCaps)
                    m_y = y_justBelowDropCaps; // make sure m_y is below the dropped characters
                y_justBelowDropCaps = 0;
                m_dropCapsWidth = 0;
            }
        }
        // Expand bounding rect so if we have content outside we show it
        expandBoundingLeft(line.x());
        expandBoundingRight(line.x() + line.naturalTextWidth());

        // during fit is where documentLayout->positionInlineObjects is called
        //so now is a good time to position the obstructions
        int oldObstructionCount = documentLayout()->currentObstructions().size();

        documentLayout()->positionAnchoredObstructions();

        if (oldObstructionCount < documentLayout()->currentObstructions().size()) {
            return false;
        }

        // line fitted so try and do the next one
        line = layout->createLine();
        if (!line.isValid()) {
            break; // no more line means our job is done
        }
        cursor->lineTextStart = line.textStart();

        if (softBreak) {
            return false; // page-break means we need to start again on the next page
        }
    }

    m_bottomSpacing = format.bottomMargin();

    layout->endLayout();
    setVirginPage(false);
    cursor->lineTextStart = -1; //set lineTextStart to -1 and returning true indicate new block
    return true;
}

qreal KoTextLayoutArea::listIndent() const
{
    return m_listIndent;
}

qreal KoTextLayoutArea::textIndent(QTextBlock block, QTextList *textList) const
{
    KoParagraphStyle format (block.blockFormat(), block.charFormat());
    if (format.autoTextIndent()) {
        // if auto-text-indent is set,
        // return an indent approximately 3-characters wide as per current font
        QTextCursor blockCursor(block);
        qreal guessGlyphWidth = QFontMetricsF(blockCursor.charFormat().font()).width('x');
        return guessGlyphWidth * 3;
    }
    if (textList && textList->format().boolProperty(KoListStyle::AlignmentMode)) {
        if (! block.blockFormat().hasProperty(KoParagraphStyle::ListLevel)) {
            return textList->format().doubleProperty(KoListStyle::TextIndent);
        }
    }
    return format.textIndent().value(width());
}

qreal KoTextLayoutArea::x() const
{
    return m_x + m_indent + (m_dropCapsNChars == 0 ? m_dropCapsWidth : 0.0);
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
        // first line, lets check where the line ended up and adjust the positioning of the counter.
        if ((format.alignment() & Qt::AlignHCenter) == Qt::AlignHCenter) {
            const qreal padding = (line.width() - line.naturalTextWidth() ) / 2;
            qreal newX;
            if (m_isRtl) {
                newX = line.x() + line.width() - padding + blockData->counterSpacing();
            } else {
                newX = line.x() + padding - blockData->counterWidth() - blockData->counterSpacing();
            }
            blockData->setCounterPosition(QPointF(newX, blockData->counterPosition().y()));
        } else if (!m_isRtl && x() < line.x()) {// move the counter more left.
            blockData->setCounterPosition(blockData->counterPosition() + QPointF(line.x() - x(), 0));
        } else if (m_isRtl && x() + width() > line.x() + line.width() + 0.1) { // 0.1 to account for qfixed rounding
            const qreal newX = line.x() + line.width() + blockData->counterSpacing();
            blockData->setCounterPosition(QPointF(newX, blockData->counterPosition().y()));
        }
    }

    qreal height = 0;
    qreal ascent = 0.0;
    qreal descent = 0.0;
    const bool useFontProperties = format.boolProperty(KoParagraphStyle::LineSpacingFromFont);

    if (cursor->fragmentIterator.atEnd()) {// no text in parag.
        qreal fontStretch = 1;
        if (useFontProperties) {
            //stretch line height to powerpoint size
            fontStretch = PresenterFontStretch;
        } else if ( block.charFormat().hasProperty(KoCharacterStyle::FontStretch)) {
            // stretch line height to ms-word size
            fontStretch = block.charFormat().property(KoCharacterStyle::FontStretch).toDouble();
        }
        height = block.charFormat().fontPointSize() * fontStretch;
    } else {
        qreal fontStretch = 1;
        if (useFontProperties) {
            //stretch line height to powerpoint size
            fontStretch = PresenterFontStretch;
        } else if ( cursor->fragmentIterator.fragment().charFormat().hasProperty(KoCharacterStyle::FontStretch)) {
            // stretch line height to ms-word size
            fontStretch = cursor->fragmentIterator.fragment().charFormat().property(KoCharacterStyle::FontStretch).toDouble();
        }
        // read max font height
        height = qMax(height, cursor->fragmentIterator.fragment().charFormat().fontPointSize() * fontStretch);

        KoInlineObjectExtent pos = m_documentLayout->inlineObjectExtent(cursor->fragmentIterator.fragment());
        ascent = qMax(ascent, pos.m_ascent);
        descent = qMax(descent, pos.m_descent);

        while (!(cursor->fragmentIterator.atEnd() || cursor->fragmentIterator.fragment().contains(
                        block.position() + line.textStart() + line.textLength() - 1))) {
            cursor->fragmentIterator++;
            if (cursor->fragmentIterator.atEnd()) {
                break;
            }
            if (!m_documentLayout->changeTracker()
                || !m_documentLayout->changeTracker()->displayChanges()
                || !m_documentLayout->changeTracker()->containsInlineChanges(cursor->fragmentIterator.fragment().charFormat())
                || !m_documentLayout->changeTracker()->elementById(cursor->fragmentIterator.fragment().charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt())
                || !m_documentLayout->changeTracker()->elementById(cursor->fragmentIterator.fragment().charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt())->isEnabled()
                || (m_documentLayout->changeTracker()->elementById(cursor->fragmentIterator.fragment().charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt())->getChangeType() != KoGenChange::DeleteChange)
                || m_documentLayout->changeTracker()->displayChanges()) {
                qreal fontStretch = 1;
                if (useFontProperties) {
                    //stretch line height to powerpoint size
                    fontStretch = PresenterFontStretch;
                } else if ( cursor->fragmentIterator.fragment().charFormat().hasProperty(KoCharacterStyle::FontStretch)) {
                    // stretch line height to ms-word size
                    fontStretch = cursor->fragmentIterator.fragment().charFormat().property(KoCharacterStyle::FontStretch).toDouble();
                }
                // read max font height
                height = qMax(height, cursor->fragmentIterator.fragment().charFormat().fontPointSize() * fontStretch);

                KoInlineObjectExtent pos = m_documentLayout->inlineObjectExtent(cursor->fragmentIterator.fragment());
                ascent = qMax(ascent, pos.m_ascent);
                descent = qMax(descent, pos.m_descent);
            }
        }
    }

    height = qMax(height, ascent + descent);

    if (height < 0.01) {
        height = 12; // default size for uninitialized styles.
    }

    // Adjust the line-height according to a probably defined fixed line height,
    // a proportional (percent) line-height and/or the line-spacing. Together
    // with the line-height we maybe also need to adjust the position of the
    // line. This is for example needed if the line needs to shrink in height
    // so the line-text stays on the baseline. If the line grows in height then
    // we don't need to do anything.
    qreal lineAdjust = 0.0;
    qreal fixedLineHeight = format.doubleProperty(KoParagraphStyle::FixedLineHeight);
    if (fixedLineHeight != 0.0) {
        qreal prevHeight = height;
        height = fixedLineHeight;
        if (prevHeight > height) {
            lineAdjust = fixedLineHeight - height;
        }
    } else {
        qreal lineSpacing = format.doubleProperty(KoParagraphStyle::LineSpacing);
        if (lineSpacing == 0.0) { // unset
            int percent = format.intProperty(KoParagraphStyle::PercentLineHeight);
            if (percent != 0) {
                qreal prevHeight = height;
                height *= percent / 100.0;
                if (prevHeight > height) {
                    lineAdjust = height - prevHeight;
                }
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
        m_blockRects.last().moveTop(m_blockRects.last().top() + lineAdjust);

        if (blockData && block.textList() && block.layout()->lineCount() == 1) {
            // If this is the first line in a list (aka the first line of the first list-
            // item) then we also need to adjust the counter to match to the line again.
            blockData->setCounterPosition(QPointF(blockData->counterPosition().x(), blockData->counterPosition().y() + lineAdjust));
        }
    }

    return height;
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

        FrameIterator iter(note->textFrame());
        KoTextLayoutArea *footNoteArea = new KoTextLayoutArea(this, m_documentLayout);

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
    m_preregisteredFootNotesHeight = 0;
    m_preregisteredFootNoteAreas.clear();
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

    qreal topMargin = 0;
    if (paraTableSpacingAtStart || block->previous().isValid()) {
        topMargin = formatStyle.topMargin();
    }
    qreal spacing = qMax(m_bottomSpacing, topMargin);

    KoTextBlockBorderData border(QRectF(x(), m_y, width(), 1));
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
            qreal divider = m_y;
            if (spacing) {
                divider += spacing * m_bottomSpacing / (m_bottomSpacing + topMargin);
            }
            if (!m_blockRects.isEmpty()) {
                m_blockRects.last().setBottom(divider);
            }
            m_y += spacing;
            m_blockRects.append(QRectF(m_x, divider, m_width, 1.0));
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
            m_blockRects.append(QRectF(m_x, m_y, m_width, 1.0));
            m_y += newBorder->inset(KoTextBlockBorderData::Top);
            m_y += format.doubleProperty(KoParagraphStyle::TopPadding);
        }

        // finally, horizontal components of the borders
        m_x += border.inset(KoTextBlockBorderData::Left);
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
        m_blockRects.append(QRectF(m_x, m_y, m_width, 1.0));
    }
    // add padding inside the border
    m_x += format.doubleProperty(KoParagraphStyle::LeftPadding);
    m_width -= format.doubleProperty(KoParagraphStyle::LeftPadding);
    m_width -= format.doubleProperty(KoParagraphStyle::RightPadding);

    m_prevBorder = blockData->border();
    m_prevBorderPadding = format.doubleProperty(KoParagraphStyle::BottomPadding);
}
