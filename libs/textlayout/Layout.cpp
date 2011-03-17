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

qreal Layout::docOffsetInShape() const
{
    Q_ASSERT(m_data);
    return m_data->documentOffset();
}

QRectF Layout::expandVisibleRect(const QRectF &rect) const
{
    qreal rightAdjust = qMax<qreal>(m_allTimeMaximumRight - rect.right(), 0.0);
    return rect.adjusted(m_allTimeMinimumLeft, 0.0, rightAdjust, 0.0);
}

//local type for temporary use in addLine
struct LineKeeper
{
    int columns;
    qreal lineWidth;
    QPointF position;
};


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

/**
 * Draw a line. Typically meant to underline text or similar.
 * @param painter the painter to paint on.
 * @painter color the pen color to for the decoratoin line
 * @param type The type
 * @param style the type of line to draw.
 * @param width The thickness of the line, in pixels (the painter will be prescaled to points coordinate system).
 * @param x1 we are always drawing horizontal lines, this is the start point.
 * @param x2 we are always drawing horizontal lines, this is the end point.
 * @param y the y-offset to paint on.
 */
static void drawDecorationLine(QPainter *painter, const QColor &color, KoCharacterStyle::LineType type, KoCharacterStyle::LineStyle style, qreal width, const qreal x1, const qreal x2, const qreal y)
{
    QPen penBackup = painter->pen();
    QPen pen = painter->pen();
    pen.setColor(color);
    pen.setWidthF(width);
    if (style == KoCharacterStyle::WaveLine) {
        // Ok, try the waves :)
        pen.setStyle(Qt::SolidLine);
        painter->setPen(pen);
        qreal x = x1;
        const qreal halfWaveWidth = 2 * width;
        const qreal halfWaveLength = 6 * width;
        const int startAngle = 0 * 16;
        const int middleAngle = 180 * 16;
        const int endAngle = 180 * 16;
        while (x < x2) {
            QRectF rectangle1(x, y - halfWaveWidth, halfWaveLength, 2*halfWaveWidth);
            if (type == KoCharacterStyle::DoubleLine) {
                painter->translate(0, -pen.width());
                painter->drawArc(rectangle1, startAngle, middleAngle);
                painter->translate(0, 2*pen.width());
                painter->drawArc(rectangle1, startAngle, middleAngle);
                painter->translate(0, -pen.width());
            } else {
                painter->drawArc(rectangle1, startAngle, middleAngle);
            }
            if (x + halfWaveLength > x2)
                break;
            QRectF rectangle2(x + halfWaveLength, y - halfWaveWidth, halfWaveLength, 2*halfWaveWidth);
            if (type == KoCharacterStyle::DoubleLine) {
                painter->translate(0, -pen.width());
                painter->drawArc(rectangle2, middleAngle, endAngle);
                painter->translate(0, 2*pen.width());
                painter->drawArc(rectangle2, middleAngle, endAngle);
                painter->translate(0, -pen.width());
            } else {
                painter->drawArc(rectangle2, middleAngle, endAngle);
            }
            x = x + 2 * halfWaveLength;
        }
    } else {
        if (style == KoCharacterStyle::LongDashLine) {
            QVector<qreal> dashes;
            dashes << 12 << 2;
            pen.setDashPattern(dashes);
        } else {
            pen.setStyle((Qt::PenStyle)style);
        }
        painter->setPen(pen);
        if (type == KoCharacterStyle::DoubleLine) {
            painter->translate(0, -pen.width());
            painter->drawLine(QPointF(x1, y), QPointF(x2, y));
            painter->translate(0, 2*pen.width());
            painter->drawLine(QPointF(x1, y), QPointF(x2, y));
            painter->translate(0, -pen.width());
        } else {
            painter->drawLine(QPointF(x1, y), QPointF(x2, y));
        }
    }
    painter->setPen(penBackup);
}

static void drawDecorationText(QPainter *painter, const QTextLine &line, const QColor &color, const QString& decorText, qreal x1, qreal x2)
{
    qreal y = line.position().y();
    QPen oldPen = painter->pen();
    painter->setPen(QPen(color));
    do {
        QRectF br;
        painter->drawText(QRectF(QPointF(x1, y), QPointF(x2, y + line.height())), Qt::AlignLeft | Qt::AlignVCenter, decorText, &br);
        x1 = br.right();
    } while (x1 <= x2);
    painter->setPen(oldPen);
}

