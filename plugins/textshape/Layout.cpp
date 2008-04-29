/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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
#include "ListItemsHelper.h"
#include "TextShape.h"

#include <KoTextDocumentLayout.h>
#include <KoTextShapeData.h>
#include <KoParagraphStyle.h>
#include <KoCharacterStyle.h>
#include <KoListStyle.h>
#include <KoStyleManager.h>
#include <KoTextBlockData.h>
#include <KoTextBlockBorderData.h>
#include <KoInlineNote.h>
#include <KoInlineTextObjectManager.h>
#include <KoShape.h>
#include <KoUnit.h>

#include <kdeversion.h>
#include <KDebug>
#include <QTextList>
#include <QStyle>

// ---------------- layout helper ----------------
Layout::Layout(KoTextDocumentLayout *parent)
   : m_styleManager(0),
    m_blockData(0),
    m_data(0),
    m_reset(true),
    m_isRtl(false),
    m_demoText(false),
    m_endOfDemoText(false),
    m_defaultTabSizing(MM_TO_POINT(15))
{
    m_parent = parent;
    layout = 0;
}

bool Layout::start() {
    if(m_reset)
        resetPrivate();
    else if(shape)
        nextParag();
    m_reset = false;
    return !(layout == 0 || m_parent->shapes().count() <= shapeNumber);
}

void Layout::end() {
    if(layout)
        layout->endLayout();
    layout = 0;
}

void Layout::reset() {
    m_reset = true;
}

bool Layout::interrupted() {
    return m_reset;
}

double Layout::width() {
    Q_ASSERT(shape);
    double ptWidth = shape->size().width() - m_format.leftMargin() - m_format.rightMargin();
    if(m_newParag)
        ptWidth -= m_format.textIndent();
    if(m_newParag && m_blockData)
        ptWidth -= m_blockData->counterWidth() + m_blockData->counterSpacing();
    ptWidth -= m_borderInsets.left + m_borderInsets.right + m_shapeBorder.right;
    return ptWidth;
}

double Layout::x() {
    double result = m_newParag?m_format.textIndent():0.0;
    result += m_isRtl ? m_format.rightMargin() : m_format.leftMargin();
    result += listIndent();
    result += m_borderInsets.left + m_shapeBorder.left;
    return result;
}

double Layout::y() {
    return m_y;
}

double Layout::docOffsetInShape() const {
    Q_ASSERT(m_data);
    return m_data->documentOffset();
}

bool Layout::addLine(QTextLine &line) {
    double height = m_format.doubleProperty(KoParagraphStyle::FixedLineHeight);
    double objectHeight = 0.0;
    bool useFixedLineHeight = height != 0.0;
    if(! useFixedLineHeight) {
        const bool useFontProperties = m_format.boolProperty(KoParagraphStyle::LineSpacingFromFont);
        if(useFontProperties)
            height = line.height();
        else {
            if(m_fragmentIterator.atEnd()) // no text in parag.
                height = m_block.charFormat().fontPointSize();
            else {
                // read max font height
                height = qMax(height, m_fragmentIterator.fragment().charFormat().fontPointSize());
                objectHeight= qMax(objectHeight, inlineCharHeight(m_fragmentIterator.fragment()));
                while(! (m_fragmentIterator.atEnd() || m_fragmentIterator.fragment().contains(
                               m_block.position() + line.textStart() + line.textLength() -1))) {
                    m_fragmentIterator++;
                    height = qMax(height, m_fragmentIterator.fragment().charFormat().fontPointSize());
                    objectHeight= qMax(objectHeight, inlineCharHeight(m_fragmentIterator.fragment()));
                }
            }
            if(height < 0.01) height = 12; // default size for uninitialized styles.
        }
    }

    const double footnoteHeight = findFootnote(line);

    if(m_data->documentOffset() + shape->size().height() - footnoteHeight < m_y + line.height() + m_shapeBorder.bottom) {
        // line does not fit.
        m_data->setEndPosition(m_block.position() + line.textStart()-1);

        bool ignoreLine = true;
        if(! m_newParag && m_format.nonBreakableLines()) { // then revert layouting of parag
            // TODO check height of parag so far; and if it does not fit in the rest of the shapes, just continue.
            m_data->setEndPosition(m_block.position() -1);
            m_block.layout()->endLayout();
            m_block.layout()->beginLayout();
            line = m_block.layout()->createLine();
            ignoreLine = true;
        }
        if(m_data->endPosition() == -1) // no text at all fit in the shape!
            m_data->setEndPosition( m_data->position() );
        m_data->wipe();
        m_textShape->markLayoutDone();
        nextShape();
        if(m_data)
            m_data->setPosition(m_block.position() + ignoreLine?0:line.textStart());

        // the demo-text feature means we have exactly the same amount of text as we have frame-space
        if(m_demoText)
            m_endOfDemoText = false;
        return ignoreLine;
    }

    // add linespacing
    if(! useFixedLineHeight) {
        double linespacing = m_format.doubleProperty(KoParagraphStyle::LineSpacing);;
        if(linespacing == 0.0) { // unset
            int percent = m_format.intProperty(KoParagraphStyle::PercentLineHeight);
            if(percent != 0)
                linespacing = height * ((percent - 100) / 100.0);
            else if(linespacing == 0.0)
                linespacing = height * 0.2; // default
        }
        height = qMax(height, objectHeight) + linespacing;
    }

    double minimum = m_format.doubleProperty(KoParagraphStyle::MinimumLineHeight);
    if(minimum > 0.0)
        height = qMax(height, minimum);
    if(qAbs(m_y - line.y()) < 0.126) // rounding problems due to Qt-scribe internally using ints.
        m_y += height;
    else
        m_y = line.y() + height; // The line got a pos <> from y(), follow that lead.
    m_newShape = false;
    m_newParag = false;

    return false;
}

