/* This file is part of the KDE project
 *
 * Copyright (C) 2010 Boudewijn Rempt <boud@valdyas.org>
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
