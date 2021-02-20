/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2011 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoShapeClipCommand.h"
#include "KoClipPath.h"
#include "KoShape.h"
#include "KoShapeContainer.h"
#include "KoPathShape.h"
#include "KoShapeControllerBase.h"

#include <klocalizedstring.h>

#include "kis_pointer_utils.h"

class Q_DECL_HIDDEN KoShapeClipCommand::Private
{
public:
    Private(KoShapeControllerBase *c)
            : controller(c), executed(false) {
    }

    ~Private() {
        if (executed) {
            qDeleteAll(oldClipPaths);
        } else {
            qDeleteAll(newClipPaths);
        }
    }

    QList<KoShape*> shapesToClip;
    QList<KoClipPath*> oldClipPaths;
    QList<KoPathShape*> clipPathShapes;
    QList<KoClipPath*> newClipPaths;
    QList<KoShapeContainer*> oldParents;
    KoShapeControllerBase *controller;
    bool executed;
};

KoShapeClipCommand::KoShapeClipCommand(KoShapeControllerBase *controller, const QList<KoShape*> &shapes, const QList<KoPathShape*> &clipPathShapes, KUndo2Command *parent)
        : KUndo2Command(parent), d(new Private(controller))
{
    d->shapesToClip = shapes;
    d->clipPathShapes = clipPathShapes;

    Q_FOREACH (KoShape *shape, d->shapesToClip) {
        d->oldClipPaths.append(shape->clipPath());
        d->newClipPaths.append(new KoClipPath(implicitCastList<KoShape*>(clipPathShapes), KoFlake::UserSpaceOnUse));
    }

    Q_FOREACH (KoPathShape *path, clipPathShapes) {
        d->oldParents.append(path->parent());
    }

    setText(kundo2_i18n("Clip Shape"));
}

KoShapeClipCommand::KoShapeClipCommand(KoShapeControllerBase *controller, KoShape *shape, const QList<KoPathShape*> &clipPathShapes, KUndo2Command *parent)
        : KUndo2Command(parent), d(new Private(controller))
{
    d->shapesToClip.append(shape);
    d->clipPathShapes = clipPathShapes;
    d->oldClipPaths.append(shape->clipPath());
    d->newClipPaths.append(new KoClipPath(implicitCastList<KoShape*>(clipPathShapes), KoFlake::UserSpaceOnUse));

    Q_FOREACH (KoPathShape *path, clipPathShapes) {
        d->oldParents.append(path->parent());
    }

    setText(kundo2_i18n("Clip Shape"));
}

KoShapeClipCommand::~KoShapeClipCommand()
{
    delete d;
}

void KoShapeClipCommand::redo()
{
    const uint shapeCount = d->shapesToClip.count();
    for (uint i = 0; i < shapeCount; ++i) {
        d->shapesToClip[i]->setClipPath(d->newClipPaths[i]);
        d->shapesToClip[i]->update();
    }

    const uint clipPathCount = d->clipPathShapes.count();
    for (uint i = 0; i < clipPathCount; ++i) {
        if (d->oldParents.at(i)) {
            d->oldParents.at(i)->removeShape(d->clipPathShapes[i]);
        }
    }

    d->executed = true;

    KUndo2Command::redo();
}

void KoShapeClipCommand::undo()
{
    KUndo2Command::undo();

    const uint shapeCount = d->shapesToClip.count();
    for (uint i = 0; i < shapeCount; ++i) {
        d->shapesToClip[i]->setClipPath(d->oldClipPaths[i]);
        d->shapesToClip[i]->update();
    }

    const uint clipPathCount = d->clipPathShapes.count();
    for (uint i = 0; i < clipPathCount; ++i) {
        if (d->oldParents.at(i)) {
            d->oldParents.at(i)->addShape(d->clipPathShapes[i]);
        }
    }

    d->executed = false;
}