extern int qt_defaultDpiX();

bool Layout::nextParag() {
    m_inlineObjectHeights.clear();
    if(layout) { // guard against first time
        layout->endLayout();
        m_block = m_block.next();
        if(m_endOfDemoText) {
            layout = 0;
            m_blockData = 0;
            return false;
        }
        double borderBottom = m_y;
        if(m_block.isValid() && !m_newShape) { // only add bottom of prev parag if we did not go to a new shape for this parag.
            if(m_format.pageBreakPolicy() == QTextFormat::PageBreak_AlwaysAfter ||
                    m_format.boolProperty(KoParagraphStyle::BreakAfter)) {
                m_data->setEndPosition(m_block.position()-1);
                m_data->wipe();
                nextShape();
                if(m_data)
                    m_data->setPosition(m_block.position());
            }
            m_y += m_borderInsets.bottom;
            borderBottom = m_y; // don't inlude the bottom margin!
            m_y += m_format.bottomMargin();
        }

        if(m_blockData) {
            if(m_blockData->border())
                m_blockData->border()->setParagraphBottom(borderBottom);
        }
    }
    layout = 0;
    m_blockData = 0;
    if(m_data == 0) // no shape to layout, so stop here.
        return true;
    if(! m_block.isValid()) {
        QTextBlock block = m_block.previous(); // last correct one.
        m_data->setEndPosition(block.position() + block.length());

        // repaint till end of shape.
        const double offsetInShape = m_y - m_data->documentOffset();
        shape->update(QRectF(0.0, offsetInShape, shape->size().width(), shape->size().width() - offsetInShape));
        // cleanup and repaint rest of shapes.
        m_textShape->markLayoutDone();
        cleanupShapes();
        return false;
    }
    m_format = m_block.blockFormat();
    m_blockData = dynamic_cast<KoTextBlockData*> (m_block.userData());
    KoText::Direction dir = static_cast<KoText::Direction> (m_format.intProperty(KoParagraphStyle::TextProgressionDirection));
    if(dir == KoText::AutoDirection)
        m_isRtl = m_block.text().isRightToLeft();
    else
        m_isRtl =  dir == KoText::RightLeftTopBottom || dir == KoText::PerhapsRightLeftTopBottom;

    // initialize list item stuff for this parag.
    QTextList *textList = m_block.textList();
    if(textList) {
        QTextListFormat format = textList->format();
        int styleId = format.intProperty(KoListStyle::CharacterStyleId);
        KoCharacterStyle *charStyle = 0;
        if(styleId > 0 && m_styleManager)
            charStyle = m_styleManager->characterStyle(styleId);
        if(!charStyle && m_styleManager) { // try the one from paragraph style
            KoParagraphStyle *ps = m_styleManager->paragraphStyle(
                    m_format.intProperty(KoParagraphStyle::StyleId));
            if(ps)
                charStyle = ps->characterStyle();
        }

        if(! (m_blockData && m_blockData->hasCounterData())) {
            QFont font;
            if(charStyle)
                font = QFont(charStyle->fontFamily(), qRound(charStyle->fontPointSize()),
                        charStyle->fontWeight(), charStyle->fontItalic());
            else {
                QTextCursor cursor(m_block);
                font = cursor.charFormat().font();
            }
            ListItemsHelper lih(textList, font);
            lih.recalculate();
            m_blockData = dynamic_cast<KoTextBlockData*> (m_block.userData());
        }
    }
    else if(m_blockData) { // make sure it is empty
        m_blockData->setCounterText(QString());
        m_blockData->setCounterSpacing(0.0);
        m_blockData->setCounterWidth(0.0);
    }

    updateBorders(); // fill the border inset member vars.
    m_y += m_borderInsets.top;

    if(!m_newShape && (m_format.pageBreakPolicy() == QTextFormat::PageBreak_AlwaysBefore ||
            m_format.boolProperty(KoParagraphStyle::BreakBefore))) {
        m_data->setEndPosition(m_block.position()-1);
        nextShape();
        if(m_data)
            m_data->setPosition(m_block.position());
    }
    m_y += topMargin();
    layout = m_block.layout();
    QTextOption option = layout->textOption();
    option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere );
    option.setTabStop (m_defaultTabSizing);

