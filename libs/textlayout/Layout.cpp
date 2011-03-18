/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2008 Roopesh Chander <roop@forwardbias.in>
 * Copyright (C) 2007-2008 Pierre Ducroquet <pinaraf@pinaraf.info>
 * Copyright (C) 2009-2010 KO GmbH <cbo@kogmbh.com>
 * Copyright (C) 2009-2010 Casper Boemann <cbo@boemann.dk>
 * Copyright (C) 2010 Nandita Suri <suri.nandita@gmail.com>
 * Copyright (C) 2010 Ajay Pundhir <ajay.pratap@iiitb.net>
 * Copyright (C) 2011 Lukáš Tvrdý <lukas.tvrdy@ixonos.com>
 * Copyright (C) 2011 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
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

#include "Layout.h"
#include "TableLayout.h"
#include "ListItemsHelper.h"
#include "TextShape.h"
#include "ToCGenerator.h"
#include "FloatingAnchorStrategy.h"
#include "InlineAnchorStrategy.h"

#include <KoTextDocumentLayout.h>
#include <KoTextShapeData.h>
#include <KoParagraphStyle.h>
#include <KoCharacterStyle.h>
#include <KoListStyle.h>
#include <KoTableStyle.h>
#include <KoTableRowStyle.h>
#include <KoTableColumnAndRowStyleManager.h>
#include <KoStyleManager.h>
#include <KoTextBlockData.h>
#include <KoTextBlockBorderData.h>
#include <KoInlineNote.h>
#include <KoInlineTextObjectManager.h>
#include <KoShape.h>
#include <KoUnit.h>
#include <KoTextDocument.h>
#include <KoChangeTracker.h>
#include <KoChangeTrackerElement.h>
#include <KoGenChange.h>
#include <KoTextBlockPaintStrategyBase.h>
#include <KoImageData.h>
#include <KoImageCollection.h>
#include <KoShapeContainerModel.h>

#include <KDebug>
#include <QTextList>
#include <QStyle>
#include <QFontMetrics>
#include <QTextTableCell>
#include <QTextFragment>

extern int qt_defaultDpiY();

#define DropCapsAdditionalFormattingId 25602902


// ---------------- layout helper ----------------
Layout::Layout(KoTextDocumentLayout *parent)
        :
        m_blockData(0),
        m_data(0),
        m_reset(true),
        m_isRtl(false),
        m_inTable(false),
        m_parent(parent),
        m_textShape(0),
        m_demoText(false),
        m_endOfDemoText(false),
        m_currentTabStop(0),
        m_dropCapsNChars(0),
        m_dropCapsAffectsNMoreLines(0),
        m_dropCapsAffectedLineWidthAdjust(0),
        m_y_justBelowDropCaps(0),
        m_dropCapsPositionAdjust(0),
        m_restartingAfterTableBreak(false),
        m_restartingFirstCellAfterTableBreak(false),
        m_allTimeMinimumLeft(0),
        m_allTimeMaximumRight(0),
        m_maxLineHeight(0),
        m_scaleFactor(1.0)
{
    m_frameStack.reserve(5); // avoid reallocs
}

Layout::~Layout()
{
}

bool Layout::start()
{
    m_styleManager = KoTextDocument(m_parent->document()).styleManager();
    m_changeTracker = KoTextDocument(m_parent->document()).changeTracker();
    m_relativeTabs = KoTextDocument(m_parent->document()).relativeTabs();
    if (m_reset) {
        resetPrivate();
    } else if (shape) {
        nextParag();
    }
    m_reset = false;

    bool ok = !(layout == 0 || m_parent->shapes().count() <= shapeNumber);
#if 0
    if (!ok) {
        kDebug() << "Starting layouting failed, shapeCount=" << m_parent->shapes().count() << "shapeNumber=" << shapeNumber << "layout=" << (layout ? layout->boundingRect() : QRectF()) << (layout ? layout->position() : QPointF());
    }
#endif
    return ok;
}

void Layout::end()
{
    if (layout)
        layout->endLayout();
    layout = 0;
}

void Layout::reset()
{
    m_reset = true;
}

bool Layout::isInterrupted() const
{
    return m_reset;
}

qreal Layout::y()
{
    return m_y;
}


