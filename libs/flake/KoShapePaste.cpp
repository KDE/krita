/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2009 Thomas Zander <zander@kde.org>

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

#include <kdebug.h>
#include <klocale.h>

#include <KoOdfLoadingContext.h>
#include <KoOdfReadStore.h>

#include "KoCanvasBase.h"
#include "KoShapeController.h"
#include "KoShape.h"
#include "KoShapeLayer.h"
#include "KoShapeLoadingContext.h"
#include "KoShapeManager.h"
#include "KoShapeControllerBase.h"
#include "KoShapeRegistry.h"
#include "commands/KoShapeCreateCommand.h"

#include <KGlobal>
#include <KConfig>
#include <KConfigGroup>

class KoShapePaste::Private
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

    QList<KoShape*> shapes(d->layer ? d->layer->childShapes(): d->canvas->shapeManager()->topLevelShapes());

    int zIndex = 0;
    if (!shapes.isEmpty()) {
        zIndex = shapes.first()->zIndex();
        foreach (KoShape * shape, shapes) {
            zIndex = qMax(zIndex, shape->zIndex());
        }
        ++zIndex;
    }
    context.setZIndex(zIndex);

    QUndoCommand *cmd = 0;

    QPointF copyOffset(10.0, 10.0);
    // read copy offset from settings
    KSharedConfigPtr config = KGlobal::config();
    if (config->hasGroup("Misc")) {
        const qreal offset = config->group("Misc").readEntry("CopyOffset", 10.0);
        copyOffset = QPointF(offset, offset);
    }

    // TODO if this is a text create a text shape and load the text inside the new shape.
    KoXmlElement element;
    forEachElement(element, body) {
        kDebug(30006) << "loading shape" << element.localName();

        KoShape * shape = KoShapeRegistry::instance()->createShapeFromOdf(element, context);
        if (shape) {
            if (!cmd)
                cmd = new QUndoCommand(i18n("Paste Shapes"));
            if (! shape->parent()) {
                shape->setParent(d->layer);
            }
            KoShapeManager *sm = d->canvas->shapeManager();
            Q_ASSERT(sm);
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
                    QPointF move(copyOffset);
                    d->canvas->clipToDocument(shape, move);
                    if (move.x() != 0 || move.y() != 0) {
                        shape->setPosition(shape->position() + move);
                        done = false;
                        break;
                    }
                }
            } while (!done);

            d->canvas->shapeController()->addShapeDirect(shape, cmd);
            d->pastedShapes << shape;
        }
    }
    if (cmd)
        d->canvas->addCommand(cmd);
    return true;
}

QList<KoShape*> KoShapePaste::pastedShapes() const
{
    return d->pastedShapes;
}
