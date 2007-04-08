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

void KoTextShapeContainerModel::proposeMove(KoShape *child, QPointF &move) {
    Relation *relation = d->children.value( child);
    if(relation == 0 || relation->anchor == 0)
        return;

    QPointF newPosition = child->position() + move;
    QRectF parentShapeRect(QPointF(0,0), child->parent()->size());
//kDebug() << "proposeMove: " << move << " | " << newPosition << " | " << parentShapeRect << endl;

    if(qAbs(newPosition.x()) < 10) // align left
        relation->anchor->setAlignment(KoTextAnchor::Left);
    else if(qAbs(parentShapeRect.width() - newPosition.x()) < 10.0)
        relation->anchor->setAlignment(KoTextAnchor::Right);
    else if(qAbs(parentShapeRect.width() / 2.0 - newPosition.x()) < 10.0)
        relation->anchor->setAlignment(KoTextAnchor::Center);
    /*else {
        relation->anchor->setAlignment(KoTextAnchor::HorizontalOffset);
        // TODO
        //QPointF offset = relation->anchor->offset();
        //offset.setX(offset.x() + move.x());
        //relation->anchor->setOffset(offset);
    } */

    if(qAbs(newPosition.y() < 10)) // TopOfFrame
{//kDebug() << "  TopOfFrame\n";
        relation->anchor->setAlignment(KoTextAnchor::TopOfFrame);
}
    else if(qAbs(parentShapeRect.height() - newPosition.y()) < 10.0)
{//kDebug() << "  BottomOfFrame\n";
        relation->anchor->setAlignment(KoTextAnchor::BottomOfFrame); // TODO
}
    else { // need layout info..
        QTextBlock block = relation->anchor->document()->findBlock(relation->anchor->positionInDocument());
        QTextLayout *layout = block.layout();
        if(layout->lineCount() > 0) {
            KoTextShapeData *data = dynamic_cast<KoTextShapeData*> (child->parent()->userData());
            QTextLine tl = layout->lineAt(0);
            double y = tl.y() - data->documentOffset() - newPosition.y();
            if(y >= 0 && y < 10)
{//kDebug() << "  TopOfParagraph " << y << "\n";
                relation->anchor->setAlignment(KoTextAnchor::TopOfParagraph);
}
            else {
                tl = layout->lineAt(layout->lineCount()-1);
                y = newPosition.y() - tl.y() - data->documentOffset() - tl.ascent();
                if(y >= 0 && y < 10)
{//kDebug() << "  BottomOfParagraph " << y << endl;
                    relation->anchor->setAlignment(KoTextAnchor::BottomOfParagraph); // TODO
}
                else {
                    tl = layout->lineForTextPosition(relation->anchor->positionInDocument() - block.position());
                    y = tl.y() - data->documentOffset() - newPosition.y();
                    if(y >= 0 && y < 10)
{//kDebug() << "  AboveCurrentLine\n";
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
