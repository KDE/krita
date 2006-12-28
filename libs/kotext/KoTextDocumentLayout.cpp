/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "KoTextDocumentLayout.h"
#include "KoTextShapeData.h"
#include "styles/KoParagraphStyle.h"
#include "styles/KoCharacterStyle.h"
#include "styles/KoListStyle.h"
#include "styles/KoStyleManager.h"
#include "KoTextBlockData.h"
#include "KoTextBlockBorderData.h"
#include "KoInlineTextObjectManager.h"

#include <KoInsets.h>
#include <KoShape.h>

#include <kdebug.h>
#include <QTextBlock>
#include <QTextList>
#include <QTimer>

#include <limits.h>

/// helper class to disable any screen convertion as thats done in flake.
class PostscriptPaintDevice : public QPaintDevice {
public:
    PostscriptPaintDevice() {
    }
//   int devType () const {
//   }
    QPaintEngine *paintEngine () const {
        return 0;
    }
    int metric (PaintDeviceMetric metric) const {
        switch(metric) {
            case QPaintDevice::PdmWidth:
            case QPaintDevice::PdmHeight:
            case QPaintDevice::PdmWidthMM:
            case QPaintDevice::PdmHeightMM:
            case QPaintDevice::PdmNumColors:
                return INT_MAX;
            case QPaintDevice::PdmDepth:
                return 32;
            case QPaintDevice::PdmDpiX:
            case QPaintDevice::PdmDpiY:
            case QPaintDevice::PdmPhysicalDpiX:
            case QPaintDevice::PdmPhysicalDpiY:
                return 72;
        }
        return 0; // should never be hit
    }
};

class LayoutStatePrivate {
public:
    LayoutStatePrivate() : blockData(0), data(0), reset(true)  { }
    KoStyleManager *styleManager;

    double y;
    QTextBlock block;
    KoTextBlockData *blockData;
    QTextBlockFormat format;
    QTextBlock::Iterator fragmentIterator;
    KoTextShapeData *data;
    bool newShape, newParag, reset;
    KoInsets borderInsets;
    KoInsets shapeBorder;
    KoTextDocumentLayout *parent;
};

// ---------------- layout helper ----------------
KoTextDocumentLayout::LayoutState::LayoutState(KoTextDocumentLayout *parent) {
    d = new LayoutStatePrivate();
    d->parent = parent;
    layout = 0;
}

bool KoTextDocumentLayout::LayoutState::start() {
    if(d->reset)
        resetPrivate();
    else
        nextParag();
    d->reset = false;
    return !(layout == 0 || d->parent->shapes().count() <= shapeNumber);
}

void KoTextDocumentLayout::LayoutState::end() {
    if(layout)
        layout->endLayout();
    layout = 0;
}

void KoTextDocumentLayout::LayoutState::reset() {
    d->reset = true;
}

bool KoTextDocumentLayout::LayoutState::interrupted() {
    return d->reset;
}

void KoTextDocumentLayout::LayoutState::setStyleManager(KoStyleManager *sm) {
    d->styleManager = sm;
}

double KoTextDocumentLayout::LayoutState::width() {
    double ptWidth = shape->size().width() - d->format.leftMargin() - d->format.rightMargin();
    if(d->newParag)
        ptWidth -= d->format.textIndent();
    if(d->newParag && d->block.textList()) // is a listItem
        ptWidth -= listIndent();
    ptWidth -= d->borderInsets.left + d->borderInsets.right + d->shapeBorder.right;
    return ptWidth;
}

double KoTextDocumentLayout::LayoutState::x() {
    double result = (d->newParag?d->format.textIndent():0.0) + d->format.leftMargin();
    if(d->block.textList()) { // is a listItem
        double indent = listIndent();
        d->blockData->setCounterPosition(QPointF(result, y()));
        result += indent;
    }
    result += d->borderInsets.left + d->shapeBorder.left;
    return result;
}

double KoTextDocumentLayout::LayoutState::y() {
    return d->y;
}

double KoTextDocumentLayout::LayoutState::docOffsetInShape() const {
    return d->data->documentOffset();
}

