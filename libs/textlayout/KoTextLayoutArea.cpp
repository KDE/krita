/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2008 Roopesh Chander <roop@forwardbias.in>
 * Copyright (C) 2007-2008 Pierre Ducroquet <pinaraf@pinaraf.info>
 * Copyright (C) 2009-2011 KO GmbH <cbo@kogmbh.com>
 * Copyright (C) 2009-2011 Casper Boemann <cbo@boemann.dk>
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

#include "KoTextLayoutArea.h"

#include "KoTextLayoutTableArea.h"
#include "TableIterator.h"
#include "ListItemsHelper.h"
#include "RunAroundHelper.h"
#include "KoTextDocumentLayout.h"

#include <KoParagraphStyle.h>
#include <KoCharacterStyle.h>
#include <KoListStyle.h>
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
#include <KoInlineTextObjectManager.h>

#include <KDebug>

#include <QTextTable>
#include <QTextList>
#include <QStyle>
#include <QFontMetrics>
#include <QTextFragment>
#include <QTextLayout>
#include <QTextCursor>

extern int qt_defaultDpiY();

#define DropCapsAdditionalFormattingId 25602902

KoTextLayoutArea::KoTextLayoutArea(KoTextLayoutArea *p, KoTextDocumentLayout *documentLayout)
 : parent(p)
 , m_documentLayout(documentLayout)
 , m_borderInsets(0,0,0,0)
 , m_rootAreaIsNew(true)
 , m_dropCapsWidth(0)
 , m_startOfArea(0)
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