static void drawDecorationWords(QPainter *painter, const QTextLine &line, const QString &text, const QColor &color, KoCharacterStyle::LineType type, KoCharacterStyle::LineStyle style, const QString& decorText, qreal width, const qreal y, const int fragmentToLineOffset, const int startOfFragmentInBlock)
{
    qreal wordBeginX = -1;
    int j = line.textStart()+fragmentToLineOffset;
    while (j < line.textLength() + line.textStart() && j-startOfFragmentInBlock<text.size()) {
        if (text[j-startOfFragmentInBlock].isSpace()) {
            if (wordBeginX != -1) {
                if (decorText.isEmpty())
                    drawDecorationLine(painter, color, type, style, width, wordBeginX, line.cursorToX(j), y);
                else
                    drawDecorationText(painter, line, color, decorText, wordBeginX, line.cursorToX(j));
            }
            wordBeginX = -1;
        } else if (wordBeginX == -1) {
            wordBeginX = line.cursorToX(j);
        }
    ++j;
    }
    if (wordBeginX != -1) {
        if (decorText.isEmpty())
            drawDecorationLine(painter, color, type, style, width, wordBeginX, line.cursorToX(j), y);
        else
            drawDecorationText(painter, line, color, decorText, wordBeginX, line.cursorToX(j));
    }
}

static qreal computeWidth(KoCharacterStyle::LineWeight weight, qreal width, const QFont& font)
{
    switch (weight) {
    case KoCharacterStyle::AutoLineWeight:
    case KoCharacterStyle::NormalLineWeight:
    case KoCharacterStyle::MediumLineWeight:
    case KoCharacterStyle::DashLineWeight:
        return QFontMetricsF(font).lineWidth();
    case KoCharacterStyle::BoldLineWeight:
    case KoCharacterStyle::ThickLineWeight:
        return QFontMetricsF(font).lineWidth() * 2;
    case KoCharacterStyle::ThinLineWeight:
        return QFontMetricsF(font).lineWidth() / 2;
    case KoCharacterStyle::PercentLineWeight:
        return QFontInfo(font).pointSizeF() * width / 100;
    case KoCharacterStyle::LengthLineWeight:
        return width;
    }
    Q_ASSERT(0); // illegal weight passed
    return 0;
}