QRectF Layout::expandVisibleRect(const QRectF &rect) const
{
    qreal rightAdjust = qMax<qreal>(m_allTimeMaximumRight - rect.right(), 0.0);
    return rect.adjusted(m_allTimeMinimumLeft, 0.0, rightAdjust, 0.0);
}


qreal Layout::documentOffsetInShape()
{
    return m_data->documentOffset();
}

qreal Layout::topMargin()
{
    bool allowMargin = true; // wheather to allow margins at top of shape
    if (m_newShape) {
        allowMargin = false; // false by default, but check 2 exceptions.
        if (m_format.pageBreakPolicy () & QTextFormat::PageBreak_AlwaysBefore)
            allowMargin = true;
        else if (m_styleManager && m_format.topMargin() > 0) {
            // also allow it when the paragraph has the margin, but the style has a different one.
            KoParagraphStyle *ps = m_styleManager->paragraphStyle(
                                       m_format.intProperty(KoParagraphStyle::StyleId));
            if (ps == 0 || ps->topMargin() != m_format.topMargin())
                allowMargin = true;
        }
    }
    if (allowMargin)
        return m_format.topMargin();
    return 0.0;
}

QRectF Layout::selectionBoundingBox(QTextCursor &cursor)
{
    return selectionBoundingBoxFrame(m_parent->document()->rootFrame(), cursor);
}

QRectF Layout::selectionBoundingBoxFrame(QTextFrame *frame, QTextCursor &cursor)
{
    QRectF retval(-5E6,0,105E6,1);
    if(cursor.position() == -1)
        return retval;

    QTextFrame::iterator it;
    for (it = frame->begin(); !(it.atEnd()); ++it) {
        QTextBlock block = it.currentBlock();
        QTextTable *table = qobject_cast<QTextTable*>(it.currentFrame());

        if (table) {
            m_tableLayout.setTable(table);
            if(!m_tableLayout.isDirty() && cursor.selectionStart() >= table->firstPosition()
                        && cursor.selectionEnd() <= table->lastPosition()) {
                // TODO return tablerects of selected cells
                retval.setTop(m_tableLayout.cellBoundingRect(table->cellAt(table->firstPosition())).y());
                retval.setBottom(m_tableLayout.cellBoundingRect(table->cellAt(table->lastPosition())).bottom());
                return retval;
            }
        } /*else if (it.currentFrame()) { // subframe?
            // right now we don't care about sections
            textRectFrame(QTextFrame *frame, QTextCursor &cursor);
            continue;
        } */else {
            if (!block.isValid())
                continue;
        }
        if(cursor.selectionStart() >= block.position()
            && cursor.selectionStart() < block.position() + block.length()) {
                // TODO set top of rect
            QTextLine line = block.layout()->lineForTextPosition(cursor.selectionStart() - block.position());
            if (line.isValid())
                retval.setTop(line.y());
        }
        if(cursor.selectionEnd() >= block.position()
            && cursor.selectionEnd() < block.position() + block.length()) {
                // TODO set bottom of rect
            QTextLine line = block.layout()->lineForTextPosition(cursor.selectionEnd() - block.position());
            if (line.isValid())
                retval.setBottom
                (line.y() + line.height());
        }
    }
    return retval;
}

void Layout::draw(QPainter *painter, const KoTextDocumentLayout::PaintContext &context)
{
    drawFrame(m_parent->document()->rootFrame(), painter, context, 0);
}