int KoTextLayoutArea::hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const
{
    if (m_startOfArea == 0) // We have not been layouted yet
        return -1;

    int position = -1;
    bool basicallyFound = false;

    QTextFrame::iterator it = m_startOfArea->it;
    QTextFrame::iterator stop = m_endOfArea->it;
    if(!stop.currentBlock().isValid() || m_endOfArea->line.isValid()) {
        ++stop;
    }
    int tableAreaIndex = 0;
    for (; it != stop; ++it) {
        QTextBlock block = it.currentBlock();
        QTextTable *table = qobject_cast<QTextTable*>(it.currentFrame());
        QTextFrame *subFrame = it.currentFrame();
        QTextBlockFormat format = block.blockFormat();

        if (table) {
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
        if (basicallyFound) // a subsequent table or lines have now had their chance
            return position;

        QTextLayout *layout = block.layout();
        QTextFrame::iterator next = it;
        next++;
        if (next != stop && point.y() > layout->boundingRect().bottom()) {
            // just skip this block.
            continue;
        }

        for (int i = 0; i < layout->lineCount(); i++) {
            QTextLine line = layout->lineAt(i);
            if (point.y() > line.y() + line.height()) {
                position = block.position() + line.textStart() + line.textLength();
                continue;
            }
            if (accuracy == Qt::ExactHit && point.y() < line.y()) { // between lines
                return -1;
            }
            if (accuracy == Qt::ExactHit && // left or right of line
                    (point.x() < line.naturalTextRect().left() || point.x() > line.naturalTextRect().right())) {
                return -1;
            }
            if (point.x() > line.x() + line.naturalTextWidth() && layout->textOption().textDirection() == Qt::RightToLeft) {
                // totally right of RTL text means the position is the start of the text.
                //TODO how about the other side?
                return block.position() + line.textStart();
            }
            if (point.x() > line.x() + line.naturalTextWidth()) {
                // right of line
                basicallyFound = true;
                position = block.position() + line.textStart() + line.textLength();
                continue;
            }
            return block.position() + line.xToCursor(point.x());
        }
    }
    return position;
}

QRectF KoTextLayoutArea::selectionBoundingBox(QTextCursor &cursor) const
{
    QRectF retval(-5E6,0,105E6,1);

    if (m_startOfArea == 0) // We have not been layouted yet
        return QRectF();

    QTextFrame::iterator it = m_startOfArea->it;
    QTextFrame::iterator stop = m_endOfArea->it;
    if(!stop.currentBlock().isValid() || m_endOfArea->line.isValid()) {
        ++stop;
    }
    int tableAreaIndex = 0;
    for (; it != stop; ++it) {
        QTextBlock block = it.currentBlock();
        QTextTable *table = qobject_cast<QTextTable*>(it.currentFrame());
        QTextFrame *subFrame = it.currentFrame();
        QTextBlockFormat format = block.blockFormat();

        if (table) {
            if (cursor.selectionEnd() < table->firstPosition()) {
                return retval;
            }
            if (cursor.selectionStart() > table->lastPosition()) {
                ++tableAreaIndex;
                continue;
            }
            if (cursor.selectionStart() > table->firstPosition() && cursor.selectionEnd() < table->lastPosition()) {
                return m_tableAreas[tableAreaIndex]->selectionBoundingBox(cursor);
            }
            if (cursor.selectionStart() > table->firstPosition()) {
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

        if(cursor.selectionEnd() < block.position()) {
            return retval;
        }
        if(cursor.selectionStart() >= block.position()
            && cursor.selectionStart() < block.position() + block.length()) {
            QTextLine line = block.layout()->lineForTextPosition(cursor.selectionStart() - block.position());
            if (line.isValid())
                retval.setTop(line.y());
        }
        if(cursor.selectionEnd() >= block.position()
            && cursor.selectionEnd() < block.position() + block.length()) {
            QTextLine line = block.layout()->lineForTextPosition(cursor.selectionEnd() - block.position());
            if (line.isValid())
                retval.setBottom
                (line.y() + line.height());
        }
    }
    return retval;
}

bool KoTextLayoutArea::layout(FrameIterator *cursor)
{
    m_startOfArea = new FrameIterator(cursor);
    m_y = top();
    m_bottomSpacing = 0;
    m_footNotesHeight = 0;
    m_preregisteredFootNotesHeight = 0;
    m_rootAreaIsNew = true;

    while (true) {
        QTextBlock block = cursor->it.currentBlock();
        QTextTable *table = qobject_cast<QTextTable*>(cursor->it.currentFrame());
        QTextFrame *subFrame = cursor->it.currentFrame();
        if (table) {
            // Let's create KoTextLayoutTableArea and let that handle the table
            KoTextLayoutTableArea *tableArea = new KoTextLayoutTableArea(table, this, m_documentLayout);
            m_tableAreas.append(tableArea);
            m_y += m_bottomSpacing;
            tableArea->setReferenceRect(left(), right(), m_y, maximumAllowedBottom());
            if (tableArea->layout(cursor->tableIterator(table)) == false) {
                m_endOfArea = new FrameIterator(cursor);
                m_y = tableArea->bottom();
                setBottom(m_y);
                return false;
            }
            m_bottomSpacing = 0;
            m_y = tableArea->bottom();
            delete cursor->currentTableIterator;
            cursor->currentTableIterator = 0;
        } else if (subFrame) {
            // Right now we'll just skip it, as we know of no subframes
        } else if (block.isValid()) {
            if (layoutBlock(cursor) == false) {
                m_endOfArea = new FrameIterator(cursor);
                setBottom(m_y);
                return false;
            }
        }
        if (cursor->it.atEnd()) {
            m_endOfArea = new FrameIterator(cursor);
            m_y += m_bottomSpacing;
            setBottom(m_y);
            return true; // we have layouted till the end of the frame
        }

        ++(cursor->it);
    }
}

// layoutBlock() method is structured like this:
//
// 1) Setup various helper values
//   a) related to or influenced by lists
//   b) related to or influenced by tabs
//   c) related to or influenced by dropcaps
// 2)layout each line (possibly restarting where we stopped earlier)
//   a) fit line into sub lines with as needed for text runaround
//   b) make sure we keep above maximumAllowedBottom
//   c) calls addLine()
//   d) update dropcaps related variables
bool KoTextLayoutArea::layoutBlock(FrameIterator *cursor)
{
    QTextBlock block(cursor->it.currentBlock());
    KoTextBlockData *blockData = dynamic_cast<KoTextBlockData *>(block.userData());
    QTextBlockFormat format = block.blockFormat();

    int dropCapsAffectsNMoreLines = 0;
    qreal dropCapsPositionAdjust;

    KoText::Direction dir = static_cast<KoText::Direction>(format.intProperty(KoParagraphStyle::TextProgressionDirection));
    if (dir == KoText::InheritDirection)
        dir = parentTextDirection();
    if (dir == KoText::AutoDirection)
        m_isRtl = block.text().isRightToLeft();
    else
        m_isRtl =  dir == KoText::RightLeftTopBottom || dir == KoText::PerhapsRightLeftTopBottom;


    // initialize list item stuff for this parag.
    QTextList *textList = block.textList();
    if (textList) {
        QTextListFormat format = textList->format();
        int styleId = format.intProperty(KoListStyle::CharacterStyleId);
        KoCharacterStyle *charStyle = 0;
        if (styleId > 0 && m_documentLayout->styleManager())
            charStyle = m_documentLayout->styleManager()->characterStyle(styleId);
        if (!charStyle && m_documentLayout->styleManager()) { // try the one from paragraph style
            KoParagraphStyle *ps = m_documentLayout->styleManager()->paragraphStyle(
                                       format.intProperty(KoParagraphStyle::StyleId));
            if (ps)
                charStyle = ps->characterStyle();
        }

        if (!(blockData && blockData->hasCounterData())) {
            QFont font;
            if (charStyle)
                font = QFont(charStyle->fontFamily(), qRound(charStyle->fontPointSize()),
                             charStyle->fontWeight(), charStyle->fontItalic());
            else {
                QTextCursor cursor(block);
                font = cursor.charFormat().font();
            }
            ListItemsHelper lih(textList, font);
            lih.recalculateBlock(block);
            blockData = dynamic_cast<KoTextBlockData*>(block.userData());
        }
    } else if (blockData) { // make sure it is empty
        blockData->setCounterText(QString());
        blockData->setCounterSpacing(0.0);
        blockData->setCounterWidth(0.0);
        blockData->setCounterIsImage(false);
    }

    if (blockData == 0) {
        blockData = new KoTextBlockData();
        block.setUserData(blockData);
    }

    m_y += qMax(m_bottomSpacing, format.topMargin());

    blockData->setEffectiveTop(m_y);

    QTextLayout *layout = block.layout();

    QTextOption option = layout->textOption();
    option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    qreal tabStopDistance =  format.property(KoParagraphStyle::TabStopDistance).toDouble();
    if (tabStopDistance > 0) {
        tabStopDistance *= qt_defaultDpiY() / 72.;
        option.setTabStop(tabStopDistance);
    } else {
        option.setTabStop(m_defaultTabSizing);
        tabStopDistance = m_defaultTabSizing;
    }

    // tabs
    QList<QTextOption::Tab> tabs;
    QVariant variant = format.property(KoParagraphStyle::TabPositions);
    qreal tabOffset = - left();
    if (m_documentLayout->relativeTabs()) {
        tabOffset += (m_isRtl ? format.rightMargin() : (format.leftMargin() + listIndent())) ;
    }
    // Set up a var to keep track of where last added tab is. Conversion of tabOffset is required because Qt thinks in device units and we don't
    qreal position = tabOffset * qt_defaultDpiY() / 72.;

    if (!variant.isNull()) {
        foreach(const QVariant &tv, qvariant_cast<QList<QVariant> >(variant)) {
            KoText::Tab koTab = tv.value<KoText::Tab>();
            QTextOption::Tab tab;

            // conversion here is required because Qt thinks in device units and we don't
            position = (koTab.position + tabOffset) * qt_defaultDpiY() / 72.;

            tab.position = position;
            tab.type = koTab.type;
            tab.delimiter = koTab.delimiter;
            tabs.append(tab);
        }
    }

    // Since we might have tabs relative to first indent we need to always specify a lot of
    // regular interval tabs (relative to the indent naturally)
    // So first figure out where the first regular interval tab should be.
    position -= tabOffset * qt_defaultDpiY() / 72.;
    position = (int(position / tabStopDistance) + 1) * tabStopDistance + tabOffset * qt_defaultDpiY() / 72.;
    for(int i=0 ; i<16; ++i) { // let's just add 16 but we really should limit to pagewidth
        QTextOption::Tab tab;

        // conversion here is required because Qt thinks in device units and we don't
        tab.position = position;
        tabs.append(tab);
        position += tabStopDistance;
    }
    option.setTabs(tabs);

    option.setAlignment(QStyle::visualAlignment(m_isRtl ? Qt::RightToLeft : Qt::LeftToRight, format.alignment()));
    if (m_isRtl)
        option.setTextDirection(Qt::RightToLeft);
    else
        option.setTextDirection(Qt::LeftToRight);

    option.setUseDesignMetrics(true);

    // Drop caps
    // first remove any drop-caps related formatting that's already there in the layout.
    // we'll do it all afresh now.
    QList<QTextLayout::FormatRange> formatRanges = layout->additionalFormats();
    for (QList< QTextLayout::FormatRange >::Iterator iter = formatRanges.begin();
            iter != formatRanges.end();
        ++iter) {
        if (iter->format.boolProperty(DropCapsAdditionalFormattingId)) {
            formatRanges.erase(iter);
        }
    }
    if (formatRanges.count() != layout->additionalFormats().count())
        layout->setAdditionalFormats(formatRanges);

    int dropCaps = format.boolProperty(KoParagraphStyle::DropCaps);
    int dropCapsLength = format.intProperty(KoParagraphStyle::DropCapsLength);
    int dropCapsLines = format.intProperty(KoParagraphStyle::DropCapsLines);
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
        qreal lineHeight = format.doubleProperty(KoParagraphStyle::FixedLineHeight);
        qreal dropCapsHeight = 0;
        if (lineHeight == 0) {
            lineHeight = lineRepresentative.charFormat().fontPointSize();
            qreal linespacing = format.doubleProperty(KoParagraphStyle::LineSpacing);
            if (linespacing == 0) { // unset
                int percent = format.intProperty(KoParagraphStyle::PercentLineHeight);
                if (percent != 0)
                    linespacing = lineHeight * ((percent - 100) / 100.0);
                else if (linespacing == 0)
                    linespacing = lineHeight * 0.2; // default
            }
            dropCapsHeight = linespacing * (dropCapsLines-1);
        }
        const qreal minimum = format.doubleProperty(KoParagraphStyle::MinimumLineHeight);
        if (minimum > 0.0) {
            lineHeight = qMax(lineHeight, minimum);
        }

        dropCapsHeight += lineHeight * dropCapsLines;

        int dropCapsStyleId = format.intProperty(KoParagraphStyle::DropCapsTextStyle);
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

    layout->setTextOption(option);

    layout->beginLayout();

    cursor->fragmentIterator = block.begin();

    if (!m_rootAreaIsNew) {
        m_y += m_borderInsets.bottom;
    }

    //Now once we know the physical context we can work on the borders of the paragraph
    updateBorders(blockData, &block); // fill the border inset member vars.

    m_y += m_borderInsets.top;

    m_listIndent = 0;
    if (textList) {
        m_listIndent = textList->format().doubleProperty(KoListStyle::Indent);
        // if list set list-indent. Do this after borders init to we can account for them.
        // Also after we account for indents etc so the y() pos is correct.
        if (m_isRtl)
            blockData->setCounterPosition(QPointF(right() -
                        blockData->counterWidth() - format.leftMargin(), m_y));
        else {
            blockData->setCounterPosition(QPointF(left(), m_y));
            m_listIndent += blockData->counterSpacing() + blockData->counterWidth();
        }
    }

    m_width = right() - left() - listIndent();
    m_width -= format.leftMargin() + format.rightMargin();

    if (m_isRtl) {
        m_x = left() + format.rightMargin();
        m_width -= blockData->counterWidth() + blockData->counterSpacing();
    } else {
        m_x = left() + format.leftMargin() + m_listIndent;
    }

    m_x += m_borderInsets.left;
    m_width -= m_borderInsets.left + m_borderInsets.right;

    m_indent = 0;
    if (cursor->line.isValid() == false) {
        m_indent = textIndent(block);
        cursor->line = layout->createLine();
        cursor->fragmentIterator = block.begin();
    }

    // So now is the time to create the lines of this paragraph
    RunAroundHelper runAroundHelper;
    runAroundHelper.setLine(this, cursor->line);

    //FIXME refreshCurrentPageOutlines();
    //FIXME runAroundHelper.setOutlines(m_currentLineOutlines);

    qreal maxLineHeight = 0;
    qreal y_justBelowDropCaps = 0;

    while (cursor->line.isValid()) {
        //FIXME runAroundHelper.fit( /* resetHorizontalPosition */ false, QPointF(x, m_y));
        //FIXME When above runaround is no longer commented out we should not set width and pos
        cursor->line.setLineWidth(width());
        cursor->line.setPosition(QPointF(x(), m_y));

        qreal bottomOfText = cursor->line.y() + cursor->line.height();

        findFootNotes(block, cursor->line);

        if (bottomOfText > maximumAllowedBottom()) {
            // We can not fit line within our allowed space
            if (format.nonBreakableLines()) {
                //set an invalid line so we start this block from beginning next time
                cursor->line = QTextLine();
            }
            clearPreregisteredFootNotes();
            return false; //to indicate block was not done!
        }
        confirmFootNotes();

        maxLineHeight = qMax(maxLineHeight, addLine(cursor, blockData));
        //FIXME runAroundHelper.fit( /* resetHorizontalPosition */ true, QPointF(x, m_y));

        if (true /*FIXME !runAroundHelper.stayOnBaseline()*/) {
            m_y += maxLineHeight;
            maxLineHeight = 0;
            m_indent = 0;
        }

        // drop caps
        if (m_dropCapsNChars > 0) { // we just laid out the dropped chars
            y_justBelowDropCaps = m_y; // save the y position just below the dropped characters
            m_y = cursor->line.y();              // keep the same y for the next line
            cursor->line.setPosition(cursor->line.position() - QPointF(0, dropCapsPositionAdjust));
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


        m_boundingRect.setLeft(qMin(cursor->line.x(), m_boundingRect.x()));
        m_boundingRect.setRight(qMax(cursor->line.x()+cursor->line.width(),
                                     m_boundingRect.right()));

        // line fitted so try and do the next one
        cursor->line = layout->createLine();
        runAroundHelper.setLine(this, cursor->line);
        //FIXME refreshCurrentPageOutlines();
        //FIXME runAroundHelper.setOutlines(m_currentLineOutlines);
    }
    cursor->line = QTextLine(); //set an invalid line to indicate we are done with block

    m_bottomSpacing = format.bottomMargin();

    return true;
}

qreal KoTextLayoutArea::listIndent() const
{
    return m_listIndent;
}

qreal KoTextLayoutArea::textIndent(QTextBlock block) const
{
    if ((block.blockFormat().property(KoParagraphStyle::AutoTextIndent).toBool())) {
        // if auto-text-indent is set,
        // return an indent approximately 3-characters wide as per current font
        QTextCursor blockCursor(block);
        qreal guessGlyphWidth = QFontMetricsF(blockCursor.charFormat().font()).width('x');
        return guessGlyphWidth * 3;
    }
    return block.blockFormat().textIndent();
}

qreal KoTextLayoutArea::x() const
{
    return m_x + m_indent + (m_dropCapsNChars == 0 ? m_dropCapsWidth : 0.0);
}

qreal KoTextLayoutArea::width() const
{
    if (m_dropCapsNChars > 0) {
        return m_dropCapsWidth + 10;
    }
    return m_width - m_indent - m_dropCapsWidth;
}

qreal KoTextLayoutArea::addLine(FrameIterator *cursor, KoTextBlockData *blockData)
{
    QTextBlock block = cursor->it.currentBlock();
    QTextLine line =  cursor->line;
    QTextBlockFormat format = block.blockFormat();

    if (blockData && block.textList() && block.layout()->lineCount() == 1) {
        // first line, lets check where the line ended up and adjust the positioning of the counter.
        if ((format.alignment() & Qt::AlignHCenter) == Qt::AlignHCenter) {
            const qreal padding = (line.width() - line.naturalTextWidth() ) / 2;
            qreal newX;
            if (m_isRtl)
                newX = line.x() + line.width() - padding + blockData->counterSpacing();
            else
                newX = line.x() + padding - blockData->counterWidth() - blockData->counterSpacing();
            blockData->setCounterPosition(QPointF(newX, blockData->counterPosition().y()));
        }
        else if (!m_isRtl && x() < line.x()) {// move the counter more left.
            blockData->setCounterPosition(blockData->counterPosition() + QPointF(line.x() - x(), 0));
        } else if (m_isRtl && x() + width() > line.x() + line.width() + 0.1) { // 0.1 to account for qfixed rounding
           const qreal newX = line.x() + line.width() + blockData->counterSpacing();
           blockData->setCounterPosition(QPointF(newX, blockData->counterPosition().y()));
        }
    }

    qreal height = format.doubleProperty(KoParagraphStyle::FixedLineHeight);
    qreal objectAscent = 0.0;
    qreal objectDescent = 0.0;
    bool useFixedLineHeight = height != 0.0;
    if (useFixedLineHeight) {
        // QTextLine has its position at the top of the line. So if the ascent changes between lines in a parag
        // because of different font sizes, for example, we have to adjust the position to make the fixed
        // line height be from baseline to baseline instead of from top-of-line to top-of-line
        QTextLayout *layout = block.layout();
        if (layout->lineCount() > 1) {
            QTextLine prevLine = layout->lineAt(layout->lineCount()-2);
            Q_ASSERT(prevLine.isValid());
            if (qAbs(prevLine.y() + height - line.y()) < 0.15) { // don't adjust when the line is not where we expect it.
                const qreal prevBaseline = prevLine.y() + prevLine.ascent();
                line.setPosition(QPointF(line.x(), prevBaseline + height - line.ascent()));
            }
        }
    } else { // not fixed lineheight
        const bool useFontProperties = format.boolProperty(KoParagraphStyle::LineSpacingFromFont);
        if (useFontProperties) {
            height = line.height();
        } else {
            if (cursor->fragmentIterator.atEnd()) {// no text in parag.

                qreal fontStretch = 1;
                // stretch line height to ms-word size
                if (block.charFormat().hasProperty(KoCharacterStyle::FontStretch)) {
                    fontStretch = block.charFormat().property(KoCharacterStyle::FontStretch).toDouble();
                }
                height = block.charFormat().fontPointSize() * fontStretch;
            } else {

                qreal fontStretch = 1;
                // stretch line height to ms-word size
                if (cursor->fragmentIterator.fragment().charFormat().hasProperty(KoCharacterStyle::FontStretch)) {
                    fontStretch = cursor->fragmentIterator.fragment().charFormat().property(KoCharacterStyle::FontStretch).toDouble();
                }
                // read max font height
                height = qMax(height, cursor->fragmentIterator.fragment().charFormat().fontPointSize() * fontStretch);

                InlineObjectExtend pos = m_documentLayout->inlineObjectExtend(cursor->fragmentIterator.fragment());
                objectAscent = qMax(objectAscent, pos.m_ascent);
                objectDescent = qMax(objectDescent, pos.m_descent);

                while (!(cursor->fragmentIterator.atEnd() || cursor->fragmentIterator.fragment().contains(
                             block.position() + line.textStart() + line.textLength() - 1))) {
                    cursor->fragmentIterator++;
                    if (cursor->fragmentIterator.atEnd()) {
                     break;
                    }
                    if (!m_documentLayout->changeTracker()
                        || !m_documentLayout->changeTracker()->displayChanges()
                        || !m_documentLayout->changeTracker()->containsInlineChanges(cursor->fragmentIterator.fragment().charFormat())
                        || !m_documentLayout->changeTracker()->elementById(cursor->fragmentIterator.fragment().charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt())->isEnabled()
                        || (m_documentLayout->changeTracker()->elementById(cursor->fragmentIterator.fragment().charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt())->getChangeType() != KoGenChange::DeleteChange)
                        || m_documentLayout->changeTracker()->displayChanges()) {
                        qreal fontStretch = 1;
                        // stretch line height to ms-word size
                        if (cursor->fragmentIterator.fragment().charFormat().hasProperty(KoCharacterStyle::FontStretch)) {
                            fontStretch = cursor->fragmentIterator.fragment().charFormat().property(KoCharacterStyle::FontStretch).toDouble();
                        }
                        // read max font height
                        height = qMax(height, cursor->fragmentIterator.fragment().charFormat().fontPointSize() * fontStretch);

                        InlineObjectExtend pos = m_documentLayout->inlineObjectExtend(cursor->fragmentIterator.fragment());
                        objectAscent = qMax(objectAscent, pos.m_ascent);
                        objectDescent = qMax(objectDescent, pos.m_descent);
                    }
                }
            }
            if (height < 0.01) height = 12; // default size for uninitialized styles.
        }
    }

    // add linespacing
    if (! useFixedLineHeight) {
        qreal linespacing = format.doubleProperty(KoParagraphStyle::LineSpacing);
        if (linespacing == 0.0) { // unset
            int percent = format.intProperty(KoParagraphStyle::PercentLineHeight);
            if (percent != 0)
                linespacing = height * ((percent - 100) / 100.0);
            else if (linespacing == 0.0)
                linespacing = height * 0.2; // default
        }
        height = qMax(height, objectAscent) + objectDescent + linespacing;
    }
    qreal minimum = format.doubleProperty(KoParagraphStyle::MinimumLineHeight);
    if (minimum > 0.0)
        height = qMax(height, minimum);
    //rounding problems due to Qt-scribe internally using ints.
    //also used when line was moved down because of intersections with other shapes
    if (qAbs(m_y - line.y()) >= 0.126) {
        m_y = line.y();
    }

    m_rootAreaIsNew = false;

/*FIXME
    // position inline objects
    while (positionInlineObjects()) {
        if (m_textAnchors[m_textAnchorIndex - 1]->anchorStrategy()->isRelayoutNeeded()) {

            if (moveLayoutPosition(m_textAnchors[m_textAnchorIndex - 1]) == true) {
                return 0;
            }
        }
    }
*/
    return height; // line successfully added
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

KoText::Direction KoTextLayoutArea::parentTextDirection() const
{
    Q_ASSERT(parent); //Root areas should overload this method
    return parent->parentTextDirection();
}

void KoTextLayoutArea::setReferenceRect(qreal left, qreal right, qreal top, qreal maximumAllowedBottom)
{
    m_left = left;
    m_right = right;
    m_top = top;
    m_boundingRect.setTop(top);
    m_maximalAllowedBottom = maximumAllowedBottom;
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
    m_boundingRect.setBottom(bottom);
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

void KoTextLayoutArea::preregisterFootNote(KoInlineNote *note)
{
    if (parent == 0) {
        // TODO once we support footnotes at end of document this is
        // where we need to add some code

        // FIXME actually create an footNoteArea and append it
        m_preregisteredFootNotesHeight += 14;
        //m_preregisteredFootNoteAreas.append(footNoteArea);
        return;
    }
    parent->preregisterFootNote(note);
}

void KoTextLayoutArea::confirmFootNotes()
{
    m_footNotesHeight += m_preregisteredFootNotesHeight;
    m_footNoteAreas.append(m_preregisteredFootNoteAreas);
    m_preregisteredFootNotesHeight = 0;
    m_preregisteredFootNoteAreas.clear();
    if (parent) {
        parent->confirmFootNotes();
    }
}

void KoTextLayoutArea::clearPreregisteredFootNotes()
{
    m_preregisteredFootNotesHeight = 0;
    m_preregisteredFootNoteAreas.clear();
    parent->clearPreregisteredFootNotes();
}

void KoTextLayoutArea::paint(QPainter *painter, const KoTextDocumentLayout::PaintContext &context)
{
    painter->setPen(context.textContext.palette.color(QPalette::Text)); // for text that has no color.
    const QRegion clipRegion = painter->clipRegion();
    KoTextBlockBorderData *lastBorder = 0;

    if (m_startOfArea == 0) // We have not been layouted yet
        return;

    
    QTextFrame::iterator it = m_startOfArea->it;
    QTextFrame::iterator stop = m_endOfArea->it;
    if(!stop.currentBlock().isValid() || m_endOfArea->line.isValid()) {
        ++stop;
    }
    int tableAreaIndex = 0;
    for (; it != stop; ++it) {
        QTextBlock block = it.currentBlock();
        QTextTable *table = qobject_cast<QTextTable*>(it.currentFrame());
        QTextFrame *subFrame = it.currentFrame();
        QTextBlockFormat format = block.blockFormat();

        if (table) {
            m_tableAreas[tableAreaIndex]->paint(painter, context);
            ++tableAreaIndex;
            continue;
        } else if (subFrame) {
            // we know of no frames at this point so just skip it
            continue;
        } else {
            if (!block.isValid())
                continue;
        }

        QTextLayout *layout = block.layout();

        if (!painter->hasClipping() || clipRegion.intersects(layout->boundingRect().toRect())) {
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
                    continue; // selections of several table cells are covered by the within drawBorders above.
                }
                if (!m_documentLayout->changeTracker()
                    || m_documentLayout->changeTracker()->displayChanges()
                    || !m_documentLayout->changeTracker()->containsInlineChanges(selection.format)
                    || !m_documentLayout->changeTracker()->elementById(selection.format.property(KoCharacterStyle::ChangeTrackerId).toInt())->isEnabled()
                    || (m_documentLayout->changeTracker()->elementById(selection.format.property(KoCharacterStyle::ChangeTrackerId).toInt())->getChangeType() != KoGenChange::DeleteChange)) {
                    QTextLayout::FormatRange fr;
                    fr.start = begin - block.position();
                    fr.length = end - begin;
                    fr.format = selection.format;
                    selections.append(fr);
                }
            }

            for (QTextBlock::iterator it = block.begin(); !(it.atEnd()); ++it) {
                QTextFragment currentFragment = it.fragment();
                if (currentFragment.isValid()) {
                    QTextCharFormat format = currentFragment.charFormat();
                    int changeId = format.intProperty(KoCharacterStyle::ChangeTrackerId);
                    if (changeId && m_documentLayout->changeTracker() && m_documentLayout->changeTracker()->displayChanges()) {
                        KoChangeTrackerElement *changeElement = m_documentLayout->changeTracker()->elementById(changeId);
                        switch(changeElement->getChangeType()) {
                            case (KoGenChange::InsertChange):
                            format.setBackground(QBrush(m_documentLayout->changeTracker()->getInsertionBgColor()));
                            break;
                            case (KoGenChange::FormatChange):
                            format.setBackground(QBrush(m_documentLayout->changeTracker()->getFormatChangeBgColor()));
                            break;
                            case (KoGenChange::DeleteChange):
                            format.setBackground(QBrush(m_documentLayout->changeTracker()->getDeletionBgColor()));
                            break;
                        }

                        QTextLayout::FormatRange fr;
                        fr.start = currentFragment.position() - block.position();
                        fr.length = currentFragment.length();
                        fr.format = format;
                        selections.prepend(fr);
                    }
                }
            }

            layout->draw(painter, QPointF(0, 0), selections);

            decorateParagraph(painter, block);

            if (lastBorder && lastBorder != border) {
                lastBorder->paint(*painter);
            }
            painter->restore();
            lastBorder = border;
        }
    }
    if (lastBorder)
        lastBorder->paint(*painter);
}

void KoTextLayoutArea::drawListItem(QPainter *painter, const QTextBlock &block, KoImageCollection *imageCollection)
{
    KoTextBlockData *data = dynamic_cast<KoTextBlockData*>(block.userData());
    if (data == 0)
        return;

    QTextList *list = block.textList();
    if (list && data->hasCounterData()) {
        QTextListFormat listFormat = list->format();
        QTextCharFormat chFormatMaxFontSize;

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
            QFont font(chFormatMaxFontSize.font(), m_documentLayout->paintDevice());

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

            QTextLayout layout(result , font, m_documentLayout->paintDevice());

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

        KoListStyle::Style listStyle = static_cast<KoListStyle::Style>(listFormat.style());
        if (listStyle == KoListStyle::ImageItem && imageCollection) {
            QFontMetricsF fm(chFormatMaxFontSize.font(), m_documentLayout->paintDevice());
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

void KoTextLayoutArea::decorateParagraph(QPainter *painter, const QTextBlock &block)
{
    QTextLayout *layout = block.layout();
    QTextOption textOption = layout->textOption();

    QTextBlockFormat bf = block.blockFormat();
    QVariantList tabList = bf.property(KoParagraphStyle::TabPositions).toList();
    QFont oldFont = painter->font();

    QTextBlock::iterator it;
    int startOfBlock = -1;
    int currentTabStop = 0;
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
                        decorateTabs(painter, tabList, line, currentFragment, startOfBlock, currentTabStop);
                    }
                }
            }
        }
    }
    painter->setFont(oldFont);
}

void KoTextLayoutArea::drawStrikeOuts(QPainter *painter, const QTextFragment &currentFragment, const QTextLine &line, qreal x1, qreal x2, const int startOfFragmentInBlock, const int fragmentToLineOffset) const
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
        QFontMetricsF metrics(font, m_documentLayout->paintDevice());

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

void KoTextLayoutArea::drawUnderlines(QPainter *painter, const QTextFragment &currentFragment, const QTextLine &line, qreal x1, qreal x2, const int startOfFragmentInBlock, const int fragmentToLineOffset) const
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
        QFontMetricsF metrics(font, m_documentLayout->paintDevice());

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

// Decorate any tabs ('\t's) in 'currentFragment' and laid out in 'line'.
int KoTextLayoutArea::decorateTabs(QPainter *painter, const QVariantList& tabList, const QTextLine &line, const QTextFragment& currentFragment, int startOfBlock, int currentTabStop)
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
        currentTabStop = 0;
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
        if (currentTabStop >= tabList.size()) // no more decorations
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
                tab = qvariant_cast<KoText::Tab>(tabList[currentTabStop]);
                currentTabStop++;
                // comparing with x1 should work for all of left/right/center/char tabs
            } while (tab.position <= x1 && currentTabStop < tabList.size());
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
    return currentTabStop;
}

