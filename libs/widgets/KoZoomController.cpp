/* This file is part of the KDE project
 * Copyright (C) 2007 Casper Boemann <cbr@boemann.dk>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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
#include <KoZoomController.h>

#include <KActionCollection>
#include <KLocale>
#include <KDebug>

#include <KoZoomHandler.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>

class KoZoomController::Private
{
public:
    Private(KoZoomController *p, KoZoomAction::SpecialButtons specialButtons)
        : canvasController(0), zoomHandler(0), fitMargin(0), parent(p)
    {
        action = new KoZoomAction(KoZoomMode::ZOOM_WIDTH | KoZoomMode::ZOOM_PAGE, i18n("Zoom"), 0);
        action->setSpecialButtons(specialButtons);
    }
    ~Private()
    {
        delete action;
    }

    /// so we know when the canvasController changes size
    void setAvailableSize()
    {
        if(zoomHandler->zoomMode() == KoZoomMode::ZOOM_WIDTH)
            setZoom(KoZoomMode::ZOOM_WIDTH, -1);
        if(zoomHandler->zoomMode() == KoZoomMode::ZOOM_PAGE)
            setZoom(KoZoomMode::ZOOM_PAGE, -1);
    }

    /// when the canvas controller wants us to change zoom
    void requestZoomBy(const qreal factor)
    {
        qreal zoom = zoomHandler->zoom();
        action->setZoom(factor*zoom);
        setZoom(KoZoomMode::ZOOM_CONSTANT, factor*zoom);
        action->setEffectiveZoom(factor*zoom);
    }

    void setZoom(KoZoomMode::Mode mode, qreal zoom)
    {
        parent->setZoom(mode, zoom);
    }

    KoCanvasController *canvasController;
    KoZoomHandler *zoomHandler;
    KoZoomAction *action;
    QSizeF pageSize;
    QSizeF documentSize;
    int fitMargin;
    KoZoomController *parent;
};

KoZoomController::KoZoomController(KoCanvasController *co, KoZoomHandler *zh, KActionCollection *actionCollection, KoZoomAction::SpecialButtons specialButtons, QObject *parent)
    : QObject(parent),
    d(new Private(this, specialButtons))
{
    d->canvasController = co;
    d->fitMargin = co->margin();
    d->zoomHandler = zh;
    connect(d->action, SIGNAL(zoomChanged(KoZoomMode::Mode, qreal)),
            this, SLOT(setZoom(KoZoomMode::Mode, qreal)));
    connect(d->action, SIGNAL(aspectModeChanged(bool)),
            this, SIGNAL(aspectModeChanged(bool)));
    connect(d->action, SIGNAL(zoomedToSelection()),
            this, SIGNAL(zoomedToSelection()));
    connect(d->action, SIGNAL(zoomedToAll()),
            this, SIGNAL(zoomedToAll()));

    actionCollection->addAction("view_zoom", d->action);
    actionCollection->addAction(KStandardAction::ZoomIn,  "zoom_in", d->action, SLOT(zoomIn()));
    actionCollection->addAction(KStandardAction::ZoomOut,  "zoom_out", d->action, SLOT(zoomOut()));

    connect(d->canvasController->proxyObject, SIGNAL( sizeChanged(const QSize & ) ), this, SLOT( setAvailableSize() ) );

    connect(d->canvasController->proxyObject, SIGNAL( zoomBy(const qreal ) ), this, SLOT( requestZoomBy( const qreal ) ) );
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
    setZoom(mode, 1);
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

void KoZoomController::setDocumentSize( const QSizeF &documentSize )
{
    d->documentSize = documentSize;
    d->canvasController->updateDocumentSize( d->zoomHandler->documentToView(d->documentSize).toSize(), false );

    // Finally ask the canvasController to recenter
    d->canvasController->recenterPreferred();
}

void KoZoomController::setZoom(KoZoomMode::Mode mode, qreal zoom)
{
    // Seems to be unused
//    if (d->zoomHandler->zoomMode() == mode && d->zoomHandler->zoom() == zoom)
//        return; // no change
    d->zoomHandler->setZoomMode(mode);

    if(mode == KoZoomMode::ZOOM_CONSTANT)
    {
        if(zoom == 0.0) return;
        d->action->setZoom(zoom);
    }
    else if(mode == KoZoomMode::ZOOM_WIDTH)
    {
        zoom = (d->canvasController->viewportSize().width() - 2*d->fitMargin)
                         / (d->zoomHandler->resolutionX() * d->pageSize.width());
        d->action->setSelectedZoomMode(mode);
        d->action->setEffectiveZoom(zoom);
    }
    else if(mode == KoZoomMode::ZOOM_PAGE)
    {
        zoom = (d->canvasController->viewportSize().width() - 2*d->fitMargin)
                         / (d->zoomHandler->resolutionX() * d->pageSize.width());
        zoom = qMin(zoom, (d->canvasController->viewportSize().height() - 2*d->fitMargin)
                     / (d->zoomHandler->resolutionY() * d->pageSize.height()));

        d->action->setSelectedZoomMode(mode);
        d->action->setEffectiveZoom(zoom);
    }

    d->zoomHandler->setZoom(zoom);

    // Tell the canvasController that the zoom has changed
    // Actually canvasController doesn't know about zoom, but the document in pixels
    // has change as a result of the zoom change
#ifdef DEBUG
    if(! d->documentSize.isValid())
        kWarning(30004) << "Setting zoom while there is no document size set, this will fail";
    else if(d->pageSize.width() > d->documentSize.width() || d->pageSize.height() > d->documentSize.height())
        kWarning(30004) << "ZoomController; Your page size is larger than your document size (" <<
            d->pageSize << " > " << d->documentSize << ")\n";
#endif
    d->canvasController->updateDocumentSize( d->zoomHandler->documentToView(d->documentSize).toSize(), true );

    // Finally ask the canvasController to recenter
    d->canvasController->recenterPreferred();

    emit zoomChanged(mode, zoom);
}

void KoZoomController::setAspectMode(bool status)
{
    d->action->setAspectMode(status);
}

#include <KoZoomController.moc>
