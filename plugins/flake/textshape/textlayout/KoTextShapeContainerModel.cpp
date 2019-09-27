/* This file is part of the KDE project
 * Copyright (C) 2007,2009,2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 C. Boemann <cbo@kogmbh.com>
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

#include "KoTextShapeContainerModel.h"

#include "KoAnchorInlineObject.h"
#include "KoTextShapeData.h"
#include "KoShapeContainer.h"

#include <QTextBlock>
#include <QTextLayout>
#include <QTextLine>
#include <QTextDocument>

#include <TextLayoutDebug.h>

struct Relation
{
    Relation(KoShape *shape = 0)
        : child(shape),
        anchor(0),
        nested(false),
        inheritsTransform(false)
    {
    }
    KoShape *child;
    KoShapeAnchor *anchor;
    uint nested : 1;
    uint inheritsTransform :1;
};

class Q_DECL_HIDDEN KoTextShapeContainerModel::Private
{
public:
    QHash<const KoShape*, Relation> children;
    QList<KoShapeAnchor *> shapeRemovedAnchors;
};

KoTextShapeContainerModel::KoTextShapeContainerModel()
        : d(new Private())
{
}

KoTextShapeContainerModel::~KoTextShapeContainerModel()
{
    delete d;
}

void KoTextShapeContainerModel::add(KoShape *child)
{
    if (d->children.contains(child))
        return;
    Relation relation(child);
    d->children.insert(child, relation);

    KoShapeAnchor *toBeAddedAnchor = 0;
    foreach (KoShapeAnchor *anchor, d->shapeRemovedAnchors) {
        if (child == anchor->shape()) {
            toBeAddedAnchor = anchor;
            break;
        }
    }

    if (toBeAddedAnchor) {
        addAnchor(toBeAddedAnchor);
        d->shapeRemovedAnchors.removeAll(toBeAddedAnchor);
    }
}

void KoTextShapeContainerModel::remove(KoShape *child)
{
    Relation relation = d->children.value(child);
    d->children.remove(child);
    if (relation.anchor) {
        relation.anchor->placementStrategy()->detachFromModel();
        d->shapeRemovedAnchors.append(relation.anchor);
    }
}

void KoTextShapeContainerModel::setClipped(const KoShape *child, bool clipping)
{
    Q_ASSERT(d->children.contains(child));
    d->children[child].nested = clipping;
}

bool KoTextShapeContainerModel::isClipped(const KoShape *child) const
{
    Q_ASSERT(d->children.contains(child));
    return d->children[child].nested;
}

void KoTextShapeContainerModel::setInheritsTransform(const KoShape *shape, bool inherit)
{
    Q_ASSERT(d->children.contains(shape));
    d->children[shape].inheritsTransform = inherit;
}

bool KoTextShapeContainerModel::inheritsTransform(const KoShape *shape) const
{
    Q_ASSERT(d->children.contains(shape));
    return d->children[shape].inheritsTransform;
}


int KoTextShapeContainerModel::count() const
{
    return d->children.count();
}

QList<KoShape*> KoTextShapeContainerModel::shapes() const
{
    QList<KoShape*> answer;
    answer.reserve(d->children.count());
    foreach (const Relation &relation, d->children) {
        answer << relation.child;
    }
    return answer;
}

void KoTextShapeContainerModel::containerChanged(KoShapeContainer *container, KoShape::ChangeType type)
{
    Q_UNUSED(container);
    Q_UNUSED(type);
}

void KoTextShapeContainerModel::childChanged(KoShape *child, KoShape::ChangeType type)
{
    if (((type == KoShape::RotationChanged ||
          type == KoShape::ScaleChanged ||
          type == KoShape::ShearChanged ||
          type == KoShape::ClipPathChanged ||
          type == KoShape::PositionChanged ||
          type == KoShape::SizeChanged) && child->textRunAroundSide() != KoShape::RunThrough) ||
          type == KoShape::TextRunAroundChanged) {

        relayoutInlineObject(child);
    }
    KoShapeContainerModel::childChanged( child, type );
}

void KoTextShapeContainerModel::addAnchor(KoShapeAnchor *anchor)
{
    Q_ASSERT(anchor);
    Q_ASSERT(anchor->shape());
    Q_ASSERT(d->children.contains(anchor->shape()));
    d->children[anchor->shape()].anchor = anchor;
}

void KoTextShapeContainerModel::removeAnchor(KoShapeAnchor *anchor)
{
    if (d->children.contains(anchor->shape())) {
        d->children[anchor->shape()].anchor = 0;
        d->shapeRemovedAnchors.removeAll(anchor);
    }
}

void KoTextShapeContainerModel::proposeMove(KoShape *child, QPointF &move)
{
    if (!d->children.contains(child))
        return;
    Relation relation = d->children.value(child);
    if (relation.anchor == 0)
        return;

    QPointF newPosition = child->position() + move/* + relation.anchor->offset()*/;
    const QRectF parentShapeRect(QPointF(0, 0), child->parent()->size());
//warnTextLayout <<"proposeMove:" /*<< move <<" |"*/ << newPosition <<" |" << parentShapeRect;

    QTextLayout *layout = 0;
    int anchorPosInParag = -1;

    if (relation.anchor->anchorType() == KoShapeAnchor::AnchorAsCharacter) {
        int posInDocument = relation.anchor->textLocation()->position();
        const QTextDocument *document = relation.anchor->textLocation()->document();
        QTextBlock block = document->findBlock(posInDocument);
        layout = block.layout();
        anchorPosInParag = posInDocument - block.position();
        if (layout) {
            QTextLine tl = layout->lineForTextPosition(anchorPosInParag);
            Q_ASSERT(tl.isValid());
            relation.anchor->setOffset(QPointF(newPosition.x() - tl.cursorToX(anchorPosInParag)
                + tl.x(), 0));
            relayoutInlineObject(child);
        }

        // the rest of the code uses the shape baseline, at this time the bottom. So adjust
        newPosition.setY(newPosition.y() + child->size().height());
        if (layout == 0) {
            QTextBlock block = document->findBlock(posInDocument);
            layout = block.layout();
            anchorPosInParag = posInDocument - block.position();
        }
        if (layout->lineCount() > 0) {
            KoTextShapeData *data = qobject_cast<KoTextShapeData*>(child->parent()->userData());
            Q_ASSERT(data);
            QTextLine tl = layout->lineForTextPosition(anchorPosInParag);
            Q_ASSERT(tl.isValid());
            qreal y = tl.y() - data->documentOffset() - newPosition.y() + child->size().height();
            relation.anchor->setOffset(QPointF(relation.anchor->offset().x(), -y));
            relayoutInlineObject(child);
        }
    } else {
        //TODO pavolk: handle position type change: absolute to relative, etc ..
        child->setPosition(newPosition);
        relation.anchor->setOffset(relation.anchor->offset() + move);
        relayoutInlineObject(child);
    }

    move.setX(0); // let the text layout move it.
    move.setY(0);
}

void KoTextShapeContainerModel::relayoutInlineObject(KoShape *child)
{
    if (child == 0) {
        return;
    }
    KoTextShapeData *data  = qobject_cast<KoTextShapeData*>(child->parent()->userData());
    Q_ASSERT(data);
    data->setDirty();
}