// Decorate any tabs ('\t's) in 'currentFragment' and laid out in 'line'.
void Layout::decorateTabs(QPainter *painter, const QVariantList& tabList, const QTextLine &line, const QTextFragment& currentFragment, int startOfBlock)
{
    // If a line in the layout represent multiple text fragments, this function will
    // be called multiple times on the same line, with different fragments.
    // Likewise, if a fragment spans two lines, then this function will be called twice
    // on the same fragment, once for each line.
    QString fragText = currentFragment.text();
    int fragmentOffset = currentFragment.position() - startOfBlock;

    QFontMetricsF fm(currentFragment.charFormat().font());
    qreal tabStyleLineMargin = fm.averageCharWidth() / 4; // leave some margin for the tab decoration line

    // currentFragment.position() : start of this fragment w.r.t. the document
    // startOfBlock : start of this block w.r.t. the document
    // fragmentOffset : start of this fragment w.r.t. the block
    // line.textStart() : start of this line w.r.t. the block

    int searchForTabFrom; // search for \t from this point onwards in fragText
    int searchForTabTill; // search for \t till this point in fragText

    if (line.textStart() >= fragmentOffset) { // fragment starts at or before the start of line
        // we are concerned with only that part of the fragment displayed in this line
        searchForTabFrom = line.textStart() - fragmentOffset;
        // It's a new line. So we should look at the first tab-stop properties for the next \t.
        m_currentTabStop = 0;
    } else { // fragment starts in the middle of the line
        searchForTabFrom = 0;
    }
    if (line.textStart() + line.textLength() > fragmentOffset + currentFragment.length()) {
        // fragment ends before the end of line. need to see only till the end of the fragment.
        searchForTabTill = currentFragment.length();
    } else {
        // line ends before the fragment ends. need to see only till the end of this line.
        // but then, we need to convert the end of line to an index into fragText
        searchForTabTill = line.textLength() + line.textStart() - fragmentOffset;
    }
    for (int i = searchForTabFrom ; i < searchForTabTill; i++) {
        qreal tabStyleLeftLineMargin = tabStyleLineMargin;
        qreal tabStyleRightLineMargin = tabStyleLineMargin;
        if (m_currentTabStop >= tabList.size()) // no more decorations
            break;
        if (fragText[i] == '\t') {
            // no margin if its adjacent char is also a tab
            if (i > searchForTabFrom && fragText[i-1] == '\t')
                tabStyleLeftLineMargin = 0;
            if (i < (searchForTabTill - 1) && fragText[i+1] == '\t')
                tabStyleRightLineMargin = 0;

            qreal x1 = line.cursorToX(currentFragment.position() - startOfBlock + i);
            qreal x2 = line.cursorToX(currentFragment.position() - startOfBlock + i + 1);

            // find a tab-stop decoration for this tab position
            // for eg., if there's a tab-stop at 1in, but the text before \t already spans 1.2in,
            // we should look at the next tab-stop
            KoText::Tab tab;
            do {
                tab = qvariant_cast<KoText::Tab>(tabList[m_currentTabStop]);
                m_currentTabStop++;
                // comparing with x1 should work for all of left/right/center/char tabs
            } while (tab.position <= x1 && m_currentTabStop < tabList.size());
            if (tab.position <= x1) // no appropriate tab-stop found
                break;

            qreal y = line.position().y() + line.ascent() - 1 ;
            x1 += tabStyleLeftLineMargin;
            x2 -= tabStyleRightLineMargin;
            QColor tabDecorColor = currentFragment.charFormat().foreground().color();
            if (tab.leaderColor.isValid())
                tabDecorColor = tab.leaderColor;
            qreal width = computeWidth(tab.leaderWeight, tab.leaderWidth, painter->font());
            if (x1 < x2) {
                if (tab.leaderText.isEmpty())
                    drawDecorationLine(painter, tabDecorColor, tab.leaderType, tab.leaderStyle, width, x1, x2, y);
                else
                    drawDecorationText(painter, line, tabDecorColor, tab.leaderText, x1, x2);
            }
        }
    }
}

