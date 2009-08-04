/* This file is part of the KDE project
 * Copyright (C) 2007,2009 Thomas Zander <zander@kde.org>
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
    Relation(KoShape *shape) : child(shape), anchor(0), nested(false) {}
    ~Relation();
    KoShape *child;
    KoTextAnchor *anchor;
    bool nested;
};


Relation::~Relation()
{
    delete anchor;
}

class KoTextShapeContainerModel::Private
{
public:
    Private() {}
    QHash<KoShape*, Relation*> children;
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
}

void KoTextShapeContainerModel::remove(KoShape *child)
{
    Relation *relation = d->children.value(child);
    d->children.remove(child);
    delete relation;
}

void KoTextShapeContainerModel::setClipping(const KoShape *child, bool clipping)
{
    Relation *relation = d->children.value(const_cast<KoShape*>(child));
    Q_ASSERT(relation);
    relation->nested = clipping;
}

bool KoTextShapeContainerModel::childClipped(const KoShape *child) const
{
    Relation *relation = d->children.value(const_cast<KoShape*>(child));
    Q_ASSERT(relation);
    return relation->nested;
}

int KoTextShapeContainerModel::count() const
{
    return d->children.count();
}

QList<KoShape*> KoTextShapeContainerModel::childShapes() const
{
    return d->children.keys();
}

void KoTextShapeContainerModel::containerChanged(KoShapeContainer *container)
{
    Q_UNUSED(container);
    kDebug(32500) << "KoTextShapeContainerModel::containerChanged";
    // TODO
    // For children which are aligned to the side of the page we may need to update the position so they will stay at the same vertical position.
}

void KoTextShapeContainerModel::childChanged(KoShape *child, KoShape::ChangeType type)
{
    if (type == KoShape::RotationChanged || type == KoShape::ScaleChanged ||
            type == KoShape::ShearChanged || type == KoShape::SizeChanged) {

        KoTextShapeData *data  = qobject_cast<KoTextShapeData*>(child->parent()->userData());
        Q_ASSERT(data);
        data->foul();

        KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*>(data->document()->documentLayout());
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
    if (relation)
        relation->anchor = 0;
}

void KoTextShapeContainerModel::proposeMove(KoShape *child, QPointF &move)
{
    Relation *relation = d->children.value(child);
    if (relation == 0 || relation->anchor == 0)
        return;

    QPointF newPosition = child->position() + move;
    QRectF parentShapeRect(QPointF(0, 0), child->parent()->size());
//kDebug(32500) <<"proposeMove:" << move <<" |" << newPosition <<" |" << parentShapeRect;

    if (qAbs(newPosition.x()) < 10) // align left
        relation->anchor->setAlignment(KoTextAnchor::Left);
    else if (qAbs(parentShapeRect.width() - newPosition.x()) < 10.0)
        relation->anchor->setAlignment(KoTextAnchor::Right);
    else if (qAbs(parentShapeRect.width() / 2.0 - newPosition.x()) < 10.0)
        relation->anchor->setAlignment(KoTextAnchor::Center);
    /*else {
        relation->anchor->setAlignment(KoTextAnchor::HorizontalOffset);
        // TODO
        //QPointF offset = relation->anchor->offset();
        //offset.setX(offset.x() + move.x());
        //relation->anchor->setOffset(offset);
    } */

    if (qAbs(newPosition.y()) < 10.0) // TopOfFrame
    {
        kDebug(32500) <<"  TopOfFrame";
        relation->anchor->setAlignment(KoTextAnchor::TopOfFrame);
    } else if (qAbs(parentShapeRect.height() - newPosition.y()) < 10.0) {
        kDebug(32500) <<"  BottomOfFrame";
        relation->anchor->setAlignment(KoTextAnchor::BottomOfFrame); // TODO
    } else { // need layout info..
        QTextBlock block = relation->anchor->document()->findBlock(relation->anchor->positionInDocument());
        QTextLayout *layout = block.layout();
        if (layout->lineCount() > 0) {
            KoTextShapeData *data = qobject_cast<KoTextShapeData*>(child->parent()->userData());
            Q_ASSERT(data);
            QTextLine tl = layout->lineAt(0);
            qreal y = tl.y() - data->documentOffset() - newPosition.y();
            if (y >= 0 && y < 10) {
                kDebug(32500) <<"  TopOfParagraph" << y <<"";
                relation->anchor->setAlignment(KoTextAnchor::TopOfParagraph);
            } else {
                tl = layout->lineAt(layout->lineCount() - 1);
                y = newPosition.y() - tl.y() - data->documentOffset() - tl.ascent();
                if (y >= 0 && y < 10) {
                    kDebug(32500) <<"  BottomOfParagraph" << y;
                    relation->anchor->setAlignment(KoTextAnchor::BottomOfParagraph); // TODO
                } else {
                    tl = layout->lineForTextPosition(relation->anchor->positionInDocument() - block.position());
                    y = tl.y() - data->documentOffset() - newPosition.y();
                    if (y >= 0 && y < 10) {
                        kDebug(32500) <<"  AboveCurrentLine";
                        relation->anchor->setAlignment(KoTextAnchor::AboveCurrentLine);
                    }
                    //else  do VerticalOffset here as well?
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
