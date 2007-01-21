/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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
#include <KoPostscriptPaintDevice.h>
#include <KoShape.h>

#include <kdebug.h>
#include <QTextBlock>
#include <QTextList>
#include <QTimer>

class LayoutStateDummy : public KoTextDocumentLayout::LayoutState {
public:
    LayoutStateDummy() {}
    bool start() { return false; }
    void end() {}
    void reset() {}
    bool interrupted() { return false; }
    double width() { return 0; }
    double x() { return 0; }
    double y() { return 0; }
    double docOffsetInShape() const { return 0; }
    bool addLine(QTextLine &) { return false; }
    bool nextParag() { return false; }
    double documentOffsetInShape() { return 0; }

protected:
    void setStyleManager(KoStyleManager *) {}
};


// ------------------- KoTextDocumentLayout --------------------
KoTextDocumentLayout::KoTextDocumentLayout(QTextDocument *doc, KoTextDocumentLayout::LayoutState *layout)
    : QAbstractTextDocumentLayout(doc),
    m_state(layout),
    m_styleManager(0),
    m_inlineTextObjectManager(0),
    m_scheduled(false)
{
    setPaintDevice( new KoPostscriptPaintDevice() );
    if(m_state == 0)
        m_state = new LayoutStateDummy();
}

KoTextDocumentLayout::~KoTextDocumentLayout() {
    m_styleManager = 0;
    delete m_state;
    m_state = 0;
}

void KoTextDocumentLayout::setLayout(LayoutState *layout) {
    Q_ASSERT(layout);
    delete m_state;
    m_state = layout;
    relayout();
}

bool KoTextDocumentLayout::hasLayouter() const {
    if(dynamic_cast<LayoutStateDummy*> (m_state) != 0)
        return false;
    if(m_state == 0)
        return false;
    return true;
}

void KoTextDocumentLayout::addShape(KoShape *shape) {
    m_shapes.append(shape);

    KoTextShapeData *data = dynamic_cast<KoTextShapeData*> (shape->userData());
    if(data) {
        data->faul();
        m_state->reset();
    }
}

void KoTextDocumentLayout::setStyleManager(KoStyleManager *sm) {
    m_styleManager = sm;
    m_state->setStyleManager(sm);
}

void KoTextDocumentLayout::setInlineObjectTextManager(KoInlineTextObjectManager *iom) {
    m_inlineTextObjectManager = iom;
}

KoInlineTextObjectManager *KoTextDocumentLayout::inlineObjectTextManager() {
    return m_inlineTextObjectManager;
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
            KoTextBlockData *blockData = dynamic_cast<KoTextBlockData*> (block.userData());
            if(blockData) {
                KoTextBlockBorderData *border = blockData->border();
                if(blockData->hasCounterData()) {
                    if(block.layout()->textOption().textDirection() == Qt::RightToLeft)
                        parag.setRight(parag.right() + blockData->counterWidth() + blockData->counterSpacing());
                    else
                        parag.setLeft(blockData->counterPosition().x());
                }
                if(border) {
                    KoInsets insets;
                    border->applyInsets(insets, parag.top(), true);
                    parag.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
                }
            }
            if(!painter->hasClipping() || ! clipRegion.intersect(QRegion(parag.toRect())).isEmpty()) {
                started=true;
                painter->save();
                decorateParagraph(painter, block);
                painter->restore();
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
                listStyle == KoListStyle::CircleItem || listStyle == KoListStyle::BoxItem) {
            QFontMetricsF fm(cf.font(), paintDevice());
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
                case KoListStyle::SquareItem:
                    painter->fillRect(QRectF(x, y, width, width), QBrush(Qt::black));
                    break;
                case KoListStyle::DiscItem:
                    painter->setBrush(QBrush(Qt::black));
                    // fall through!
                case KoListStyle::CircleItem:
                    painter->drawEllipse(QRectF(x, y, width, width));
                    break;
                case KoListStyle::BoxItem:
                    painter->drawRect(QRectF(x, y, width, width));
                    break;
                default:; // others we ignore.
            }
        }
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
    KoInlineObject *obj = m_inlineTextObjectManager->inlineTextObject(cf);
    if(obj)
        obj->paint(*painter, paintDevice(), document(), rect, object, position, cf);
}

void KoTextDocumentLayout::positionInlineObject(QTextInlineObject item, int position, const QTextFormat &format) {
    Q_ASSERT(format.isCharFormat());
    if(m_inlineTextObjectManager == 0)
        return;
    QTextCharFormat cf = format.toCharFormat();
    KoInlineObject *obj = m_inlineTextObjectManager->inlineTextObject(cf);
    if(obj)
        obj->updatePosition(document(), item, position, cf);
}

void KoTextDocumentLayout::resizeInlineObject(QTextInlineObject item, int position, const QTextFormat &format) {
    Q_ASSERT(format.isCharFormat());
    if(m_inlineTextObjectManager == 0)
        return;
    QTextCharFormat cf = format.toCharFormat();
    KoInlineObject *obj = m_inlineTextObjectManager->inlineTextObject(cf);
    if(obj)
        obj->resize(document(), item, position, cf, paintDevice());
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

#include "KoTextDocumentLayout.moc"