void Layout::drawTrackedChangeItem(QPainter *painter, QTextBlock &block, int selectionStart, int selectionEnd, const KoViewConverter *converter)
{
    Q_UNUSED(selectionStart);
    Q_UNUSED(selectionEnd);
    Q_UNUSED(converter);
    if (!m_changeTracker)
        return;
    QTextLayout *layout = block.layout();
//    QList<QTextLayout::FormatRange> ranges = layout->additionalFormats();

    QTextBlock::iterator it;
    int startOfBlock = -1;
    QFont oldFont = painter->font();

    for (it = block.begin(); !it.atEnd(); ++it) {
        QTextFragment currentFragment = it.fragment();
        if (currentFragment.isValid()) {
            QTextCharFormat fmt = currentFragment.charFormat();
            painter->setFont(fmt.font());
            if (startOfBlock == -1)
                startOfBlock = currentFragment.position(); // start of this block w.r.t. the document
            if (m_changeTracker->containsInlineChanges(fmt)) {
                KoChangeTrackerElement *changeElement = m_changeTracker->elementById(fmt.property(KoCharacterStyle::ChangeTrackerId).toInt());
                if (m_changeTracker->displayChanges() && changeElement->isEnabled()) {
                    int firstLine = layout->lineForTextPosition(currentFragment.position() - startOfBlock).lineNumber();
                    int lastLine = layout->lineForTextPosition(currentFragment.position() + currentFragment.length() - startOfBlock).lineNumber();
                    for (int i = firstLine ; i <= lastLine ; i++) {
                        QTextLine line = layout->lineAt(i);
                        if (layout->isValidCursorPosition(currentFragment.position() - startOfBlock)) {
                            qreal x1 = line.cursorToX(currentFragment.position() - startOfBlock);
                            qreal x2 = line.cursorToX(currentFragment.position() + currentFragment.length() - startOfBlock);

                            switch(changeElement->getChangeType()) {
                                case (KoGenChange::InsertChange):
                                    if (m_changeTracker->displayChanges())
                                        painter->fillRect(x1, line.y(), x2-x1, line.height(), m_changeTracker->getInsertionBgColor());
                                    break;
                                case (KoGenChange::FormatChange):
                                    if (m_changeTracker->displayChanges())
                                        painter->fillRect(x1, line.y(), x2-x1, line.height(), m_changeTracker->getFormatChangeBgColor());
                                    break;
                                case (KoGenChange::DeleteChange):
                                    if (m_changeTracker->displayChanges())
                                        painter->fillRect(x1, line.y(), x2-x1, line.height(), m_changeTracker->getDeletionBgColor());
                                    break;
                            }
                        }
                    }
                }
/*
                QList<QTextLayout::FormatRange>::Iterator iter = ranges.begin();
                const int fragmentBegin = currentFragment.position() - startOfBlock;
                const int fragmentEnd = fragmentBegin + currentFragment.length;
                while (iter != ranges.end()) {
                    QTextLayout::FormatRange r = *(iter);
                    const int rStart = r.start;
                    const int rEnd = r.start + r.length;
                    QTextCharFormat rFormat = r.format;
                    if ((rEnd >= fragmentBegin && rEnd <= fragmentEnd) || (fragmentEnd >= rStart && fragmentEnd <= rEnd)) { //intersect
                        ranges.erase(iter);

                        break;
                    }
                ++iter;
                }



                QTextLayout::FormatRange range;
                range.start = currentFragment.position() - startOfBlock;
                range.length = currentFragment.length();
                KoChangeTrackerElement *changeElement = m_changeTracker->elementById(fmt.property(KoCharacterStyle::ChangeTrackerId).toInt());
                QTextCharFormat format;
                switch(changeElement->getChangeType()) {
                    case (KoGenChange::insertChange):
                        //                painter->save();
                        //                painter->setBackground(QBrush(Qt::green));
                        format.setBackground(QBrush(Qt::green));
//                        painter->fillRect(x1, line.y(), x2-x1, line.height(), QColor(0,255,0,255));
                        //                painter->restore();
                        break;
                    case (KoGenChange::formatChange):
                        //                painter->save();
                        //                painter->setBackground(QBrush(Qt::blue));
//                        painter->fillRect(x1, line.y(), x2-x1, line.height(), QColor(0,0,255,255));
                        format.setBackground(QBrush(Qt::blue));
                        //                painter->restore();
                        break;
                    case (KoGenChange::deleteChange):
                        break;
                }
                range.format = format;
                kDebug() << "added range: pos: " << range.start << " length: " << range.length << " color: " << range.format.background().color();
                ranges.append(range);*/
            }
        }
    }
    painter->setFont(oldFont);
//    layout->setAdditionalFormats(ranges);
}

void Layout::drawStrikeOuts(QPainter *painter, const QTextFragment &currentFragment, const QTextLine &line, qreal x1, qreal x2, const int startOfFragmentInBlock, const int fragmentToLineOffset) const
{
    QTextCharFormat fmt = currentFragment.charFormat();
    KoCharacterStyle::LineStyle strikeOutStyle = (KoCharacterStyle::LineStyle)
            fmt.intProperty(KoCharacterStyle::StrikeOutStyle);
    KoCharacterStyle::LineType strikeOutType = (KoCharacterStyle::LineType)
            fmt.intProperty(KoCharacterStyle::StrikeOutType);
    if ((strikeOutStyle != KoCharacterStyle::NoLineStyle) &&
            (strikeOutType != KoCharacterStyle::NoLineType)) {
        QTextCharFormat::VerticalAlignment valign = fmt.verticalAlignment();

        QFont font(fmt.font());
        if (valign == QTextCharFormat::AlignSubScript
                || valign == QTextCharFormat::AlignSuperScript)
            font.setPointSize(qRound(font.pointSize() * 2 / 3.));
        QFontMetricsF metrics(font, m_parent->paintDevice());

        qreal y = line.position().y();
        if (valign == QTextCharFormat::AlignSubScript)
            y += line.height() - metrics.descent() - metrics.strikeOutPos();
        else if (valign == QTextCharFormat::AlignSuperScript)
            y += metrics.ascent() - metrics.strikeOutPos();
        else
            y += line.ascent() - metrics.strikeOutPos();

        QColor color = fmt.colorProperty(KoCharacterStyle::StrikeOutColor);
        if (!color.isValid())
            color = fmt.foreground().color();
        KoCharacterStyle::LineMode strikeOutMode =
            (KoCharacterStyle::LineMode) fmt.intProperty(KoCharacterStyle::StrikeOutMode);

        QString strikeOutText = fmt.stringProperty(KoCharacterStyle::StrikeOutText);
        qreal width = 0; // line thickness
        if (strikeOutText.isEmpty()) {
            width = computeWidth(
                        (KoCharacterStyle::LineWeight) fmt.intProperty(KoCharacterStyle::StrikeOutWeight),
                        fmt.doubleProperty(KoCharacterStyle::StrikeOutWidth),
                        font);
        }
        if (valign == QTextCharFormat::AlignSubScript
                || valign == QTextCharFormat::AlignSuperScript) // adjust size.
            width = width * 2 / 3;

        if (strikeOutMode == KoCharacterStyle::SkipWhiteSpaceLineMode) {
            drawDecorationWords(painter, line, currentFragment.text(), color, strikeOutType,
                    strikeOutStyle, strikeOutText, width, y, fragmentToLineOffset,
                    startOfFragmentInBlock);
        } else {
            if (strikeOutText.isEmpty())
                drawDecorationLine(painter, color, strikeOutType, strikeOutStyle, width, x1, x2, y);
            else
                drawDecorationText(painter, line, color, strikeOutText, x1, x2);
        }
    }
}

