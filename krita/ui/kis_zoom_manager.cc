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


#include <QGridLayout>

#include <kactioncollection.h>
#include <ktoggleaction.h>
#include <kstatusbar.h>
#include <ktoggleaction.h>
#include <kis_debug.h>

#include <KisView.h>
#include <KoZoomAction.h>
#include <KoRuler.h>
#include <KoZoomHandler.h>
#include <KoZoomController.h>
#include <KoCanvasControllerWidget.h>
#include <KoRulerController.h>
#include <KoUnit.h>
#include <KoDpi.h>

#include "KisDocument.h"
#include "KisViewManager.h"
#include "KisView.h"
#include "canvas/kis_canvas2.h"
#include "kis_coordinates_converter.h"
#include "kis_image.h"
#include "kis_statusbar.h"
#include "kis_config.h"
#include "krita_utils.h"
#include "kis_canvas_resource_provider.h"
#include "opengl/kis_opengl.h"


class KisZoomController : public KoZoomController
{
public:
    KisZoomController(KoCanvasController *co, KisCoordinatesConverter *zh, KActionCollection *actionCollection, KoZoomAction::SpecialButtons specialButtons, QObject *parent)
        : KoZoomController(co, zh, actionCollection, false, specialButtons, parent),
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


KisZoomManager::KisZoomManager(QPointer<KisView> view, KoZoomHandler * zoomHandler,
                               KoCanvasController * canvasController)
        : m_view(view)
        , m_zoomHandler(zoomHandler)
        , m_canvasController(canvasController)
        , m_horizontalRuler(0)
        , m_verticalRuler(0)
        , m_zoomAction(0)
        , m_zoomActionWidget(0)
{
}

KisZoomManager::~KisZoomManager()
{
    KisConfig cfg;
    cfg.setShowRulers(m_horizontalRuler->isVisible());

    if (m_zoomActionWidget && !m_zoomActionWidget->parent()) {
        delete m_zoomActionWidget;
    }
}

void KisZoomManager::setup(KActionCollection * actionCollection)
{

    KisImageWSP image = m_view->image();
    if (!image) return;

    connect(image, SIGNAL(sigSizeChanged(const QPointF &, const QPointF &)), this, SLOT(setMinMaxZoom()));

    KisCoordinatesConverter *converter =
        dynamic_cast<KisCoordinatesConverter*>(m_zoomHandler);

    m_zoomController = new KisZoomController(m_canvasController, converter, actionCollection, KoZoomAction::AspectMode, this);
    m_zoomHandler->setZoomMode(KoZoomMode::ZOOM_PIXELS);
    m_zoomHandler->setZoom(1.0);

    m_zoomController->setPageSize(QSizeF(image->width() / image->xRes(), image->height() / image->yRes()));
    m_zoomController->setDocumentSize(QSizeF(image->width() / image->xRes(), image->height() / image->yRes()), true);

    m_zoomAction = m_zoomController->zoomAction();

    setMinMaxZoom();

    m_zoomActionWidget = m_zoomAction->createWidget(0);


    // Put the canvascontroller in a layout so it resizes with us
    QGridLayout * layout = new QGridLayout(m_view);
    layout->setSpacing(0);
    layout->setMargin(0);
    m_view->setLayout(layout);

    m_view->document()->setUnit(KoUnit(KoUnit::Pixel));

    m_horizontalRuler = new KoRuler(m_view, Qt::Horizontal, m_zoomHandler);
    m_horizontalRuler->setShowMousePosition(true);
    m_horizontalRuler->createGuideToolConnection(m_view->canvasBase());

    new KoRulerController(m_horizontalRuler, m_canvasController->canvas()->resourceManager());
    m_verticalRuler = new KoRuler(m_view, Qt::Vertical, m_zoomHandler);
    m_verticalRuler->setShowMousePosition(true);
    m_verticalRuler->createGuideToolConnection(m_view->canvasBase());


    QList<QAction*> unitActions = m_view->createChangeUnitActions(true);
    m_horizontalRuler->setPopupActionList(unitActions);
    m_verticalRuler->setPopupActionList(unitActions);

    connect(m_view->document(), SIGNAL(unitChanged(const KoUnit&)), SLOT(applyRulersUnit(const KoUnit&)));

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

    applyRulersUnit(m_view->document()->unit());

    KisConfig cfg;
    toggleShowRulers(cfg.showRulers());

}

void KisZoomManager::mousePositionChanged(const QPoint &viewPos)
{
    QPoint pt = viewPos - m_rulersOffset;

    m_horizontalRuler->updateMouseCoordinate(pt.x());
    m_verticalRuler->updateMouseCoordinate(pt.y());
}

bool KisZoomManager::horizontalRulerVisible() const
{
    return m_horizontalRuler->isVisible();
}

bool KisZoomManager::verticalRulerVisible() const
{
    return m_verticalRuler->isVisible();
}

void KisZoomManager::toggleShowRulers(bool show)
{
    m_horizontalRuler->setVisible(show);
    m_verticalRuler->setVisible(show);
}

void KisZoomManager::applyRulersUnit(const KoUnit &baseUnit)
{
    m_horizontalRuler->setUnit(KoUnit(baseUnit.type(), m_view->image()->xRes()));
    m_verticalRuler->setUnit(KoUnit(baseUnit.type(), m_view->image()->yRes()));
}

void KisZoomManager::setMinMaxZoom()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    QSize imageSize = image->size();
    qreal minDimension = qMin(imageSize.width(), imageSize.height());
    qreal minZoom = qMin(100.0 / minDimension, 0.1);