bool KoTextDocumentLayout::LayoutState::addLine(const QTextLine &line) {
    double height = d->format.doubleProperty(KoParagraphStyle::FixedLineHeight);
    bool useFixedLineHeight = height != 0.0;
    bool useFontProperties = d->format.boolProperty(KoParagraphStyle::LineSpacingFromFont);
    if(! useFixedLineHeight) {
        if(useFontProperties)
            height = line.height();
        else {
            if(d->fragmentIterator.atEnd()) // no text in parag.
                height = d->block.charFormat().fontPointSize();
            else {
                // read max font height
                height = qMax(height,
                        d->fragmentIterator.fragment().charFormat().fontPointSize());
                while(! (d->fragmentIterator.atEnd() || d->fragmentIterator.fragment().contains(
                               d->block.position() + line.textStart() + line.textLength() -1))) {
                    d->fragmentIterator++;
                    height = qMax(height, d->fragmentIterator.fragment().charFormat().fontPointSize());
                }
            }
            if(height < 0.01) height = 12; // default size for uninitialized styles.
        }
    }

    if(d->data->documentOffset() + shape->size().height() < d->y + height + d->shapeBorder.bottom) {
//kDebug() << "   NEXT shape" << endl;
        // line does not fit.
        d->data->setEndPosition(d->block.position() + line.textStart()-1);
        nextShape();
        if(d->data)
            d->data->setPosition(d->block.position() + line.textStart());
        return true;
    }

    // add linespacing
    if(! useFixedLineHeight) {
        double linespacing = d->format.doubleProperty(KoParagraphStyle::LineSpacing);;
        if(linespacing == 0.0) { // unset
            int percent = d->format.intProperty(KoParagraphStyle::FixedLineHeight);
            if(percent != 0)
                linespacing = height * ((percent - 100) / 100.0);
            else if(linespacing == 0.0)
                linespacing = height * 0.2; // default
        }
        height += linespacing;
    }

    double minimum = d->format.doubleProperty(KoParagraphStyle::MinimumLineHeight);
    if(minimum > 0.0)
        height = qMax(height, minimum);
    if(qAbs(d->y - line.y()) < 0.126) // rounding problems due to Qt-scribe internally using ints.
        d->y += height;
    else
        d->y = line.y() + height; // The line got a pos <> from y(), follow that lead.
    d->newShape = false;
    d->newParag = false;
    return false;
}

bool KoTextDocumentLayout::LayoutState::nextParag() {
    if(layout) { // guard against first time
        layout->endLayout();
        d->block = d->block.next();
        if(!d->newShape) {// this parag is first line on new shape
            d->y += d->format.bottomMargin();
            if(d->format.pageBreakPolicy() == QTextFormat::PageBreak_AlwaysAfter ||
                    d->format.boolProperty(KoParagraphStyle::BreakAfter)) {
                d->data->setEndPosition(d->block.position()-1);
                nextShape();
                if(d->data)
                    d->data->setPosition(d->block.position());
            }
        }
        if(!d->newShape) // only add bottom of prev parag if we did not go to a new shape for this parag.
            d->y += d->borderInsets.bottom;
    }
    layout = 0;
    if(d->blockData && d->blockData->border())
        d->blockData->border()->setParagraphBottom(d->y);
    d->blockData = 0;
    if(! d->block.isValid()) {
        QTextBlock block = d->block.previous(); // last correct one.
        d->data->setEndPosition(block.position() + block.length());

        // cleanupShapes();
        return false;
    }
    d->format = d->block.blockFormat();
    d->blockData = dynamic_cast<KoTextBlockData*> (d->block.userData());

    updateBorders(); // fill the border inset member vars.
    d->y += d->borderInsets.top;

    if(!d->newShape && (d->format.pageBreakPolicy() == QTextFormat::PageBreak_AlwaysBefore ||
            d->format.boolProperty(KoParagraphStyle::BreakBefore))) {
        d->data->setEndPosition(d->block.position()-1);
        nextShape();
        if(d->data)
            d->data->setPosition(d->block.position());
    }
    d->y += topMargin();
    layout = d->block.layout();
    QTextOption options = layout->textOption();
    options.setAlignment(d->format.alignment());
    layout->setTextOption(options);

    layout->beginLayout();
    d->fragmentIterator = d->block.begin();
    d->newParag = true;

//kDebug()<< "nextParag " << d->block.textList() << " for " << d->block.text() << endl;
    return true;
}

double KoTextDocumentLayout::LayoutState::documentOffsetInShape() {
    return d->data->documentOffset();
}