void Layout::drawUnderlines(QPainter *painter, const QTextFragment &currentFragment, const QTextLine &line, qreal x1, qreal x2, const int startOfFragmentInBlock, const int fragmentToLineOffset) const
{
    QTextCharFormat fmt = currentFragment.charFormat();
    KoCharacterStyle::LineStyle fontUnderLineStyle = (KoCharacterStyle::LineStyle) fmt.intProperty(KoCharacterStyle::UnderlineStyle);
    KoCharacterStyle::LineType fontUnderLineType = (KoCharacterStyle::LineType) fmt.intProperty(KoCharacterStyle::UnderlineType);
    if ((fontUnderLineStyle != KoCharacterStyle::NoLineStyle) &&
            (fontUnderLineType != KoCharacterStyle::NoLineType)) {
        QTextCharFormat::VerticalAlignment valign = fmt.verticalAlignment();

        QFont font(fmt.font());
        if (valign == QTextCharFormat::AlignSubScript
                || valign == QTextCharFormat::AlignSuperScript)
            font.setPointSize(font.pointSize() * 2 / 3);
        QFontMetricsF metrics(font, m_parent->paintDevice());

        qreal y = line.position().y();
        if (valign == QTextCharFormat::AlignSubScript)
            y += line.height() - metrics.descent() + metrics.underlinePos();
        else if (valign == QTextCharFormat::AlignSuperScript)
            y += metrics.ascent() + metrics.underlinePos();
        else
            y += line.ascent() + metrics.underlinePos();

        QColor color = fmt.underlineColor();
        if (!color.isValid())
            color = fmt.foreground().color();
        KoCharacterStyle::LineMode underlineMode =
            (KoCharacterStyle::LineMode) fmt.intProperty(KoCharacterStyle::UnderlineMode);
        qreal width = computeWidth( // line thickness
                          (KoCharacterStyle::LineWeight) fmt.intProperty(KoCharacterStyle::UnderlineWeight),
                          fmt.doubleProperty(KoCharacterStyle::UnderlineWidth),
                          font);
        if (valign == QTextCharFormat::AlignSubScript
                || valign == QTextCharFormat::AlignSuperScript) // adjust size.
            width = width * 2 / 3;

        if (underlineMode == KoCharacterStyle::SkipWhiteSpaceLineMode) {
            drawDecorationWords(painter, line, currentFragment.text(), color, fontUnderLineType,
                    fontUnderLineStyle, QString(), width, y, fragmentToLineOffset, startOfFragmentInBlock);
        } else {
            drawDecorationLine(painter, color, fontUnderLineType, fontUnderLineStyle, width, x1, x2, y);
        }
    }
}

