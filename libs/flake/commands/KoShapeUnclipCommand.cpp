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
#include "KoShapeBasedDocumentBase.h"
#include "KoShapeRegistry.h"
#include "KoCanvasController.h"
#include "KoToolManager.h"
#include "KoShapeLoadingContext.h"
#include "KoShapeOdfSaveHelper.h"
#include "KoDrag.h"
#include "KoCanvasBase.h"
#include <KoOdfPaste.h>
#include <KoOdfLoadingContext.h>
#include <KoOdfReadStore.h>

#include <KLocale>

class KoShapeUnclipCommand::Private : public KoOdfPaste
{
public:
    Private(KoShapeBasedDocumentBase *c)
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

        foreach(KoShape *shape, shapesToUnclip) {
            KoClipPath *clipPath = shape->clipPath();
            if (!clipPath)
                continue;
            QList<KoShape*> shapes;
            foreach(KoShape *clipShape, clipPath->clipPathShapes()) {
                shapes.append(clipShape);
            }
            KoShapeOdfSaveHelper saveHelper(shapes);
            KoDrag drag;
            drag.setOdf(KoOdf::mimeType(KoOdf::Text), saveHelper);

            const int pathShapeCount = clipPathShapes.count();

            paste(KoOdf::Text, drag.mimeData());

            // apply transformations
            for (int i = pathShapeCount; i < clipPathShapes.count(); ++i) {
                KoPathShape *pathShape = clipPathShapes[i];
                // apply transformation so that it matches the current clipped shapes clip path
                pathShape->applyAbsoluteTransformation(clipPath->clipDataTransformation(shape));
                pathShape->setZIndex(shape->zIndex()+1);
                clipPathParents.append(shape->parent());
            }
        }
    }

    /// reimplemented from KoOdfPaste
    virtual bool process(const KoXmlElement &body, KoOdfReadStore &odfStore) {
        KoOdfLoadingContext loadingContext(odfStore.styles(), odfStore.store());
        KoShapeLoadingContext context(loadingContext, controller->resourceManager());

        KoXmlElement element;
        forEachElement(element, body) {
            KoShape *shape = KoShapeRegistry::instance()->createShapeFromOdf(element, context);
            if (!shape)
                continue;
            KoPathShape *pathShape = dynamic_cast<KoPathShape*>(shape);
            if (!pathShape) {
                delete shape;
                continue;
            }
            clipPathShapes.append(pathShape);
        }
        return true;
    }

    QList<KoShape*> shapesToUnclip;
    QList<KoClipPath*> oldClipPaths;
    QList<KoPathShape*> clipPathShapes;
    QList<KoShapeContainer*> clipPathParents;
    KoShapeBasedDocumentBase *controller;

    bool executed;
};

KoShapeUnclipCommand::KoShapeUnclipCommand(KoShapeBasedDocumentBase *controller, const QList<KoShape*> &shapes, KUndo2Command *parent)
        : KUndo2Command(parent), d(new Private(controller))
{
    d->shapesToUnclip = shapes;
    foreach(KoShape *shape, d->shapesToUnclip) {
        d->oldClipPaths.append(shape->clipPath());
    }

    setText(i18nc("(qtundo-format)", "Unclip Shape"));
}

KoShapeUnclipCommand::KoShapeUnclipCommand(KoShapeBasedDocumentBase *controller, KoShape *shape, KUndo2Command *parent)
        : KUndo2Command(parent), d(new Private(controller))
{
    d->shapesToUnclip.append(shape);
    d->oldClipPaths.append(shape->clipPath());

    setText(i18nc("(qtundo-format)", "Unclip Shapes"));
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
        // the parent has to be there when it is added to the KoShapeBasedDocumentBase
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
