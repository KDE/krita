/* This file is part of the KDE project
 * Copyright (C) 2007 Casper Boemann <cbr@boemann.dk>
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

KoZoomController::KoZoomController(KoCanvasController *co, KoZoomHandler *zh, KActionCollection *actionCollection)
    : m_canvasController(co)
    , m_zoomHandler(zh)
    , m_fitMargin( 0 )
{
    m_action = new KoZoomAction(KoZoomMode::ZOOM_PIXELS | KoZoomMode::ZOOM_WIDTH | KoZoomMode::ZOOM_PAGE, i18n("Zoom"), 0);
    connect(m_action, SIGNAL(zoomChanged(KoZoomMode::Mode, double)),
            this, SLOT(setZoom(KoZoomMode::Mode, double)));

    actionCollection->addAction("view_zoom", m_action);
    actionCollection->addAction(KStandardAction::ZoomIn,  "zoom_in", m_action, SLOT(zoomIn()));
    actionCollection->addAction(KStandardAction::ZoomOut,  "zoom_out", m_action, SLOT(zoomOut()));

    connect(m_canvasController, SIGNAL( sizeChanged(const QSize & ) ), this, SLOT( setAvailableSize( const QSize & ) ) );

    connect(m_canvasController, SIGNAL( zoomBy(const double ) ), this, SLOT( requestZoomBy( const double ) ) );
}

KoZoomAction *KoZoomController::zoomAction() const
{
    return m_action;
}

void KoZoomController::setZoom(double /*zoom*/)
{
}

void KoZoomController::setZoomMode(KoZoomMode /*mode*/)
{
}

void KoZoomController::setPageSize(const QSizeF &pageSize)
{
    m_pageSize = pageSize;

    if(m_zoomHandler->zoomMode() == KoZoomMode::ZOOM_WIDTH)
        setZoom(KoZoomMode::ZOOM_WIDTH, 0);
    if(m_zoomHandler->zoomMode() == KoZoomMode::ZOOM_PAGE)
        setZoom(KoZoomMode::ZOOM_PAGE, 0);
}

void KoZoomController::setDocumentSize( const QSizeF &documentSize )
{
    m_documentSize = documentSize;
    m_canvasController->setDocumentSize(
            QSize( int(0.5 + m_zoomHandler->documentToViewX(m_documentSize.width())),
                   int(0.5 + m_zoomHandler->documentToViewY(m_documentSize.height())) ) );
}

void KoZoomController::setZoom(KoZoomMode::Mode mode, double zoom)
{
    m_zoomHandler->setZoomMode(mode);

    if(mode == KoZoomMode::ZOOM_CONSTANT)
    {
        if(zoom == 0.0) return;
    }
    else if(mode == KoZoomMode::ZOOM_WIDTH)
    {
        zoom = (m_canvasController->viewport()->size().width() - 2*m_fitMargin)
                         / (m_zoomHandler->resolutionX() * m_pageSize.width());
        m_action->setEffectiveZoom(zoom);
    }
    else if(mode == KoZoomMode::ZOOM_PAGE)
    {
        zoom = (m_canvasController->viewport()->size().width() - 2*m_fitMargin)
                         / (m_zoomHandler->resolutionX() * m_pageSize.width());
        zoom = qMin(zoom, (m_canvasController->viewport()->size().height() - 2*m_fitMargin)
                     / (m_zoomHandler->resolutionY() * m_pageSize.height()));

        m_action->setEffectiveZoom(zoom);
    }

    m_zoomHandler->setZoom(zoom);
    emit zoomChanged(mode, zoom);

    // Tell the canvasController that the zoom has changed
    // Actually canvasController doesn't know about zoom, but the document in pixels
    // has change as a result of the zoom change
    m_canvasController->setDocumentSize(
            QSize( int(0.5 + m_zoomHandler->documentToViewX(m_documentSize.width())),
                   int(0.5 + m_zoomHandler->documentToViewY(m_documentSize.height())) ) );

    // Finally ask the canvasController to recenter
    m_canvasController->recenterPreferred();
}

void KoZoomController::setAvailableSize(const QSize &/*size*/)
{
    if(m_zoomHandler->zoomMode() == KoZoomMode::ZOOM_WIDTH)
        setZoom(KoZoomMode::ZOOM_WIDTH, 0);
    if(m_zoomHandler->zoomMode() == KoZoomMode::ZOOM_PAGE)
        setZoom(KoZoomMode::ZOOM_PAGE, 0);
}

void KoZoomController::requestZoomBy(const double factor)
{
    double zoom = m_zoomHandler->zoomInPercent() / 100.0;
    m_action->setZoom(factor*zoom);
    setZoom(KoZoomMode::ZOOM_CONSTANT, factor*zoom);
    m_action->setEffectiveZoom(factor*zoom);
}

void KoZoomController::setFitMargin( int margin )
{
    m_fitMargin = margin;
}

#include "KoZoomController.moc"