void Layout::drawFrame(QTextFrame *frame, QPainter *painter, const KoTextDocumentLayout::PaintContext &context, int inTable)
{
    painter->setPen(context.textContext.palette.color(QPalette::Text)); // for text that has no color.
    const QRegion clipRegion = painter->clipRegion();
    KoTextBlockBorderData *lastBorder = 0;
    bool started = false;
    int selectionStart = -1, selectionEnd = -1;
    if (context.textContext.selections.count()) {
        QTextCursor cursor = context.textContext.selections[0].cursor;
        selectionStart = cursor.position();
        selectionEnd = cursor.anchor();
        if (selectionStart > selectionEnd)
            qSwap(selectionStart, selectionEnd);
    }

    // we need to access the whole (parent) table when trying to find out the clipping rectangle
    // for the current table cell (we will use cellAt(...) method), thus we try to get the table from frame
    QTextTable *wholeTable = qobject_cast<QTextTable*>(frame);

    QTextFrame::iterator it;
    for (it = frame->begin(); !(it.atEnd()); ++it) {
        QTextBlock block = it.currentBlock();
        QTextTable *table = qobject_cast<QTextTable*>(it.currentFrame());
        QTextFrame *subFrame = it.currentFrame();
                QTextBlockFormat format = block.blockFormat();

        if (table) {
            m_tableLayout.setTable(table);
            m_tableLayout.drawBackground(painter, context);
            drawFrame(table, painter, context, inTable+1); // this actually only draws the text inside
            m_tableLayout.setTable(table); // in case there was a sub table
            QVector<QLineF> accuBlankBorders;
            m_tableLayout.drawBorders(painter, &accuBlankBorders);
            painter->setPen(QPen(QColor(0,0,0,96)));
            painter->drawLines(accuBlankBorders);
            continue;
        } else if (subFrame) {
            drawFrame(subFrame, painter, context, inTable);
            continue;
        } else {
            if (!block.isValid())
                continue;
        }

        QTextLayout *layout = block.layout();

        if (!painter->hasClipping() || clipRegion.intersects(layout->boundingRect().toRect())) {
            started = true;

            KoTextBlockData *blockData = dynamic_cast<KoTextBlockData*>(block.userData());
            KoTextBlockBorderData *border = 0;
            KoTextBlockPaintStrategyBase *paintStrategy = 0;
            if (blockData) {
                border = blockData->border();
                paintStrategy = blockData->paintStrategy();
            }
            KoTextBlockPaintStrategyBase dummyPaintStrategy;
            if (paintStrategy == 0)
                paintStrategy = &dummyPaintStrategy;
            if (!paintStrategy->isVisible())
                continue; // this paragraph shouldn't be shown so just skip it

            painter->save();
            QBrush bg = paintStrategy->background(block.blockFormat().background());
            if (bg != Qt::NoBrush) {
                    painter->fillRect(layout->boundingRect(), bg);
                    QRectF br = layout->boundingRect();
                    if (block.next().isValid())
                            br.setHeight(br.height());
                    painter->fillRect(br, bg);
            }
            paintStrategy->applyStrategy(painter);
            painter->save();
            drawListItem(painter, block, context.imageCollection);
            painter->restore();

            QVector<QTextLayout::FormatRange> selections;
            foreach(const QAbstractTextDocumentLayout::Selection & selection, context.textContext.selections) {
                QTextCursor cursor = selection.cursor;
                int begin = cursor.position();
                int end = cursor.anchor();
                if (begin > end)
                    qSwap(begin, end);

                if (end < block.position() || begin > block.position() + block.length())
                    continue; // selection does not intersect this block.
                if (selection.cursor.hasComplexSelection()) {
                    continue; // selections of several table cells are covered within drawBorders above.
                }
                if (!m_changeTracker
                    || m_changeTracker->displayChanges()
                    || !m_changeTracker->containsInlineChanges(selection.format)
                    || !m_changeTracker->elementById(selection.format.property(KoCharacterStyle::ChangeTrackerId).toInt())->isEnabled()
                    || (m_changeTracker->elementById(selection.format.property(KoCharacterStyle::ChangeTrackerId).toInt())->getChangeType() != KoGenChange::DeleteChange)) {
                    QTextLayout::FormatRange fr;
                    selection.format.property(KoCharacterStyle::ChangeTrackerId);
                    fr.start = begin - block.position();
                    fr.length = end - begin;
                    fr.format = selection.format;
                    selections.append(fr);
                } else {
                    selection.format.property(KoCharacterStyle::ChangeTrackerId);
                }
            }

            QRectF clipRect;                 // create an empty clipping rectangle

            if (wholeTable) {               // if we're in the table drawing
                m_tableLayout.setTable(wholeTable);
                QTextTableCell currentCell = wholeTable->cellAt(block.position());  // get the currently drawn cell

                if (currentCell.isValid()) {                                        // and if the cell is valid
                    clipRect = m_tableLayout.cellBoundingRect(currentCell);         // get the bounding rectangle from it
                }
            }

            drawTrackedChangeItem(painter, block, selectionStart - block.position(), selectionEnd - block.position(), context.viewConverter);
            if (clipRect.isValid()) {
                painter->save();
                painter->setClipRect(clipRect, Qt::IntersectClip);
            }
            layout->draw(painter, QPointF(0, 0), selections);
            if (clipRect.isValid()) {
                painter->restore();
            }
            decorateParagraph(painter, block, selectionStart - block.position(), selectionEnd - block.position(), context.viewConverter);

            if (lastBorder && lastBorder != border) {
                lastBorder->paint(*painter);
            }
            painter->restore();
            lastBorder = border;
        } else if (started && inTable==0) {
            // when out of the cliprect and no longer in a table, then we are done drawing.
            // The reason we need to put the table condition on is because we might reenter the cliprect later
            break;
        }
    }
    if (lastBorder)
        lastBorder->paint(*painter);
}