void KoTextDocumentLayout::LayoutState::nextShape() {
    d->newShape = true;

    if(d->data) {
// TODO add weduwen wezen algo waardoor ik eventueel een parag opnieuw moet kunnen layouten.
        Q_ASSERT(d->data->endPosition() >= d->data->position());
        d->y = d->data->documentOffset() + shape->size().height() + 10.0;
        d->data->wipe();
    }

    shape = 0;
    d->data = 0;

    QList<KoShape *> shapes = d->parent->shapes();
    for(shapeNumber++; shapeNumber < shapes.count(); shapeNumber++) {
        shape = shapes[shapeNumber];
        d->data = dynamic_cast<KoTextShapeData*> (shape->userData());
        if(d->data != 0)
            break;
        shape = 0;
        d->data = 0;
    }

    if(shape == 0)
        return;
    d->data->setDocumentOffset(d->y);
    d->data->faul(); // make dirty since this one needs relayout at this point.
    d->shapeBorder = shape->borderInsets();
    d->y += d->shapeBorder.top;
}

// and the end of text, make sure the rest of the frames have something sane to show.
void KoTextDocumentLayout::LayoutState::cleanupShapes() {
    int i = shapeNumber + 1;
    QList<KoShape *> shapes = d->parent->shapes();
    while(i < shapes.count())
        cleanupShape(shapes[i++]);
}

void KoTextDocumentLayout::LayoutState::cleanupShape(KoShape *daShape) {
    KoTextShapeData *textData = dynamic_cast<KoTextShapeData*> (daShape->userData());
    if(textData) {
        textData->setPosition(-1);
        textData->wipe();
    }
    daShape->repaint();
}

double KoTextDocumentLayout::LayoutState::listIndent() {
    Q_ASSERT(d->block.textList());
    QTextList *textList = d->block.textList();
    QTextListFormat format = textList->format();
    int styleId = format.intProperty(KoListStyle::CharacterStyleId);
    KoCharacterStyle *charStyle = 0;
    if(styleId > 0 && d->styleManager)
        charStyle = d->styleManager->characterStyle(styleId);
    if(!charStyle && d->styleManager) { // try the one from paragraph style
        KoParagraphStyle *ps = d->styleManager->paragraphStyle(
                d->format.intProperty(KoParagraphStyle::StyleId));
        if(ps)
            charStyle = ps->characterStyle();
    }

    if(! (d->blockData && d->blockData->hasCounterData())) {
        QFont font;
        if(charStyle)
            font = QFont(charStyle->fontFamily(), qRound(charStyle->fontPointSize()),
                    charStyle->fontWeight(), charStyle->fontItalic());
        else {
            QTextCursor cursor(d->block);
            font = cursor.charFormat().font();
        }
        ListItemsHelper lih(textList, font);
        lih.recalculate();
        d->blockData = dynamic_cast<KoTextBlockData*> (d->block.userData());
    }
    Q_ASSERT(d->blockData);
    return d->blockData->counterWidth();
}

void KoTextDocumentLayout::LayoutState::resetPrivate() {
    d->y = 0;
    d->data = 0;
    shape =0;
    layout = 0;
    d->newShape = true;
    d->blockData = 0;
    d->block = d->parent->document()->begin();

    shapeNumber = 0;
    int lastPos = -1;
    QList<KoShape *> shapes = d->parent->shapes();
    foreach(KoShape *shape, shapes) {
        KoTextShapeData *data = dynamic_cast<KoTextShapeData*> (shape->userData());
        Q_ASSERT(data);
        if(data->isDirty()) {
            // this shape needs to be recalculated.
            data->setPosition(lastPos+1);
            d->block = d->parent->document()->findBlock( lastPos+1 );
            d->y = data->documentOffset();
            d->format = d->block.blockFormat();

            if(d->block.layout() && d->block.layout()->lineCount() > 0) {
                // block has been layouted. So use its offset.
                d->y = d->block.layout()->lineAt(0).position().y();
                if(d->y < data->documentOffset()) {
                    Q_ASSERT(shapeNumber > 0);
                    // since we only recalc whole parags; we need to go back a little.
                    shapeNumber--;
                    shape = shapes[shapeNumber];
                    data = dynamic_cast<KoTextShapeData*> (shape->userData());
                    d->newShape = false;
                }
                // in case this parag has a border we have to subtract that as well
                d->blockData = dynamic_cast<KoTextBlockData*> (d->block.userData());
                if(d->blockData && d->blockData->border()) {
                    double top = d->blockData->border()->inset(KoTextBlockBorderData::Top);
                    // but only when this border actually makes us have an indent.
                    if(qAbs(d->blockData->border()->rect().top() + top - d->y) < 1E-10)
                        d->y -= top;
                }
                // subtract the top margins as well.
                d->y -= topMargin();
            }
            break;
        }
        lastPos = data->endPosition();
        shapeNumber++;
    }
    Q_ASSERT(shapeNumber >= 0);
    if(shapes.count() == 0)
        return;
    shape = shapes[shapeNumber];
    d->data = dynamic_cast<KoTextShapeData*> (shape->userData());
    d->shapeBorder = shape->borderInsets();
    if(d->y == 0)
        d->y = d->shapeBorder.top;

   if(! nextParag())
       shapeNumber++;
}

