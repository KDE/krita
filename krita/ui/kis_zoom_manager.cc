/*
 *  Copyright (C) 2006, 2010 Boudewijn Rempt <boud@valdyas.org>
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
#include <kstatusbar.h>
#include <kis_debug.h>

#include <KoView.h>
#include <KoZoomAction.h>
#include <KoRuler.h>
#include <KoZoomHandler.h>
#include <KoZoomController.h>
#include <KoCanvasControllerWidget.h>
#include <KoGlobal.h>
#include <KoRulerController.h>
#include <KoUnit.h>
#include <KoDpi.h>


#include "kis_doc2.h"
#include "kis_view2.h"
#include "canvas/kis_canvas2.h"
#include "kis_coordinates_converter.h"
#include "kis_image.h"
#include "kis_statusbar.h"
#include "kis_config.h"

class KisZoomController : public KoZoomController
{
public:
    KisZoomController(KoCanvasController *co, KisCoordinatesConverter *zh, KActionCollection *actionCollection, KoZoomAction::SpecialButtons specialButtons, QObject *parent)
        : KoZoomController(co, zh, actionCollection, specialButtons, parent),
          m_converter(zh)
    {
    }

protected:
    QSize documentToViewport(const QSizeF &size) {
        QRectF docRect(QPointF(), size);
        return m_converter->documentToWidget(docRect).toRect().size();
    }

private:
    KisCoordinatesConverter *m_converter;
};


KisZoomManager::KisZoomManager(KisView2 * view, KoZoomHandler * zoomHandler,
                               KoCanvasController * canvasController)
        : m_view(view)
        , m_zoomHandler(zoomHandler)
        , m_canvasController(canvasController)
        , m_horizontalRuler(0)
        , m_verticalRuler(0)
        , m_zoomAction(0)
        , m_zoomActionWidget(0)
        , m_100pct(0)
{
}

KisZoomManager::~KisZoomManager()
{
    m_view->removeStatusBarItem(m_zoomActionWidget);
    KisConfig cfg;
    cfg.setShowRulers(m_showRulersAction->isChecked());
}

void KisZoomManager::setup(KActionCollection * actionCollection)
{
    QSize imageSize = m_view->image()->size();
    qreal minDimension = qMin(imageSize.width(), imageSize.height());
    qreal minZoom = qMin(100.0 / minDimension, 0.1);

    KoZoomMode::setMinimumZoom(minZoom);
    KoZoomMode::setMaximumZoom(90.0);

    KisCoordinatesConverter *converter =
        dynamic_cast<KisCoordinatesConverter*>(m_zoomHandler);

    KisConfig cfg;
    m_zoomController = new KisZoomController(m_canvasController, converter, actionCollection, KoZoomAction::AspectMode, this);
    m_zoomHandler->setZoomMode(KoZoomMode::ZOOM_PIXELS);
    m_zoomHandler->setZoom(1.0);


    KisImageWSP image = m_view->image();
    m_zoomController->setPageSize(QSizeF(image->width() / image->xRes(), image->height() / image->yRes()));
    m_zoomController->setDocumentSize(QSizeF(image->width() / image->xRes(), image->height() / image->yRes()), true);

    m_zoomAction = m_zoomController->zoomAction();
    actionCollection->addAction("zoom", m_zoomAction);
    m_zoomActionWidget = m_zoomAction->createWidget(m_view->KoView::statusBar());
    m_view->addStatusBarItem(m_zoomActionWidget, 0, true);

    m_showRulersAction  = new KToggleAction(i18n("Show Rulers"), this);
    actionCollection->addAction("view_ruler", m_showRulersAction);
    m_showRulersAction->setWhatsThis(i18n("The rulers show the horizontal and vertical positions of the mouse on the image "
                                          "and can be used to position your mouse at the right place on the canvas. <p>Uncheck this to hide the rulers.</p>"));
    connect(m_showRulersAction, SIGNAL(toggled(bool)), SLOT(toggleShowRulers(bool)));

    m_100pct = new KAction(i18n("Reset zoom"), this);
    actionCollection->addAction("zoom_to_100pct", m_100pct);
    m_100pct->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_0 ) );
    connect(m_100pct, SIGNAL(triggered()), SLOT(zoomTo100()));

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
    new KoRulerController(m_horizontalRuler, m_canvasController->canvas()->resourceManager());
    m_verticalRuler = new KoRuler(m_view, Qt::Vertical, m_zoomHandler);
    m_verticalRuler->setShowMousePosition(true);
    m_verticalRuler->setUnit(KoUnit(KoUnit::Point));
    m_verticalRuler->setVisible(show);
    m_showRulersAction->setChecked(show);

    QList<QAction*> unitActions = m_view->createChangeUnitActions();
    m_horizontalRuler->setPopupActionList(unitActions);
    m_verticalRuler->setPopupActionList(unitActions);

    connect(m_view->document(), SIGNAL(unitChanged(const KoUnit&)), m_horizontalRuler, SLOT(setUnit(const KoUnit&)));
    connect(m_view->document(), SIGNAL(unitChanged(const KoUnit&)), m_verticalRuler, SLOT(setUnit(const KoUnit&)));


    layout->addWidget(m_horizontalRuler, 0, 1);
    layout->addWidget(m_verticalRuler, 1, 0);
    layout->addWidget(static_cast<KoCanvasControllerWidget*>(m_canvasController), 1, 1);

    connect(m_canvasController->proxyObject, SIGNAL(canvasOffsetXChanged(int)),
            this, SLOT(pageOffsetChanged()));

    connect(m_canvasController->proxyObject, SIGNAL(canvasOffsetYChanged(int)),
            this, SLOT(pageOffsetChanged()));

    connect(m_canvasController->proxyObject,
            SIGNAL(canvasMousePositionChanged(const QPoint &)),
            SLOT(mousePositionChanged(const QPoint &)));

    connect(m_zoomController, SIGNAL(zoomChanged(KoZoomMode::Mode, qreal)),
            this, SLOT(slotZoomChanged(KoZoomMode::Mode, qreal)));

    connect(m_zoomController, SIGNAL(aspectModeChanged(bool)),
            this, SLOT(changeAspectMode(bool)));
}

void KisZoomManager::mousePositionChanged(const QPoint &pos)
{
    QPoint canvasShift = m_view->canvasBase()->coordinatesConverter()->flakeToWidget(QPointF(m_canvasController->canvasOffsetX(), m_canvasController->canvasOffsetY())).toPoint();
    QPoint viewPos = pos - canvasShift;

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
    QRectF widgetRect = m_view->canvasBase()->coordinatesConverter()->imageRectInWidgetPixels();
    QSize documentSize = m_view->canvasBase()->viewConverter()->viewToDocument(widgetRect).toAlignedRect().size();

    m_horizontalRuler->setRulerLength(documentSize.width());
    m_verticalRuler->setRulerLength(documentSize.height());
}

void KisZoomManager::slotZoomChanged(KoZoomMode::Mode mode, qreal zoom)
{
    Q_UNUSED(mode);
    Q_UNUSED(zoom);

    m_view->canvasBase()->notifyZoomChanged();
}

void KisZoomManager::slotScrollAreaSizeChanged()
{
    pageOffsetChanged();
    updateGUI();
}

void KisZoomManager::changeAspectMode(bool aspectMode)
{
    KisImageWSP image = m_view->image();

    KoZoomMode::Mode newMode = KoZoomMode::ZOOM_CONSTANT;
    qreal newZoom = m_zoomHandler->zoom();

    qreal resolutionX = aspectMode ? image->xRes() : POINT_TO_INCH(static_cast<qreal>(KoDpi::dpiX()));
    qreal resolutionY = aspectMode ? image->yRes() : POINT_TO_INCH(static_cast<qreal>(KoDpi::dpiY()));

    m_zoomController->setZoom(newMode, newZoom, resolutionX, resolutionY);
    m_view->canvasBase()->notifyZoomChanged();
}


void KisZoomManager::pageOffsetChanged()
{
    QRectF widgetRect = m_view->canvasBase()->coordinatesConverter()->imageRectInWidgetPixels();
    QPoint canvasShift = widgetRect.topLeft().toPoint();

    m_horizontalRuler->setOffset(canvasShift.x());
    m_verticalRuler->setOffset(canvasShift.y());
}

void KisZoomManager::zoomTo100()
{
    m_zoomController->setZoom(KoZoomMode::ZOOM_CONSTANT, 1.0);
    m_view->canvasBase()->notifyZoomChanged();
}


#include "kis_zoom_manager.moc"
