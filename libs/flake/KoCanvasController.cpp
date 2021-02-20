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
        : margin(0)
        , preferredCenterFractionX(0.5)
        , preferredCenterFractionY(0.5)
        , actionCollection(0)
    {
    }

    int margin;
    QSizeF documentSize;
    QPoint documentOffset;
    qreal preferredCenterFractionX;
    qreal preferredCenterFractionY;
    KActionCollection* actionCollection;
};

KoCanvasController::KoCanvasController(KActionCollection* actionCollection)
    : d(new Private())
{
    proxyObject = new KoCanvasControllerProxyObject(this);
    d->actionCollection = actionCollection;
}

KoCanvasController::~KoCanvasController()
{
    KoToolManager::instance()->removeCanvasController(this);
    delete d;
    delete proxyObject;
}

void KoCanvasController::setMargin(int margin)
{
    d->margin = margin;
}

int KoCanvasController::margin() const
{
    return d->margin;
}

KoCanvasBase* KoCanvasController::canvas() const
{
    return 0;
}

void KoCanvasController::setDocumentSize(const QSizeF &sz)
{
    d->documentSize = sz;
}

QSizeF KoCanvasController::documentSize() const
{
    return d->documentSize;
}

void KoCanvasController::setPreferredCenterFractionX(qreal x)
{
    d->preferredCenterFractionX = x;
}

qreal KoCanvasController::preferredCenterFractionX() const
{
    return d->preferredCenterFractionX;
}

void KoCanvasController::setPreferredCenterFractionY(qreal y)
{
    d->preferredCenterFractionY = y;
}

qreal KoCanvasController::preferredCenterFractionY() const
{
    return d->preferredCenterFractionY;
}

void KoCanvasController::setDocumentOffset(QPoint &offset)
{
    d->documentOffset = offset;
}

QPoint KoCanvasController::documentOffset() const
{
    return d->documentOffset;
}

KoCanvasControllerProxyObject::KoCanvasControllerProxyObject(KoCanvasController *controller, QObject *parent)
    : QObject(parent)
    , m_canvasController(controller)
{
}

void KoCanvasControllerProxyObject::updateDocumentSize(const QSize &newSize, bool recalculateCenter)
{
    m_canvasController->updateDocumentSize(newSize, recalculateCenter);
}

KActionCollection* KoCanvasController::actionCollection() const
{
    return d->actionCollection;
}
