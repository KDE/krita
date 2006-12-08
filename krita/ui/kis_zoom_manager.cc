/*
 *  Copyright (C) 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "kis_zoom_manager.h"

#include <QToolBar>

#include <kactioncollection.h>
#include <kstdaction.h>
#include <kicon.h>
#include <kstatusbar.h>

#include <KoView.h>
#include <KoZoomAction.h>

#include <KoViewConverter.h>
#include "kis_view2.h"
#include "kis_canvas2.h"
#include "kis_image.h"
#include "kis_statusbar.h"

KisZoomManager::KisZoomManager( KisView2 * view, KoViewConverter * viewConverter )
    : m_view( view )
    , m_viewConverter( viewConverter )
    , m_zoomAction( 0 )
    , m_zoomIn( 0 )
    , m_zoomOut( 0 )
    , m_actualPixels( 0 )
    , m_actualSize( 0 )
    , m_fitToCanvas( 0 )
{
}

KisZoomManager::~KisZoomManager()
{
}

void KisZoomManager::setup( KActionCollection * actionCollection )
{

    // view actions
    m_zoomAction = new KoZoomAction(KoZoomMode::ZOOM_PIXELS |
                     KoZoomMode::ZOOM_WIDTH | KoZoomMode::ZOOM_PAGE, i18n("Zoom"), KIcon("14_zoom"), KShortcut(), actionCollection, "zoom" );
    connect(m_zoomAction, SIGNAL(zoomChanged(KoZoomMode::Mode, int)),
          this, SLOT(slotZoomChanged(KoZoomMode::Mode, int)));

    QToolBar *tbar = new QToolBar(m_view->KoView::statusBar());
    m_view->KoView::statusBar()->addWidget(tbar);
    tbar->addAction(m_zoomAction);
}

void KisZoomManager::slotActualSize()
{
    slotZoomChanged(KoZoomMode::ZOOM_CONSTANT, 100);
}

void KisZoomManager::slotZoomChanged(KoZoomMode::Mode mode, int zoom)
{
    KoZoomHandler *zoomHandler = (KoZoomHandler*)m_viewConverter;

    if(mode == KoZoomMode::ZOOM_CONSTANT)
    {
        double zoomF = zoom / 100.0;
        if(zoomF == 0.0) return;
        m_view->setZoom(zoomF);
        zoomHandler->setZoom(zoomF);
    }
    kDebug() << "zoom changed to: " << zoom <<  endl;

    zoomHandler->setZoomMode(mode);
    KisImageSP img = m_view->image();
    m_view->canvasBase()->setCanvasSize(
                    int(zoomHandler->documentToViewX(img->width() / img->xRes())),
                    int(zoomHandler->documentToViewY(img->height() / img->yRes())));
    m_view->canvas()->update();
}

void KisZoomManager::slotZoomIn()
{
    KoZoomHandler *zoomHandler = (KoZoomHandler*)m_viewConverter;

    double zoomF = 4;
    m_view->setZoom(zoomF);
    zoomHandler->setZoom(zoomF);

    int zoom = int(zoomF * 100);
    kDebug() << "zoom in. now at: " << zoom <<  endl;

    zoomHandler->setZoomMode(KoZoomMode::ZOOM_CONSTANT);
    KisImageSP img = m_view->image();
    m_view->canvasBase()->setCanvasSize(
                    int(zoomHandler->documentToViewX(img->width() / img->xRes())),
                    int(zoomHandler->documentToViewY(img->height() / img->yRes())));
    m_view->canvas()->update();
}

void KisZoomManager::slotZoomOut()
{
    KoZoomHandler *zoomHandler = (KoZoomHandler*)m_viewConverter;

    double zoomF = 1;
    m_view->setZoom(zoomF);
    zoomHandler->setZoom(zoomF);

    int zoom = int(zoomF * 100);
    kDebug() << "zoom out. now at: " << zoom <<  endl;

    zoomHandler->setZoomMode(KoZoomMode::ZOOM_CONSTANT);
    KisImageSP img = m_view->image();
    m_view->canvasBase()->setCanvasSize(
                    int(zoomHandler->documentToViewX(img->width() / img->xRes())),
                    int(zoomHandler->documentToViewY(img->height() / img->yRes())));
    m_view->canvas()->update();
}

#include "kis_zoom_manager.moc"