#if QT_VERSION >= KDE_MAKE_VERSION(4,4,0)
    // tabs
    QList<QTextOption::Tab> tabs;
    QList<KoText::Tab> koTabs;
    QVariant variant = m_format.property(KoParagraphStyle::TabPositions);
    if(! variant.isNull())
        foreach(QVariant tv, qvariant_cast<QList<QVariant> >(variant)) {
            KoText::Tab koTab = tv.value<KoText::Tab>();
            QTextOption::Tab tab;
            tab.position = koTab.position * qt_defaultDpiX() / 72.; // convertion here is required because Qt thinks in device units and we don't
            tab.type = koTab.type;
            tab.delimiter = koTab.delimiter;
            tabs.append(tab);
        }
    option.setTabs(tabs);
#endif

    option.setAlignment( QStyle::visualAlignment(m_isRtl ? Qt::RightToLeft : Qt::LeftToRight, m_format.alignment()) );
    if(m_isRtl)
        option.setTextDirection(Qt::RightToLeft);
    else
        option.setTextDirection(Qt::LeftToRight);
    layout->setTextOption(option);

    layout->beginLayout();
    m_fragmentIterator = m_block.begin();
    m_newParag = true;

    if(textList) {
        // if list set list-indent. Do this after borders init to we can account for them.
        // Also after we account for indents etc so the y() pos is correct.
        if(m_isRtl)
            m_blockData->setCounterPosition(QPointF(shape->size().width() - m_borderInsets.right -
                m_shapeBorder.right - m_format.leftMargin() - m_blockData->counterWidth(), y()));
        else
            m_blockData->setCounterPosition(QPointF(m_borderInsets.left + m_shapeBorder.left +
                        m_format.textIndent() + m_format.leftMargin() , y()));
    }

    return true;
}

double Layout::documentOffsetInShape() {
    return m_data->documentOffset();
}

void Layout::nextShape() {
    m_newShape = true;

    if(m_data) {
        Q_ASSERT(m_data->endPosition() >= m_data->position());
        m_y = m_data->documentOffset() + shape->size().height() + 10.0;
        m_data->wipe();
    }

    shape = 0;
    m_data = 0;

    QList<KoShape *> shapes = m_parent->shapes();
    for(shapeNumber++; shapeNumber < shapes.count(); shapeNumber++) {
        shape = shapes[shapeNumber];
        m_data = dynamic_cast<KoTextShapeData*> (shape->userData());
        if(m_data != 0)
            break;
        shape = 0;
        m_data = 0;
    }

    if(shape == 0)
        return;
    m_data->setDocumentOffset(m_y);
    m_data->foul(); // make dirty since this one needs relayout at this point.
    m_textShape = dynamic_cast<TextShape*>(shape);
    Q_ASSERT(m_textShape);
    if(m_textShape->hasFootnoteDocument())
        m_textShape->footnoteDocument()->clear();
    m_shapeBorder = shape->borderInsets();
    m_y += m_shapeBorder.top;
}

// and the end of text, make sure the rest of the frames have something sane to show.
void Layout::cleanupShapes() {
    int i = shapeNumber + 1;
    QList<KoShape *> shapes = m_parent->shapes();
    while(i < shapes.count())
        cleanupShape(shapes[i++]);
}

