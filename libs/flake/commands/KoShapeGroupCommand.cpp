/* This file is part of the KDE project
 * Copyright (C) 2006,2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006,2007 Jan Hambrecht <jaham@gmx.net>
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

#include "KoShapeGroupCommand.h"
#include "KoShapeGroupCommand_p.h"
#include "KoShape.h"
#include "KoShapeGroup.h"
#include "KoShapeContainer.h"

#include <klocale.h>

// static
KoShapeGroupCommand * KoShapeGroupCommand::createCommand(KoShapeGroup *container, const QList<KoShape *> &shapes, QUndoCommand *parent)
{
    QList<KoShape*> orderedShapes(shapes);
    qSort(orderedShapes.begin(), orderedShapes.end(), KoShape::compareShapeZIndex);
    if (!orderedShapes.isEmpty()) {
        KoShape * top = orderedShapes.last();
        container->setParent(top->parent());
        container->setZIndex(top->zIndex());
    }

    return new KoShapeGroupCommand(container, orderedShapes, parent);
}

KoShapeGroupCommandPrivate::KoShapeGroupCommandPrivate(KoShapeContainer *c, const QList<KoShape *> &s, const QList<bool> &clip, const QList<bool> &it)
    : shapes(s),
    clipped(clip),
    inheritTransform(it),
    container(c)
{
}


KoShapeGroupCommand::KoShapeGroupCommand(KoShapeContainer *container, const QList<KoShape *> &shapes, const QList<bool> &clipped, const QList<bool> &inheritTransform, QUndoCommand *parent)
    : QUndoCommand(parent),
    d(new KoShapeGroupCommandPrivate(container,shapes, clipped, inheritTransform))
{
    Q_ASSERT(d->clipped.count() == d->shapes.count());
    Q_ASSERT(d->inheritTransform.count() == d->shapes.count());
    d->init(this);
}

KoShapeGroupCommand::KoShapeGroupCommand(KoShapeGroup *container, const QList<KoShape *> &shapes, QUndoCommand *parent)
    : QUndoCommand(parent),
    d(new KoShapeGroupCommandPrivate(container,shapes))
{
    for (int i = 0; i < shapes.count(); ++i) {
        d->clipped.append(false);
        d->inheritTransform.append(false);
    }
    d->init(this);
}

KoShapeGroupCommand::~KoShapeGroupCommand()
{
    delete d;
}

KoShapeGroupCommand::KoShapeGroupCommand(KoShapeGroupCommandPrivate &dd, QUndoCommand *parent)
    : QUndoCommand(parent),
    d(&dd)
{
}

void KoShapeGroupCommandPrivate::init(QUndoCommand *q)
{
    foreach(KoShape* shape, shapes) {
        oldParents.append(shape->parent());
        oldClipped.append(shape->parent() && shape->parent()->isClipped(shape));
        oldInheritTransform.append(shape->parent() && shape->parent()->inheritsTransform(shape));
        oldZIndex.append(shape->zIndex());
    }

    if (container->shapes().isEmpty()) {
        q->setText(i18n("Group shapes"));
    } else {
        q->setText(i18n("Add shapes to group"));
    }
}

void KoShapeGroupCommand::redo()
{
    QUndoCommand::redo();

    if (dynamic_cast<KoShapeGroup*>(d->container)) {
        QRectF bound = d->containerBoundingRect();
        QPointF oldGroupPosition = d->container->absolutePosition(KoFlake::TopLeftCorner);
        d->container->setAbsolutePosition(bound.topLeft(), KoFlake::TopLeftCorner);
        d->container->setSize(bound.size());

        if (d->container->shapeCount() > 0) {
            // the group has changed position and so have the group child shapes
            // -> we need compensate the group position change
            QPointF positionOffset = oldGroupPosition - bound.topLeft();
            foreach(KoShape * child, d->container->shapes())
                child->setAbsolutePosition(child->absolutePosition() + positionOffset);
        }
    }

    QMatrix groupTransform = d->container->absoluteTransformation(0).inverted();

    int zIndex=0;
    QList<KoShape*> shapes(d->container->shapes());
    if (!shapes.isEmpty()) {
        qSort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
        zIndex = shapes.last()->zIndex();
    }

    uint shapeCount = d->shapes.count();
    for (uint i = 0; i < shapeCount; ++i) {
        KoShape * shape = d->shapes[i];
        shape->setZIndex(zIndex++);
        shape->applyAbsoluteTransformation(groupTransform);
        d->container->addShape(shape);
        d->container->setClipped(shape, d->clipped[i]);
        d->container->setInheritsTransform(shape, d->inheritTransform[i]);
    }
}

void KoShapeGroupCommand::undo()
{
    QUndoCommand::undo();

    QMatrix ungroupTransform = d->container->absoluteTransformation(0);
    for (int i = 0; i < d->shapes.count(); i++) {
        KoShape * shape = d->shapes[i];
        d->container->removeShape(shape);
        if (d->oldParents.at(i)) {
            d->oldParents.at(i)->addShape(shape);
            d->oldParents.at(i)->setClipped(shape, d->oldClipped.at(i));
            d->oldParents.at(i)->setInheritsTransform(shape, d->oldInheritTransform.at(i));
        }
        shape->applyAbsoluteTransformation(ungroupTransform);
        shape->setZIndex(d->oldZIndex[i]);
    }

    if (dynamic_cast<KoShapeGroup*>(d->container)) {
        QPointF oldGroupPosition = d->container->absolutePosition(KoFlake::TopLeftCorner);
        if (d->container->shapeCount() > 0) {
            bool boundingRectInitialized = false;
            QRectF bound;
            foreach(KoShape * shape, d->container->shapes()) {
                if (! boundingRectInitialized) {
                    bound = shape->boundingRect();
                    boundingRectInitialized = true;
                } else
                    bound = bound.unite(shape->boundingRect());
            }
            // the group has changed position and so have the group child shapes
            // -> we need compensate the group position change
            QPointF positionOffset = oldGroupPosition - bound.topLeft();
            foreach(KoShape * child, d->container->shapes())
                child->setAbsolutePosition(child->absolutePosition() + positionOffset);

            d->container->setAbsolutePosition(bound.topLeft(), KoFlake::TopLeftCorner);
            d->container->setSize(bound.size());
        }
    }
}

QRectF KoShapeGroupCommandPrivate::containerBoundingRect()
{
    bool boundingRectInitialized = true;
    QRectF bound;
    if (container->shapeCount() > 0)
        bound = container->boundingRect();
    else
        boundingRectInitialized = false;

    foreach(KoShape *shape, shapes) {
        if (boundingRectInitialized)
            bound = bound.unite(shape->boundingRect());
        else {
            bound = shape->boundingRect();
            boundingRectInitialized = true;
        }
    }
    return bound;
}