void Layout::decorateParagraph(QPainter *painter, const QTextBlock &block, int selectionStart, int selectionEnd, const KoViewConverter *converter)
{
    Q_UNUSED(selectionStart);
    Q_UNUSED(selectionEnd);
    Q_UNUSED(converter);

    QTextLayout *layout = block.layout();
    QTextOption textOption = layout->textOption();

    QTextBlockFormat bf = block.blockFormat();
    QVariantList tabList = bf.property(KoParagraphStyle::TabPositions).toList();
    QFont oldFont = painter->font();

    QTextBlock::iterator it;
    int startOfBlock = -1;
    // loop over text fragments in this paragraph and draw the underline and line through.
    for (it = block.begin(); !it.atEnd(); ++it) {
        QTextFragment currentFragment = it.fragment();
        if (currentFragment.isValid()) {
            QTextCharFormat fmt = currentFragment.charFormat();
            painter->setFont(fmt.font());
            if (startOfBlock == -1)
                startOfBlock = currentFragment.position(); // start of this block w.r.t. the document
            int firstLine = layout->lineForTextPosition(currentFragment.position() - startOfBlock).lineNumber();
            int lastLine = layout->lineForTextPosition(currentFragment.position() + currentFragment.length()
                    - startOfBlock).lineNumber();
            int startOfFragmentInBlock = currentFragment.position() - startOfBlock;
            for (int i = firstLine ; i <= lastLine ; i++) {
                QTextLine line = layout->lineAt(i);
                if (layout->isValidCursorPosition(currentFragment.position() - startOfBlock)) {
                    int p1 = currentFragment.position() - startOfBlock;
                    if (block.text().at(p1) != QChar::ObjectReplacementCharacter) {
                        int p2 = currentFragment.position() + currentFragment.length() - startOfBlock;
                        int fragmentToLineOffset = qMax(currentFragment.position() - startOfBlock - line.textStart(),0);
                        qreal x1 = line.cursorToX(p1);
                        qreal x2 = line.cursorToX(p2);
                        // Following line was supposed to fix bug 171686 (I cannot reproduce the original problem) but it opens bug 260159. So, deactivated for now.
                        //x2 = qMin(x2, line.naturalTextWidth() + line.cursorToX(line.textStart()));
                        drawStrikeOuts(painter, currentFragment, line, x1, x2, startOfFragmentInBlock, fragmentToLineOffset);
                        drawUnderlines(painter, currentFragment, line, x1, x2, startOfFragmentInBlock, fragmentToLineOffset);
                        decorateTabs(painter, tabList, line, currentFragment, startOfBlock);
                    }
                }
            }
        }
    }
    painter->setFont(oldFont);
}