    m_zoomAction->setMinimumZoom(minZoom);
    m_zoomAction->setMaximumZoom(90.0);

}

void KisZoomManager::updateGUI()
{
    QRectF widgetRect = m_view->canvasBase()->coordinatesConverter()->imageRectInWidgetPixels();
    QSize documentSize = m_view->canvasBase()->viewConverter()->viewToDocument(widgetRect).toAlignedRect().size();

    m_horizontalRuler->setRulerLength(documentSize.width());
    m_verticalRuler->setRulerLength(documentSize.height());

    applyRulersUnit(m_horizontalRuler->unit());
}

QWidget *KisZoomManager::zoomActionWidget() const
{
    return m_zoomActionWidget;
}

void KisZoomManager::slotZoomChanged(KoZoomMode::Mode mode, qreal zoom)
{
    Q_UNUSED(mode);
    Q_UNUSED(zoom);
    m_view->canvasBase()->notifyZoomChanged();

    qreal humanZoom = zoom * 100.0;

// XXX: KOMVC -- this is very irritating in MDI mode

    if (m_view->viewManager()) {
        m_view->viewManager()->
                showFloatingMessage(
                    i18nc("floating message about zoom", "Zoom: %1 \%",
                          KritaUtils::prettyFormatReal(humanZoom)),
                    QIcon(), 500, KisFloatingMessage::Low, Qt::AlignCenter);
    }
    qreal scaleX, scaleY;
    m_view->canvasBase()->coordinatesConverter()->imageScale(&scaleX, &scaleY);

    if (scaleX != scaleY) {
        qWarning() << "WARNING: Zoom is not isotropic!"  << ppVar(scaleX) << ppVar(scaleY) << ppVar(qFuzzyCompare(scaleX, scaleY));
    }

    // zoom by average of x and y
    const qreal effectiveZoom = 0.5 * (scaleX + scaleY);
    m_view->canvasBase()->resourceManager()->setResource(KisCanvasResourceProvider::EffectiveZoom, effectiveZoom);

    nofityLevelOfDetailChange(effectiveZoom);
}

void KisZoomManager::nofityLevelOfDetailChange(qreal zoom)
{
    KisCanvas2 *kritaCanvas = m_view->canvasBase();

    int lod = 0;

#ifdef HAVE_OPENGL

    if (!kritaCanvas->canvasIsOpenGL() ||
        !KisOpenGL::supportsGLSL13()) {

        qWarning() << "WARNING: Level of Detail functionality is available only with openGL + GLSL 1.3 support";
    } else {
        KisConfig cfg;
        const int maxLod = cfg.numMipmapLevels();

        lod = qMin(maxLod, qMax(0, qFloor(log2(1.0 / zoom))));
        qDebug() << ppVar(lod);
    }
#endif // HAVE_OPENGL


    m_view->image()->setDesiredLevelOfDetail(lod);
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
    m_rulersOffset = widgetRect.topLeft().toPoint();

    m_horizontalRuler->setOffset(m_rulersOffset.x());
    m_verticalRuler->setOffset(m_rulersOffset.y());
}

void KisZoomManager::zoomTo100()
{
    m_zoomController->setZoom(KoZoomMode::ZOOM_CONSTANT, 1.0);
    m_view->canvasBase()->notifyZoomChanged();
}

void KisZoomManager::showGuides(bool toggle)
{
    m_view->document()->guidesData().setShowGuideLines(toggle);
    m_view->canvasBase()->canvasWidget()->update();
}


#include "kis_zoom_manager.moc"