void KoTextDocumentLayout::LayoutState::updateBorders() {
    d->borderInsets.top = d->format.doubleProperty(KoParagraphStyle::TopPadding);
    d->borderInsets.left = d->format.doubleProperty(KoParagraphStyle::LeftPadding);
    d->borderInsets.bottom = d->format.doubleProperty(KoParagraphStyle::BottomPadding);
    d->borderInsets.right = d->format.doubleProperty(KoParagraphStyle::RightPadding);

    KoTextBlockBorderData border(QRectF(x(), d->y + d->borderInsets.top, width(), 1.));
    border.setEdge(border.Left, d->format, KoParagraphStyle::LeftBorderStyle,
        KoParagraphStyle::LeftBorderWidth, KoParagraphStyle::LeftBorderColor,
        KoParagraphStyle::LeftBorderSpacing, KoParagraphStyle::LeftInnerBorderWidth);
    border.setEdge(border.Right, d->format, KoParagraphStyle::RightBorderStyle,
        KoParagraphStyle::RightBorderWidth, KoParagraphStyle::RightBorderColor,
        KoParagraphStyle::RightBorderSpacing, KoParagraphStyle::RightInnerBorderWidth);
    border.setEdge(border.Top, d->format, KoParagraphStyle::TopBorderStyle,
        KoParagraphStyle::TopBorderWidth, KoParagraphStyle::TopBorderColor,
        KoParagraphStyle::TopBorderSpacing, KoParagraphStyle::TopInnerBorderWidth);
    border.setEdge(border.Bottom, d->format, KoParagraphStyle::BottomBorderStyle,
        KoParagraphStyle::BottomBorderWidth, KoParagraphStyle::BottomBorderColor,
        KoParagraphStyle::BottomBorderSpacing, KoParagraphStyle::BottomInnerBorderWidth);

    // check if prev parag had a border.
    QTextBlock prev = d->block.previous();
    KoTextBlockBorderData *prevBorder = 0;
    if(prev.isValid()) {
        KoTextBlockData *bd = dynamic_cast<KoTextBlockData*> (prev.userData());
        if(bd)
            prevBorder = bd->border();
    }
    if(border.hasBorders()) {
        if(d->blockData == 0) {
            d->blockData = new KoTextBlockData();
            d->block.setUserData(d->blockData);
        }

        // then check if we can merge with the previous parags border.
        if(prevBorder && prevBorder->equals(border))
            d->blockData->setBorder(prevBorder);
        else {
            // can't merge; then these are our new borders.
            KoTextBlockBorderData *newBorder = new KoTextBlockBorderData(border);
            d->blockData->setBorder(newBorder);
            if(prevBorder && !d->newShape)
                d->y += prevBorder->inset(KoTextBlockBorderData::Bottom);
        }
        d->blockData->border()->applyInsets(d->borderInsets, d->y + d->borderInsets.top);
    }
    else { // this parag has no border.
        if(prevBorder && !d->newShape)
            d->y += prevBorder->inset(KoTextBlockBorderData::Bottom);
        if(d->blockData)
            d->blockData->setBorder(0); // remove an old one, if there was one.
    }
}

