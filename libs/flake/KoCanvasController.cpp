/* This file is part of the KDE project
 *
 * Copyright (C) 2010 Boudewijn Rempt <boud@kogmbh.com>
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

#include <QSize>
#include <QPoint>

class KoCanvasController::Private {
public:
    Private()
        : canvasMode(Centered)
        , margin(0)
        , preferredCenterFractionX(0.5)
        , preferredCenterFractionY(0.5)
    {
    }

    CanvasMode canvasMode;
    int margin;
    QSize documentSize;
    QPoint documentOffset;
    qreal preferredCenterFractionX;
    qreal preferredCenterFractionY;
};

KoCanvasController::KoCanvasController()
    : d(new Private())
{
    proxyObject = new KoCanvasControllerProxyObject(this);
}

KoCanvasController::~KoCanvasController()
{
    delete d;
    delete proxyObject;
}

void KoCanvasController::setCanvasMode(CanvasMode mode)
{
    d->canvasMode = mode;
    switch (mode) {
    case AlignTop:
        d->preferredCenterFractionX = 0;
        d->preferredCenterFractionY = 0.5;
        break;
    case Infinite:
    case Centered:
        d->preferredCenterFractionX = 0.5;
        d->preferredCenterFractionY = 0.5;
        break;
    case Presentation:
    case Spreadsheet:
        d->preferredCenterFractionX = 0;
        d->preferredCenterFractionY = 0;
        break;
    };
}

void KoCanvasController::setMargin(int margin)
{
    d->margin = margin;
}

int KoCanvasController::margin() const
{
    return d->margin;
}


KoCanvasController::CanvasMode KoCanvasController::canvasMode() const
{
    return d->canvasMode;
}

void KoCanvasController::setDocumentSize(const QSize &sz)
{
    d->documentSize = sz;
}

QSize KoCanvasController::documentSize() const
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


#include <KoCanvasController.moc>
