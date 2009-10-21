/*
 *  Copyright (C) 2006 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (C) 2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include <QGridLayout>

#include <kactioncollection.h>
#include <kstandardaction.h>
#include <ktoggleaction.h>
#include <kicon.h>
#include <kstatusbar.h>
#include <kis_debug.h>

#include <KoView.h>
#include <KoZoomAction.h>
#include <KoRuler.h>
#include <KoZoomHandler.h>
#include <KoZoomController.h>
#include <KoCanvasController.h>
#include <KoGlobal.h>
#include <KoRulerController.h>

#include "kis_view2.h"
#include "canvas/kis_canvas2.h"
#include "kis_image.h"
#include "kis_statusbar.h"
#include "kis_config.h"

KisZoomManager::KisZoomManager(KisView2 * view, KoZoomHandler * zoomHandler,
                               KoCanvasController * canvasController)
        : m_view(view)
        , m_zoomHandler(zoomHandler)
        , m_canvasController(canvasController)
        , m_horizontalRuler(0)
        , m_verticalRuler(0)
        , m_zoomAction(0)
        , m_zoomIn(0)
        , m_zoomOut(0)
        , m_actualPixels(0)
        , m_actualSize(0)
        , m_fitToCanvas(0)
{
}

KisZoomManager::~KisZoomManager()
{
    KisConfig cfg;
    cfg.setShowRulers(m_showRulersAction->isChecked());
}

void KisZoomManager::setup(KActionCollection * actionCollection)
{
    KisConfig cfg;
    m_zoomController = new KoZoomController(m_canvasController, m_zoomHandler, actionCollection, KoZoomAction::AspectMode, this);
    m_zoomHandler->setZoomMode(KoZoomMode::ZOOM_PIXELS);

    KisImageWSP img = m_view->image();
    m_zoomController->setPageSize(QSizeF(img->width() / img->xRes(), img->height() / img->yRes()));
    m_zoomController->setDocumentSize(QSizeF(img->width() / img->xRes(), img->height() / img->yRes()));

    m_zoomAction = m_zoomController->zoomAction();
    actionCollection->addAction("zoom", m_zoomAction);
    m_view->addStatusBarItem(m_zoomAction->createWidget(m_view->KoView::statusBar()), 0, true);

    m_showRulersAction  = new KToggleAction(i18n("Show Rulers"), this);
    actionCollection->addAction("view_ruler", m_showRulersAction);
    m_showRulersAction->setWhatsThis(i18n("The rulers show the horizontal and vertical positions of the mouse on the image "
                                          "and can be used to position your mouse at the right place on the canvas. <p>Uncheck this to hide the rulers."));
    connect(m_showRulersAction, SIGNAL(toggled(bool)), SLOT(toggleShowRulers(bool)));

    // Put the canvascontroller in a layout so it resizes with us
    QGridLayout * layout = new QGridLayout(m_view);
    layout->setSpacing(0);
    layout->setMargin(0);
    m_view->setLayout(layout);

    bool show = cfg.showRulers();

    m_horizontalRuler = new KoRuler(m_view, Qt::Horizontal, m_zoomHandler);
    m_horizontalRuler->setShowMousePosition(true);
    m_horizontalRuler->setUnit(KoUnit(KoUnit::Point));
    m_horizontalRuler->setVisible(show);
    new KoRulerController(m_horizontalRuler, m_canvasController->canvas()->resourceProvider());
    m_verticalRuler = new KoRuler(m_view, Qt::Vertical, m_zoomHandler);
    m_verticalRuler->setShowMousePosition(true);
    m_verticalRuler->setUnit(KoUnit(KoUnit::Point));
    m_verticalRuler->setVisible(show);
    m_showRulersAction->setChecked(show);

    layout->addWidget(m_horizontalRuler, 0, 1);
    layout->addWidget(m_verticalRuler, 1, 0);
    layout->addWidget(m_canvasController, 1, 1);

    connect(m_canvasController, SIGNAL(canvasOffsetXChanged(int)),
            this, SLOT(pageOffsetChanged()));

    connect(m_canvasController, SIGNAL(canvasOffsetYChanged(int)),
            this, SLOT(pageOffsetChanged()));

    connect(m_canvasController,
            SIGNAL(canvasMousePositionChanged(const QPoint &)),
            SLOT(mousePositionChanged(const QPoint &)));

    connect(m_zoomController, SIGNAL(zoomChanged(KoZoomMode::Mode, qreal)),
            this, SLOT(slotZoomChanged(KoZoomMode::Mode, qreal)));

    connect(m_zoomController, SIGNAL(aspectModeChanged(bool)),
            this, SLOT(changeAspectMode(bool)));
}

void KisZoomManager::mousePositionChanged(const QPoint &pos)
{
    QPoint canvasOffset(m_canvasController->canvasOffsetX(), m_canvasController->canvasOffsetY());
    QPoint viewPos = pos - m_canvasController->canvas()->documentOrigin() - canvasOffset;

    m_horizontalRuler->updateMouseCoordinate(viewPos.x());
    m_verticalRuler->updateMouseCoordinate(viewPos.y());
}

void KisZoomManager::toggleShowRulers(bool show)
{
    m_horizontalRuler->setVisible(show);
    m_verticalRuler->setVisible(show);
}

void KisZoomManager::updateGUI()
{
    KisImageWSP img = m_view->image();

    m_horizontalRuler->setRulerLength(img->width() / img->xRes());
    m_verticalRuler->setRulerLength(img->height() / img->yRes());
}

void KisZoomManager::slotZoomChanged(KoZoomMode::Mode mode, qreal zoom)
{
    Q_UNUSED(mode);
    Q_UNUSED(zoom);
    KisImageWSP img = m_view->image();

    m_view->canvasBase()->preScale();
    m_view->canvasBase()->adjustOrigin();
    m_view->canvas()->update();
}

void KisZoomManager::changeAspectMode(bool aspectMode)
{
    KisImageWSP img = m_view->image();

    if (aspectMode)
        m_zoomHandler->setResolution(img->xRes(), img->yRes());
    else
        m_zoomHandler->setResolutionToStandard();

    m_zoomController->setZoom(m_zoomHandler->zoomMode(), m_zoomHandler->zoom());
}


void KisZoomManager::pageOffsetChanged()
{
    m_horizontalRuler->setOffset(m_canvasController->canvasOffsetX() + m_view->canvasBase()->documentOrigin().x());
    m_verticalRuler->setOffset(m_canvasController->canvasOffsetY() + m_view->canvasBase()->documentOrigin().y());
}


#include "kis_zoom_manager.moc"