double KoTextDocumentLayout::LayoutState::topMargin() {
    bool allowMargin = true; // wheather to allow margins at top of shape
    if(d->newShape) {
        allowMargin = false; // false by default, but check 2 exceptions.
        if(d->format.boolProperty(KoParagraphStyle::BreakBefore))
            allowMargin = true;
        else if( d->styleManager && d->format.topMargin() > 0) {
            // also allow it when the paragraph has the margin, but the style has a different one.
            KoParagraphStyle *ps = d->styleManager->paragraphStyle(
                    d->format.intProperty(KoParagraphStyle::StyleId));
            if(ps == 0 || ps->topMargin() != d->format.topMargin())
                allowMargin = true;
        }
    }
    if(allowMargin)
        return d->format.topMargin();
    return 0.0;
}

void KoTextDocumentLayout::addShape(KoShape *shape) {
    m_shapes.append(shape);

    KoTextShapeData *data = dynamic_cast<KoTextShapeData*> (shape->userData());
    if(data) {
        data->faul();
        m_state->reset();
    }
}

// ------------------- KoTextDocumentLayout --------------------
KoTextDocumentLayout::KoTextDocumentLayout(QTextDocument *doc)
    : QAbstractTextDocumentLayout(doc),
    m_state(0),
    m_styleManager(0),
    m_inlineTextObjectManager(0),
    m_scheduled(false)
{
    m_state = new KoTextDocumentLayout::LayoutState(this);
    setPaintDevice( new PostscriptPaintDevice() );
}

KoTextDocumentLayout::~KoTextDocumentLayout() {
    m_styleManager = 0;
    delete m_state;
}


void KoTextDocumentLayout::setStyleManager(KoStyleManager *sm) {
    m_styleManager = sm;
    m_state->setStyleManager(sm);
}

void KoTextDocumentLayout::setInlineObjectTextManager(KoInlineTextObjectManager *iom) {
    m_inlineTextObjectManager = iom;
}

QRectF KoTextDocumentLayout::blockBoundingRect(const QTextBlock &block) const {
    // nobody calls this code and I have no way of implementing it anyway...
    Q_UNUSED(block);
    kWarning() << "KoTextDocumentLayout::blockBoundingRect is not implemented"<< endl;
    return QRectF(0, 0, 10, 10);
}

QSizeF KoTextDocumentLayout::documentSize() const {
    // nobody calls this code and I have no way of implementing it anyway...
    kWarning() << "KoTextDocumentLayout::documentSize is not implemented"<< endl;
    return QSizeF(10, 10);
}

void KoTextDocumentLayout::draw(QPainter *painter, const PaintContext &context) {
painter->setPen(Qt::black); // TODO use theme color, or a kword wide hardcoded default.
    Q_UNUSED(context);
    const QRegion clipRegion = painter->clipRegion();
    // da real work
    QTextBlock block = document()->begin();
    bool started=false;
    while(block.isValid()) {
        QTextLayout *layout = block.layout();

        // the following line is simpler, but due to a Qt bug doesn't work. Try to see if enabling this for Qt4.3
        // will not paint all paragraphs.
        //if(!painter->hasClipping() || ! clipRegion.intersect(QRegion(layout->boundingRect().toRect())).isEmpty()) {
        if(layout->lineCount() >= 1) {
            QTextLine first = layout->lineAt(0);
            QTextLine last = layout->lineAt(layout->lineCount()-1);
            QRectF parag(qMin(first.x(), last.x()), first.y(), qMax(first.width(), last.width()), last.y() + last.height());
            if(!painter->hasClipping() || ! clipRegion.intersect(QRegion(parag.toRect())).isEmpty()) {
                started=true;
                decorateParagraph(painter, block);
                layout->draw(painter, QPointF(0,0));
            }
            else if(started) // when out of the cliprect, then we are done drawing.
                return;
        }
        block = block.next();
    }
}

