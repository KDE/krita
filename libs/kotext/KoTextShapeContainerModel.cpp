/* This file is part of the KDE project
 * Copyright (C) 2007,2009,2010 Thomas Zander <zander@kde.org>
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
#include "KoTextAnchor.h"
#include "KoTextShapeData.h"
#include "KoTextDocumentLayout.h"

#include <QTextBlock>
#include <QTextLayout>
#include <QTextLine>
#include <QTextDocument>

#include <KDebug>

class Relation
{
public:
    Relation(KoShape *shape) : child(shape), anchor(0), nested(false), inheritsTransform(false) {}
    ~Relation();
    KoShape *child;
    KoTextAnchor *anchor;
    uint nested : 1;
    uint inheritsTransform :1;
};


Relation::~Relation()
{
}

class KoTextShapeContainerModel::Private
{
public:
    QHash<const KoShape*, Relation*> children;
    QList<KoTextAnchor *> shapeRemovedAnchors;
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
    Relation *relation = new Relation(child);
    d->children.insert(child, relation);
   
    KoTextAnchor *toBeAddedAnchor = 0; 
    foreach (KoTextAnchor *anchor, d->shapeRemovedAnchors) {
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
    Relation *relation = d->children.value(child);
    d->children.remove(child);
    if (relation && relation->anchor)
        d->shapeRemovedAnchors.append(relation->anchor);
    delete relation;
}

void KoTextShapeContainerModel::setClipped(const KoShape *child, bool clipping)
{
    Relation *relation = d->children.value(child);
    Q_ASSERT(relation);
    relation->nested = clipping;
}

bool KoTextShapeContainerModel::isClipped(const KoShape *child) const
{
    Relation *relation = d->children.value(child);
    Q_ASSERT(relation);
    return relation->nested;
}

void KoTextShapeContainerModel::setInheritsTransform(const KoShape *shape, bool inherit)
{
    Relation *relation = d->children.value(shape);
    Q_ASSERT(relation);
    relation->inheritsTransform = inherit;
}

bool KoTextShapeContainerModel::inheritsTransform(const KoShape *shape) const
{
    Relation *relation = d->children.value(shape);
    Q_ASSERT(relation);
    return relation->inheritsTransform;
}


int KoTextShapeContainerModel::count() const
{
    return d->children.count();
}

QList<KoShape*> KoTextShapeContainerModel::shapes() const
{
    QList<KoShape*> answer;
    // when Qt 4.7 is more widespread, use answer.reserve(d->children.count());
    foreach (Relation *relation, d->children) {
        answer << relation->child;
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
    if (type == KoShape::RotationChanged || type == KoShape::ScaleChanged ||
            type == KoShape::ShearChanged || type == KoShape::SizeChanged) {

        KoTextShapeData *data  = qobject_cast<KoTextShapeData*>(child->parent()->userData());
        Q_ASSERT(data);
        data->foul();

        KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(data->document()->documentLayout());
        if (lay)
            lay->interruptLayout();
        data->fireResizeEvent();
    }
    KoShapeContainerModel::childChanged( child, type );
}

void KoTextShapeContainerModel::addAnchor(KoTextAnchor *anchor)
{
    Relation *relation = d->children.value(anchor->shape());
    Q_ASSERT(relation);
    relation->anchor = anchor;
}

void KoTextShapeContainerModel::removeAnchor(KoTextAnchor *anchor)
{
    Relation *relation = d->children.value(anchor->shape());
    if (relation) {
        relation->anchor = 0;
        d->shapeRemovedAnchors.removeAll(anchor);
    }
}

void KoTextShapeContainerModel::proposeMove(KoShape *child, QPointF &move)
{
    Relation *relation = d->children.value(child);
    if (relation == 0 || relation->anchor == 0)
        return;

    QPointF newPosition = child->position() + move/* + relation->anchor->offset()*/;
    const QRectF parentShapeRect(QPointF(0, 0), child->parent()->size());