void KoTextLayoutArea::updateBorders(KoTextBlockData *blockData, QTextBlock *block)
{
    m_borderInsets = KoInsets(0,0,0,0);
    QTextBlockFormat format = block->blockFormat();
    KoTextBlockBorderData border(QRectF(this->x() - resolveTextIndent(block) - listIndent(blockData,block), m_y + m_borderInsets.top, width() + resolveTextIndent(block), 1));
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

    // check if prev parag had a border.
    QTextBlock prev = block->previous();
    KoTextBlockBorderData *prevBorder = 0;
    if (prev.isValid()) {
        KoTextBlockData *bd = dynamic_cast<KoTextBlockData*>(prev.userData());
        if (bd)
            prevBorder = bd->border();
    }
    if (border.hasBorders()) {
        if (blockData == 0) {
            blockData = new KoTextBlockData();
            block->setUserData(blockData);
        }

        // then check if we can merge with the previous parags border.
        if (prevBorder && prevBorder->equals(border))
            blockData->setBorder(prevBorder);
        else {
            // can't merge; then these are our new borders.
            KoTextBlockBorderData *newBorder = new KoTextBlockBorderData(border);
            blockData->setBorder(newBorder);
            if (prevBorder && !m_rootAreaIsNew)
                m_y += prevBorder->inset(KoTextBlockBorderData::Bottom);
        }
        blockData->border()->applyInsets(m_borderInsets, m_y + m_borderInsets.top, false);
    } else { // this parag has no border.
        if (prevBorder && !m_rootAreaIsNew)
            m_y += prevBorder->inset(KoTextBlockBorderData::Bottom);
        if (blockData)
            blockData->setBorder(0); // remove an old one, if there was one.
    }
    // add padding inside the border
    m_borderInsets.top += format.doubleProperty(KoParagraphStyle::TopPadding);
    m_borderInsets.left += format.doubleProperty(KoParagraphStyle::LeftPadding);
    m_borderInsets.bottom += format.doubleProperty(KoParagraphStyle::BottomPadding);
    m_borderInsets.right += format.doubleProperty(KoParagraphStyle::RightPadding);
}

qreal KoTextLayoutArea::resolveTextIndent(QTextBlock *block)
{
    if ((block->blockFormat().property(KoParagraphStyle::AutoTextIndent).toBool())) {
        // if auto-text-indent is set,
        // return an indent approximately 3-characters wide as per current font
        QTextCursor blockCursor(*block);
        qreal guessGlyphWidth = QFontMetricsF(blockCursor.charFormat().font()).width('x');
        return guessGlyphWidth * 3;
    }
    return block->blockFormat().textIndent();
}

qreal KoTextLayoutArea::listIndent(KoTextBlockData *blockData, QTextBlock *block)
{
    if (blockData == 0)
        return 0;
    qreal indent = 0;
    if (block->textList())
        indent = block->textList()->format().doubleProperty(KoListStyle::Indent);
    if (m_isRtl)
        return indent;
    return blockData->counterSpacing() + blockData->counterWidth() + indent;
}