void Layout::drawListItem(QPainter *painter, const QTextBlock &block, KoImageCollection *imageCollection)
{
    KoTextBlockData *data = dynamic_cast<KoTextBlockData*>(block.userData());
    if (data == 0)
        return;

    QTextList *list = block.textList();
    if (list && data->hasCounterData()) {
        QTextListFormat listFormat = list->format();
        QTextCharFormat chFormatMaxFontSize;

        KoCharacterStyle *cs = 0;
        if (m_styleManager) {
            const int id = listFormat.intProperty(KoListStyle::CharacterStyleId);
            cs = m_styleManager->characterStyle(id);
            if (!cs) {
                KoParagraphStyle *ps = m_styleManager->paragraphStyle(
                                       block.blockFormat().intProperty(KoParagraphStyle::StyleId));
                if (ps && !ps->hasDefaults()) {
                    cs = ps->characterStyle();
                }
            }
        }

        if ( cs && cs->hasProperty(QTextFormat::FontPointSize) ) {
                cs->applyStyle(chFormatMaxFontSize);
        } else {
            // use format from the actual block of the list item
            QTextCharFormat chFormatBlock;
            if (block.text().size() == 0) {
                chFormatBlock = block.charFormat();
            } else {
                chFormatBlock = block.begin().fragment().charFormat();
            }

            chFormatMaxFontSize = chFormatBlock;

            QTextBlock::iterator it;
            QTextFragment currentFragment;
            for (it = block.begin(); !it.atEnd(); ++it) {
                currentFragment = it.fragment();
                if ( currentFragment.isValid() && (chFormatMaxFontSize.fontPointSize() < currentFragment.charFormat().fontPointSize()) ) {
                    chFormatMaxFontSize = currentFragment.charFormat();
                }
            }
        }

        // fetch the text properties of the list-level-style-bullet
        if (listFormat.hasProperty(KoListStyle::MarkCharacterStyleId)) {
            QVariant v = listFormat.property(KoListStyle::MarkCharacterStyleId);
            QSharedPointer<KoCharacterStyle> textPropertiesCharStyle = v.value< QSharedPointer<KoCharacterStyle> >();
            if (!textPropertiesCharStyle.isNull()) {
                textPropertiesCharStyle->applyStyle( chFormatMaxFontSize );
            }
        }

        if (! data->counterText().isEmpty()) {
            QFont font(chFormatMaxFontSize.font(), m_parent->paintDevice());

            QString result = data->counterText();
            KoListStyle::Style listStyle = static_cast<KoListStyle::Style>(listFormat.style());
            if (listStyle == KoListStyle::SquareItem            || listStyle == KoListStyle::DiscItem       ||
                listStyle == KoListStyle::CircleItem            || listStyle == KoListStyle::BoxItem        ||
                listStyle == KoListStyle::RhombusItem           || listStyle == KoListStyle::CustomCharItem ||
                listStyle == KoListStyle::HeavyCheckMarkItem    || listStyle == KoListStyle::BallotXItem    ||
                listStyle == KoListStyle::RightArrowItem        || listStyle == KoListStyle::RightArrowHeadItem)
            {
                QChar bulletChar(listFormat.intProperty(KoListStyle::BulletCharacter));
                result = bulletChar;
            }

            QTextLayout layout(result , font, m_parent->paintDevice());

            QList<QTextLayout::FormatRange> layouts;
            QTextLayout::FormatRange format;
            format.start = 0;
            format.length = data->counterText().length();
            format.format = chFormatMaxFontSize;

            layouts.append(format);
            layout.setAdditionalFormats(layouts);

            Qt::Alignment align = static_cast<Qt::Alignment>(listFormat.intProperty(KoListStyle::Alignment));

            if (align == 0) {
                align = Qt::AlignLeft;
            }
            else if (align != Qt::AlignLeft) {
                align |= Qt::AlignAbsolute;
            }

            QTextOption option(align);
            option.setTextDirection(block.layout()->textOption().textDirection());

            if (option.textDirection() == Qt::RightToLeft || data->counterText().isRightToLeft()) {
                option.setAlignment(Qt::AlignRight);
            }

            layout.setTextOption(option);
            layout.beginLayout();

            QTextLine line = layout.createLine();
            line.setLineWidth(data->counterWidth());
            layout.endLayout();

            QPointF counterPosition = data->counterPosition();
            if (block.layout()->lineCount() > 0) {
                // if there is text, then baseline align the counter.
                QTextLine firstParagLine = block.layout()->lineAt(0);
                counterPosition += QPointF(0, firstParagLine.ascent() - layout.lineAt(0).ascent());
            }

            layout.draw(painter, counterPosition);
        }

#if 0
            QFontMetricsF fm(chFormatMaxFontSize.font(), m_parent->paintDevice());
            // helper lines to show the anatomy of this font.
            painter->setPen(Qt::green);
            painter->drawLine(QLineF(-1, data->counterPosition().y(), 200, data->counterPosition().y()));
            painter->setPen(Qt::yellow);
            painter->drawLine(QLineF(-1, data->counterPosition().y() + fm.ascent() - fm.xHeight(), 200, data->counterPosition().y() + fm.ascent() - fm.xHeight()));
            painter->setPen(Qt::blue);
            painter->drawLine(QLineF(-1, data->counterPosition().y() + fm.ascent(), 200, data->counterPosition().y() + fm.ascent()));
            painter->setPen(Qt::gray);
            painter->drawLine(QLineF(-1, data->counterPosition().y() + fm.height(), 200, data->counterPosition().y() + fm.height()));
#endif

        KoListStyle::Style listStyle = static_cast<KoListStyle::Style>(listFormat.style());
        if (listStyle == KoListStyle::ImageItem && imageCollection) {
            QFontMetricsF fm(chFormatMaxFontSize.font(), m_parent->paintDevice());
            qreal x = qMax(qreal(1), data->counterPosition().x());
            qreal width = qMax(listFormat.doubleProperty(KoListStyle::Width), (qreal)1.0);
            qreal height = qMax(listFormat.doubleProperty(KoListStyle::Height), (qreal)1.0);
            qreal y = data->counterPosition().y() + fm.ascent() - fm.xHeight()/2 - height/2; // centered
            qint64 key = listFormat.property(KoListStyle::BulletImageKey).value<qint64>();
            KoImageData idata;
            imageCollection->fillFromKey(idata, key);
            painter->drawPixmap(x, y, width, height, idata.pixmap());
        }
    }
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

