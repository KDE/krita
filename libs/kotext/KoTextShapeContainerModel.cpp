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

#include "KoTextShapeContainerModel.h"
#include "KoTextAnchor.h"
#include "KoTextShapeData.h"

#include <QTextBlock>
#include <QTextLayout>
#include <QTextLine>
#include <QTextDocument>

#include <KDebug>

class Relation {
public:
    Relation(KoShape *shape) :child(shape), anchor(0), nested(false) {}
    KoShape *child;
    KoTextAnchor *anchor;
    bool nested;
};

class KoTextShapeContainerModel::Private {
public:
    Private() : textShape(0) {}
    KoShapeContainer *textShape;
    QHash<KoShape*, Relation*> children;
};

KoTextShapeContainerModel::KoTextShapeContainerModel()
    : d(new Private())
{
}

void KoTextShapeContainerModel::add(KoShape *child) {
    if(d->children.contains(child))
        return;
    Relation *relation = new Relation(child);
    d->children.insert(child, relation);
}

void KoTextShapeContainerModel::remove(KoShape *child) {
    Relation *relation = d->children.value(child);
    delete relation;
    d->children.remove(child);
}

void KoTextShapeContainerModel::setClipping(const KoShape *child, bool clipping) {
    Relation *relation = d->children.value( const_cast<KoShape*>(child));
    Q_ASSERT(relation);
    relation->nested = clipping;
}

bool KoTextShapeContainerModel::childClipped(const KoShape *child) const {
    Relation *relation = d->children.value( const_cast<KoShape*>(child));
    Q_ASSERT(relation);
    return relation->nested;
}

int KoTextShapeContainerModel::count() const {
    return d->children.count();
}

QList<KoShape*> KoTextShapeContainerModel::iterator() const {
    return d->children.keys();
}

void KoTextShapeContainerModel::containerChanged(KoShapeContainer *container) {
    // TODO
    // For children which are aligned to the side of the page we may need to update the position so they will stay at the same vertical position.
}

void KoTextShapeContainerModel::addAnchor(KoTextAnchor *anchor) {
kDebug() << "addAnchor!\n";
    Relation *relation = d->children.value(anchor->shape());
    Q_ASSERT(relation);
    relation->anchor = anchor;
}

void KoTextShapeContainerModel::removeAnchor(KoTextAnchor *anchor) {
    Relation *relation = d->children.value(anchor->shape());
    if(relation)
        relation->anchor = 0;
}

void KoTextShapeContainerModel::reposition(KoShape *shape) {
kDebug() << "KoTextShapeContainerModel::reposition\n";
    Q_ASSERT(shape->parent());
    Relation *relation = d->children.value(shape);
kDebug() << "relation: " << relation << endl;
    if(relation == 0 || relation->anchor == 0)
        return;
kDebug() << "  still here\n";

    const int positionInDocument = relation->anchor->positionInDocument();
    QTextBlock block = relation->anchor->document()->findBlock( positionInDocument );
    QPointF newPosition;

// TODO rewrite the below to account for rotation etc.
    QRectF boundingRect = shape->boundingRect();
    QRectF containerBoundingRect = shape->parent()->boundingRect();
    switch(relation->anchor->horizontalAlignment()) {
        case KoTextAnchor::ClosestToBinding: // TODO figure out a way to do pages...
        case KoTextAnchor::Left:
            /*newPosition.setX(0);*/ break;
        case KoTextAnchor::FurtherFromBinding: // TODO figure out a way to do pages...
        case KoTextAnchor::Right:
            newPosition.setX(containerBoundingRect.width() - boundingRect.width());
            break;
        case KoTextAnchor::Center:
            newPosition.setX(containerBoundingRect.width() - boundingRect.width() / 2.0);
            break;
        case KoTextAnchor::HorizontalOffset: {
kDebug() << "  HorizontalOffset!\n";
            QTextLayout *layout = block.layout();
return;
            Q_ASSERT(layout);
            QTextLine tl = layout->lineForTextPosition(positionInDocument);
if(! tl.isValid()) return;
            Q_ASSERT(tl.isValid());
            double x = tl.cursorToX(positionInDocument - block.position());
            newPosition.setX(x + relation->anchor->offset().x());
kDebug() << "    x: " << newPosition.x() << endl;
            break;
        }
        default:
            Q_ASSERT(false); // new enum added?
    }
    switch(relation->anchor->verticalAlignment()) {
        case KoTextAnchor::TopOfFrame: /*newPosition.setY(0);*/ break;
        case KoTextAnchor::TopOfParagraph: {
            QTextLayout *layout = block.layout();
            double topOfParagraph = layout->lineAt(0).y();
            KoTextShapeData *data = dynamic_cast<KoTextShapeData*> (shape->parent()->userData());
            Q_ASSERT(data);
            newPosition.setY(topOfParagraph - data->documentOffset());
            break;
        }
        case KoTextAnchor::AboveCurrentLine: {
            QTextLayout *layout = block.layout();
            QTextLine tl = layout->lineForTextPosition(positionInDocument);
            newPosition.setY(tl.y() - boundingRect.height());
            break;
        }
        case KoTextAnchor::BelowCurrentLine: {
            QTextLayout *layout = block.layout();
            QTextLine tl = layout->lineForTextPosition(positionInDocument);
            newPosition.setY(tl.y() + tl.ascent() - boundingRect.height());
            break;
        }
        case KoTextAnchor::BottomOfParagraph: {
            QTextLayout *layout = block.layout();
            QTextLine tl = layout->lineAt(layout->lineCount()-1);
            newPosition.setY(tl.y() + tl.ascent() - boundingRect.height() );
            break;
        }
        case KoTextAnchor::BottomOfFrame:
            break;
        case KoTextAnchor::VerticalOffset: {
kDebug() << "  VerticalOffset!\n";
            QTextLayout *layout = block.layout();
            QTextLine tl = layout->lineForTextPosition(positionInDocument);
            newPosition.setX(tl.y() + tl.ascent() + relation->anchor->offset().y());
kDebug() << "    y: " << newPosition.y() << endl;
            break;
        }
        default:
            Q_ASSERT(false); // new enum added?
    }

    // TODO check for difference?  If the offset is tiny; just ignore it to avoid endless loops
    shape->setPosition(newPosition);
}

/* workflows

1) child shape moves
Position is determined by this model; so we need the shapeMoveStrategy to get vetoable coordinate changes through this model.
When we have that we can alter the anchor to the new alignment policies.  Which will do a relayout

2) text anchor moves
The anchor get notified.  The anchor tells us that we need to reposition the child.
  void reposition(anchor, shape);
Using the data from the anchor, we place the shape at the right position.

3) parent shape moves
Children move automatically; little to do.
For children which are aligned to the side of the page we may need to update the position so they will stay at the same vertical position.


Init
4) loading
??

5) inlining by user [done]
New anchor is created and added with the child as member.
When layout comes the anchor figures out which frame is the 'parent' and it adds the child.
This class will then get notified.
The anchor should also register itself with this model.
  void addAnchor(KoTextAnchor *)
point 2 is followed next.

*/
