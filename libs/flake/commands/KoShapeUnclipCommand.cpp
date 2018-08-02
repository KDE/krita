/* This file is part of the KDE project
 * Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>
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

#include "KoShapeUnclipCommand.h"
#include "KoClipPath.h"
#include "KoShape.h"
#include "KoShapeContainer.h"
#include "KoPathShape.h"
#include "KoShapeControllerBase.h"
#include <kis_assert.h>

#include <klocalizedstring.h>

class KoShapeUnclipCommand::Private
{
public:
    Private(KoShapeControllerBase *c)
            : controller(c), executed(false) {
    }

    ~Private() {
        if (executed) {
            qDeleteAll(oldClipPaths);
        } else {
            qDeleteAll(clipPathShapes);
        }
    }

    void createClipPathShapes() {
        // check if we have already created the clip path shapes
        if (!clipPathShapes.isEmpty())
            return;

        Q_FOREACH (KoShape *shape, shapesToUnclip) {
            KoClipPath *clipPath = shape->clipPath();
            if (!clipPath)
                continue;

            Q_FOREACH (KoShape *clipShape, clipPath->clipPathShapes()) {
                KoShape *clone = clipShape->cloneShape();

                KoPathShape *pathShape = dynamic_cast<KoPathShape*>(clone);
                KIS_SAFE_ASSERT_RECOVER(pathShape) {
                    delete clone;
                    continue;
                }

                clipPathShapes.append(pathShape);
            }

            // apply transformations
            Q_FOREACH (KoPathShape *pathShape, clipPathShapes) {
                // apply transformation so that it matches the current clipped shapes clip path
                pathShape->applyAbsoluteTransformation(clipPath->clipDataTransformation(shape));
                pathShape->setZIndex(shape->zIndex() + 1);
                clipPathParents.append(shape->parent());
            }
        }
    }

    QList<KoShape*> shapesToUnclip;
    QList<KoClipPath*> oldClipPaths;
    QList<KoPathShape*> clipPathShapes;
    QList<KoShapeContainer*> clipPathParents;

    // TODO: damn! this is not a controller, this is a document! Needs refactoring!
    KoShapeControllerBase *controller;

    bool executed;
};

KoShapeUnclipCommand::KoShapeUnclipCommand(KoShapeControllerBase *controller, const QList<KoShape*> &shapes, KUndo2Command *parent)
        : KUndo2Command(parent), d(new Private(controller))
{
    d->shapesToUnclip = shapes;
    Q_FOREACH (KoShape *shape, d->shapesToUnclip) {
        d->oldClipPaths.append(shape->clipPath());
    }

    setText(kundo2_i18n("Unclip Shape"));
}

KoShapeUnclipCommand::KoShapeUnclipCommand(KoShapeControllerBase *controller, KoShape *shape, KUndo2Command *parent)
        : KUndo2Command(parent), d(new Private(controller))
{
    d->shapesToUnclip.append(shape);
    d->oldClipPaths.append(shape->clipPath());

    setText(kundo2_i18n("Unclip Shapes"));
}

KoShapeUnclipCommand::~KoShapeUnclipCommand()
{
    delete d;
}

void KoShapeUnclipCommand::redo()
{
    d->createClipPathShapes();

    const uint shapeCount = d->shapesToUnclip.count();
    for (uint i = 0; i < shapeCount; ++i) {
        d->shapesToUnclip[i]->setClipPath(0);
        d->shapesToUnclip[i]->update();
    }

    const uint clipPathCount = d->clipPathShapes.count();
    for (uint i = 0; i < clipPathCount; ++i) {
        // the parent has to be there when it is added to the KoShapeControllerBase
        if (d->clipPathParents.at(i))
            d->clipPathParents.at(i)->addShape(d->clipPathShapes[i]);
        d->controller->addShape(d->clipPathShapes[i]);
    }

    d->executed = true;

    KUndo2Command::redo();
}

void KoShapeUnclipCommand::undo()
{
    KUndo2Command::undo();

    const uint shapeCount = d->shapesToUnclip.count();
    for (uint i = 0; i < shapeCount; ++i) {
        d->shapesToUnclip[i]->setClipPath(d->oldClipPaths[i]);
        d->shapesToUnclip[i]->update();
    }

    const uint clipPathCount = d->clipPathShapes.count();
    for (uint i = 0; i < clipPathCount; ++i) {
        d->controller->removeShape(d->clipPathShapes[i]);
        if (d->clipPathParents.at(i))
            d->clipPathParents.at(i)->removeShape(d->clipPathShapes[i]);
    }

    d->executed = false;
}