void Layout::clearTillEnd()
{
    QTextBlock block = m_block.next();
    while (block.isValid()) {
        if (block.layout()->lineCount() == 0)
            return;
        // erase the layouted lines
        block.layout()->beginLayout();
        block.layout()->endLayout();
        block = block.next();
    }
}

int Layout::cursorPosition() const
{
    int answer = m_block.position();
    if (!m_newParag && layout && layout->lineCount()) {
        QTextLine tl = layout->lineAt(layout->lineCount() - 1);
        answer += tl.textStart() + tl.textLength();
    }
    return answer;
}

qreal Layout::findFootnote(const QTextLine &line, int *oldLength)
{
    if (m_parent->inlineTextObjectManager() == 0 || m_textShape == 0)
        return 0;
    Q_ASSERT(oldLength);
    QString text = m_block.text();
    int pos = text.indexOf(QChar::ObjectReplacementCharacter, line.textStart());
    bool firstFootnote = true;
    while (pos >= 0 && pos <= line.textStart() + line.textLength()) {
        QTextCursor c1(m_block);
        c1.setPosition(m_block.position() + pos);
        pos = text.indexOf(QChar::ObjectReplacementCharacter, pos + 1);
        c1.setPosition(c1.position() + 1, QTextCursor::KeepAnchor);
        KoInlineNote *note = dynamic_cast<KoInlineNote*>(m_parent->inlineTextObjectManager()->inlineTextObject(c1));
        if (note && note->type() == KoInlineNote::Footnote) {
            QTextBlock footnoteBlock = m_textShape->footnoteDocument()->begin();
            bool alreadyPresent = false;
            while (footnoteBlock.isValid()) {
                if (footnoteBlock.blockFormat().property(13471293).value<void*>() == note) {
                    alreadyPresent = true;
                    break;
                }
                footnoteBlock = footnoteBlock.next();
            }
            if (alreadyPresent)
                continue;
            QTextCursor cursor(m_textShape->footnoteDocument());
            cursor.movePosition(QTextCursor::End);
            if (firstFootnote) {
                (*oldLength) = cursor.position();
                firstFootnote = false;
            }
            if (cursor.position() > 1)
                cursor.insertBlock();
            QTextBlockFormat bf;
            QVariant variant;
            variant.setValue<void*>(note);
            bf.setProperty(13471293, variant);
            cursor.mergeBlockFormat(bf);

            QTextCharFormat cf;
            cf.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
            cursor.mergeCharFormat(cf);
            cursor.insertText(note->label() + ' ');
            cf.setVerticalAlignment(QTextCharFormat::AlignNormal);
            cursor.mergeCharFormat(cf);
            cursor.insertFragment(note->text());
        }
    }
    if (m_textShape->hasFootnoteDocument())
        return m_textShape->footnoteDocument()->size().height();
    return 0;
}

QTextTableCell Layout::hitTestTable(QTextTable *table, const QPointF &point)
{
    m_tableLayout.setTable(table);
    return m_tableLayout.hitTestTable(point);
}

