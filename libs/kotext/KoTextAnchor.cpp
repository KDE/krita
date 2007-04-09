/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "KoTextAnchor.h"
#include "KoTextDocumentLayout.h"
#include "KoTextShapeContainerModel.h"

#include <KoShapeContainer.h>

#include <QTextInlineObject>
#include <QFontMetricsF>
#include <KDebug>

class KoTextAnchor::Private {
public:
    Private(KoTextAnchor *p, KoShape *s)
        : parent(p),
        shape(s),
        horizontalAlignment(HorizontalOffset),
        verticalAlignment(VerticalOffset),
        document(0),
        position(-1),
        model(0)
    {
    }

    void relayout() {
        if(document) {
            KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*> (document->documentLayout());
            if(lay)
                lay->documentChanged(position, 0, 0);
        }
    }

    void setContainer(KoShapeContainer *container) {
        if(container == 0) {
            model = 0;
            return;
        }
        bool first = model == 0; // first time
        model = dynamic_cast<KoTextShapeContainerModel*> (container->model());
        if(first)
            model->addAnchor(parent);
    }

    KoTextAnchor * const parent;
    KoShape * const shape;
    AnchorHorizontal horizontalAlignment;
    AnchorVertical verticalAlignment;
    const QTextDocument *document;
    int position;
    KoTextShapeContainerModel *model;
    QPointF distance;
};

KoTextAnchor::KoTextAnchor(KoShape *shape)
    : KoInlineObject(false),
    d(new Private(this, shape))
{
}

KoTextAnchor::~KoTextAnchor() {
    if(d->model)
        d->model->removeAnchor(this);
    delete d;
}

KoShape *KoTextAnchor::shape() const {
    return d->shape;
}

void KoTextAnchor::setAlignment(KoTextAnchor::AnchorHorizontal horizontal) {
    if(d->horizontalAlignment == horizontal)
        return;
    d->horizontalAlignment = horizontal;
    d->relayout();
}

void KoTextAnchor::setAlignment(KoTextAnchor::AnchorVertical vertical) {
    if(d->verticalAlignment == vertical)
        return;
    d->verticalAlignment = vertical;
    d->relayout();
}

KoTextAnchor::AnchorVertical KoTextAnchor::verticalAlignment() const {
    return d->verticalAlignment;
}

KoTextAnchor::AnchorHorizontal KoTextAnchor::horizontalAlignment() const {
    return d->horizontalAlignment;
}

void KoTextAnchor::updatePosition(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format) {
    Q_UNUSED(object);
    Q_UNUSED(format);
    d->document = document;
    d->position = posInDocument;
    d->setContainer(dynamic_cast<KoShapeContainer*> (shapeForPosition(document, posInDocument)));
}

void KoTextAnchor::resize(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format, QPaintDevice *pd) {
    Q_UNUSED(document);
    Q_UNUSED(object);
    Q_UNUSED(posInDocument);
    Q_UNUSED(format);
    Q_UNUSED(pd);

    if(horizontalAlignment() == HorizontalOffset && verticalAlignment() == VerticalOffset) {
        object.setWidth(d->shape->size().width());
        object.setAscent(d->shape->size().height());
    }
    else {
        QFontMetricsF fm(format.font());
        object.setWidth(0);
        object.setAscent(fm.ascent());
    }
    object.setDescent(0);
}

void KoTextAnchor::paint (QPainter &painter, QPaintDevice *pd, const QTextDocument *document, const QRectF &rect, QTextInlineObject object, int posInDocument, const QTextCharFormat &format) {
    Q_UNUSED(document);
    Q_UNUSED(painter);
    Q_UNUSED(pd);
    Q_UNUSED(rect);
    Q_UNUSED(object);
    Q_UNUSED(posInDocument);
    Q_UNUSED(format);
    // we never paint ourselves; the shape can do that.
}

int KoTextAnchor::positionInDocument() const {
    return d->position;
}

const QTextDocument *KoTextAnchor::document() const {
    return d->document;
}

const QPointF &KoTextAnchor::offset() const {
    return d->distance;
}

void KoTextAnchor::setOffset(const QPointF &offset) {
    if(d->distance == offset)
        return;
    d->distance = offset;
    d->relayout();
}

