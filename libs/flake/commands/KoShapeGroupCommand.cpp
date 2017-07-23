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

#include <klocalizedstring.h>

// static
KoShapeGroupCommand * KoShapeGroupCommand::createCommand(KoShapeGroup *container, const QList<KoShape *> &shapes, KUndo2Command *parent)
{
    QList<KoShape*> orderedShapes(shapes);
    std::sort(orderedShapes.begin(), orderedShapes.end(), KoShape::compareShapeZIndex);
    if (!orderedShapes.isEmpty()) {
        KoShape * top = orderedShapes.last();
        container->setParent(top->parent());
        container->setZIndex(top->zIndex());
    }

    return new KoShapeGroupCommand(container, orderedShapes, parent);
}

KoShapeGroupCommandPrivate::KoShapeGroupCommandPrivate(KoShapeContainer *c, const QList<KoShape *> &s, const QList<bool> &clip, const QList<bool> &it, bool _shouldNormalize)
    : shapes(s),
    clipped(clip),
    inheritTransform(it),
    shouldNormalize(_shouldNormalize),
    container(c)

{
}


KoShapeGroupCommand::KoShapeGroupCommand(KoShapeContainer *container, const QList<KoShape *> &shapes, const QList<bool> &clipped, const QList<bool> &inheritTransform, KUndo2Command *parent)
    : KUndo2Command(parent),
    d(new KoShapeGroupCommandPrivate(container,shapes, clipped, inheritTransform, true))
{
    Q_ASSERT(d->clipped.count() == d->shapes.count());
    Q_ASSERT(d->inheritTransform.count() == d->shapes.count());
    d->init(this);
}

KoShapeGroupCommand::KoShapeGroupCommand(KoShapeGroup *container, const QList<KoShape *> &shapes, KUndo2Command *parent)
    : KUndo2Command(parent),
    d(new KoShapeGroupCommandPrivate(container,shapes, QList<bool>(), QList<bool>(), true))
{
    for (int i = 0; i < shapes.count(); ++i) {
        d->clipped.append(false);
        d->inheritTransform.append(false);
    }
    d->init(this);
}

KoShapeGroupCommand::KoShapeGroupCommand(KoShapeContainer *container, const QList<KoShape *> &shapes,
                                         bool clipped, bool inheritTransform, bool shouldNormalize, KUndo2Command *parent)
    : KUndo2Command(parent),
      d(new KoShapeGroupCommandPrivate(container,shapes, QList<bool>(), QList<bool>(), shouldNormalize))
{
    for (int i = 0; i < shapes.count(); ++i) {
        d->clipped.append(clipped);
        d->inheritTransform.append(inheritTransform);
    }
    d->init(this);
}

KoShapeGroupCommand::~KoShapeGroupCommand()
{
    delete d;
}

KoShapeGroupCommand::KoShapeGroupCommand(KoShapeGroupCommandPrivate &dd, KUndo2Command *parent)
    : KUndo2Command(parent),
    d(&dd)
{
}

void KoShapeGroupCommandPrivate::init(KUndo2Command *q)
{
    Q_FOREACH (KoShape* shape, shapes) {
        oldParents.append(shape->parent());
        oldClipped.append(shape->parent() && shape->parent()->isClipped(shape));
        oldInheritTransform.append(shape->parent() && shape->parent()->inheritsTransform(shape));
        oldZIndex.append(shape->zIndex());
    }

    if (container->shapes().isEmpty()) {
        q->setText(kundo2_i18n("Group shapes"));
    } else {
        q->setText(kundo2_i18n("Add shapes to group"));
    }
}

