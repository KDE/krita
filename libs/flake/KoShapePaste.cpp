/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2009 Thomas Zander <zander@kde.org>
   Copyright (C) 2010-2011 Jan Hambrecht <jaham@gmx.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoShapePaste.h"

#include <FlakeDebug.h>
#include <klocalizedstring.h>

#include <KoOdfLoadingContext.h>
#include <KoOdfReadStore.h>

#include "KoCanvasBase.h"
#include "KoShapeController.h"
#include "KoShape.h"
#include "KoShapeLayer.h"
#include "KoShapeLoadingContext.h"
#include "KoShapeManager.h"
#include "KoShapeBasedDocumentBase.h"
#include "KoShapeRegistry.h"
#include "KoCanvasController.h"
#include "KoDocumentResourceManager.h"
#include "commands/KoShapeCreateCommand.h"

class Q_DECL_HIDDEN KoShapePaste::Private
{
public:
    Private(KoCanvasBase *cb, KoShapeLayer *l) : canvas(cb), layer(l) {}

    KoCanvasBase *canvas;
    KoShapeLayer *layer;
    QList<KoShape*> pastedShapes;
};

KoShapePaste::KoShapePaste(KoCanvasBase *canvas, KoShapeLayer *layer)
        : d(new Private(canvas, layer))
{
}

KoShapePaste::~KoShapePaste()
{
    delete d;
}

bool KoShapePaste::process(const KoXmlElement & body, KoOdfReadStore & odfStore)
{
    d->pastedShapes.clear();
    KoOdfLoadingContext loadingContext(odfStore.styles(), odfStore.store());
    KoShapeLoadingContext context(loadingContext, d->canvas->shapeController()->resourceManager());

    QList<KoShape*> shapes(d->layer ? d->layer->shapes(): d->canvas->shapeManager()->topLevelShapes());

    int zIndex = 0;
    if (!shapes.isEmpty()) {
        zIndex = shapes.first()->zIndex();
        foreach (KoShape * shape, shapes) {
            zIndex = qMax(zIndex, shape->zIndex());
        }
        ++zIndex;
    }
    context.setZIndex(zIndex);

    KoDocumentResourceManager *rm = d->canvas->shapeController()->resourceManager();
    Q_ASSERT(rm);

    QPointF pasteOffset(rm->pasteOffset(), rm->pasteOffset());
    const bool pasteAtCursor = rm->pasteAtCursor();

    // get hold of the canvas' shape manager
    KoShapeManager *sm = d->canvas->shapeManager();
    Q_ASSERT(sm);

    // TODO if this is a text create a text shape and load the text inside the new shape.
    // create the shape from the clipboard
    KoXmlElement element;
    forEachElement(element, body) {
        debugFlake << "loading shape" << element.localName();

        KoShape * shape = KoShapeRegistry::instance()->createShapeFromOdf(element, context);
        if (shape) {
            d->pastedShapes << shape;
        }
    }

    if (d->pastedShapes.isEmpty())
        return true;

    // position shapes
    if (pasteAtCursor) {
        QRectF bbox;
        // determine bounding rect of all pasted shapes
        foreach (KoShape *shape, d->pastedShapes) {
            if (bbox.isEmpty())
                bbox = shape->boundingRect();
            else
                bbox |= shape->boundingRect();
        }
        // where is the cursor now?
        QWidget *canvasWidget = d->canvas->canvasWidget();
        KoCanvasController *cc = d->canvas->canvasController();
        // map mouse screen position to the canvas widget coordinates
        QPointF mouseCanvasPos = canvasWidget->mapFromGlobal(QCursor::pos());
        // apply the canvas offset
        mouseCanvasPos -= QPoint(cc->canvasOffsetX(), cc->canvasOffsetY());
        // apply offset of document origin
        mouseCanvasPos -= d->canvas->documentOrigin();
        // convert to document coordinates
        QPointF mouseDocumentPos = d->canvas->viewConverter()->viewToDocument(mouseCanvasPos);
        // now we can determine the offset to apply, with the center of the pasted shapes
        // bounding rect at the current mouse position
        QPointF pasteOffset = mouseDocumentPos - bbox.center();
        foreach (KoShape *shape, d->pastedShapes) {
            QPointF move(pasteOffset);
            d->canvas->clipToDocument(shape, move);
            if (move.x() != 0 || move.y() != 0) {
                shape->setPosition(shape->position() + move);
            }
        }
    } else {
        foreach (KoShape *shape, d->pastedShapes) {
            bool done = true;
            do {
                // find a nice place for our shape.
                done = true;
                foreach (const KoShape *s, sm->shapesAt(shape->boundingRect()) + d->pastedShapes) {
                    if (d->layer && s->parent() != d->layer)
                        continue;
                    if (s->name() != shape->name())
                        continue;
                    if (qAbs(s->position().x() - shape->position().x()) > 0.001)
                        continue;
                    if (qAbs(s->position().y() - shape->position().y()) > 0.001)
                        continue;
                    if (qAbs(s->size().width() - shape->size().width()) > 0.001)
                        continue;
                    if (qAbs(s->size().height() - shape->size().height()) > 0.001)
                        continue;
                    // move it and redo our iteration.
                    QPointF move(pasteOffset);
                    d->canvas->clipToDocument(shape, move);
                    if (move.x() != 0 || move.y() != 0) {
                        shape->setPosition(shape->position() + move);
                        done = false;
                        break;
                    }
                }
            } while (!done);
        }
    }

    KUndo2Command *cmd = new KUndo2Command(kundo2_i18n("Paste Shapes"));
    if (!cmd) {
        qDeleteAll(d->pastedShapes);
        d->pastedShapes.clear();
        return false;
    }

    // add shapes to the document
    foreach (KoShape *shape, d->pastedShapes) {
        if (!shape->parent()) {
            shape->setParent(d->layer);
        }
        d->canvas->shapeController()->addShapeDirect(shape, cmd);
    }

    d->canvas->addCommand(cmd);

    return true;
}

QList<KoShape*> KoShapePaste::pastedShapes() const
{
    return d->pastedShapes;
}
