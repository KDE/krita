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

#include "ListItemsHelper.h"
#include "RunAroundHelper.h"
#include "KoTextDocumentLayout.h"

#include <KoParagraphStyle.h>
#include <KoCharacterStyle.h>
#include <KoListStyle.h>
#include <KoStyleManager.h>
#include <KoTextBlockData.h>
#include <KoTextBlockBorderData.h>
#include <KoText.h>
#include <KoChangeTracker.h>
#include <KoChangeTrackerElement.h>

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

KoTextLayoutArea::KoTextLayoutArea(KoTextLayoutArea *p)
 : parent(p)
{
}

KoTextLayoutArea::~KoTextLayoutArea()
{
}

void KoTextLayoutArea::layout(HierarchicalCursor *cursor)
{
    while(!(cursor->it.atEnd())) {
        QTextBlock block = cursor->it.currentBlock();
        QTextTable *table = qobject_cast<QTextTable*>(cursor->it.currentFrame());
        QTextFrame *subFrame = cursor->it.currentFrame();

        if (table) {
            // We are currently dealing with a table
            // Let's create KoTextLayoutTableArea and let that handle the table
            // tableArea = ...;
            if (true /*tableArea->layout(cursor->subCursor())*/) {
                ++(cursor->it);
            }
            continue;
        } else if (subFrame) {
            // We are currently dealing with a frame of some sorts besides table
            // Right now we'll just skip it as we know of no such subframes
            ++(cursor->it);
        } else {
            if (block.isValid()) {
                layoutBlock(cursor);
            }
        }
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
//   b) make sure we keep above maximalAllowedY
//   c) calls addLine()
void KoTextLayoutArea::layoutBlock(HierarchicalCursor *cursor)
{
    QTextBlock block = cursor->it.currentBlock();
    KoTextBlockData *blockData = dynamic_cast<KoTextBlockData *>(block.userData());
    QTextBlockFormat format = block.blockFormat();

    int dropCapsNChars;
    int dropCapsAffectsNMoreLines = 0;
    qreal dropCapsAffectedLineWidthAdjust;
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

    // drop caps

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
            dropCapsAffectedLineWidthAdjust = rect.width();
            dropCapsAffectedLineWidthAdjust += fm.rightBearing(lastChar);
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

        dropCapsNChars = dropCapsLength + firstNonSpace;
        dropCapsAffectsNMoreLines = (dropCapsNChars > 0) ? dropCapsLines : 0;
    } else {
        dropCapsNChars = 0;
    }

    layout->setTextOption(option);

    layout->beginLayout();

    cursor->fragmentIterator = block.begin();

    //Now once we know the physical context we can work on the borders of the paragraph
    //FIXME updateBorders(); // fill the border inset member vars.
    //FIXME m_y += m_borderInsets.top;

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

    //FIXME m_x += m_borderInsets.left;
    //FIXME m_width -= m_borderInsets.left + m_borderInsets.right;

    if (dropCapsNChars == 0) {
        m_x += dropCapsAffectedLineWidthAdjust;
        m_width -= dropCapsAffectedLineWidthAdjust;
    }

    if (dropCapsNChars>0)
        m_width =  dropCapsAffectedLineWidthAdjust+10;

    if (cursor->line.isValid()==false) {
        m_x += textIndent(block);
        m_width -= textIndent(block);
    } else
        cursor->line = layout->createLine();

    // So now is the time to create the lines of this paragraph
    RunAroundHelper runAroundHelper;
    runAroundHelper.setLine(this, cursor->line);

    //FIXME refreshCurrentPageOutlines();
    //FIXME runAroundHelper.setOutlines(m_currentLineOutlines);

    qreal maxLineHeight = 0;
    qreal y_justBelowDropCaps = 0;

    while (cursor->line.isValid()) {
        //FIXME runAroundHelper.fit( /* resetHorizontalPosition */ false, QPointF(x, m_y));
        //FIXME While above is commented out we should set width
        cursor->line.setLineWidth(width());
        cursor->line.setPosition(QPointF(x(), m_y));

        qreal bottomOfText = cursor->line.y() + cursor->line.height();
        if (bottomOfText > maximalAllowedY()) {
            // We can not fit line within our allowed space
            if (format.nonBreakableLines()) {
                //set an invalid line so we start this block from beginning next time
                cursor->line = QTextLine();
            }
            return /*FIXME false*/; //to indicate block was not done!
        }

        maxLineHeight = qMax(maxLineHeight, addLine(cursor, blockData));
        //FIXME runAroundHelper.fit( /* resetHorizontalPosition */ true, QPointF(x, m_y));

        if (true /*FIXME !runAroundHelper.stayOnBaseline()*/) {
            m_y += maxLineHeight;
            maxLineHeight = 0;
        }

        // drop caps
        if (dropCapsNChars > 0) { // we just laid out the dropped chars
            y_justBelowDropCaps = m_y; // save the y position just below the dropped characters
            m_y = cursor->line.y();              // keep the same y for the next line
            cursor->line.setPosition(cursor->line.position() - QPointF(0, dropCapsPositionAdjust));
            dropCapsNChars = 0;
        } else if (dropCapsAffectsNMoreLines > 0) { // we just laid out a drop-cap-affected line
            dropCapsAffectsNMoreLines--;
            if (dropCapsAffectsNMoreLines == 0) {   // no more drop-cap-affected lines
                if (m_y < y_justBelowDropCaps)
                    m_y = y_justBelowDropCaps; // make sure m_y is below the dropped characters
                y_justBelowDropCaps = 0;
                dropCapsAffectedLineWidthAdjust = 0;
            }
        }


        // line fitted so try and do the next one
        cursor->line = layout->createLine();
        runAroundHelper.setLine(this, cursor->line);
        //FIXME refreshCurrentPageOutlines();
        //FIXME runAroundHelper.setOutlines(m_currentLineOutlines);
    }
    cursor->line = QTextLine(); //set an invalid line to indicate we are done with block
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
    return m_x;
}

qreal KoTextLayoutArea::width() const
{
    return m_width;
}

qreal KoTextLayoutArea::addLine(HierarchicalCursor *cursor, KoTextBlockData *blockData)
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
            if (cursor->fragmentIterator.atEnd()) // no text in parag.
                height = block.charFormat().fontPointSize();
            else {
                // read max font height
                height = qMax(height, cursor->fragmentIterator.fragment().charFormat().fontPointSize());

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
                        height = qMax(height, cursor->fragmentIterator.fragment().charFormat().fontPointSize());

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

/*
void KoTextLayoutArea::updateBorders(KoTextBlockData *blockData)
{
    Q_ASSERT(m_data);
    m_borderInsets = m_data->shapeMargins();
    KoTextBlockBorderData border(QRectF(this->x() - resolveTextIndent() - listIndent(), m_y + m_borderInsets.top, width() + resolveTextIndent(), 1));
    border.setEdge(border.Left, m_format, KoParagraphStyle::LeftBorderStyle,
                   KoParagraphStyle::LeftBorderWidth, KoParagraphStyle::LeftBorderColor,
                   KoParagraphStyle::LeftBorderSpacing, KoParagraphStyle::LeftInnerBorderWidth);
    border.setEdge(border.Right, m_format, KoParagraphStyle::RightBorderStyle,
                   KoParagraphStyle::RightBorderWidth, KoParagraphStyle::RightBorderColor,
                   KoParagraphStyle::RightBorderSpacing, KoParagraphStyle::RightInnerBorderWidth);
    border.setEdge(border.Top, m_format, KoParagraphStyle::TopBorderStyle,
                   KoParagraphStyle::TopBorderWidth, KoParagraphStyle::TopBorderColor,
                   KoParagraphStyle::TopBorderSpacing, KoParagraphStyle::TopInnerBorderWidth);
    border.setEdge(border.Bottom, m_format, KoParagraphStyle::BottomBorderStyle,
                   KoParagraphStyle::BottomBorderWidth, KoParagraphStyle::BottomBorderColor,
                   KoParagraphStyle::BottomBorderSpacing, KoParagraphStyle::BottomInnerBorderWidth);

    // check if prev parag had a border.
    QTextBlock prev = m_block.previous();
    KoTextBlockBorderData *prevBorder = 0;
    if (prev.isValid()) {
        KoTextBlockData *bd = dynamic_cast<KoTextBlockData*>(prev.userData());
        if (bd)
            prevBorder = bd->border();
    }
    if (border.hasBorders()) {
        if (blockData == 0) {
            blockData = new KoTextBlockData();
            m_block.setUserData(blockData);
        }

        // then check if we can merge with the previous parags border.
        if (prevBorder && prevBorder->equals(border))
            blockData->setBorder(prevBorder);
        else {
            // can't merge; then these are our new borders.
            KoTextBlockBorderData *newBorder = new KoTextBlockBorderData(border);
            blockData->setBorder(newBorder);
            if (prevBorder && !m_newShape)
                m_y += prevBorder->inset(KoTextBlockBorderData::Bottom);
        }
        blockData->border()->applyInsets(m_borderInsets, m_y + m_borderInsets.top, false);
    } else { // this parag has no border.
        if (prevBorder && !m_newShape)
            m_y += prevBorder->inset(KoTextBlockBorderData::Bottom);
        if (blockData)
            blockData->setBorder(0); // remove an old one, if there was one.
    }

    // add padding inside the border
    m_borderInsets.top += m_format.doubleProperty(KoParagraphStyle::TopPadding);
    m_borderInsets.left += m_format.doubleProperty(KoParagraphStyle::LeftPadding);
    m_borderInsets.bottom += m_format.doubleProperty(KoParagraphStyle::BottomPadding);
    m_borderInsets.right += m_format.doubleProperty(KoParagraphStyle::RightPadding);
}
*/
qreal KoTextLayoutArea::maximalAllowedY() const
{
    Q_ASSERT(parent); //Root areas (only to have parent==0) should overload this method
    return parent->maximalAllowedY();
}

KoText::Direction KoTextLayoutArea::parentTextDirection() const
{
    return KoText::LeftRightTopBottom;
}

qreal KoTextLayoutArea::left() const
{
    Q_ASSERT(parent); //Root areas should overload this method
    return parent->left();
}

qreal KoTextLayoutArea::right() const
{
    Q_ASSERT(parent); //Root areas should overload this method
    return parent->right();
}