void KoShapeGroupCommand::redo()
{
    KUndo2Command::redo();

    if (d->shouldNormalize &&  dynamic_cast<KoShapeGroup*>(d->container)) {
        QRectF bound = d->containerBoundingRect();
        QPointF oldGroupPosition = d->container->absolutePosition(KoFlake::TopLeft);
        d->container->setAbsolutePosition(bound.topLeft(), KoFlake::TopLeft);
        d->container->setSize(bound.size());

        if (d->container->shapeCount() > 0) {
            // the group has changed position and so have the group child shapes
            // -> we need compensate the group position change
            QPointF positionOffset = oldGroupPosition - bound.topLeft();
            Q_FOREACH (KoShape * child, d->container->shapes())
                child->setAbsolutePosition(child->absolutePosition() + positionOffset);
        }
    }

    QTransform groupTransform = d->container->absoluteTransformation(0).inverted();

    int zIndex=0;
    QList<KoShape*> shapes(d->container->shapes());
    if (!shapes.isEmpty()) {
        std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
        zIndex = shapes.last()->zIndex();
    }

    uint shapeCount = d->shapes.count();
    for (uint i = 0; i < shapeCount; ++i) {
        KoShape * shape = d->shapes[i];
        shape->setZIndex(zIndex++);

        if(d->inheritTransform[i]) {
            shape->applyAbsoluteTransformation(groupTransform);
        }
        else {
            QSizeF containerSize = d->container->size();
            QPointF containerPos = d->container->absolutePosition() - QPointF(0.5 * containerSize.width(), 0.5 * containerSize.height());

            QTransform matrix;
            matrix.translate(containerPos.x(), containerPos.y());
            shape->applyAbsoluteTransformation(matrix.inverted());
        }

        d->container->addShape(shape);
        d->container->setClipped(shape, d->clipped[i]);
        d->container->setInheritsTransform(shape, d->inheritTransform[i]);
    }
}

void KoShapeGroupCommand::undo()
{
    KUndo2Command::undo();

    QTransform ungroupTransform = d->container->absoluteTransformation(0);
    for (int i = 0; i < d->shapes.count(); i++) {
        KoShape * shape = d->shapes[i];
        const bool inheritedTransform = d->container->inheritsTransform(shape);
        d->container->removeShape(shape);
        if (d->oldParents.at(i)) {
            d->oldParents.at(i)->addShape(shape);
            d->oldParents.at(i)->setClipped(shape, d->oldClipped.at(i));
            d->oldParents.at(i)->setInheritsTransform(shape, d->oldInheritTransform.at(i));
        }
        if(inheritedTransform) {
            shape->applyAbsoluteTransformation(ungroupTransform);
        }
        else {
            QSizeF containerSize = d->container->size();
            QPointF containerPos = d->container->absolutePosition() - QPointF(0.5 * containerSize.width(), 0.5 * containerSize.height());

            QTransform matrix;
            matrix.translate(containerPos.x(), containerPos.y());
            shape->applyAbsoluteTransformation(matrix);
        }
        shape->setZIndex(d->oldZIndex[i]);
    }

    if (d->shouldNormalize && dynamic_cast<KoShapeGroup*>(d->container)) {
        QPointF oldGroupPosition = d->container->absolutePosition(KoFlake::TopLeft);
        if (d->container->shapeCount() > 0) {
            bool boundingRectInitialized = false;
            QRectF bound;
            Q_FOREACH (KoShape * shape, d->container->shapes()) {
                if (! boundingRectInitialized) {
                    bound = shape->boundingRect();
                    boundingRectInitialized = true;
                } else
                    bound = bound.united(shape->boundingRect());
            }
            // the group has changed position and so have the group child shapes
            // -> we need compensate the group position change
            QPointF positionOffset = oldGroupPosition - bound.topLeft();
            Q_FOREACH (KoShape * child, d->container->shapes())
                child->setAbsolutePosition(child->absolutePosition() + positionOffset);

            d->container->setAbsolutePosition(bound.topLeft(), KoFlake::TopLeft);
            d->container->setSize(bound.size());
        }
    }
}

QRectF KoShapeGroupCommandPrivate::containerBoundingRect()
{
    QRectF bound;
    if (container->shapeCount() > 0) {
        bound = container->absoluteTransformation(0).mapRect(container->outlineRect());
    }

    Q_FOREACH (KoShape *shape, shapes) {
        bound |= shape->absoluteTransformation(0).mapRect(shape->outlineRect());
    }

    return bound;
}
