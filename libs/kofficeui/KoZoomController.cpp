/* This file is part of the KDE project
 * Copyright (C) 2007 Casper Boemann <cbr@boemann.dk>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#include <kactioncollection.h>
#include <kicon.h>
#include <klocale.h>
#include <kdebug.h>

#include <KoView.h>
#include <KoZoomAction.h>
#include <KoZoomHandler.h>
#include <KoCanvasController.h>

class KoZoomController::Private
{
public:
    // should be a d-pointer...
    KoCanvasController *canvasController;
    KoZoomHandler *zoomHandler;
    KoZoomAction *action;
    QSizeF pageSize;
    QSizeF documentSize;
    int fitMargin;
};

KoZoomController::KoZoomController(KoCanvasController *co, KoZoomHandler *zh, KActionCollection *actionCollection, bool doSpecialAspectMode)
    : d(new Private)
{
    d->canvasController = co;
    d->zoomHandler = zh;
    d->fitMargin = 0;
    d->action = new KoZoomAction(KoZoomMode::ZOOM_WIDTH | KoZoomMode::ZOOM_PAGE, i18n("Zoom"), doSpecialAspectMode, 0);
    connect(d->action, SIGNAL(zoomChanged(KoZoomMode::Mode, double)),
            this, SLOT(setZoom(KoZoomMode::Mode, double)));
    connect(d->action, SIGNAL(aspectModeChanged(bool)),
            this, SIGNAL(aspectModeChanged(bool)));

    actionCollection->addAction("view_zoom", d->action);
    actionCollection->addAction(KStandardAction::ZoomIn,  "zood->in", d->action, SLOT(zoomIn()));
    actionCollection->addAction(KStandardAction::ZoomOut,  "zood->out", d->action, SLOT(zoomOut()));

    connect(d->canvasController, SIGNAL( sizeChanged(const QSize & ) ), this, SLOT( setAvailableSize( const QSize & ) ) );

    connect(d->canvasController, SIGNAL( zoomBy(const double ) ), this, SLOT( requestZoomBy( const double ) ) );
}

KoZoomController::~KoZoomController()
{
    delete d;
}

KoZoomAction *KoZoomController::zoomAction() const
{
    return d->action;
}

void KoZoomController::setZoom(double /*zoom*/)
{
}

void KoZoomController::setZoomMode(KoZoomMode::Mode mode)
{
    setZoom(mode, 1);
}

void KoZoomController::setPageSize(const QSizeF &pageSize)
{
    d->pageSize = pageSize;

    if(d->zoomHandler->zoomMode() == KoZoomMode::ZOOM_WIDTH)
        setZoom(KoZoomMode::ZOOM_WIDTH, 0);
    if(d->zoomHandler->zoomMode() == KoZoomMode::ZOOM_PAGE)
        setZoom(KoZoomMode::ZOOM_PAGE, 0);
}

void KoZoomController::setDocumentSize( const QSizeF &documentSize )
{
    d->documentSize = documentSize;
    d->canvasController->setDocumentSize( d->zoomHandler->documentToView(d->documentSize).toSize() );
}

void KoZoomController::setZoom(KoZoomMode::Mode mode, double zoom)
{
    d->zoomHandler->setZoomMode(mode);

    if(mode == KoZoomMode::ZOOM_CONSTANT)
    {
        if(zoom == 0.0) return;
    }
    else if(mode == KoZoomMode::ZOOM_WIDTH)
    {
        zoom = (d->canvasController->viewport()->size().width() - 2*d->fitMargin)
                         / (d->zoomHandler->resolutionX() * d->pageSize.width());
        d->action->setEffectiveZoom(zoom);
    }
    else if(mode == KoZoomMode::ZOOM_PAGE)
    {
        zoom = (d->canvasController->viewport()->size().width() - 2*d->fitMargin)
                         / (d->zoomHandler->resolutionX() * d->pageSize.width());
        zoom = qMin(zoom, (d->canvasController->viewport()->size().height() - 2*d->fitMargin)
                     / (d->zoomHandler->resolutionY() * d->pageSize.height()));

        d->action->setEffectiveZoom(zoom);
    }

    d->zoomHandler->setZoom(zoom);
    emit zoomChanged(mode, zoom);

    // Tell the canvasController that the zoom has changed
    // Actually canvasController doesn't know about zoom, but the document in pixels
    // has change as a result of the zoom change
    d->canvasController->setDocumentSize(
            QSize( int(0.5 + d->zoomHandler->documentToViewX(d->documentSize.width())),
                   int(0.5 + d->zoomHandler->documentToViewY(d->documentSize.height())) ) );

    // Finally ask the canvasController to recenter
    d->canvasController->recenterPreferred();
}

void KoZoomController::setAvailableSize(const QSize &/*size*/)
{
    if(d->zoomHandler->zoomMode() == KoZoomMode::ZOOM_WIDTH)
        setZoom(KoZoomMode::ZOOM_WIDTH, 0);
    if(d->zoomHandler->zoomMode() == KoZoomMode::ZOOM_PAGE)
        setZoom(KoZoomMode::ZOOM_PAGE, 0);
}

void KoZoomController::requestZoomBy(const double factor)
{
    double zoom = d->zoomHandler->zoomInPercent() / 100.0;
    d->action->setZoom(factor*zoom);
    setZoom(KoZoomMode::ZOOM_CONSTANT, factor*zoom);
    d->action->setEffectiveZoom(factor*zoom);
}

void KoZoomController::setFitMargin( int margin )
{
    d->fitMargin = margin;
}

#include "KoZoomController.moc"
