/*
 *  SPDX-FileCopyrightText: 2006, 2010 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2009 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_zoom_manager.h"


#include <QGridLayout>

#include <kactioncollection.h>
#include <ktoggleaction.h>
#include <kis_debug.h>

#include <KisView.h>
#include <KoZoomAction.h>
#include <KoRuler.h>
#include <KoZoomHandler.h>
#include <KoZoomController.h>
#include <KoCanvasControllerWidget.h>
#include <KoUnit.h>

#include "KisDocument.h"
#include "KisViewManager.h"
#include "canvas/kis_canvas2.h"
#include "kis_coordinates_converter.h"
#include "kis_image.h"
#include "kis_statusbar.h"
#include "kis_config.h"
#include "krita_utils.h"
#include "kis_canvas_resource_provider.h"
#include "kis_lod_transform.h"
#include "kis_snap_line_strategy.h"
#include "kis_guides_config.h"
#include "kis_guides_manager.h"


class KisZoomController : public KoZoomController
{
public:
    KisZoomController(KoCanvasController *co, KisCoordinatesConverter *zh, KisKActionCollection *actionCollection, QObject *parent)
        : KoZoomController(co, zh, actionCollection, parent),
          m_converter(zh)
    {
    }

protected:
    QSizeF documentToViewport(const QSizeF &size) override {
        QRectF docRect(QPointF(), size);
        QSizeF viewport = m_converter->documentToWidget(docRect).size();
        QPointF adjustedViewport = m_converter->snapToDevicePixel(QPointF(viewport.width(), viewport.height()));
        return QSizeF(adjustedViewport.x(), adjustedViewport.y());
    }

private:
    KisCoordinatesConverter *m_converter;
};


KisZoomManager::KisZoomManager(QPointer<KisView> view, KoZoomHandler * zoomHandler,
                               KoCanvasController * canvasController)
        : m_view(view)
        , m_zoomHandler(zoomHandler)
        , m_canvasController(canvasController)
        , m_guiUpdateCompressor(80, KisSignalCompressor::FIRST_ACTIVE)
        , m_previousZoomMode(KoZoomMode::ZOOM_PAGE)
        , m_previousZoomPoint(QPointF(0.0, 0.0))
{
}

KisZoomManager::~KisZoomManager()
{
    if (m_zoomActionWidget && !m_zoomActionWidget->parent()) {
        delete m_zoomActionWidget;
    }
}

void KisZoomManager::updateScreenResolution(QWidget *parentWidget)
{
    if (qFuzzyCompare(parentWidget->physicalDpiX(), m_physicalDpiX) &&
        qFuzzyCompare(parentWidget->physicalDpiY(), m_physicalDpiY) &&
        qFuzzyCompare(parentWidget->devicePixelRatioF(), m_devicePixelRatio)) {

        return;
    }

    m_physicalDpiX = parentWidget->physicalDpiX();
    m_physicalDpiY = parentWidget->physicalDpiY();
    m_devicePixelRatio = parentWidget->devicePixelRatioF();

    KisCoordinatesConverter *converter =
        dynamic_cast<KisCoordinatesConverter*>(m_zoomHandler);
    KIS_ASSERT_RECOVER_RETURN(converter);

    converter->setDevicePixelRatio(m_devicePixelRatio);

    changeCanvasMappingMode(m_canvasMappingMode);
}

void KisZoomManager::setup(KisKActionCollection * actionCollection)
{

    KisImageWSP image = m_view->image();
    if (!image) return;

    connect(image, SIGNAL(sigSizeChanged(QPointF,QPointF)), this, SLOT(setMinMaxZoom()));

    KisCoordinatesConverter *converter =
        dynamic_cast<KisCoordinatesConverter*>(m_zoomHandler);

    m_zoomController = new KisZoomController(m_canvasController, converter, actionCollection, this);
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

    m_view->document()->setUnit(KoUnit(KoUnit::Pixel));

    m_horizontalRuler = new KoRuler(m_view, Qt::Horizontal, m_zoomHandler);
    m_horizontalRuler->setShowMousePosition(true);
    m_horizontalRuler->createGuideToolConnection(m_view->canvasBase());
    m_horizontalRuler->setVisible(false); // this prevents the rulers from flashing on to off when a new document is created

    m_verticalRuler = new KoRuler(m_view, Qt::Vertical, m_zoomHandler);
    m_verticalRuler->setShowMousePosition(true);
    m_verticalRuler->createGuideToolConnection(m_view->canvasBase());
    m_verticalRuler->setVisible(false);


    QAction *rulerAction = actionCollection->action("ruler_pixel_multiple2");
    if (m_view->document()->guidesConfig().rulersMultiple2()) {
        m_horizontalRuler->setUnitPixelMultiple2(true);
        m_verticalRuler->setUnitPixelMultiple2(true);
    }
    QList<QAction*> unitActions = m_view->createChangeUnitActions(true);
    unitActions.append(rulerAction);
    m_horizontalRuler->setPopupActionList(unitActions);
    m_verticalRuler->setPopupActionList(unitActions);

    connect(m_view->document(), SIGNAL(unitChanged(KoUnit)), SLOT(applyRulersUnit(KoUnit)));
    connect(rulerAction, SIGNAL(toggled(bool)), SLOT(setRulersPixelMultiple2(bool)));

    layout->addWidget(m_horizontalRuler, 0, 1);
    layout->addWidget(m_verticalRuler, 1, 0);
    layout->addWidget(static_cast<KoCanvasControllerWidget*>(m_canvasController), 1, 1);

    connect(m_canvasController->proxyObject, SIGNAL(canvasOffsetXChanged(int)),
            this, SLOT(pageOffsetChanged()));

    connect(m_canvasController->proxyObject, SIGNAL(canvasOffsetYChanged(int)),
            this, SLOT(pageOffsetChanged()));

    connect(m_zoomController, SIGNAL(zoomChanged(KoZoomMode::Mode,qreal)),
            this, SLOT(slotZoomChanged(KoZoomMode::Mode,qreal)));

    connect(m_zoomController, SIGNAL(canvasMappingModeChanged(bool)),
            this, SLOT(changeCanvasMappingMode(bool)));

    applyRulersUnit(m_view->document()->unit());

    connect(&m_guiUpdateCompressor, SIGNAL(timeout()), SLOT(slotUpdateGuiAfterZoomChange()));
}

void KisZoomManager::updateImageBoundsSnapping()
{
    const QRectF docRect = m_view->canvasBase()->coordinatesConverter()->imageRectInDocumentPixels();
    const QPointF docCenter = docRect.center();

    KoSnapGuide *snapGuide = m_view->canvasBase()->snapGuide();

    {
        KisSnapLineStrategy *boundsSnap =
            new KisSnapLineStrategy(KoSnapGuide::DocumentBoundsSnapping);

        boundsSnap->addLine(Qt::Horizontal, docRect.y());
        boundsSnap->addLine(Qt::Horizontal, docRect.bottom());
        boundsSnap->addLine(Qt::Vertical, docRect.x());
        boundsSnap->addLine(Qt::Vertical, docRect.right());

        snapGuide->overrideSnapStrategy(KoSnapGuide::DocumentBoundsSnapping, boundsSnap);
    }

    {
        KisSnapLineStrategy *centerSnap =
            new KisSnapLineStrategy(KoSnapGuide::DocumentCenterSnapping);

        centerSnap->addLine(Qt::Horizontal, docCenter.y());
        centerSnap->addLine(Qt::Vertical, docCenter.x());

        snapGuide->overrideSnapStrategy(KoSnapGuide::DocumentCenterSnapping, centerSnap);
    }
}

void KisZoomManager::updateCurrentZoomResource()
{
    const qreal effectiveZoom =
        m_view->canvasBase()->coordinatesConverter()->effectiveZoom();
    const qreal effectivePhysicalZoom =
        m_view->canvasBase()->coordinatesConverter()->effectivePhysicalZoom();

    m_view->canvasBase()->resourceManager()->setResource(KoCanvasResource::EffectiveZoom, effectiveZoom);
    m_view->canvasBase()->resourceManager()->setResource(KoCanvasResource::EffectivePhysicalZoom, effectivePhysicalZoom);
}

void KisZoomManager::updateMouseTrackingConnections()
{
    bool value = m_horizontalRuler->isVisible() &&
        m_verticalRuler->isVisible() &&
        m_horizontalRuler->showMousePosition() &&
        m_verticalRuler->showMousePosition();

    m_mouseTrackingConnections.clear();

    if (value) {
        m_mouseTrackingConnections.addConnection(m_canvasController->proxyObject,
                SIGNAL(canvasMousePositionChanged(QPoint)),
                this,
                SLOT(mousePositionChanged(QPoint)));
    }
}

KoRuler* KisZoomManager::horizontalRuler() const
{
    return m_horizontalRuler;
}

KoRuler* KisZoomManager::verticalRuler() const
{
    return m_verticalRuler;
}

qreal KisZoomManager::zoom() const
{
    qreal zoomX;
    qreal zoomY;
    m_zoomHandler->zoom(&zoomX, &zoomY);
    return zoomX;
}

qreal KisZoomManager::resolutionX() const
{
    KisImageSP image = m_view->image();
    return m_canvasMappingMode ? POINT_TO_INCH(m_physicalDpiX) : image->xRes() / m_devicePixelRatio;
}

qreal KisZoomManager::resolutionY() const
{
    KisImageSP image = m_view->image();
    return m_canvasMappingMode ? POINT_TO_INCH(m_physicalDpiY) : image->yRes() / m_devicePixelRatio;
}

void KisZoomManager::mousePositionChanged(const QPoint &viewPos)
{
    QPoint pt = viewPos - m_rulersOffset;

    m_horizontalRuler->updateMouseCoordinate(pt.x());
    m_verticalRuler->updateMouseCoordinate(pt.y());
}

void KisZoomManager::setShowRulers(bool show)
{
    m_horizontalRuler->setVisible(show);
    m_verticalRuler->setVisible(show);
    updateMouseTrackingConnections();
}

void KisZoomManager::setRulersTrackMouse(bool value)
{
    m_horizontalRuler->setShowMousePosition(value);
    m_verticalRuler->setShowMousePosition(value);
    updateMouseTrackingConnections();
}

void KisZoomManager::applyRulersUnit(const KoUnit &baseUnit)
{
    if (m_view && m_view->image()) {
        m_horizontalRuler->setUnit(KoUnit(baseUnit.type(), m_view->image()->xRes()));
        m_verticalRuler->setUnit(KoUnit(baseUnit.type(), m_view->image()->yRes()));
    }
    if (m_view->viewManager()) {
        m_view->viewManager()->guidesManager()->setUnitType(baseUnit.type());
    }
}

void KisZoomManager::setRulersPixelMultiple2(bool enabled)
{
    m_horizontalRuler->setUnitPixelMultiple2(enabled);
    m_verticalRuler->setUnitPixelMultiple2(enabled);
    if (m_view->viewManager()) {
        m_view->viewManager()->guidesManager()->setRulersMultiple2(enabled);
    }
}

void KisZoomManager::slotUpdateGuiAfterZoomChange()
{
    const qreal zoomValue = m_view->canvasBase()->coordinatesConverter()->zoom();
    const qreal humanZoom = zoomValue * 100.0;

    // XXX: KOMVC -- this is very irritating in MDI mode

    if (m_view->viewManager()) {
        m_view->viewManager()->
                showFloatingMessage(
                    i18nc("floating message about zoom", "Zoom: %1 %",
                          KritaUtils::prettyFormatReal(humanZoom)),
                    QIcon(), 500, KisFloatingMessage::Low, Qt::AlignCenter);
    }

    updateCurrentZoomResource();
}

void KisZoomManager::setMinMaxZoom()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    QSize imageSize = image->size();
    qreal minDimension = qMin(imageSize.width(), imageSize.height());
    qreal minZoom = qMin(100.0 / minDimension, 0.1);

    m_zoomAction->setMinMaxZoom(minZoom, 90.0);
}

void KisZoomManager::updateGuiAfterDocumentSize()
{
    QRectF widgetRect = m_view->canvasBase()->coordinatesConverter()->imageRectInWidgetPixels();
    QSize documentSize = m_view->canvasBase()->viewConverter()->viewToDocument(widgetRect).toAlignedRect().size();

    m_horizontalRuler->setRulerLength(documentSize.width());
    m_verticalRuler->setRulerLength(documentSize.height());

    applyRulersUnit(m_horizontalRuler->unit());

    updateZoomMarginSize();
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
    m_guiUpdateCompressor.start();
}

void KisZoomManager::slotZoomLevelsChanged()
{
    m_zoomAction->slotUpdateZoomLevels();
}

void KisZoomManager::slotScrollAreaSizeChanged()
{
    pageOffsetChanged();
    updateGuiAfterDocumentSize();
}

void KisZoomManager::changeCanvasMappingMode(bool canvasMappingMode)
{
    KisImageSP image = m_view->image();

    // changeCanvasMappingMode is called with the same canvasMappingMode when the window is
    // moved across screens. Preserve the old zoomMode if this is the case.
    const KoZoomMode::Mode newMode =
            canvasMappingMode == m_canvasMappingMode ? m_zoomHandler->zoomMode() : KoZoomMode::ZOOM_CONSTANT;
    const qreal newZoom = m_zoomHandler->zoom();

    m_canvasMappingMode = canvasMappingMode;
    m_zoomController->setZoom(newMode, newZoom, resolutionX(), resolutionY());
    m_view->canvasBase()->notifyZoomChanged();

    m_view->viewManager()->updatePrintSizeAction(canvasMappingMode);
}

void KisZoomManager::pageOffsetChanged()
{
    QRectF widgetRect = m_view->canvasBase()->coordinatesConverter()->imageRectInWidgetPixels();
    const QPoint newRulersOffset = widgetRect.topLeft().toPoint();

    if (m_rulersOffset == newRulersOffset) return;

    m_rulersOffset = newRulersOffset;

    m_horizontalRuler->setOffset(m_rulersOffset.x());
    m_verticalRuler->setOffset(m_rulersOffset.y());
}

void KisZoomManager::zoomTo100()
{
    m_zoomController->setZoom(KoZoomMode::ZOOM_CONSTANT, 1.0);
    m_view->canvasBase()->notifyZoomChanged();
}

void KisZoomManager::slotZoomToFit()
{
    m_zoomController->setZoom(KoZoomMode::ZOOM_PAGE, 0);
    m_view->canvasBase()->notifyZoomChanged();
}

void KisZoomManager::slotZoomToFitWidth()
{
    m_zoomController->setZoom(KoZoomMode::ZOOM_WIDTH, 0);
    m_view->canvasBase()->notifyZoomChanged();
}
void KisZoomManager::slotZoomToFitHeight()
{
    m_zoomController->setZoom(KoZoomMode::ZOOM_HEIGHT, 0);
    m_view->canvasBase()->notifyZoomChanged();
}

void KisZoomManager::slotToggleZoomToFit()
{
    KoZoomMode::Mode currentZoomMode = m_zoomController->zoomMode();
    if (currentZoomMode == KoZoomMode::ZOOM_CONSTANT) {
        m_previousZoomLevel = m_zoomController->zoomAction()->effectiveZoom();
        m_previousZoomPoint = m_canvasController->preferredCenter();
        m_zoomController->setZoom(m_previousZoomMode, 0);
    }
    else {
        m_previousZoomMode = currentZoomMode;
        m_zoomController->setZoom(KoZoomMode::ZOOM_CONSTANT, m_previousZoomLevel);
        m_canvasController->setPreferredCenter(m_previousZoomPoint);
    }
    m_view->canvasBase()->notifyZoomChanged();
}

void KisZoomManager::updateZoomMarginSize()
{
    KisConfig cfg(true);
    m_zoomController->setZoomMarginSize(cfg.zoomMarginSize());
}