//kDebug(32500) <<"proposeMove:" /*<< move <<" |"*/ << newPosition <<" |" << parentShapeRect;

    QTextLayout *layout = 0;
    int anchorPosInParag = -1;
    if (qAbs(newPosition.x()) < 10) { // align left
        relation->anchor->setAlignment(KoTextAnchor::Left);
        relation->anchor->setOffset(QPointF(0, relation->anchor->offset().y()));
    } else if (qAbs(parentShapeRect.width() - newPosition.x() - child->size().width()) < 10.0) {
        relation->anchor->setAlignment(KoTextAnchor::Right);
        relation->anchor->setOffset(QPointF(0, relation->anchor->offset().y()));
    } else if (qAbs(parentShapeRect.width() / 2.0 - (newPosition.x() + child->size().width() / 2.0)) < 10.0) {
        relation->anchor->setAlignment(KoTextAnchor::Center);
        relation->anchor->setOffset(QPointF(0, relation->anchor->offset().y()));
    } else {
        relation->anchor->setAlignment(KoTextAnchor::HorizontalOffset);
        QTextBlock block = relation->anchor->document()->findBlock(relation->anchor->positionInDocument());
        layout = block.layout();
        anchorPosInParag = relation->anchor->positionInDocument() - block.position();
        QTextLine tl = layout->lineForTextPosition(anchorPosInParag);
        relation->anchor->setOffset(QPointF(newPosition.x() - tl.cursorToX(anchorPosInParag) + tl.x(),
                    relation->anchor->offset().y()));
    }

    if (qAbs(newPosition.y()) < 10.0) { // TopOfFrame
        kDebug(32500) <<"  TopOfFrame";
        relation->anchor->setAlignment(KoTextAnchor::TopOfFrame);
        relation->anchor->setOffset(QPointF(relation->anchor->offset().x(), 0));
    } else if (qAbs(parentShapeRect.height() - newPosition.y()) < 10.0) {
        kDebug(32500) <<"  BottomOfFrame";
        relation->anchor->setAlignment(KoTextAnchor::BottomOfFrame); // TODO
        relation->anchor->setOffset(QPointF(relation->anchor->offset().x(), 0));
    } else { // need layout info..
        relation->anchor->setOffset(QPointF(relation->anchor->offset().x(), 0));
        // the rest of the code uses the shape baseline, at this time the bottom. So adjust
        newPosition.setY(newPosition.y() + child->size().height());
        if (layout == 0) {
            QTextBlock block = relation->anchor->document()->findBlock(relation->anchor->positionInDocument());
            layout = block.layout();
            anchorPosInParag = relation->anchor->positionInDocument() - block.position();
        }
        if (layout->lineCount() > 0) {
            KoTextShapeData *data = qobject_cast<KoTextShapeData*>(child->parent()->userData());
            Q_ASSERT(data);
            QTextLine tl = layout->lineAt(0);
            qreal y = tl.y() - data->documentOffset() - newPosition.y() + child->size().height();
            if (y >= -5 && y < 10) {
                kDebug(32500) <<"  TopOfParagraph" << y;
                relation->anchor->setAlignment(KoTextAnchor::TopOfParagraph);
            } else {
                tl = layout->lineAt(layout->lineCount() - 1);
                y = newPosition.y() - tl.y() - data->documentOffset() - tl.ascent() - child->size().height();
                if (y >= 0 && y < 10) {
                    kDebug(32500) <<"  BottomOfParagraph" << y;
                    relation->anchor->setAlignment(KoTextAnchor::BottomOfParagraph); // TODO
                } else {
                    tl = layout->lineForTextPosition(anchorPosInParag);
                    y = tl.y() - data->documentOffset() - newPosition.y() + child->size().height();
                    if (y >= 0 && y < 10) {
                        kDebug(32500) <<"  AboveCurrentLine";
                        relation->anchor->setAlignment(KoTextAnchor::AboveCurrentLine);
                    }
                    else {
                        relation->anchor->setAlignment(KoTextAnchor::VerticalOffset);
                        relation->anchor->setOffset(QPointF(relation->anchor->offset().x(), -y));
                    }
                }
            }
        }
    }

    move.setX(0); // let the text layout move it.
    move.setY(0);
}

bool KoTextShapeContainerModel::isChildLocked(const KoShape *child) const
{
    return child->isGeometryProtected();
}
