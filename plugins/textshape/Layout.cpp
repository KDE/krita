/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2008 Roopesh Chander <roop@forwardbias.in>
 * Copyright (C) 2009-2010 KO GmbH <cbo@kogmbh.com>
 * Copyright (C) 2009-2010 Casper Boemann <cbo@boemann.dk>
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
        : m_styleManager(0),
        m_changeTracker(0),
        m_blockData(0),
        m_data(0),
        m_reset(true),
        m_isRtl(false),
        m_inTable(false),
        m_parent(parent),
        m_textShape(0),
        m_demoText(false),
        m_endOfDemoText(false),
        m_defaultTabSizing(0),
        m_currentTabStop(0),
        m_dropCapsNChars(0),
        m_dropCapsAffectsNMoreLines(0),
        m_dropCapsAffectedLineWidthAdjust(0),
        m_y_justBelowDropCaps(0),
        m_dropCapsPositionAdjust(0),
        m_restartingAfterTableBreak(false),
        m_restartingFirstCellAfterTableBreak(false),
        m_allTimeMinimumLeft(0),
        m_maxLineHeight(0)
{
    m_frameStack.reserve(5); // avoid reallocs
    setTabSpacing(MM_TO_POINT(15));
}

bool Layout::start()
{
    m_styleManager = KoTextDocument(m_parent->document()).styleManager();
    m_changeTracker = KoTextDocument(m_parent->document()).changeTracker();
    if (m_reset)
        resetPrivate();
    else if (shape)
        nextParag();
    m_reset = false;
    return !(layout == 0 || m_parent->shapes().count() <= shapeNumber);
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

qreal Layout::width()
{
    Q_ASSERT(shape);
    if (m_dropCapsNChars>0)
        return m_dropCapsAffectedLineWidthAdjust+10;
    qreal ptWidth = m_inTable ? m_tableLayout.cellContentRect(m_tableCell).width() : shape->size().width();
    if (m_newParag)
        ptWidth -= resolveTextIndent();
    if (m_blockData)
        ptWidth -= listIndent();
    if (m_blockData && m_isRtl)
        ptWidth -= m_blockData->counterWidth() + m_blockData->counterSpacing();
    ptWidth -= m_format.leftMargin() + m_format.rightMargin();
    ptWidth -= m_borderInsets.left + m_borderInsets.right + m_shapeBorder.right;
    if (m_dropCapsNChars == 0)
        ptWidth -= m_dropCapsAffectedLineWidthAdjust;
    return ptWidth;
}

qreal Layout::x()
{
    qreal result = m_newParag ? resolveTextIndent() : 0.0;
    result += m_isRtl ? m_format.rightMargin() : (m_format.leftMargin() + listIndent());
    result += m_borderInsets.left + m_shapeBorder.left;
//    if (m_block.layout()->lineCount() > 1)
    if (m_dropCapsNChars == 0)
        result += m_dropCapsAffectedLineWidthAdjust;
    m_allTimeMinimumLeft = qMin(m_allTimeMinimumLeft, result);
    if (m_inTable) {
        result += m_tableLayout.cellContentRect(m_tableCell).x();
    }
    return result;
}

qreal Layout::y()
{
    return m_y;
}

qreal Layout::resolveTextIndent()
{
    if ((m_block.blockFormat().property(KoParagraphStyle::AutoTextIndent).toBool())) {
        // if auto-text-indent is set,
        // return an indent approximately 3-characters wide as per current font
        QTextCursor blockCursor(m_block);
        qreal guessGlyphWidth = QFontMetricsF(blockCursor.charFormat().font()).width('x');
        return guessGlyphWidth * 3;
    }
    return m_block.blockFormat().textIndent();
}

qreal Layout::docOffsetInShape() const
{
    Q_ASSERT(m_data);
    return m_data->documentOffset();
}

QRectF Layout::expandVisibleRect(const QRectF &rect) const
{
    return rect.adjusted(m_allTimeMinimumLeft, 0.0, 50.0, 0.0);
}

bool Layout::addLine(QTextLine &line, bool processingLine)
{
    if (m_blockData && m_block.textList() && m_block.layout()->lineCount() == 1) {
        // first line, lets check where the line ended up and adjust the positioning of the counter.
        QTextBlockFormat fmt = m_block.blockFormat();
        if ((fmt.alignment() & Qt::AlignHCenter) == Qt::AlignHCenter) {
            const qreal padding = (line.width() - line.naturalTextWidth() ) / 2;
            qreal newX;
            if (m_isRtl)
                newX = line.x() + line.width() - padding + m_blockData->counterSpacing();
            else
                newX = line.x() + padding - m_blockData->counterWidth() - m_blockData->counterSpacing();
            m_blockData->setCounterPosition(QPointF(newX, m_blockData->counterPosition().y()));
        }
        else if (!m_isRtl && x() < line.x()) {// move the counter more left.
            m_blockData->setCounterPosition(m_blockData->counterPosition() + QPointF(line.x() - x(), 0));
        } else if (m_isRtl && x() + width() > line.x() + line.width() + 0.1) { // 0.1 to account for qfixed rounding
           const qreal newX = line.x() + line.width() + m_blockData->counterSpacing();
           m_blockData->setCounterPosition(QPointF(newX, m_blockData->counterPosition().y()));
        }
    }

    qreal height = m_format.doubleProperty(KoParagraphStyle::FixedLineHeight);
    qreal objectHeight = 0.0;
    if (line.textLength() == 1 && m_block.text().at(line.textStart()) == QChar::ObjectReplacementCharacter && line.descent() == 0.0 && line.ascent() == 0.0) {
        // This is an anchor but not an inline anchor so set to some very small hight
        height = 0.1;
    }
    bool useFixedLineHeight = height != 0.0;
    if (useFixedLineHeight) {
        // QTextLine has its position at the top of the line. So if the ascent changes between lines in a parag
        // because of different font sizes, for example, we have to adjust the position to make the fixed
        // line height be from baseline to baseline instead of from top-of-line to top-of-line
        QTextLayout *layout = m_block.layout();
        if (layout->lineCount() > 1) {
            QTextLine prevLine = layout->lineAt(layout->lineCount()-2);
            Q_ASSERT(prevLine.isValid());
            if (qAbs(prevLine.y() + height - line.y()) < 0.15) { // don't adjust when the line is not where we expect it.
                const qreal prevBaseline = prevLine.y() + prevLine.ascent();
                line.setPosition(QPointF(line.x(), prevBaseline + height - line.ascent()));
            }
        }
    } else { // not fixed lineheight
        const bool useFontProperties = m_format.boolProperty(KoParagraphStyle::LineSpacingFromFont);
        if (useFontProperties) {
            height = line.height();
        } else {
            if (m_fragmentIterator.atEnd()) // no text in parag.
                height = m_block.charFormat().fontPointSize();
            else {
                // read max font height
                height = qMax(height, m_fragmentIterator.fragment().charFormat().fontPointSize());
                objectHeight = qMax(objectHeight, inlineCharHeight(m_fragmentIterator.fragment()));
                while (!(m_fragmentIterator.atEnd() || m_fragmentIterator.fragment().contains(
                             m_block.position() + line.textStart() + line.textLength() - 1))) {
                    m_fragmentIterator++;
                    if (!m_changeTracker
                        || !m_changeTracker->displayChanges()
                        || !m_changeTracker->containsInlineChanges(m_fragmentIterator.fragment().charFormat())
                        || !m_changeTracker->elementById(m_fragmentIterator.fragment().charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt())->isEnabled()
                        || (m_changeTracker->elementById(m_fragmentIterator.fragment().charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt())->getChangeType() != KoGenChange::DeleteChange)
                        || m_changeTracker->displayChanges()) {
                        height = qMax(height, m_fragmentIterator.fragment().charFormat().fontPointSize());
                        objectHeight = qMax(objectHeight, inlineCharHeight(m_fragmentIterator.fragment()));
                    }
                }
            }
            if (height < 0.01) height = 12; // default size for uninitialized styles.
        }
    }

    int oldFootnoteDocLength = -1;
    const qreal footnoteHeight = findFootnote(line, &oldFootnoteDocLength);

    if (!m_newShape
            // In case the text shape automatically resizes itself to fit all contents, every line will fit,
            // so just lay them out (i.e. do not return true)
            && m_parent->resizeMethod() == KoTextDocument::NoResize
            // line does not fit.
            && m_data->documentOffset() + shape->size().height() - footnoteHeight
              < line.y() + line.height() + m_shapeBorder.bottom
            // but make sure we don't leave the shape empty.
            && m_block.position() + line.textStart() > m_data->position()) {
        if (oldFootnoteDocLength >= 0) {
            QTextCursor cursor(m_textShape->footnoteDocument());
            cursor.setPosition(oldFootnoteDocLength);
            cursor.setPosition(m_textShape->footnoteDocument()->characterCount()-1, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
        }

        m_data->setEndPosition(m_block.position() + line.textStart() - 1);

        bool entireParagraphMoved = false;
        if (! m_newParag && m_format.nonBreakableLines()) { // then revert layouting of parag
            // TODO check height of parag so far; and if it does not fit in the rest of the shapes, just continue.
            m_data->setEndPosition(m_block.position() - 1);
            m_block.layout()->endLayout();
            m_block.layout()->beginLayout();
            line = m_block.layout()->createLine(); //TODO should probably not create this line
            entireParagraphMoved = true;
        } else {
            // unfortunately we can't undo a single line, so we undo entire paragraph
            // and redo all previous lines
            QList<LineKeeper> lineKeeps;
            m_block.layout()->endLayout();
            for(int i = 0; i < m_block.layout()->lineCount()-1; i++) {
                QTextLine l = m_block.layout()->lineAt(i);
                LineKeeper lk;
                lk.lineWidth = l.width();
                lk.columns = l.textLength();
                lk.position = l.position();
                lineKeeps.append(lk);
            }
            m_block.layout()->clearLayout();
            m_block.layout()->beginLayout();
            foreach(const LineKeeper &lk, lineKeeps) {
                line = m_block.layout()->createLine();
                line.setNumColumns(lk.columns, lk.lineWidth);
                line.setPosition(lk.position);
            }
            if(lineKeeps.isEmpty()) {
                entireParagraphMoved = true;
            }
        }
        if (m_data->endPosition() == -1) // no text at all fit in the shape!
            m_data->setEndPosition(m_data->position());
        m_data->wipe();
        if (m_textShape) // kword uses a dummy shape which is not a text shape
            m_textShape->markLayoutDone();
        nextShape();
        if (m_data)
            m_data->setPosition(m_block.position() + (entireParagraphMoved ? 0 : line.textStart()));

        // the demo-text feature means we have exactly the same amount of text as we have frame-space
        if (m_demoText)
            m_endOfDemoText = false;
        return true;
    }

    // add linespacing
    if (! useFixedLineHeight) {
        qreal linespacing = m_format.doubleProperty(KoParagraphStyle::LineSpacing);
        if (linespacing == 0.0) { // unset
            int percent = m_format.intProperty(KoParagraphStyle::PercentLineHeight);
            if (percent != 0)
                linespacing = height * ((percent - 100) / 100.0);
            else if (linespacing == 0.0)
                linespacing = height * 0.2; // default
        }
        height = qMax(height, objectHeight) + linespacing;
    }
    qreal minimum = m_format.doubleProperty(KoParagraphStyle::MinimumLineHeight);
    if (minimum > 0.0)
        height = qMax(height, minimum);
    //rounding problems due to Qt-scribe internally using ints.
    //also used when line was moved down because of intersections with other shapes
    if (qAbs(m_y - line.y()) >= 0.126) {
        m_y = line.y();
    }
    m_maxLineHeight = qMax(m_maxLineHeight, height);
    if (! processingLine) {
        m_y += m_maxLineHeight;
        m_maxLineHeight = 0;
    }
    m_newShape = false;
    m_newParag = false;

    // drop caps
    if (m_dropCapsNChars > 0) { // we just laid out the dropped chars
        m_y_justBelowDropCaps = m_y; // save the y position just below the dropped characters
        m_y = line.y();              // keep the same y for the next line
        line.setPosition(line.position() - QPointF(0, m_dropCapsPositionAdjust));
        m_dropCapsNChars = 0;
    } else if (m_dropCapsAffectsNMoreLines > 0) { // we just laid out a drop-cap-affected line
        m_dropCapsAffectsNMoreLines--;
        if (m_dropCapsAffectsNMoreLines == 0) {   // no more drop-cap-affected lines
            if (m_y < m_y_justBelowDropCaps)
                m_y = m_y_justBelowDropCaps; // make sure m_y is below the dropped characters
            m_y_justBelowDropCaps = 0;
            m_dropCapsAffectedLineWidthAdjust = 0;
        }
    }

    // call update to repaint the new line
    QRectF repaintRect = line.rect();
    if (layout->lineCount() > 1) { // cover big linespacing.
        QTextLine prevLine = layout->lineAt(layout->lineCount()-2);
        const qreal bottom = prevLine.y() + prevLine.height();
        if (repaintRect.top() > bottom) {
            repaintRect.setTop(bottom); //expand to cover space between lines.
        }
    } else { // cover space between paragraphs
        QTextBlock prev = m_block.previous();
        if (prev.isValid() && prev.layout()->lineCount() > 0) {
            QTextLine prevLine = prev.layout()->lineAt(layout->lineCount()-1);
            const qreal bottom = prevLine.y() + prevLine.height();
            if (repaintRect.top() > bottom) {
                repaintRect.setTop(bottom); //expand to cover space between paragraphs.
            }
        }
    }
    repaintRect.moveTop(repaintRect.y() - docOffsetInShape());
    repaintRect.setX(0.0); // just take full width since we can't force a repaint of
    repaintRect.setWidth(shape->size().width()); // where lines were before layout.
    shape->update(repaintRect);

    return false;
}

bool Layout::nextParag()
{
    Q_ASSERT(shape);
    m_inlineObjectHeights.clear();
    if (layout && !m_restartingFirstCellAfterTableBreak) { // guard against first time or first time after table relayout
        layout->endLayout();
        m_block = m_block.next();
        updateFrameStack();
        if (m_endOfDemoText) {
            layout = 0;
            m_blockData = 0;
            return false;
        }
        qreal borderBottom = m_y;
        if (m_block.isValid() && !m_newShape) { // only add bottom of prev parag if we did not go to a new shape for this parag.
            if (m_format.pageBreakPolicy() & QTextFormat::PageBreak_AlwaysAfter) {
                m_data->setEndPosition(m_block.position() - 1);
                m_data->wipe();
                nextShape();
                if (m_data)
                    m_data->setPosition(m_block.position());
            }
            m_y += m_borderInsets.bottom;
            borderBottom = m_y; // don't include the bottom margin!
            m_y += m_format.bottomMargin();
        }

        if (m_blockData) {
            if (m_blockData->border())
                m_blockData->border()->setParagraphBottom(borderBottom);
        }
    }

    layout = 0;
    m_blockData = 0;
    if (m_data == 0) // no shape to layout, so stop here.
        return true;
    if (! m_block.isValid()) {
        QTextBlock block = m_block.previous(); // last correct one.
        m_data->setEndPosition(block.position() + block.length());
        if (m_data->position() > m_data->endPosition()) // we have no text for this shape
            m_data->setEndPosition(-1);

        // repaint till end of shape.
        const qreal offsetInShape = m_y - m_data->documentOffset();
        shape->update(QRectF(0.0, offsetInShape, shape->size().width(), shape->size().height() - offsetInShape));
        // cleanup and repaint rest of shapes.
        m_data->wipe();
        if (m_textShape) { // may be null in relation to setFollowupShape
            m_textShape->markLayoutDone();
        }
        cleanupShapes();
        return false;
    }

    // make sure m_tableLayout is refering to the right table
    QTextCursor tableFinder(m_block);
    QTextTable *table = tableFinder.currentTable();
    if (table) {
        // Set the current table on the table layout.
        m_tableLayout.setTable(table);
        // Save the current table cell.
        m_tableCell = table->cellAt(m_block.position());
        Q_ASSERT(m_tableCell.isValid());
    }
    m_format = m_block.blockFormat();
    m_blockData = dynamic_cast<KoTextBlockData*>(m_block.userData());
    KoText::Direction dir = static_cast<KoText::Direction>(m_format.intProperty(KoParagraphStyle::TextProgressionDirection));
    if (dir == KoText::InheritDirection)
        dir = m_data->pageDirection();
    if (dir == KoText::AutoDirection)
        m_isRtl = m_block.text().isRightToLeft();
    else
        m_isRtl =  dir == KoText::RightLeftTopBottom || dir == KoText::PerhapsRightLeftTopBottom;

    // initialize list item stuff for this parag.
    QTextList *textList = m_block.textList();
    if (textList) {
        QTextListFormat format = textList->format();
        int styleId = format.intProperty(KoListStyle::CharacterStyleId);
        KoCharacterStyle *charStyle = 0;
        if (styleId > 0 && m_styleManager)
            charStyle = m_styleManager->characterStyle(styleId);
        if (!charStyle && m_styleManager) { // try the one from paragraph style
            KoParagraphStyle *ps = m_styleManager->paragraphStyle(
                                       m_format.intProperty(KoParagraphStyle::StyleId));
            if (ps)
                charStyle = ps->characterStyle();
        }

        if (!(m_blockData && m_blockData->hasCounterData())) {
            QFont font;
            if (charStyle)
                font = QFont(charStyle->fontFamily(), qRound(charStyle->fontPointSize()),
                             charStyle->fontWeight(), charStyle->fontItalic());
            else {
                QTextCursor cursor(m_block);
                font = cursor.charFormat().font();
            }
            ListItemsHelper lih(textList, font);
            lih.recalculate();
            m_blockData = dynamic_cast<KoTextBlockData*>(m_block.userData());
        }
    } else if (m_blockData) { // make sure it is empty
        m_blockData->setCounterText(QString());
        m_blockData->setCounterSpacing(0.0);
        m_blockData->setCounterWidth(0.0);
    }

    bool pagebreak = m_format.pageBreakPolicy() & QTextFormat::PageBreak_AlwaysBefore;

    const QVariant masterPageName = m_format.property(KoParagraphStyle::MasterPageName);
    if (! masterPageName.isNull() && m_currentMasterPage != masterPageName.toString()) {
        m_currentMasterPage = masterPageName.toString();
        pagebreak = true; // new master-page means new page
    }

    // start a new shape if requested, but not if that would leave the current shape empty.
    if (!m_newShape && pagebreak && m_block.position() > m_data->position()) {
        m_data->setEndPosition(m_block.position() - 1);
        nextShape();
        if (m_data)
            m_data->setPosition(m_block.position());

        // in case there are no more shapes to layout on we should stop now - and prevent subsequent crashes
        if (shape==0) {
            return true;
        }
    }

    m_y += topMargin();
    layout = m_block.layout();
    QTextOption option = layout->textOption();
    option.setWrapMode(m_parent->resizeMethod() == KoTextDocument::NoResize ? QTextOption::WrapAtWordBoundaryOrAnywhere : QTextOption::NoWrap);
    qreal tabStopDistance =  m_format.property(KoParagraphStyle::TabStopDistance).toDouble();
    if (tabStopDistance > 0)
        option.setTabStop(tabStopDistance * qt_defaultDpiY() / 72.);
    else
        option.setTabStop(m_defaultTabSizing);

    // tabs
    QList<QTextOption::Tab> tabs;
    QVariant variant = m_format.property(KoParagraphStyle::TabPositions);
    if (!variant.isNull()) {
        const qreal tabOffset = x();
        foreach(const QVariant &tv, qvariant_cast<QList<QVariant> >(variant)) {
            KoText::Tab koTab = tv.value<KoText::Tab>();
            QTextOption::Tab tab;

            // conversion here is required because Qt thinks in device units and we don't
            tab.position = (koTab.position - tabOffset) * qt_defaultDpiY() / 72.;
            tab.type = koTab.type;
            tab.delimiter = koTab.delimiter;
            tabs.append(tab);
        }
    }
    option.setTabs(tabs);

    option.setAlignment(QStyle::visualAlignment(m_isRtl ? Qt::RightToLeft : Qt::LeftToRight, m_format.alignment()));
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

    int dropCaps = m_format.boolProperty(KoParagraphStyle::DropCaps);
    int dropCapsLength = m_format.intProperty(KoParagraphStyle::DropCapsLength);
    int dropCapsLines = m_format.intProperty(KoParagraphStyle::DropCapsLines);
    if (dropCaps && dropCapsLength != 0 && dropCapsLines > 1
            && m_dropCapsAffectsNMoreLines == 0 // first line of this para is not affected by a previous drop-cap
            && m_block.length() > 1) {
        QString blockText = m_block.text();
        // ok, now we can drop caps for this block
        int firstNonSpace = blockText.indexOf(QRegExp("[^ ]")); // since QTextLine::setNumColumns()
        // skips blankspaces, we will too
        if (dropCapsLength < 0) // means whole word is to be dropped
            dropCapsLength = blockText.indexOf(QRegExp("\\W"), firstNonSpace);
        // increase the size of the dropped chars
        QTextCursor blockStart(m_block);
        QTextLayout::FormatRange dropCapsFormatRange;
        dropCapsFormatRange.format = blockStart.charFormat();

        // find out lineHeight for this block.
        QTextBlock::iterator it = m_block.begin();
        QTextFragment lineRepresentative = it.fragment();
        qreal lineHeight = m_format.doubleProperty(KoParagraphStyle::FixedLineHeight);
        qreal dropCapsHeight = 0;
        if (lineHeight == 0) {
            lineHeight = lineRepresentative.charFormat().fontPointSize();
            qreal linespacing = m_format.doubleProperty(KoParagraphStyle::LineSpacing);
            if (linespacing == 0) { // unset
                int percent = m_format.intProperty(KoParagraphStyle::PercentLineHeight);
                if (percent != 0)
                    linespacing = lineHeight * ((percent - 100) / 100.0);
                else if (linespacing == 0)
                    linespacing = lineHeight * 0.2; // default
            }
            dropCapsHeight = linespacing * (dropCapsLines-1);
        }
        const qreal minimum = m_format.doubleProperty(KoParagraphStyle::MinimumLineHeight);
        if (minimum > 0.0) {
            lineHeight = qMax(lineHeight, minimum);
        }

        dropCapsHeight += lineHeight * dropCapsLines;

        int dropCapsStyleId = m_format.intProperty(KoParagraphStyle::DropCapsTextStyle);
        KoCharacterStyle *dropCapsCharStyle = 0;
        if (dropCapsStyleId > 0 && m_styleManager) {
            dropCapsCharStyle = m_styleManager->characterStyle(dropCapsStyleId);
            dropCapsCharStyle->applyStyle(dropCapsFormatRange.format);
        }
        QFont f(dropCapsFormatRange.format.font(), m_parent->paintDevice());
        QString dropCapsText(m_block.text().left(dropCapsLength));
        f.setPointSizeF(dropCapsHeight);
        QChar lastChar = dropCapsText.at(dropCapsText.length()-1);
        for (int i=0; i < 5; ++i) {
            QFontMetricsF fm(f);
            QRectF rect = fm.tightBoundingRect(dropCapsText);
            m_dropCapsAffectedLineWidthAdjust = rect.width();
            m_dropCapsAffectedLineWidthAdjust += fm.rightBearing(lastChar);
            const qreal diff = dropCapsHeight - rect.height();
            m_dropCapsPositionAdjust = rect.top() + fm.ascent();
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
        // bookkeep
        m_dropCapsNChars = dropCapsLength + firstNonSpace;
        m_dropCapsAffectsNMoreLines = (m_dropCapsNChars > 0) ? dropCapsLines : 0;
    } else {
        m_dropCapsNChars = 0;
    }

    layout->setTextOption(option);

    layout->beginLayout();
    m_fragmentIterator = m_block.begin();
    m_newParag = true;

    handleTable();

    //Now once we know the physical context we can work on the borders of the paragraph
    updateBorders(); // fill the border inset member vars.
    m_y += m_borderInsets.top;

    if (textList) {
        // if list set list-indent. Do this after borders init to we can account for them.
        // Also after we account for indents etc so the y() pos is correct.
        if (m_isRtl)
            m_blockData->setCounterPosition(QPointF(shape->size().width() - m_borderInsets.right -
                        m_blockData->counterWidth() - m_shapeBorder.right - m_format.leftMargin(), y()));
        else
            m_blockData->setCounterPosition(QPointF(x() - m_blockData->counterSpacing() - m_blockData->counterWidth(), y()));
    }

    return true;
}

qreal Layout::documentOffsetInShape()
{
    return m_data->documentOffset();
}

void Layout::handleTable()
{
    // Check if we are inside a table.
    QTextCursor tableFinder(m_block);
    QTextTable *table = tableFinder.currentTable();
    if (table) {
        // previousCell is the cell that the previous blocks are in. It can be
        // the same as the current cell, or it can be different, or it can be
        // invalid (if the previous cell is not in a table at all).
        QTextTableCell previousCell = table->cellAt(m_block.previous().position());

        if (!previousCell.isValid()) {
            // The previous cell is invalid, which means we have entered a table.

            // Position the table. Use position of previous block if it's empty.
            QTextBlock prevBlock = m_block.previous();
            QPointF pos = prevBlock.length() == 1 ? prevBlock.layout()->lineAt(0).position() : QPointF(x(), y());

            // Start the first rect of the table.
            //kDebug(32500) << "initial layout about to start at " << pos;
            m_tableLayout.startNewTableRect(pos, shape->size().width(), 0);
            m_restartingAfterTableBreak = false; // make sure
            m_restartingFirstCellAfterTableBreak = false;
        }

        //kDebug(32500) << "working on cell row" << m_tableCell.row() << "col" << m_tableCell.column() << m_y;
        if (m_tableCell != previousCell) {
            // The current cell is not the same as the one the previous block
            // was in. This means the layout processed stepped into
            // a cell.
            //kDebug(32500) << "into cell row" << m_tableCell.row() << "col" << m_tableCell.column();
            if (previousCell.isValid()) {
                // The previous cell was valid, which means we just left a cell,
                // so tell the table layout to calculate its height.
                // however don't do it if the cell has already been treated once
                if (!m_restartingFirstCellAfterTableBreak) {
                    m_tableLayout.setCellContentHeight(previousCell, m_y);
                }

                if (m_tableCell.row() != previousCell.row()) {
                    // The row of the current and previous cell is different,
                    // which means that not only did we leave a cell; we also
                    // left a row.

                    if (m_restartingFirstCellAfterTableBreak) {
                        // In a previous run we have detected that current row should not be on the shape it was placed.
                        // A new shape was ordered and the Y have already been roughly set.
                        // Now is the time to set the Y correctly, but we shouldn't do any detecting of breaks
                        //kDebug(32500) << "[re-layout run] break after row" << previousCell.row() << "and next row at y " << m_y;
                        m_tableLayout.startNewTableRect(QPointF(0.0, y()), shape->size().width(), m_tableCell.row());
                        m_restartingFirstCellAfterTableBreak= false;
                    } else {
                        m_tableLayout.layoutRow(previousCell.row());
                        shape->update(m_tableLayout.rowBoundingRect(previousCell.row()));
                        //kDebug(32500) << "(in table)layouted row" << previousCell.row() << "and next row at y" << m_y;

                        handleTableBreak(previousCell, table);
                    }
                }
            }

            if (!m_restartingFirstCellAfterTableBreak) {
                // No break was scheduled so lets just get on with the layouting
                // Since we have just stepped into this cell lets
                // adjust the Y position of the layout to the Y
                // position of the cell content rectangle.
                m_y = m_tableLayout.cellContentY(m_tableCell);
            }
        }
        m_inTable = true; // We are inside a table.
    }
    else {
        // We are not inside a table, but we have to check if we just left one.
        QTextCursor lookBehind(m_block.previous());
        QTextTable *previousTable = lookBehind.currentTable();

        if (previousTable) {
            m_tableLayout.setTable(previousTable);
            /*
             * We just left a table, so make sure the table layout updates
             * the height of the last cell in it, and reset the table state.
             */
            QTextTableCell previousCell = previousTable->cellAt(m_block.previous().position());
            QTextTableFormat previousFormat = previousTable->format();
            if (previousCell.isValid()) {
                // Tell the table layout to calculate height of last cell.
                m_tableLayout.setCellContentHeight(previousCell, m_y);
                m_tableLayout.layoutRow(previousCell.row());
                //kDebug(32500) << "(after table)layouted row" << previousCell.row() <<"and next row at y" << m_y;
            }

            handleTableBreak(previousCell, previousTable);

            if (!m_restartingFirstCellAfterTableBreak) {
                // No break was needed before the final row so finish of the table layout
                // Position the layout process after the table.
                m_y = m_tableLayout.yAfterTable();

                m_inTable = false; // Reset table state.
                m_tableCell = QTextTableCell(); // Set the current cell to an invalid one.
            }
        }
        m_inTable = false;
    }
}

void Layout::handleTableBreak(QTextTableCell &previousCell, QTextTable *table)
{
    // Get the column and row style manager.
    QTextTableFormat tableFormat = table->format();
    KoTableColumnAndRowStyleManager *carsManager =
    reinterpret_cast<KoTableColumnAndRowStyleManager *>(
            tableFormat.property(KoTableStyle::ColumnAndRowStyleManager).value<void *>());

    if (!carsManager)
        carsManager = new KoTableColumnAndRowStyleManager();

    //kDebug(32500) << "[in handle TableBreak]" << m_restartingAfterTableBreak;

    // Implementation note about break handling:
    // There are a break and 3 rows in play:  some row, [break], previous row, current row
    if (m_restartingAfterTableBreak) {
        // In a previous run we have detected that previous row should not be on the shape it was placed.
        // A new shape was ordered and the Y have already been set.
        // Furthermore, the text in that row have been layouted again and we are now in current row again
        // There is not much for us to do except make sure we don't detect the break again
        m_restartingAfterTableBreak = false;
    } else {
        // Figure out if we should break before previous row
        // This could happen if the previous row was too high
        // TODO

        // It could also be that the previous row had a breakBefore property
        KoTableRowStyle rowStyle = carsManager->rowStyle(previousCell.row());
        if (rowStyle.breakBefore()) {
            m_restartingAfterTableBreak = true;
        }

        // It could even be that the row before the previous row had a breakAfter property
        // We handle that here too (detecting late) because then we do all breaking in one place (simpler code)
        int row= previousCell.row()-1;
        if (row >= 0) {
            rowStyle = carsManager->rowStyle(row);
            if (rowStyle.breakAfter()) {
                m_restartingAfterTableBreak = true;
            }
        }
        m_restartingFirstCellAfterTableBreak = m_restartingAfterTableBreak;
    }

    // at this point m_restartingAfterTableBreak is an indication if we should order a new shape
    if (m_restartingAfterTableBreak) {
        //kDebug(32500) << "[row " << previousCell.row() << "should be moved to new shape and re layouted]";
        layout->endLayout();
        //Find the first block in the previous row
        QTextCursor cur = previousCell.firstCursorPosition();
        cur = table->rowStart(cur);
        m_block = cur.block().next();
        updateFrameStack();
        //kDebug(32500) << "pos" << m_data->position() << " and block position" << m_block.position() << "and shape" << shape;
        if (!m_newShape && m_block.position() > m_data->position()) {
            m_data->setEndPosition(m_block.position() - 1);
            nextShape();
            if (m_data) {
                m_data->setPosition(m_block.position());
            }
            //kDebug(32500) << "  requested new shape at " << y();
        } else {
            // We already have the correct shape due to earlier layout setting data position and endPosition
            m_y= m_data->documentOffset();
        }
        layout = m_block.layout();
        //since the height of m_block will unwantedly be added to m_y after we leave, lets just subtract it to counter that
        m_y -= layout->lineAt(layout->lineCount() - 1).y() + layout->lineAt(layout->lineCount() - 1).height() - layout->lineAt(0).y();

        m_format = m_block.blockFormat();
        m_blockData = dynamic_cast<KoTextBlockData*>(m_block.userData());
        m_isRtl = m_block.text().isRightToLeft();
        m_fragmentIterator = m_block.begin();
        layout->beginLayout();
    }
}

void Layout::nextShape()
{
    m_newShape = true;

    if (m_data) {
        Q_ASSERT(m_data->endPosition() >= m_data->position());
        m_y = m_data->documentOffset() + shape->size().height() + 10.0;
        m_data->wipe();
    }

    shape = 0;
    m_data = 0;
    m_textShape = 0;

    QList<KoShape *> shapes = m_parent->shapes();
    for (shapeNumber++; shapeNumber < shapes.count(); shapeNumber++) {
        shape = shapes[shapeNumber];
        m_data = qobject_cast<KoTextShapeData*>(shape->userData());
        if (m_data != 0)
            break;
        shape = 0;
        m_data = 0;
    }

    if (shape == 0)
        return;
    m_data->setDocumentOffset(m_y);
    m_data->foul(); // make dirty since this one needs relayout at this point.
    m_textShape = dynamic_cast<TextShape*>(shape);
    Q_ASSERT(m_textShape);
    m_textShape->setDemoText(m_demoText);
    if (m_textShape->hasFootnoteDocument()) {
        QTextCursor cursor(m_textShape->footnoteDocument());
        cursor.select(QTextCursor::Document);
        cursor.removeSelectedText();
        Q_ASSERT(!m_textShape->hasFootnoteDocument());
    }
    m_shapeBorder = shape->borderInsets();
    m_y += m_shapeBorder.top;
}

// and the end of text, make sure the rest of the frames have something sane to show.
void Layout::cleanupShapes()
{
    int i = shapeNumber + 1;
    QList<KoShape *> shapes = m_parent->shapes();
    while (i < shapes.count())
        cleanupShape(shapes[i++]);
}

void Layout::cleanupShape(KoShape *daShape)
{
    TextShape *ts = dynamic_cast<TextShape*>(daShape);
    if (ts) {
        ts->markLayoutDone();
        ts->setDemoText(m_demoText);
    }
    KoTextShapeData *textData = qobject_cast<KoTextShapeData*>(daShape->userData());
    if (textData == 0)
        return;
    if (textData->position() == -1)
        return;
    textData->setPosition(-1);
    textData->setDocumentOffset(m_y + 10);
    textData->wipe();
    daShape->update();
}

qreal Layout::listIndent()
{
    if (m_blockData == 0)
        return 0;
    qreal indent = 0;
    if (m_block.textList())
        indent = m_block.textList()->format().doubleProperty(KoListStyle::Indent);
    if (m_isRtl)
        return indent;
    return m_blockData->counterSpacing() + m_blockData->counterWidth() + indent;
}

void Layout::resetPrivate()
{
    m_demoText = false;
    m_endOfDemoText = false;
    m_y = 0;
    m_data = 0;
    shape = 0;
    layout = 0;
    m_newShape = true;
    m_textShape = 0;
    m_blockData = 0;
    m_newParag = true;
    m_block = m_parent->document()->begin();
    updateFrameStack();
    m_currentMasterPage.clear();
    m_dropCapsPositionAdjust = 0;
    m_dropCapsAffectedLineWidthAdjust = 0;

    shapeNumber = 0;
    int lastPos = -1;
    QList<KoShape *> shapes = m_parent->shapes();
    foreach(KoShape *shape, shapes) {
        KoTextShapeData *data = qobject_cast<KoTextShapeData*>(shape->userData());
        Q_ASSERT(data);
        if (data->isDirty()) {
            // this shape needs to be recalculated.
            data->setPosition(lastPos + 1);
            m_block = m_parent->document()->findBlock(lastPos + 1);
            updateFrameStack();
            m_format = m_block.blockFormat();
            if (data->documentOffset() > 0)
                m_y = data->documentOffset();
            else
                data->setDocumentOffset(m_y);

            if (shapeNumber == 0) {
                // no matter what the previous data says, just start from zero.
                m_y = 0;
                data->setDocumentOffset(0);
                Q_ASSERT(lastPos == -1);
                break;
            }
            if (m_block.layout() && m_block.layout()->lineCount() > 0) {
                // block has been layouted. So use its offset.
                m_y = m_block.layout()->lineAt(0).position().y();
                if (m_y < data->documentOffset() - 0.126) { // 0.126 to account of rounding in Qt-scribe
                    // the last layed-out parag
                    Q_ASSERT(shapeNumber > 0);
                    // since we only recalc whole parags; we need to go back a little.
                    shapeNumber--;
                    shape = shapes[shapeNumber];
                    data = qobject_cast<KoTextShapeData*>(shape->userData());
                    m_newShape = false;
                }
                if (m_y > data->documentOffset() + shape->size().height()) {
                    // hang on; this line is explicitly placed outside the shape. Shape is empty!
                    m_y = data->documentOffset();
                    break;
                }
                // in case this parag has a border we have to subtract that as well
                m_blockData = dynamic_cast<KoTextBlockData*>(m_block.userData());
                if (m_blockData && m_blockData->border()) {
                    qreal top = m_blockData->border()->inset(KoTextBlockBorderData::Top);
                    // but only when this border actually makes us have an indent.
                    if (qAbs(m_blockData->border()->rect().top() + top - m_y) < 1E-10)
                        m_y -= top;
                }
                // subtract the top margins as well.
                m_y -= topMargin();
            }
            break;
        }
        m_y = data->documentOffset() + shape->size().height() + 10;
        lastPos = data->endPosition();
        shapeNumber++;
    }
    Q_ASSERT(shapeNumber >= 0);
    if (shapes.count() == 0 || shapes.count() <= shapeNumber)
        return;
    Q_ASSERT(shapeNumber < shapes.count());
    shape = shapes[shapeNumber];
    m_data = qobject_cast<KoTextShapeData*>(shape->userData());
    m_textShape = dynamic_cast<TextShape*>(shape);
    Q_ASSERT(m_textShape);
    if (m_textShape->hasFootnoteDocument() && m_y == m_data->documentOffset()) {
        QTextCursor cursor(m_textShape->footnoteDocument());
        cursor.select(QTextCursor::Document);
        cursor.removeSelectedText();
    }
    m_demoText = m_textShape->demoText();
    m_shapeBorder = shape->borderInsets();
    if (m_y == 0)
        m_y = m_shapeBorder.top;

    if (! nextParag())
        shapeNumber++;
}

void Layout::updateBorders()
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
        if (m_blockData == 0) {
            m_blockData = new KoTextBlockData();
            m_block.setUserData(m_blockData);
        }

        // then check if we can merge with the previous parags border.
        if (prevBorder && prevBorder->equals(border))
            m_blockData->setBorder(prevBorder);
        else {
            // can't merge; then these are our new borders.
            KoTextBlockBorderData *newBorder = new KoTextBlockBorderData(border);
            m_blockData->setBorder(newBorder);
            if (prevBorder && !m_newShape)
                m_y += prevBorder->inset(KoTextBlockBorderData::Bottom);
        }
        m_blockData->border()->applyInsets(m_borderInsets, m_y + m_borderInsets.top, false);
    } else { // this parag has no border.
        if (prevBorder && !m_newShape)
            m_y += prevBorder->inset(KoTextBlockBorderData::Bottom);
        if (m_blockData)
            m_blockData->setBorder(0); // remove an old one, if there was one.
    }

    // add padding inside the border
    m_borderInsets.top += m_format.doubleProperty(KoParagraphStyle::TopPadding);
    m_borderInsets.left += m_format.doubleProperty(KoParagraphStyle::LeftPadding);
    m_borderInsets.bottom += m_format.doubleProperty(KoParagraphStyle::BottomPadding);
    m_borderInsets.right += m_format.doubleProperty(KoParagraphStyle::RightPadding);
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

        if (table) {
            m_tableLayout.setTable(table);
            m_tableLayout.drawBackground(painter);
            drawFrame(table, painter, context, inTable+1); // this actually only draws the text inside
            QPainterPath accuBlankBorders;
            m_tableLayout.drawBorders(painter, &accuBlankBorders);
            painter->strokePath(accuBlankBorders, QPen(QColor(0,0,0,96)));
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
            if (bg != Qt::NoBrush)
                painter->fillRect(layout->boundingRect(), bg);
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
                    qreal x1 = line.cursorToX(currentFragment.position() - startOfBlock);
                    qreal x2 = line.cursorToX(currentFragment.position() + currentFragment.length() - startOfBlock);
                    x2 = qMin(line.naturalTextWidth() + line.cursorToX(line.textStart()), x2);

                    // sometimes a fragment starts in the middle of a line, so calc offset
                    int fragmentToLineOffset = qMax(currentFragment.position() - startOfBlock - line.textStart(),0);

                    drawStrikeOuts(painter, currentFragment, line, x1, x2, startOfFragmentInBlock, fragmentToLineOffset);
                    drawUnderlines(painter, currentFragment, line, x1, x2, startOfFragmentInBlock, fragmentToLineOffset);
                    decorateTabs(painter, tabList, line, currentFragment, startOfBlock);
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
        QTextCharFormat cf;
        bool filled = false;
        if (m_styleManager) {
            const int id = listFormat.intProperty(KoListStyle::CharacterStyleId);
            KoCharacterStyle *cs = m_styleManager->characterStyle(id);
            if (!cs) {
                KoParagraphStyle *ps = m_styleManager->paragraphStyle(
                                       block.blockFormat().intProperty(KoParagraphStyle::StyleId));
                if (ps)
                    cs = ps->characterStyle();
            }
            if (cs) {
                cs->applyStyle(cf);
                filled = true;
            }
        }
        if (!filled) {
            // use first char of first block.
            QTextBlock firstBlockOfList = list->item(0);
            QTextCursor cursor(firstBlockOfList);
            cf = cursor.charFormat();
        }

        if (! data->counterText().isEmpty()) {
            QFont font(cf.font(), m_parent->paintDevice());
            QTextLayout layout(data->counterText(), font, m_parent->paintDevice());
            layout.setCacheEnabled(true);
            QList<QTextLayout::FormatRange> layouts;
            QTextLayout::FormatRange format;
            format.start = 0;
            format.length = data->counterText().length();
            format.format = cf;
            layouts.append(format);
            layout.setAdditionalFormats(layouts);

            Qt::Alignment align = static_cast<Qt::Alignment>(listFormat.intProperty(KoListStyle::Alignment));
            if (align == 0)
                align = Qt::AlignLeft;
            else if (align != Qt::AlignLeft)
                align |= Qt::AlignAbsolute;
            QTextOption option(align);
            option.setTextDirection(block.layout()->textOption().textDirection());
            if (option.textDirection() == Qt::RightToLeft || data->counterText().isRightToLeft())
                option.setAlignment(Qt::AlignRight);
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
        if (listStyle == KoListStyle::SquareItem || listStyle == KoListStyle::DiscItem ||
                listStyle == KoListStyle::CircleItem || listStyle == KoListStyle::BoxItem ||
                listStyle == KoListStyle::RhombusItem || listStyle == KoListStyle::CustomCharItem ||
                listStyle == KoListStyle::HeavyCheckMarkItem || listStyle == KoListStyle::BallotXItem ||
                listStyle == KoListStyle::RightArrowItem || listStyle == KoListStyle::RightArrowHeadItem
           ) {
            QFontMetricsF fm(cf.font(), m_parent->paintDevice());
#if 0
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

            qreal width = fm.xHeight();
            qreal y = data->counterPosition().y() + fm.ascent() - fm.xHeight(); // at top of text.
            int percent = listFormat.intProperty(KoListStyle::BulletSize);
            if (percent > 0)
                width *= percent / 100.0;
            y -= width / 10.; // move it up just slightly
            qreal x = qMax(qreal(1), data->counterPosition().x() + fm.width(listFormat.stringProperty(KoListStyle::ListItemPrefix)));
            switch (listStyle) {
            case KoListStyle::SquareItem: {
                painter->fillRect(QRectF(x, y, width, width), QBrush(Qt::black));
            }
            break;
            case KoListStyle::DiscItem:
                painter->setBrush(QBrush(Qt::black));
                // fall through!
            case KoListStyle::CircleItem: {
                painter->drawEllipse(QRectF(x, y, width, width));
            }
            break;
            case KoListStyle::BoxItem: {
                painter->drawRect(QRectF(x, y, width, width));
            }
            break;
            case KoListStyle::RhombusItem: {
                painter->translate(QPointF(x + (width / 2.0), y));
                painter->rotate(45.0);
                painter->fillRect(QRectF(0, 0, width, width), QBrush(Qt::black));
            }
            break;
            case KoListStyle::RightArrowItem: {
                const qreal half = width / 2.0;
                painter->translate(QPointF(x, y));
                QPointF points[] = { QPointF(half, 0), QPointF(width, half), QPointF(half, width) };
                painter->drawPolyline(points, 3);
                painter->drawLine(QLineF(0, half, width, half));
            }
            break;
            case KoListStyle::RightArrowHeadItem: {
                painter->translate(QPointF(x, y));
                QPointF points[] = { QPointF(0, 0), QPointF(width, width / 2.0), QPointF(0, width) };
                painter->drawPolyline(points, 3);
            }
            break;
            case KoListStyle::HeavyCheckMarkItem: {
                const qreal half = width / 2.0;
                painter->translate(QPointF(x, y));
                QPointF points[] = { QPointF(half, half), QPointF(half, width), QPointF(width, 0) };
                painter->drawPolyline(points, 3);
            }
            break;
            case KoListStyle::BallotXItem: {
                painter->translate(QPointF(x, y));
                painter->drawLine(QLineF(0.0, 0.0, width, width));
                painter->drawLine(QLineF(0.0, width, width, 0.0));
            }
            break;
            case KoListStyle::CustomCharItem:
                if (!QChar(listFormat.intProperty(KoListStyle::BulletCharacter)).isNull()){
                    painter->drawText(0, 0, QChar(listFormat.intProperty(KoListStyle::BulletCharacter)));
                }
                break;
            default:; // others we ignore.
            }
        } else if (listStyle == KoListStyle::ImageItem && imageCollection) {
            QFontMetricsF fm(cf.font(), m_parent->paintDevice());
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

bool Layout::setFollowupShape(KoShape *followupShape)
{
    if (m_demoText)
        return false;
    Q_ASSERT(shape == 0);
    Q_ASSERT(followupShape);

    m_data = qobject_cast<KoTextShapeData*>(followupShape->userData());
    if (m_data == 0)
        return false;

    m_newShape = false;
    shape = followupShape;
    m_textShape = 0;
    m_data->setDocumentOffset(m_y);
    m_shapeBorder = shape->borderInsets();
    return true;
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

bool Layout::previousParag()
{
    if (m_block.position() == 0 && layout->lineCount() == 0)
        return false;

    layout->endLayout();
    if (layout->lineCount() == 0) {
        m_block = m_block.previous();
        layout = m_block.layout();
        updateFrameStack();
    }
    QTextLine tl = layout->lineAt(0);
    Q_ASSERT(tl.isValid());
    m_y = tl.y();

    m_format = m_block.blockFormat();
    m_blockData = dynamic_cast<KoTextBlockData*>(m_block.userData());
    m_isRtl = m_block.text().isRightToLeft();

    m_fragmentIterator = m_block.begin();
    m_newParag = true;

    if (m_data->position() > m_block.position()) { // go back a shape.
        QList<KoShape *> shapes = m_parent->shapes();
        for (--shapeNumber; shapeNumber >= 0; shapeNumber--) {
            shape = shapes[shapeNumber];
            m_textShape = dynamic_cast<TextShape*>(shape);
            m_data = qobject_cast<KoTextShapeData*>(shape->userData());
            if (m_data != 0)
                break;
        }
        Q_ASSERT(m_data); // should never happen since the first shape is always a proper shape.
        if (m_data == 0)
            return false;

        m_shapeBorder = shape->borderInsets();
    }
    m_newShape = m_block.position() == m_data->position();
    updateBorders(); // fill the border inset member vars.
    layout->beginLayout();
    return true;
}

void Layout::registerInlineObject(const QTextInlineObject &inlineObject)
{
    m_inlineObjectHeights.insert(m_block.position() + inlineObject.textPosition(), inlineObject.height());
}

qreal Layout::inlineCharHeight(const QTextFragment &fragment)
{
    if (m_inlineObjectHeights.contains(fragment.position()))
        return m_inlineObjectHeights[fragment.position()];
    return 0.0;
}

qreal Layout::findFootnote(const QTextLine &line, int *oldLength)
{
    if (m_parent->inlineTextObjectManager() == 0 || m_textShape == 0)
        return 0;
    Q_ASSERT(oldLength);
    QString text = m_block.text();
    int pos = text.indexOf(QChar(0xFFFC), line.textStart());
    bool firstFootnote = true;
    while (pos >= 0 && pos <= line.textStart() + line.textLength()) {
        QTextCursor c1(m_block);
        c1.setPosition(m_block.position() + pos);
        pos = text.indexOf(QChar(0xFFFC), pos + 1);
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
            bool found = false;
            QList<QWeakPointer<ToCGenerator> >::Iterator iter = m_tocGenerators.begin();
            while (iter != m_tocGenerators.end()) {
                QWeakPointer<ToCGenerator> item = *iter;
                if (item.isNull()) {
                    iter = m_tocGenerators.erase(iter);
                    continue;
                }
                if (item.data()->tocFrame() == frame) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                ToCGenerator *tg = new ToCGenerator(frame);
                m_tocGenerators.append(QWeakPointer<ToCGenerator>(tg));
                // connect to FinishedLayout
                QObject::connect(m_parent, SIGNAL(finishedLayout()),
                        tg, SLOT(documentLayoutFinished()));
            }
        }
    }
}

void Layout::setTabSpacing(qreal spacing)
{
    m_defaultTabSizing = spacing * qt_defaultDpiY() / 72.;
}
