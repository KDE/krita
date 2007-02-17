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
    LayoutStateDummy() : m_styleManager(0) {}
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
    void draw(QPainter *, const QAbstractTextDocumentLayout::PaintContext &) {}
    KoStyleManager *styleManager() const { return m_styleManager; }
    void setStyleManager(KoStyleManager *sm) { m_styleManager = sm; }

    KoStyleManager *m_styleManager;
};

class KoTextDocumentLayout::Private {
public:
    Private()
        : inlineTextObjectManager(0),
        scheduled(false)
    {
    }

    QList<KoShape *> shapes;
    KoInlineTextObjectManager *inlineTextObjectManager;
    bool scheduled;
};

// ------------------- KoTextDocumentLayout --------------------
KoTextDocumentLayout::KoTextDocumentLayout(QTextDocument *doc, KoTextDocumentLayout::LayoutState *layout)
    : QAbstractTextDocumentLayout(doc),
    m_state(layout),
    d(new Private())
{
    setPaintDevice( new KoPostscriptPaintDevice() );
    if(m_state == 0)
        m_state = new LayoutStateDummy();
}

KoTextDocumentLayout::~KoTextDocumentLayout() {
    delete m_state;
    m_state = 0;
}

void KoTextDocumentLayout::setLayout(LayoutState *layout) {
    Q_ASSERT(layout);
    KoStyleManager *sm = 0;
    if(m_state)
        sm = m_state->styleManager();
    delete m_state;
    m_state = layout;
    m_state->setStyleManager(sm);
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
    d->shapes.append(shape);

    KoTextShapeData *data = dynamic_cast<KoTextShapeData*> (shape->userData());
    if(data) {
        data->faul();
        m_state->reset();
    }
}

void KoTextDocumentLayout::setStyleManager(KoStyleManager *sm) {
    m_state->setStyleManager(sm);
    if(document()->isEmpty()) {
        QTextBlock block = document()->begin();
        if(block.blockFormat().intProperty(KoParagraphStyle::StyleId) < 100)
            sm->defaultParagraphStyle()->applyStyle(block);
    }
}

void KoTextDocumentLayout::setInlineObjectTextManager(KoInlineTextObjectManager *iom) {
    d->inlineTextObjectManager = iom;
}

KoInlineTextObjectManager *KoTextDocumentLayout::inlineObjectTextManager() {
    return d->inlineTextObjectManager;
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
    m_state->draw(painter, context);
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
            position = block.position() + block.length() -1;
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
    if(d->inlineTextObjectManager == 0)
        return;
    QTextCharFormat cf = format.toCharFormat();
    KoInlineObject *obj = d->inlineTextObjectManager->inlineTextObject(cf);
    if(obj)
        obj->paint(*painter, paintDevice(), document(), rect, object, position, cf);
}

void KoTextDocumentLayout::positionInlineObject(QTextInlineObject item, int position, const QTextFormat &format) {
    Q_ASSERT(format.isCharFormat());
    if(d->inlineTextObjectManager == 0)
        return;
    QTextCharFormat cf = format.toCharFormat();
    KoInlineObject *obj = d->inlineTextObjectManager->inlineTextObject(cf);
    if(obj)
        obj->updatePosition(document(), item, position, cf);
}

void KoTextDocumentLayout::resizeInlineObject(QTextInlineObject item, int position, const QTextFormat &format) {
    Q_ASSERT(format.isCharFormat());
    if(d->inlineTextObjectManager == 0)
        return;
    QTextCharFormat cf = format.toCharFormat();
    KoInlineObject *obj = d->inlineTextObjectManager->inlineTextObject(cf);
    if(obj)
        obj->resize(document(), item, position, cf, paintDevice());
}

void KoTextDocumentLayout::scheduleLayout() {
    if(d->scheduled)
        return;
    d->scheduled = true;
    QTimer::singleShot(0, this, SLOT(relayout()));
}

void KoTextDocumentLayout::relayout() {
    layout();
}

void KoTextDocumentLayout::interruptLayout() {
    m_state->reset();
}

void KoTextDocumentLayout::layout() {
    d->scheduled = false;
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

QList<KoShape*> KoTextDocumentLayout::shapes() const {
    return d->shapes;
}

KoStyleManager *KoTextDocumentLayout::styleManager() const {
    return m_state->styleManager();
}

#include "KoTextDocumentLayout.moc"
