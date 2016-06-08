/* This file is part of the KDE project
 * Copyright (C) 2007, 2009-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 Ko Gmbh <cbo@kogmbh.com>
 * Copyright (C) 2011 Matus Hanzes <matus.hanzes@ixonos.com>
 * Copyright (C) 2013 C. Boemann <cbo@boemann.dk>
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

#include "KoAnchorInlineObject.h"
#include "KoInlineObject_p.h"
#include "KoShapeAnchor.h"

#include <KoShapeSavingContext.h>
#include <KoShapeLoadingContext.h>
#include <KoShape.h>

#include <QTextInlineObject>
#include <QFontMetricsF>
#include "TextDebug.h"

// #define DEBUG_PAINTING

class KoAnchorInlineObjectPrivate : public KoInlineObjectPrivate
{
public:
    KoAnchorInlineObjectPrivate(KoShapeAnchor *p)
        : parent(p)
        , document(0)
        , position(-1)
        , inlineObjectAscent(0)
        , inlineObjectDescent(0)
    {
    }

    KoShapeAnchor *parent;
    const QTextDocument *document;
    int position;
    QTextCharFormat format;
    qreal inlineObjectAscent;
    qreal inlineObjectDescent;
};

KoAnchorInlineObject::KoAnchorInlineObject(KoShapeAnchor *parent)
    : KoInlineObject(*(new KoAnchorInlineObjectPrivate(parent)), false)
{
    Q_ASSERT(parent);
    parent->setTextLocation(this);
}

KoAnchorInlineObject::~KoAnchorInlineObject()
{
}

KoShapeAnchor *KoAnchorInlineObject::anchor() const
{
    Q_D(const KoAnchorInlineObject);
    return d->parent;
}

void KoAnchorInlineObject::updatePosition(const QTextDocument *document, int posInDocument, const QTextCharFormat &format)
{
    Q_D(KoAnchorInlineObject);
    d->document = document;
    d->position = posInDocument;
    d->format = format;
    if (d->parent->placementStrategy() != 0) {
        d->parent->placementStrategy()->updateContainerModel();
    }
}

void KoAnchorInlineObject::resize(const QTextDocument *document, QTextInlineObject &object, int posInDocument, const QTextCharFormat &format, QPaintDevice *pd)
{
    Q_UNUSED(document);
    Q_UNUSED(posInDocument);
    Q_D(KoAnchorInlineObject);

    if (!d->parent->shape()->isVisible()) {
        // Per default the shape this anchor presents is hidden and we only make it visible once an explicit resize-request
        // was made. This prevents shapes that are anchored at e.g. hidden textboxes to not become visible as long as they
        // are not asked to resize.
        d->parent->shape()->setVisible(true);
    }

    // important detail; top of anchored shape is at the baseline.
    QFontMetricsF fm(format.font(), pd);
    if (d->parent->anchorType() == KoShapeAnchor::AnchorAsCharacter) {
        QPointF offset = d->parent->offset();
        offset.setX(0);
        d->parent->setOffset(offset);
        object.setWidth(d->parent->shape()->size().width());
        if (d->parent->verticalRel() == KoShapeAnchor::VBaseline) {
            // baseline implies special meaning of the position attribute:
            switch (d->parent->verticalPos()) {
            case KoShapeAnchor::VFromTop:
                object.setAscent(qMax((qreal) 0, -offset.y()));
                object.setDescent(qMax((qreal) 0, d->parent->shape()->size().height() + offset.y()));
                break;
            case KoShapeAnchor::VTop:
                object.setAscent(d->parent->shape()->size().height());
                object.setDescent(0);
                break;
            case KoShapeAnchor::VMiddle:
                object.setAscent(d->parent->shape()->size().height()/2);
                object.setDescent(d->parent->shape()->size().height()/2);
                break;
            case KoShapeAnchor::VBottom:
                object.setAscent(0);
                object.setDescent(d->parent->shape()->size().height());
                break;
            default:
                break;
            }
        } else {
            qreal boundTop = fm.ascent();
            switch (d->parent->verticalPos()) {
            case KoShapeAnchor::VFromTop:
                 object.setAscent(qMax((qreal) 0, -offset.y()));
                 object.setDescent(qMax((qreal) 0, d->parent->shape()->size().height() + offset.y()));
                 break;
            case KoShapeAnchor::VTop:
                object.setAscent(boundTop);
                object.setDescent(qMax((qreal) 0, d->parent->shape()->size().height() - boundTop));
                break;
            case KoShapeAnchor::VMiddle:
                object.setAscent(d->parent->shape()->size().height()/2);
                object.setDescent(d->parent->shape()->size().height()/2);
                break;
            case KoShapeAnchor::VBottom:
                object.setAscent(0);
                object.setDescent(d->parent->shape()->size().height());
                break;
            default:
                break;
            }
        }
        d->inlineObjectAscent = object.ascent();
        d->inlineObjectDescent = object.descent();
    } else {
        object.setWidth(0);
        object.setAscent(0);
        object.setDescent(0);
    }
}

void KoAnchorInlineObject::paint(QPainter &, QPaintDevice *, const QTextDocument *, const QRectF &, const QTextInlineObject &, int , const QTextCharFormat &)
{
}

int KoAnchorInlineObject::position() const
{
    Q_D(const KoAnchorInlineObject);
    return d->position;
}

const QTextDocument *KoAnchorInlineObject::document() const
{
    Q_D(const KoAnchorInlineObject);
    return d->document;
}

qreal KoAnchorInlineObject::inlineObjectAscent() const
{
    Q_D(const KoAnchorInlineObject);
    return d->inlineObjectAscent;
}

qreal KoAnchorInlineObject::inlineObjectDescent() const
{
    Q_D(const KoAnchorInlineObject);
    return d->inlineObjectDescent;
}

bool KoAnchorInlineObject::loadOdf(const KoXmlElement &, KoShapeLoadingContext &)
{
    // do nothing
    return true;
}

void KoAnchorInlineObject::saveOdf(KoShapeSavingContext &context)
{
    Q_D(KoAnchorInlineObject);
    d->parent->saveOdf(context);
}
