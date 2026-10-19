/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2010 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoCanvasController.h"
#include "KoToolManager.h"

#include <QSize>
#include <QPoint>

class Q_DECL_HIDDEN KoCanvasController::Private
{
public:
    Private()
        : preferredCenterFractionX(0.5)
        , preferredCenterFractionY(0.5)
        , actionCollection(0)
    {
    }

    QSizeF documentSize;
    QPoint documentOffset;
    qreal preferredCenterFractionX;
    qreal preferredCenterFractionY;
    KisKActionCollection* actionCollection;
    QPointer<KoToolManager> toolManager;
};

KoCanvasController::KoCanvasController(KisKActionCollection* actionCollection)
    : d(new Private())
{
    proxyObject = new KoCanvasControllerProxyObject(this);
    d->actionCollection = actionCollection;
}

KoCanvasController::~KoCanvasController()
{
    if (d->toolManager) {
        d->toolManager->removeCanvasController(this);
    }
    delete d;
    delete proxyObject;
}

void KoCanvasController::setToolManager(KoToolManager *toolManager)
{
    if (toolManager == d->toolManager) return;

    if (d->toolManager) {
        d->toolManager->removeCanvasController(this);
    }

    d->toolManager = toolManager;

    if (d->toolManager) {
        d->toolManager->addController(this);
    }
}

KoToolManager *KoCanvasController::toolManager() const
{
    return d->toolManager.data();
}

KoCanvasBase* KoCanvasController::canvas() const
{
    return 0;
}

KoCanvasControllerProxyObject::KoCanvasControllerProxyObject(KoCanvasController *controller, QObject *parent)
    : QObject(parent)
    , m_canvasController(controller)
{
}

KisKActionCollection* KoCanvasController::actionCollection() const
{
    return d->actionCollection;
}
