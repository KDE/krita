/* This file is part of the KDE project
 * Copyright (C) 2007 C. Boemann <cbo@boemann.dk>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2010 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2011 Arjen Hiemstra <ahiemstra@heimr.nl>
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
#include <KoZoomController.h>
#include <KoZoomController_p.h>

#include <kactioncollection.h>
#include <klocalizedstring.h>
#include <WidgetsDebug.h>

#include <KoZoomHandler.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>

void KoZoomController::Private::init(KoCanvasController *co,
                                     KoZoomHandler *zh,
                                     KActionCollection *actionCollection)
{
    canvasController = co;
    fitMargin = co->margin();
    zoomHandler = zh;
    connect(action, SIGNAL(zoomChanged(KoZoomMode::Mode, qreal)),
            parent, SLOT(setZoom(KoZoomMode::Mode, qreal)));
    connect(action, SIGNAL(aspectModeChanged(bool)),
            parent, SIGNAL(aspectModeChanged(bool)));
    connect(action, SIGNAL(zoomedToSelection()),
            parent, SIGNAL(zoomedToSelection()));
    connect(action, SIGNAL(zoomedToAll()),
            parent, SIGNAL(zoomedToAll()));

    actionCollection->addAction("view_zoom", action);

    connect(canvasController->proxyObject, SIGNAL( sizeChanged(const QSize & ) ), parent, SLOT( setAvailableSize() ) );

    connect(canvasController->proxyObject, SIGNAL( zoomRelative(const qreal, const QPointF& ) ), parent, SLOT( requestZoomRelative( const qreal, const QPointF& ) ) );
}

KoZoomController::KoZoomController(KoCanvasController *co, KoZoomHandler *zh, KActionCollection *actionCollection, KoZoomAction::SpecialButtons specialButtons, QObject *parent)
    : QObject(parent),
    d(new Private(this, specialButtons))
{
    d->init(co, zh, actionCollection);
}

KoZoomController::~KoZoomController()
{
    delete d;
}

KoZoomAction *KoZoomController::zoomAction() const
{
    return d->action;
}

void KoZoomController::setZoomMode(KoZoomMode::Mode mode)
{
    setZoom(mode, 1.0);
}

KoZoomMode::Mode KoZoomController::zoomMode() const
{
    return d->zoomHandler->zoomMode();
}

void KoZoomController::setPageSize(const QSizeF &pageSize)
{
    if(d->pageSize == pageSize) return;
    d->pageSize = pageSize;

    if(d->zoomHandler->zoomMode() == KoZoomMode::ZOOM_WIDTH)
        setZoom(KoZoomMode::ZOOM_WIDTH, 0);
    if(d->zoomHandler->zoomMode() == KoZoomMode::ZOOM_PAGE)
        setZoom(KoZoomMode::ZOOM_PAGE, 0);
}

QSizeF KoZoomController::pageSize() const
{
    return d->pageSize;
}

void KoZoomController::setTextMinMax(qreal min, qreal max)
{
    if(d->textMinX == min && d->textMaxX == max) {
        return;
    }
    d->textMinX = min;
    d->textMaxX = max;

    if(d->zoomHandler->zoomMode() == KoZoomMode::ZOOM_TEXT)
        setZoom(KoZoomMode::ZOOM_TEXT, 0);
}

void KoZoomController::setDocumentSize(const QSizeF &documentSize, bool recalculateCenter)
{
    d->documentSize = documentSize;
    d->canvasController->updateDocumentSize(documentToViewport(d->documentSize), recalculateCenter);

    // Finally ask the canvasController to recenter
    d->canvasController->recenterPreferred();
}

QSizeF KoZoomController::documentSize() const
{
    return d->documentSize;
}

void KoZoomController::setZoom(KoZoomMode::Mode mode, qreal zoom)
{
    setZoom(mode, zoom, d->canvasController->preferredCenter());
}

void KoZoomController::setZoom(KoZoomMode::Mode mode, qreal zoom, const QPointF &stillPoint)
{
    setZoom(mode, zoom, d->zoomHandler->resolutionX(), d->zoomHandler->resolutionY(), stillPoint);
}

void KoZoomController::setZoom(KoZoomMode::Mode mode, qreal zoom, qreal resolutionX, qreal resolutionY)
{
    setZoom(mode, zoom, resolutionX, resolutionY, d->canvasController->preferredCenter());
}

void KoZoomController::setZoom(KoZoomMode::Mode mode, qreal zoom, qreal resolutionX, qreal resolutionY, const QPointF &stillPoint)
{
    if (d->zoomHandler->zoomMode() == mode &&
        qFuzzyCompare(d->zoomHandler->zoom(), zoom) &&
        qFuzzyCompare(d->zoomHandler->resolutionX(), resolutionX) &&
        qFuzzyCompare(d->zoomHandler->resolutionY(), resolutionY)) {
        return; // no change
    }

    qreal oldEffectiveZoom = d->action->effectiveZoom();
    QSize oldPageViewportSize = documentToViewport(d->pageSize);
    QSize oldTextViewportSize = documentToViewport(QSizeF(d->textMaxX-d->textMinX, 1));
    qreal yfixAlignTop = d->canvasController->viewportSize().height();

    if(!qFuzzyCompare(d->zoomHandler->resolutionX(), resolutionX) ||
       !qFuzzyCompare(d->zoomHandler->resolutionY(), resolutionY)) {

        d->zoomHandler->setResolution(resolutionX, resolutionY);
    }

    if(mode == KoZoomMode::ZOOM_CONSTANT) {
        if(zoom == 0.0) return;
        d->action->setZoom(zoom);
    }
    else if(mode == KoZoomMode::ZOOM_WIDTH) {
        zoom = (d->canvasController->viewportSize().width() - 2 * d->fitMargin)
                    / (oldPageViewportSize.width() / d->zoomHandler->zoom());
        d->action->setSelectedZoomMode(mode);
        d->action->setEffectiveZoom(zoom);
    }
    else if(mode == KoZoomMode::ZOOM_PAGE) {
        zoom = (d->canvasController->viewportSize().width() - 2 * d->fitMargin)
                     / (oldPageViewportSize.width() / d->zoomHandler->zoom());
        zoom = qMin(zoom, (d->canvasController->viewportSize().height() - 2 * d->fitMargin)
                     / (oldPageViewportSize.height() / d->zoomHandler->zoom()));

        d->action->setSelectedZoomMode(mode);
        d->action->setEffectiveZoom(zoom);
    }
    else if (mode == KoZoomMode::ZOOM_TEXT) {
        zoom = (d->canvasController->viewportSize().width() - 2 * d->fitMargin)
                    / (oldTextViewportSize.width() / d->zoomHandler->zoom());
        d->action->setSelectedZoomMode(mode);
        d->action->setEffectiveZoom(zoom);
    }

    d->zoomHandler->setZoomMode(mode);
    d->zoomHandler->setZoom(d->action->effectiveZoom());

#ifdef DEBUG
    if(! d->documentSize.isValid())
        warnWidgets << "Setting zoom while there is no document size set, this will fail";
    else if (d->pageSize.width() > d->documentSize.width() || d->pageSize.height() > d->documentSize.height())
        warnWidgets << "ZoomController; Your page size is larger than your document size (" <<
            d->pageSize << " > " << d->documentSize << ")\n";
#endif

    QSize documentViewportSize = documentToViewport(d->documentSize);

    // Tell the canvasController that the zoom has changed
    // Actually canvasController doesn't know about zoom, but the document in pixels
    // has changed as a result of the zoom change
    // To allow listeners of offset changes to react on the real new offset and not on the
    // intermediate offsets, we block the signals here, and emit by ourselves later.
    d->canvasController->proxyObject->blockSignals(true);
    d->canvasController->updateDocumentSize(documentViewportSize, true);
    d->canvasController->proxyObject->blockSignals(false);

    // Finally ask the canvasController to recenter
    if (d->canvasController->canvasMode() == KoCanvasController::Infinite) {
        QPointF documentCenter;
        if (mode == KoZoomMode::ZOOM_WIDTH || mode == KoZoomMode::ZOOM_PAGE) {
            documentCenter = QRectF(QPointF(), documentViewportSize).center();
        }
        else {
            qreal zoomCoeff = d->action->effectiveZoom() / oldEffectiveZoom;
            QPointF oldCenter = d->canvasController->preferredCenter();
            documentCenter = stillPoint * zoomCoeff - (stillPoint - 1.0 / zoomCoeff * oldCenter);
        }
        d->canvasController->setPreferredCenter(documentCenter);
    }
    else if (mode == KoZoomMode::ZOOM_TEXT) {
            QPointF documentCenter = d->canvasController->preferredCenter();
            yfixAlignTop -= d->canvasController->viewportSize().height();

            documentCenter.setX(d->zoomHandler->documentToViewX(d->textMinX + d->textMaxX) * 0.5);
            documentCenter.setY(documentCenter.y() - yfixAlignTop);
            d->canvasController->setPreferredCenter(documentCenter);
    } else {
        if (d->canvasController->canvasMode() == KoCanvasController::AlignTop) {
            QPointF documentCenter = d->canvasController->preferredCenter();
            documentCenter.setX(0.0);
            d->canvasController->setPreferredCenter(documentCenter);
        } else {
            d->canvasController->recenterPreferred();
        }
    }

    // now that we have the final offset, let's emit some signals
    //d->canvasController->proxyObject->emitCanvasOffsetXChanged(d->canvasController->canvasOffsetX());
    //d->canvasController->proxyObject->emitCanvasOffsetYChanged(d->canvasController->canvasOffsetY());
    emit zoomChanged(mode, d->action->effectiveZoom());
}

QSize KoZoomController::documentToViewport(const QSizeF &size)
{
    return d->zoomHandler->documentToView(size).toSize();
}

void KoZoomController::setAspectMode(bool status)
{
    if (d->action) {
        d->action->setAspectMode(status);
    }
}
//have to include this because of Q_PRIVATE_SLOT
#include <moc_KoZoomController.cpp>