void KoTextDocumentLayout::decorateParagraph(QPainter *painter, const QTextBlock &block) {
    KoTextBlockData *data = dynamic_cast<KoTextBlockData*> (block.userData());
    if(data == 0)
        return;

    QTextList *list = block.textList();
    if(list && data->hasCounterData()) {
        QTextCharFormat cf;
        bool filled=false;
        if(m_styleManager) {
            const int id = list->format().intProperty(KoListStyle::CharacterStyleId);
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
        QFont font(cf.font(), paintDevice());
        QTextLayout layout(data->counterText(), font, paintDevice());
        layout.setCacheEnabled(true);
        QList<QTextLayout::FormatRange> layouts;
        QTextLayout::FormatRange format;
        format.start=0;
        format.length=data->counterText().length();
        format.format = cf;
        layouts.append(format);
        layout.setAdditionalFormats(layouts);

        QTextOption option(Qt::AlignLeft | Qt::AlignAbsolute);
        option.setTextDirection(block.blockFormat().layoutDirection());
        layout.setTextOption(option);
        layout.beginLayout();
        layout.createLine();
        layout.endLayout();
        layout.draw(painter, data->counterPosition());
    }

    KoTextBlockBorderData *border = dynamic_cast<KoTextBlockBorderData*> (data->border());
    // TODO make sure we only paint a border-set one time.
    if(border) {
        painter->save();
        border->paint(*painter);
        painter->restore();
    }
}

QRectF KoTextDocumentLayout::frameBoundingRect(QTextFrame *frame) const {
    Q_UNUSED(frame);
    // nobody calls this code and I have no way of implementing it anyway...
    kWarning() << "KoTextDocumentLayout::frameBoundingRect is not implemented"<< endl;
    return QRectF(0, 0, 10, 10);
}

int KoTextDocumentLayout::hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const {
    // kDebug() << "hitTest[" << point.x() << "," << point.y() << "]" << endl;
    QTextBlock block = document()->begin();
    int position = -1;
    while(block.isValid()) {
        QTextLayout *layout = block.layout();
        if(point.y() > layout->boundingRect().bottom()) {
            position = block.position() + block.length();
            block = block.next();
            continue;
        }
        for(int i=0; i < layout->lineCount(); i++) {
            QTextLine line = layout->lineAt(i);
            // kDebug() << " + line[" << line.textStart() << "]: " << line.y() << "-" << line.height() << endl;
            if(point.y() > line.y() + line.height()) {
                position = line.textStart() + line.textLength();
                continue;
            }
            if(accuracy == Qt::ExactHit && point.y() < line.y()) // between lines
                return -1;
            if(accuracy == Qt::ExactHit && // left or right of line
                    (point.x() < line.x() || point.x() > line.x() + line.width()))
                return -1;
            return block.position() + line.xToCursor(point.x());
        }
        block = block.next();
    }
    if(accuracy == Qt::ExactHit)
        return -1;
    return position;
}

int KoTextDocumentLayout::pageCount () const {
kDebug() << "KoTextDocumentLayout::pageCount"<< endl;
    return 1;
}

void KoTextDocumentLayout::documentChanged(int position, int charsRemoved, int charsAdded) {
    Q_UNUSED(charsAdded);
    Q_UNUSED(charsRemoved);
    if(shapes().count() == 0) // nothing to do.
        return;

    foreach(KoShape *shape, shapes()) {
        KoTextShapeData *data = dynamic_cast<KoTextShapeData*> (shape->userData());
        Q_ASSERT(data);
        if(data && data->position() <= position && data->endPosition() >= position) {
            // found our (first) shape to re-layout
            data->faul();
            m_state->reset();
            scheduleLayout();
            return;
        }
    }
    // if still here; then the change was not in any frame, lets relayout the last for now.
    KoShape *shape = shapes().last();
    KoTextShapeData *data = dynamic_cast<KoTextShapeData*> (shape->userData());
    Q_ASSERT(data);
    data->faul();
    m_state->reset();
    scheduleLayout();
}

void KoTextDocumentLayout::drawInlineObject(QPainter *painter, const QRectF &rect, QTextInlineObject object, int position, const QTextFormat &format) {
    Q_ASSERT(format.isCharFormat());
    if(m_inlineTextObjectManager == 0)
        return;
    QTextCharFormat cf = format.toCharFormat();
    KoInlineObjectBase *obj = m_inlineTextObjectManager->inlineTextObject(cf);
    if(obj)
        obj->paint(*painter, paintDevice(), *document(), rect, object, position, cf);
}

void KoTextDocumentLayout::positionInlineObject(QTextInlineObject item, int position, const QTextFormat &format) {
    Q_ASSERT(format.isCharFormat());
    if(m_inlineTextObjectManager == 0)
        return;
    QTextCharFormat cf = format.toCharFormat();
    KoInlineObjectBase *obj = m_inlineTextObjectManager->inlineTextObject(cf);
    if(obj)
        obj->updatePosition(*document(), item, position, cf);
}

void KoTextDocumentLayout::resizeInlineObject(QTextInlineObject item, int position, const QTextFormat &format) {
    Q_ASSERT(format.isCharFormat());
    if(m_inlineTextObjectManager == 0)
        return;
    QTextCharFormat cf = format.toCharFormat();
    KoInlineObjectBase *obj = m_inlineTextObjectManager->inlineTextObject(cf);
    if(obj)
        obj->resize(*document(), item, position, cf, paintDevice());
}

void KoTextDocumentLayout::scheduleLayout() {
    if(m_scheduled)
        return;
    m_scheduled = true;
    QTimer::singleShot(0, this, SLOT(relayout()));
}

void KoTextDocumentLayout::relayout() {
    layout();
}

void KoTextDocumentLayout::interruptLayout() {
    m_state->reset();
}

void KoTextDocumentLayout::layout() {
    m_scheduled = false;
//kDebug() << "KoTextDocumentLayout::layout" << endl;
    class End {
    public:
        End(LayoutState *state) { m_state = state; }
        ~End() { m_state->end(); }
    private:
        LayoutState *m_state;
    };
    End ender(m_state); // poor mans finally{}

    if(! m_state->start())
        return;
    bool newParagraph = true;
    while(m_state->shape) {
        QTextLine line = m_state->layout->createLine();
        if (!line.isValid()) { // end of parag
            double posY = m_state->y();
            bool moreText = m_state->nextParag();
            if(m_state->shape && m_state->y() > posY)
                m_state->shape->repaint(QRectF(0, posY,
                            m_state->shape->size().width(), m_state->y() - posY));

            if(! moreText)
                return; // done!
            newParagraph = true;
            continue;
        }
        newParagraph = false;
        line.setLineWidth(m_state->width());
        line.setPosition(QPointF(m_state->x(), m_state->y()));
        while(m_state->addLine(line)) {
            if(m_state->shape == 0) { // shape is full!
                line.setPosition(QPointF(0, m_state->y()+20));
                return; // done!
            }
            line.setLineWidth(m_state->width());
            line.setPosition(QPointF(m_state->x(), m_state->y()));
        }

        QRectF repaintRect = line.rect();
        repaintRect.moveTop(repaintRect.y() - m_state->docOffsetInShape());
        repaintRect.setX(0.0); // just take full width since we can't force a repaint of
        repaintRect.setWidth(m_state->shape->size().width()); // where lines were before layout.
        m_state->shape->repaint(repaintRect);
    }
}

// ------------------- ListItemsHelper ------------
class ListItemsPrivate {
public:
    ListItemsPrivate(QTextList *tl, const QFont &font)
        : textList( tl ),
          fm( font, textList->document()->documentLayout()->paintDevice() ),
          displayFont(font) {
    }
    QTextList *textList;
    QFontMetricsF fm;
    QFont displayFont;


};

ListItemsHelper::ListItemsHelper(QTextList *textList, const QFont &font) {
    d = new ListItemsPrivate(textList, font);
}

ListItemsHelper::~ListItemsHelper() {
    delete d;
}

/// is meant to take a QTextList and set the indent plus the string to render on each listitem
void ListItemsHelper::recalculate() {
    //kDebug() << "ListItemsHelper::recalculate" << endl;
    double width = 0.0;
    QTextListFormat format = d->textList->format();

    // when true don't let non-numbering parags restart numbering
    bool consecutiveNumbering = format.boolProperty(KoListStyle::ConsecutiveNumbering);
    int index = format.intProperty(KoListStyle::StartValue);
    QString prefix = format.stringProperty( KoListStyle::ListItemPrefix );
    QString suffix = format.stringProperty( KoListStyle::ListItemSuffix );
    const int level = format.intProperty(KoListStyle::Level);
    int displayLevel = format.intProperty(KoListStyle::DisplayLevel);
    if(displayLevel > level || displayLevel == 0)
        displayLevel = level;
    for(int i=0; i < d->textList->count(); i++) {
        QTextBlock tb = d->textList->item(i);
        KoTextBlockData *data = dynamic_cast<KoTextBlockData*> (tb.userData());
        if(!data) {
            data = new KoTextBlockData();
            tb.setUserData(data);
        }
        int paragIndex = tb.blockFormat().intProperty( KoListStyle::ExplicitListValue);
        if(paragIndex > 0)
            index = paragIndex;

        if(! consecutiveNumbering) { // check if before this parag there was a lower-level list
// from ODF spec: text:consecutive-numbering
// The text:consecutive-numbering attribute specifies whether or not the list style uses consecutive numbering for all list levels or whether each list level restarts the numbering.
            QTextBlock b = tb.previous();
            while(b.isValid()) {
                if(b.textList() == d->textList)
                    break; // all fine
                if(b.textList() == 0 || b.textList()->format().intProperty(KoListStyle::Level)
                        < level) {
                    index = format.intProperty(KoListStyle::StartValue);
                    break;
                }
                b = b.previous();
            }
        }

        QString item("");
        if(displayLevel > 1) {
            int checkLevel = level;
            for(QTextBlock b = tb.previous(); displayLevel > 1 && b.isValid(); b=b.previous()) {
                if(b.textList() == 0)
                    continue;
                QTextListFormat lf = b.textList()->format();
                const int otherLevel  = lf.intProperty(KoListStyle::Level);
                if(checkLevel <= otherLevel)
                    continue;
              /*if(needsRecalc(b->textList())) {
                    TODO
                } */
                KoTextBlockData *otherData = dynamic_cast<KoTextBlockData*> (b.userData());
Q_ASSERT(otherData);
                if(displayLevel-1 < otherLevel) { // can't just copy it fully since we are
                                                  // displaying less then the full counter
                    item += otherData->partialCounterText();
                    displayLevel--;
                    checkLevel--;
                    for(int i=otherLevel+1;i < level; i++) {
                        displayLevel--;
                        item += ".0"; // add missing counters.
                    }
                }
                else { // just copy previous counter as prefix
                    item += otherData->counterText();
                    for(int i=otherLevel+1;i < level; i++)
                        item += ".0"; // add missing counters.
                    break;
                }
            }
        }
        KoListStyle::Style listStyle = static_cast<KoListStyle::Style> (
                d->textList->format().style() );
        if((listStyle == KoListStyle::DecimalItem || listStyle == KoListStyle::AlphaLowerItem ||
                    listStyle == KoListStyle::UpperAlphaItem ||
                    listStyle == KoListStyle::RomanLowerItem ||
                    listStyle == KoListStyle::UpperRomanItem) &&
                !(item.isEmpty() || item.endsWith('.') || item.endsWith(' '))) {
            item += '.';
        }
        switch( listStyle ) {
            case KoListStyle::DecimalItem: {
                QString i = QString::number(index);
                data->setPartialCounterText(i);
                item += i;
                width = qMax(width, d->fm.width(item));
                break;
            }
            case KoListStyle::AlphaLowerItem:
                // TODO;
                break;
            case KoListStyle::UpperAlphaItem:
                // TODO;
                break;
            case KoListStyle::RomanLowerItem:
                // TODO;
                break;
            case KoListStyle::UpperRomanItem:
                // TODO;
                break;
            case KoListStyle::SquareItem:
            case KoListStyle::DiscItem:
            case KoListStyle::CircleItem:
            case KoListStyle::BoxItem: {
                width = d->displayFont.pointSizeF();
                int percent = format.intProperty(KoListStyle::BulletSize);
                if(percent > 0)
                    width = width * (percent / 100.0);
                break;
            }
            case KoListStyle::CustomCharItem:
                width =  19.0; //  TODO
                break;
            case KoListStyle::NoItem:
                width =  10.0; // simple indenting
                break;
            default:; // others we ignore.
        }
        data->setCounterText(prefix + item + suffix);
        index++;
    }
    width += d->fm.width(prefix + suffix); // same for all
    for(int i=0; i < d->textList->count(); i++) {
        QTextBlock tb = d->textList->item(i);
        KoTextBlockData *data = dynamic_cast<KoTextBlockData*> (tb.userData());
        data->setCounterWidth(width);
        //kDebug() << data->counterText() << " " << tb.text() << endl;
        //kDebug() << "    setCounterWidth: " << width << endl;
    }
    //kDebug() << endl;
}

// static
bool ListItemsHelper::needsRecalc(QTextList *textList) {
    Q_ASSERT(textList);
    QTextBlock tb = textList->item(0);
    KoTextBlockData *data = dynamic_cast<KoTextBlockData*> (tb.userData());
    if(data == 0)
        return true;
    return !data->hasCounterData();
}

#include "KoTextDocumentLayout.moc"
