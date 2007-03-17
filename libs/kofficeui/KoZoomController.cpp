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
{
    m_action = new KoZoomAction(KoZoomMode::ZOOM_PIXELS | KoZoomMode::ZOOM_WIDTH | KoZoomMode::ZOOM_PAGE,
                                    i18n("Zoom"),
                                    KIcon("14_zoom"),
                                    KShortcut(),
                                    actionCollection,
                                    "zoom" );
    connect(m_action, SIGNAL(zoomChanged(KoZoomMode::Mode, int)),
            this, SLOT(setZoom(KoZoomMode::Mode, int)));

    connect(m_canvasController, SIGNAL( sizeChanged(const QSize & ) ), this, SLOT( setAvailableSize( const QSize & ) ) );
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
        zoomF = m_canvasController->visibleWidth()
                         / (m_zoomHandler->resolutionX() * m_pageSize.width());
        m_action->setEffectiveZoom(int(100*zoomF+0.5));
kDebug() << "here in ZOOM_WIDTH " << zoomF << endl;
    }
    else if(mode == KoZoomMode::ZOOM_PAGE)
    {
        zoomF = m_canvasController->visibleWidth()
                         / (m_zoomHandler->resolutionX() * m_pageSize.width());
        zoomF = qMin(zoomF,
             m_canvasController->visibleHeight() / (m_zoomHandler->resolutionY() * m_pageSize.height()));

        m_action->setEffectiveZoom(int(100*zoomF+0.5));
    }

 
    m_zoomHandler->setZoom(zoomF);
    emit zoomChanged(mode, zoom);

   // Tell the canvasController that the zoom has changed
    // Actually canvasController doesn't know about zoom, but the document in pixels
    // has change as a result of the zoom change
    m_canvasController->setDocumentSize(
            QSize( int(0.5 + m_zoomHandler->documentToViewX(m_pageSize.width())),
                   int(0.5 + m_zoomHandler->documentToViewY(m_pageSize.height())) ) );
}

void KoZoomController::setZoom(int /*zoom*/)
{
}

void KoZoomController::setAvailableSize(const QSize &/*size*/)
{
    if(m_zoomHandler->zoomMode() == KoZoomMode::ZOOM_WIDTH)
        setZoom(KoZoomMode::ZOOM_WIDTH, 0);
    if(m_zoomHandler->zoomMode() == KoZoomMode::ZOOM_PAGE)
        setZoom(KoZoomMode::ZOOM_PAGE, 0);
}

#include "KoZoomController.moc"
