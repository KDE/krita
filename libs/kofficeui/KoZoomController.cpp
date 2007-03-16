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
#include <KoView.h>
#include <KoZoomAction.h>
#include <KoZoomHandler.h>
#include <KoCanvasController.h>

KoZoomController::KoZoomController(KoCanvasController *co)
{
    m_action = new KoZoomAction(KoZoomMode::ZOOM_PIXELS | KoZoomMode::ZOOM_WIDTH | KoZoomMode::ZOOM_PAGE,
                                    i18n("Zoom"),
                                    KIcon("14_zoom"),
                                    KShortcut(),
                                    actionCollection,
                                    "zoom" );
    connect(m_action, SIGNAL(zoomChanged(KoZoomMode::Mode, int)),
            this, SLOT(setZoom(KoZoomMode::Mode, int)));
}

QAction *KoZoomController::zoomAction() const
{
    return m_action;
}

void KoZoomController::setZoom(double zoom)
{
}

void KoZoomController::setZoomMode(KoZoomMode mode)
{
}

void KoZoomController::setPageSize(const QRectF &pageSize)
{
    if(m_zoomHandler->zoomMode() == KoZoomMode::ZOOM_WIDTH)
        setZoom(KoZoomMode::ZOOM_WIDTH, 0);
    if(m_zoomHandler->zoomMode() == KoZoomMode::ZOOM_PAGE)
        setZoom(KoZoomMode::ZOOM_PAGE, 0);
}

void KoZoomController::setZoom(KoZoomMode::Mode mode, int zoom)
{
    m_zoomHandler->setZoomMode(mode);

//    KisImageSP img = m_view->image();

    double zoomF;
    if(mode == KoZoomMode::ZOOM_CONSTANT)
    {
        zoomF = zoom / 100.0;
        if(zoomF == 0.0) return;
    }
    else if(mode == KoZoomMode::ZOOM_WIDTH)
    {
        zoomF = m_canvasController->visibleWidth() / (m_zoomHandler->resolutionX() * (img->width() / img->xRes()));
        m_action->setEffectiveZoom(int(100*zoomF+0.5));
    }
    else if(mode == KoZoomMode::ZOOM_PAGE)
    {
        zoomF = m_canvasController->visibleWidth() / (m_zoomHandler->resolutionX() * (img->width() / img->xRes()));
        zoomF = qMin(zoomF, m_canvasController->visibleHeight() / (m_zoomHandler->resolutionY() * (img->height() / img->yRes())));

        m_action->setEffectiveZoom(int(100*zoomF+0.5));
    }

    emit zoomChanged(mode, zoom);
    m_zoomHandler->setZoom(zoomF);
    //m_view->canvasBase()->preScale();

    // For smoother zooming in & out, repaint at the right
    // zoomlevel before changing the size of the viewport
    m_canvasController->canvas()->update();
}

void KoZoomController::setZoom(int zoom)
{
}