void Layout::cleanupShape(KoShape *daShape) {
    TextShape *ts = dynamic_cast<TextShape*>(daShape);
    if(ts)
        ts->markLayoutDone();
    KoTextShapeData *textData = dynamic_cast<KoTextShapeData*> (daShape->userData());
    if(textData == 0)
        return;
    if(textData->position() == -1)
        return;
    textData->setPosition(-1);
    textData->setDocumentOffset(m_y + 10);
    textData->wipe();
    daShape->update();
}

double Layout::listIndent() {
    if(m_blockData == 0)
        return 0;
    if(m_isRtl)
        return 0;
    return m_blockData->counterWidth();
}

void Layout::resetPrivate() {
    m_demoText = false;
    m_endOfDemoText = false;
    m_y = 0;
    m_data = 0;
    shape =0;
    layout = 0;
    m_newShape = true;
    m_blockData = 0;
    m_newParag = true;
    m_block = m_parent->document()->begin();

    shapeNumber = 0;
    int lastPos = -1;
    QList<KoShape *> shapes = m_parent->shapes();
    foreach(KoShape *shape, shapes) {
        KoTextShapeData *data = dynamic_cast<KoTextShapeData*> (shape->userData());
        Q_ASSERT(data);
        if(data->isDirty()) {
            // this shape needs to be recalculated.
            data->setPosition(lastPos+1);
            m_block = m_parent->document()->findBlock( lastPos+1 );
            m_format = m_block.blockFormat();
            if(data->documentOffset() > 0)
                m_y = data->documentOffset();
            else
                data->setDocumentOffset(m_y);

            if(shapeNumber == 0) {
                // no matter what the previous data says, just start from zero.
                m_y = 0;
                data->setDocumentOffset(0);
                Q_ASSERT(lastPos == -1);
                break;
            }
            if(m_block.layout() && m_block.layout()->lineCount() > 0) {
                // block has been layouted. So use its offset.
                m_y = m_block.layout()->lineAt(0).position().y();
                if(m_y < data->documentOffset() - 0.126) { // 0.126 to account of rounding in Qt-scribe
                    Q_ASSERT(shapeNumber > 0);
                    // since we only recalc whole parags; we need to go back a little.
                    shapeNumber--;
                    shape = shapes[shapeNumber];
                    data = dynamic_cast<KoTextShapeData*> (shape->userData());
                    m_newShape = false;
                }
                if(m_y > data->documentOffset() + shape->size().height()) {
                    // hang on; this line is explicitly placed outside the shape. Shape is empty!
                    m_y = data->documentOffset();
                    break;
                }
                // in case this parag has a border we have to subtract that as well
                m_blockData = dynamic_cast<KoTextBlockData*> (m_block.userData());
                if(m_blockData && m_blockData->border()) {
                    double top = m_blockData->border()->inset(KoTextBlockBorderData::Top);
                    // but only when this border actually makes us have an indent.
                    if(qAbs(m_blockData->border()->rect().top() + top - m_y) < 1E-10)
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
    if(shapes.count() == 0 || shapes.count() <= shapeNumber)
        return;
    Q_ASSERT(shapeNumber < shapes.count());
    shape = shapes[shapeNumber];

    m_textShape = dynamic_cast<TextShape*>(shape);
    Q_ASSERT(m_textShape);
    if(m_textShape->hasFootnoteDocument())
        m_textShape->footnoteDocument()->clear();
    m_demoText = m_textShape->demoText();
    m_data = dynamic_cast<KoTextShapeData*> (shape->userData());
    m_shapeBorder = shape->borderInsets();
    if(m_y == 0)
        m_y = m_shapeBorder.top;

   if(! nextParag())
       shapeNumber++;
}

void Layout::updateBorders() {
    Q_ASSERT(m_data);
    m_borderInsets = m_data->shapeMargins();
    KoTextBlockBorderData border(QRectF(this->x() - listIndent(), m_y + m_borderInsets.top + topMargin(), width(), 1.));
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
    if(prev.isValid()) {
        KoTextBlockData *bd = dynamic_cast<KoTextBlockData*> (prev.userData());
        if(bd)
            prevBorder = bd->border();
    }
    if(border.hasBorders()) {
        if(m_blockData == 0) {
            m_blockData = new KoTextBlockData();
            m_block.setUserData(m_blockData);
        }

        // then check if we can merge with the previous parags border.
        if(prevBorder && prevBorder->equals(border))
            m_blockData->setBorder(prevBorder);
        else {
            // can't merge; then these are our new borders.
            KoTextBlockBorderData *newBorder = new KoTextBlockBorderData(border);
            m_blockData->setBorder(newBorder);
            if(prevBorder && !m_newShape)
                m_y += prevBorder->inset(KoTextBlockBorderData::Bottom);
        }
        m_blockData->border()->applyInsets(m_borderInsets, m_y + m_borderInsets.top, false);
    }
    else { // this parag has no border.
        if(prevBorder && !m_newShape)
            m_y += prevBorder->inset(KoTextBlockBorderData::Bottom);
        if(m_blockData)
            m_blockData->setBorder(0); // remove an old one, if there was one.
    }

    // add padding inside the border
    m_borderInsets.top += m_format.doubleProperty(KoParagraphStyle::TopPadding);
    m_borderInsets.left += m_format.doubleProperty(KoParagraphStyle::LeftPadding);
    m_borderInsets.bottom += m_format.doubleProperty(KoParagraphStyle::BottomPadding);
    m_borderInsets.right += m_format.doubleProperty(KoParagraphStyle::RightPadding);
}

double Layout::topMargin() {
    bool allowMargin = true; // wheather to allow margins at top of shape
    if(m_newShape) {
        allowMargin = false; // false by default, but check 2 exceptions.
        if(m_format.boolProperty(KoParagraphStyle::BreakBefore))
            allowMargin = true;
        else if( m_styleManager && m_format.topMargin() > 0) {
            // also allow it when the paragraph has the margin, but the style has a different one.
            KoParagraphStyle *ps = m_styleManager->paragraphStyle(
                    m_format.intProperty(KoParagraphStyle::StyleId));
            if(ps == 0 || ps->topMargin() != m_format.topMargin())
                allowMargin = true;
        }
    }
    if(allowMargin)
        return m_format.topMargin();
    return 0.0;
}

void Layout::draw(QPainter *painter, const KoTextDocumentLayout::PaintContext &context) {
    painter->setPen(context.textContext.palette.color(QPalette::Text)); // for text that has no color.
    const QRegion clipRegion = painter->clipRegion();
    QTextBlock block = m_parent->document()->begin();
    KoTextBlockBorderData *lastBorder = 0;
    bool started=false;
    int selectionStart = -1, selectionEnd = -1;
    if(context.textContext.selections.count()) {
        QTextCursor cursor = context.textContext.selections[0].cursor;
        selectionStart = cursor.position();
        selectionEnd = cursor.anchor();
        if(selectionStart > selectionEnd)
            qSwap(selectionStart, selectionEnd);
    }

    while(block.isValid()) {
        QTextLayout *layout = block.layout();

        if(!painter->hasClipping() || ! clipRegion.intersect(QRegion(layout->boundingRect().toRect())).isEmpty()) {
            started=true;
            QBrush bg = block.blockFormat().background();
            if (bg != Qt::NoBrush)
                painter->fillRect(layout->boundingRect(), bg);
            painter->save();
            drawListItem(painter, block);
            painter->restore();

            QVector<QTextLayout::FormatRange> selections;
            foreach(QAbstractTextDocumentLayout::Selection selection, context.textContext.selections) {
                QTextCursor cursor = selection.cursor;
                int begin = cursor.position();
                int end = cursor.anchor();
                if(begin > end)
                    qSwap(begin, end);

               if(end < block.position() || begin > block.position() + block.length())
                   continue; // selection does not intersect this block.
                QTextLayout::FormatRange fr;
                fr.start = begin - block.position();
                fr.length = end - begin;
                fr.format = selection.format;
                selections.append(fr);
            }
            layout->draw(painter, QPointF(0,0), selections);
            decorateParagraph(painter, block, selectionStart - block.position(), selectionEnd - block.position(), context.viewConverter);

            KoTextBlockBorderData *border = 0;
            KoTextBlockData *blockData = dynamic_cast<KoTextBlockData*> (block.userData());
            if(blockData)
                border = dynamic_cast<KoTextBlockBorderData*> (blockData->border());
            if(lastBorder && lastBorder != border) {
                painter->save();
                lastBorder->paint(*painter);
                painter->restore();
            }
            lastBorder = border;
        }
        else if(started) // when out of the cliprect, then we are done drawing.
            break;
        block = block.next();
    }
    if(lastBorder)
        lastBorder->paint(*painter);
}

static void drawDecorationLine (QPainter *painter, const QColor &color, KoCharacterStyle::LineType type, KoCharacterStyle::LineStyle style, const double x1, const double x2, const double y) {
    QPen penBackup = painter->pen();
    QPen pen = painter->pen();
    pen.setColor(color);
    pen.setWidth(painter->fontMetrics().lineWidth());
    if (style == KoCharacterStyle::WaveLine) {
        // Ok, try the waves :)
        pen.setStyle(Qt::SolidLine);
        painter->setPen(pen);
        double x = x1;
        const double halfWaveWidth = 2 * painter->fontMetrics().lineWidth();
        const double halfWaveLength = 6 * painter->fontMetrics().lineWidth();
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
            x = x + 2*halfWaveLength;
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

void Layout::decorateParagraph(QPainter *painter, const QTextBlock &block, int selectionStart, int selectionEnd, const KoViewConverter *converter) {
    Q_UNUSED(selectionStart);
    Q_UNUSED(selectionEnd);
    Q_UNUSED(converter);

    QTextLayout *layout = block.layout();
    QTextOption textOption = layout->textOption();

    QTextBlockFormat bf = block.blockFormat();

    QTextBlock::iterator it;
    int offset = -1;
    // loop over text fragments in this paragraph and draw the underline and line through.
    for (it = block.begin(); !(it.atEnd()); ++it) {
        QTextFragment currentFragment = it.fragment();
        if (currentFragment.isValid()) {
            if (offset == -1)
                offset = currentFragment.position();
            int firstLine = layout->lineForTextPosition(currentFragment.position() - offset).lineNumber();
            int lastLine = layout->lineForTextPosition(currentFragment.position() + currentFragment.length() - offset).lineNumber();

            for (int i = firstLine ; i <= lastLine ; i++) {
                QTextLine line = layout->lineAt(i);
                if (layout->isValidCursorPosition(currentFragment.position() - offset)) {
                    double x1 = line.cursorToX(currentFragment.position() - offset);
                    double x2 = line.cursorToX(currentFragment.position() + currentFragment.length() - offset);
                    QTextCharFormat fmt = currentFragment.charFormat();
                    KoCharacterStyle::LineStyle fontStrikeOutStyle = (KoCharacterStyle::LineStyle)
                    fmt.intProperty(KoCharacterStyle::StrikeOutStyle);
                    KoCharacterStyle::LineType fontStrikeOutType = (KoCharacterStyle::LineType)
                    fmt.intProperty(KoCharacterStyle::StrikeOutType);
                    if ((fontStrikeOutStyle != KoCharacterStyle::NoLineStyle) &&
                        (fontStrikeOutType != KoCharacterStyle::NoLineType)) {
                        double y = line.position().y() + line.height()/2;
                        QColor color = fmt.colorProperty(KoCharacterStyle::StrikeOutColor);
                        if (!color.isValid())
                            color = fmt.foreground().color();

                        drawDecorationLine (painter, color, fontStrikeOutType, fontStrikeOutStyle, x1, x2, y);
                    }

                    QFontMetrics metrics( fmt.font() );
                    double y = line.position().y() + line.ascent() + metrics.underlinePos();
                    KoCharacterStyle::LineStyle fontUnderLineStyle = (KoCharacterStyle::LineStyle)
                    fmt.intProperty(KoCharacterStyle::UnderlineStyle);
                    KoCharacterStyle::LineType fontUnderLineType = (KoCharacterStyle::LineType)
                    fmt.intProperty(KoCharacterStyle::UnderlineType);
                    if ((fontUnderLineStyle != KoCharacterStyle::NoLineStyle) &&
                        (fontUnderLineType != KoCharacterStyle::NoLineType)) {
                        QFontMetrics metrics( fmt.font() );
                        double y = line.position().y() + line.ascent() + metrics.underlinePos();
                        QColor color = fmt.underlineColor();
                        if (!color.isValid())
                            color = fmt.foreground().color();

                        drawDecorationLine (painter, color, fontUnderLineType, fontUnderLineStyle, x1, x2, y);
                    }

                    bool misspelled = fmt.boolProperty(KoCharacterStyle::Spelling);
                    if (misspelled)
                        drawDecorationLine (painter, QColor(255,0,0), KoCharacterStyle::SingleLine, KoCharacterStyle::WaveLine, x1, x2, y);
                }
            }
        }
    }
}

void Layout::drawListItem(QPainter *painter, const QTextBlock &block) {
    KoTextBlockData *data = dynamic_cast<KoTextBlockData*> (block.userData());
    if(data == 0)
        return;

    QTextList *list = block.textList();
    if(list && data->hasCounterData()) {
        QTextListFormat listFormat = list->format();
        QTextCharFormat cf;
        bool filled=false;
        if(m_styleManager) {
            const int id = listFormat.intProperty(KoListStyle::CharacterStyleId);
            KoCharacterStyle *cs = m_styleManager->characterStyle(id);
            if(cs) {
                cs->applyStyle(cf);
                filled = true;
            }
        }
        if(! filled) {
            // use first char of block.
            QTextCursor cursor(block); // I know this is longwinded, but just using the blocks
            // charformat does not work, apparantly
            cf = cursor.charFormat();
        }

        if(! data->counterText().isEmpty()) {
            QFont font(cf.font(), m_parent->paintDevice());
            QTextLayout layout(data->counterText(), font, m_parent->paintDevice());
            layout.setCacheEnabled(true);
            QList<QTextLayout::FormatRange> layouts;
            QTextLayout::FormatRange format;
            format.start=0;
            format.length=data->counterText().length();
            format.format = cf;
            layouts.append(format);
            layout.setAdditionalFormats(layouts);

            Qt::Alignment align = static_cast<Qt::Alignment> (listFormat.intProperty(KoListStyle::Alignment));
            if(align == 0)
                align = Qt::AlignLeft;
            else if(align != Qt::AlignAuto)
                align |= Qt::AlignAbsolute;
            QTextOption option( align );
            option.setTextDirection(block.layout()->textOption().textDirection());
            if(option.textDirection() == Qt::RightToLeft || data->counterText().isRightToLeft())
                option.setAlignment(Qt::AlignRight);
            layout.setTextOption(option);
            layout.beginLayout();
            QTextLine line = layout.createLine();
            line.setLineWidth(data->counterWidth() - data->counterSpacing());
            layout.endLayout();
            layout.draw(painter, data->counterPosition());
        }

        KoListStyle::Style listStyle = static_cast<KoListStyle::Style> ( listFormat.style() );
        if(listStyle == KoListStyle::SquareItem || listStyle == KoListStyle::DiscItem ||
                listStyle == KoListStyle::CircleItem || listStyle == KoListStyle::BoxItem ||
                listStyle == KoListStyle::RhombusItem ||
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

            double width = fm.xHeight();
            double y = data->counterPosition().y() + fm.ascent() - fm.xHeight(); // at top of text.
            int percent = listFormat.intProperty(KoListStyle::BulletSize);
            if(percent > 0)
                width *= percent / 100.0;
            y -= width / 10.; // move it up just slightly
            double x = qMax(1., data->counterPosition().x() + fm.width(listFormat.stringProperty( KoListStyle::ListItemPrefix )));
                switch( listStyle ) {
                case KoListStyle::SquareItem: {
                    painter->fillRect(QRectF(x, y, width, width), QBrush(Qt::black));
                } break;
                case KoListStyle::DiscItem:
                    painter->setBrush(QBrush(Qt::black));
                    // fall through!
                case KoListStyle::CircleItem: {
                    painter->drawEllipse(QRectF(x, y, width, width));
                } break;
                case KoListStyle::BoxItem: {
                    painter->drawRect(QRectF(x, y, width, width));
                } break;
                case KoListStyle::RhombusItem: {
                    painter->translate(QPointF(x+(width/2.0),y));
                    painter->rotate(45.0);
                    painter->fillRect(QRectF(0, 0, width, width), QBrush(Qt::black));
                } break;
                case KoListStyle::RightArrowItem: {
                    const double half = width/2.0;
                    painter->translate(QPointF(x,y));
                    QPointF points[] = { QPointF(half,0), QPointF(width,half), QPointF(half,width) };
                    painter->drawPolyline(points,3);
                    painter->drawLine(QLineF(0,half,width,half));
                } break;
                case KoListStyle::RightArrowHeadItem: {
                    painter->translate(QPointF(x,y));
                    QPointF points[] = { QPointF(0,0), QPointF(width,width/2.0), QPointF(0,width) };
                    painter->drawPolyline(points,3);
                } break;
                case KoListStyle::HeavyCheckMarkItem: {
                    const double half = width/2.0;
                    painter->translate(QPointF(x,y));
                    QPointF points[] = { QPointF(half,half), QPointF(half,width), QPointF(width,0) };
                    painter->drawPolyline(points,3);
                } break;
                case KoListStyle::BallotXItem: {
                    painter->translate(QPointF(x,y));
                    painter->drawLine(QLineF(0.0,0.0,width,width));
                    painter->drawLine(QLineF(0.0,width,width,0.0));
                } break;
                default:; // others we ignore.
            }
        }
    }
}

bool Layout::setFollowupShape(KoShape *followupShape) {
    if(m_demoText)
        return false;
    Q_ASSERT(shape == 0);
    Q_ASSERT(followupShape);

    m_data = dynamic_cast<KoTextShapeData*> (followupShape->userData());
    if(m_data == 0)
        return false;

    m_newShape = false;
    shape = followupShape;
    m_data->setDocumentOffset(m_y);
    m_shapeBorder = shape->borderInsets();
    return true;
}

void Layout::clearTillEnd() {
    QTextBlock block = m_block.next();
    while(block.isValid()) {
        if(block.layout()->lineCount() == 0)
            return;
        // erase the layouted lines
        block.layout()->beginLayout();
        block.layout()->endLayout();
        block = block.next();
    }
}

int Layout::cursorPosition() const {
    int answer = m_block.position();
    if(!m_newParag && layout && layout->lineCount()) {
        QTextLine tl = layout->lineAt(layout->lineCount() -1);
        answer += tl.textStart() + tl.textLength() - 1;
    }
    return answer;
}

bool Layout::previousParag() {
    if(m_block.position() == 0 && layout->lineCount() == 0)
        return false;

    layout->endLayout();
    if(layout->lineCount() == 0) {
        m_block = m_block.previous();
        layout = m_block.layout();
    }
    QTextLine tl = layout->lineAt(0);
    Q_ASSERT(tl.isValid());
    m_y = tl.y();

    m_format = m_block.blockFormat();
    m_blockData = dynamic_cast<KoTextBlockData*> (m_block.userData());
    m_isRtl = m_block.text().isRightToLeft();

    m_fragmentIterator = m_block.begin();
    m_newParag = true;

    if(m_data->position() > m_block.position()) { // go back a shape.
        QList<KoShape *> shapes = m_parent->shapes();
        for(--shapeNumber; shapeNumber >= 0; shapeNumber--) {
            shape = shapes[shapeNumber];
            m_data = dynamic_cast<KoTextShapeData*> (shape->userData());
            if(m_data != 0)
                break;
        }
        Q_ASSERT(m_data); // should never happen since the first shape is always a proper shape.
        if(m_data == 0)
            return false;

        m_shapeBorder = shape->borderInsets();
    }
    m_newShape = m_block.position() == m_data->position();
    updateBorders(); // fill the border inset member vars.
    layout->beginLayout();
    return true;
}

void Layout::registerInlineObject(const QTextInlineObject &inlineObject) {
    m_inlineObjectHeights.insert(m_block.position() + inlineObject.textPosition(), inlineObject.height());
}

double Layout::inlineCharHeight(const QTextFragment &fragment) {
    if(m_inlineObjectHeights.contains(fragment.position()))
        return m_inlineObjectHeights[fragment.position()];
    return 0.0;
}

double Layout::findFootnote(const QTextLine &line) {
    if(m_parent->inlineObjectTextManager() == 0)
        return 0;
    QString text = m_block.text();
    int pos = text.indexOf(QChar(0xFFFC), line.textStart());
    while(pos >= 0 && pos < line.textLength()) {
        QTextCursor c1(m_block);
        c1.setPosition(m_block.position() + pos);
        c1.setPosition(c1.position() + 1, QTextCursor::KeepAnchor);
        KoInlineNote *note = dynamic_cast<KoInlineNote*> (m_parent->inlineObjectTextManager()->inlineTextObject(c1));
        if(note && note->type() == KoInlineNote::Footnote) {
            QTextBlock last = m_textShape->footnoteDocument()->end().previous();
            QTextCursor cursor(last);
            cursor.setPosition(last.position() + last.length()-1);
            if(cursor.position() > 1)
                cursor.insertText("\n");

            QTextCharFormat cf;
            cf.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
            cursor.mergeCharFormat(cf);
            cursor.insertText(note->label() +' ');
            cf.setVerticalAlignment(QTextCharFormat::AlignNormal);
            cursor.mergeCharFormat(cf);
            cursor.insertText(note->text());
        }
        pos = text.indexOf(QChar(0xFFFC), pos+1);
    }
    if(m_textShape->hasFootnoteDocument())
        return m_textShape->footnoteDocument()->size().height();
    return 0;
}

void Layout::setStyleManager(KoStyleManager *sm)
{
    m_styleManager = sm;
}