void Layout::updateFrameStack()
{
    if (!m_block.isValid()) {
        m_frameStack.clear();
        m_frameStack.append(m_parent->document()->rootFrame());
        return;
    }
    /* for each frame stack item (top first) check if the current block is out of range
        and if so discard it.  */
    for (int i = m_frameStack.count() -1; i >= 0; --i) {
        QTextFrame *frame = m_frameStack.at(i);
        if (frame->firstPosition() > m_block.position() + m_block.length()
                || frame->lastPosition() < m_block.position()) {
            m_frameStack.remove(i);
        } else {
            break;
        }
    }
    if (m_frameStack.isEmpty())
        m_frameStack.append(m_parent->document()->rootFrame());

    int changedFrameFrom = m_frameStack.count();
    /* repeatedly check the deepest nested frame childFrames() and add one if it contains our current block */
    while (true) {
        QTextFrame *frame = m_frameStack.last();
        foreach (QTextFrame *child, frame->childFrames()) {
            if (child->firstPosition() <= m_block.position() && child->lastPosition() > m_block.position()) {
                m_frameStack.append(child);
                break;
            }
        }
        if (frame == m_frameStack.last())
            break;
    }

    for (int i = changedFrameFrom ; i < m_frameStack.count(); ++i) {
        QTextFrame *frame = m_frameStack.at(i);
        QTextFrameFormat ff = frame->frameFormat();
        if (ff.hasProperty(KoText::TableOfContents) && ff.property(KoText::TableOfContents).toBool() == true) {
            // this frame is a TOC
            QList<QWeakPointer<ToCGenerator> >::Iterator iter;
            QWeakPointer<ToCGenerator> item;
            for (iter = m_tocGenerators.begin(); iter != m_tocGenerators.end(); iter++) {
                item = *iter;
                if (item.isNull()) {
                    m_tocGenerators.erase(iter);
                } else {
                    if (item.data()->tocFrame() == frame) {
                        break;
                    }
                }
            }
            if (item.isNull()) {
                ToCGenerator *tg = new ToCGenerator(frame);
                tg->setPageWidth( shape->size().width() );
                m_tocGenerators.append(QWeakPointer<ToCGenerator>(tg));
                // connect to FinishedLayout
                QObject::connect(m_parent, SIGNAL(finishedLayout()),
                                 tg, SLOT(documentLayoutFinished()));
            }
        }
    }
}

void Layout::registerRunAroundShape(KoShape *s)
{
    QTransform matrix = s->absoluteTransformation(0);
    matrix = matrix * shape->absoluteTransformation(0).inverted();
    matrix.translate(0, documentOffsetInShape());
    Outline *outline = new Outline(s, matrix);
    m_outlines.insert(s,outline);
}

void Layout::updateRunAroundShape(KoShape *s)
{
    if (m_outlines.contains(s)) {
        Outline *outline = m_outlines.value(s);
        QTransform matrix = s->absoluteTransformation(0);
        matrix = matrix * shape->absoluteTransformation(0).inverted();
        matrix.translate(0, documentOffsetInShape());
        outline->changeMatrix(matrix);
        m_textLine.updateOutline(outline);
        return;
    }

    // if no outline was found then create one
    registerRunAroundShape(s);
}


void Layout::insertInlineObject(KoTextAnchor * textAnchor)
{
    if (textAnchor != 0) {
        if (textAnchor->behavesAsCharacter()) {
            textAnchor->setAnchorStrategy(new InlineAnchorStrategy(textAnchor));
        } else {
            textAnchor->setAnchorStrategy(new FloatingAnchorStrategy(textAnchor));
        }
        m_textAnchors.append(textAnchor);
    }
}


void Layout::removeInlineObject(KoTextAnchor * textAnchor)
{
    Q_UNUSED(textAnchor);
}

bool Layout::positionInlineObjects()
{
    while (m_textAnchorIndex < m_textAnchors.size()) {
        KoTextAnchor *textAnchor = m_textAnchors[m_textAnchorIndex];
        if (textAnchor->anchorStrategy()->positionShape(this) == false) {
            break;
        }
        // move the index to next not positioned shape
        m_textAnchorIndex++;

        // create outline if the shape is positioned inside text
        if (textAnchor->shape()->textRunAroundSide() != KoShape::RunThrough &&
            !textAnchor->behavesAsCharacter()) {
            updateRunAroundShape(textAnchor->shape());
            return true;
        }
    }

    return false;
}

bool Layout::moveLayoutPosition(KoTextAnchor *textAnchor)
{
    int oldPosition = y();
    QPointF relayoutPos = textAnchor->anchorStrategy()->relayoutPosition();
    qreal recalcFrom = relayoutPos.y() + m_data->documentOffset();
    // move position layout over the textAnchor linked shape
    do {
        if (recalcFrom >= y()) {
            break;
        }
    } while (previousParag());

    if (oldPosition != y()) {
        return true;
    }
    return false;
}

