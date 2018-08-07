/* This file is part of the KDE project

   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

#include <QRectF>
#include <QPointer>
#include <QDebug>

#include "KoCanvasBase.h"
#include "KoCanvasResourceManager.h"
#include "KoShapeController.h"
#include "KoCanvasController.h"
#include "KoViewConverter.h"
#include "KoSnapGuide.h"
#include "KoShapeManager.h"
#include "KoToolProxy.h"
#include "KoSelection.h"
#include "KoSelectedShapesProxy.h"

class Q_DECL_HIDDEN KoCanvasBase::Private
{
public:
    Private()
        : shapeController(0),
        resourceManager(0),
        isResourceManagerShared(false),
        controller(0),
        snapGuide(0)
    {
    }

    ~Private() {
        delete shapeController;
        if (!isResourceManagerShared) {
            delete resourceManager;
        }
        delete snapGuide;
    }
    QPointer<KoShapeController> shapeController;
    QPointer<KoCanvasResourceManager> resourceManager;
    bool isResourceManagerShared;
    KoCanvasController *controller;
    KoSnapGuide *snapGuide;
};

KoCanvasBase::KoCanvasBase(KoShapeControllerBase *shapeController, KoCanvasResourceManager *sharedResourceManager)
        : d(new Private())
{
    d->resourceManager = sharedResourceManager ?
        sharedResourceManager : new KoCanvasResourceManager();
    d->isResourceManagerShared = sharedResourceManager;

    d->shapeController = new KoShapeController(this, shapeController);
    d->snapGuide = new KoSnapGuide(this);
}

KoCanvasBase::~KoCanvasBase()
{
    d->shapeController->reset();
    delete d;
}

QPointF KoCanvasBase::viewToDocument(const QPointF &viewPoint) const
{
    return viewConverter()->viewToDocument(viewPoint - documentOrigin());
}

KoShapeController *KoCanvasBase::shapeController() const
{
    if (d->shapeController)
        return d->shapeController;
    else
        return 0;
}

void KoCanvasBase::disconnectCanvasObserver(QObject *object)
{
    if (shapeManager()) shapeManager()->selection()->disconnect(object);
    if (resourceManager()) resourceManager()->disconnect(object);
    if (shapeManager()) shapeManager()->disconnect(object);
    if (toolProxy()) toolProxy()->disconnect(object);
    if (selectedShapesProxy()) selectedShapesProxy()->disconnect(object);
}


KoCanvasResourceManager *KoCanvasBase::resourceManager() const
{
    return d->resourceManager;
}

void KoCanvasBase::ensureVisible(const QRectF &rect)
{
    if (d->controller && d->controller->canvas())
        d->controller->ensureVisible(
                d->controller->canvas()->viewConverter()->documentToView(rect));
}

void KoCanvasBase::setCanvasController(KoCanvasController *controller)
{
    d->controller = controller;
}

KoCanvasController *KoCanvasBase::canvasController() const
{
    return d->controller;
}

void KoCanvasBase::clipToDocument(const KoShape *, QPointF &) const
{
}

KoSnapGuide * KoCanvasBase::snapGuide() const
{
    return d->snapGuide;
}
