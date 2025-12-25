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
#include "kis_snap_line_strategy.h"
#include "kis_guides_config.h"
#include "kis_guides_manager.h"
#include <kis_config_notifier.h>


// A delay longer than 80 ms is needed for a visibly smoother canvas updates
// (due to the canvas getting affected by the frequent ruler updates).
static constexpr int guiUpdateDelay = 160;

KisZoomManager::KisZoomManager(QPointer<KisView> view, KoZoomHandler * zoomHandler,
                               KoCanvasController * canvasController)
        : m_view(view)
        , m_zoomHandler(zoomHandler)
        , m_canvasController(canvasController)
        , m_zoomChangedCompressor(guiUpdateDelay, KisSignalCompressor::FIRST_ACTIVE)
        , m_documentRectChangedCompressor(guiUpdateDelay, KisSignalCompressor::FIRST_ACTIVE)
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

void KisZoomManager::setup(KisKActionCollection * actionCollection)
{

    KisImageWSP image = m_view->image();
    if (!image) return;

    m_zoomAction = new KoZoomAction(i18n("Zoom"), this);
    m_zoomActionWidget = m_zoomAction->createWidget(0);

    connect(m_zoomAction, &KoZoomAction::zoomChanged, this,
        [this](KoZoomMode::Mode mode, qreal zoom) {
            m_canvasController->setZoom(mode, zoom);
        });
    connect(m_canvasController->proxyObject, &KoCanvasControllerProxyObject::zoomStateChanged, m_zoomAction, &KoZoomAction::slotZoomStateChanged);
    m_zoomAction->slotZoomStateChanged(m_canvasController->zoomState());

    // Put the canvascontroller in a layout so it resizes with us
    QGridLayout * layout = new QGridLayout(m_view);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

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
    applyRulersUnit(m_view->document()->unit());

    layout->addWidget(m_horizontalRuler, 0, 1);
    layout->addWidget(m_verticalRuler, 1, 0);
    layout->addWidget(static_cast<KoCanvasControllerWidget*>(m_canvasController), 1, 1);

    connect(m_canvasController->proxyObject, &KoCanvasControllerProxyObject::zoomStateChanged,
            &m_zoomChangedCompressor, &KisSignalCompressor::start);
    connect(m_canvasController->proxyObject, &KoCanvasControllerProxyObject::documentRectInWidgetPixelsChanged,
            &m_documentRectChangedCompressor, &KisSignalCompressor::start);
    connect(&m_zoomChangedCompressor, &KisSignalCompressor::timeout,
            this,
            &KisZoomManager::slotUpdateGuiAfterZoomChange);
    connect(&m_documentRectChangedCompressor,
            &KisSignalCompressor::timeout,
            this,
            &KisZoomManager::slotUpdateGuiAfterDocumentRectChanged);

    connect(KisConfigNotifier::instance(), &KisConfigNotifier::configChanged,
            this, &KisZoomManager::slotConfigChanged);
    slotConfigChanged();
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

void KisZoomManager::syncOnImageResolutionChange()
{
    m_documentRectChangedCompressor.start();
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

void KisZoomManager::mousePositionChanged(const QPoint &viewPos)
{
    QPoint pt = viewPos - m_cachedRulersRect.topLeft();

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
                showFloatingZoomMessage(
                    i18nc("floating message about zoom", "Zoom: %1 %",
                          KritaUtils::prettyFormatReal(humanZoom)));
    }

    updateCurrentZoomResource();
}

KoZoomAction *KisZoomManager::zoomAction() const
{
    return m_zoomAction;
}

QWidget *KisZoomManager::zoomActionWidget() const
{
    return m_zoomActionWidget;
}

void KisZoomManager::slotUpdateGuiAfterDocumentRectChanged()
{
    const QRectF widgetRect = m_view->canvasBase()->coordinatesConverter()->imageRectInWidgetPixels();
    const QRect alignedWidgetRect = widgetRect.toAlignedRect();

    if (m_cachedRulersRect == alignedWidgetRect) return;

    applyRulersUnit(m_horizontalRuler->unit());

    // A bit weird conversion: convert the widget rect as if it were unrotated
    const QRectF documentRect = m_view->canvasBase()->coordinatesConverter()->flakeToDocument(widgetRect);
    const QSize documentSize = documentRect.toAlignedRect().size();

    m_horizontalRuler->setRulerLength(documentSize.width());
    m_verticalRuler->setRulerLength(documentSize.height());

    m_horizontalRuler->setOffset(alignedWidgetRect.x());
    m_verticalRuler->setOffset(alignedWidgetRect.y());

    m_cachedRulersRect = alignedWidgetRect;
}

void KisZoomManager::zoomTo100()
{
    m_canvasController->setZoom(KoZoomMode::ZOOM_CONSTANT, 1.0);
}

void KisZoomManager::slotZoomIn()
{
    m_canvasController->zoomIn();
}

void KisZoomManager::slotZoomOut()
{
    m_canvasController->zoomOut();
}

void KisZoomManager::slotZoomToFit()
{
    m_canvasController->setZoom(KoZoomMode::ZOOM_PAGE, 0);
}

void KisZoomManager::slotZoomToFitWidth()
{
    m_canvasController->setZoom(KoZoomMode::ZOOM_WIDTH, 0);
}
void KisZoomManager::slotZoomToFitHeight()
{
    m_canvasController->setZoom(KoZoomMode::ZOOM_HEIGHT, 0);
}

void KisZoomManager::slotToggleZoomToFit()
{
    KoZoomMode::Mode currentZoomMode = m_zoomHandler->zoomMode();
    if (currentZoomMode == KoZoomMode::ZOOM_CONSTANT) {
        m_previousZoomLevel = m_zoomHandler->zoom();
        m_previousZoomPoint = m_canvasController->preferredCenter();
        m_canvasController->setZoom(m_previousZoomMode, m_previousZoomLevel);
    }
    else {
        m_previousZoomMode = currentZoomMode;
        m_canvasController->setZoom(KoZoomMode::ZOOM_CONSTANT, m_previousZoomLevel);
        m_canvasController->setPreferredCenter(m_previousZoomPoint);
    }
}

void KisZoomManager::slotConfigChanged()
{
    KisConfig cfg(true);

    KisCoordinatesConverter *converter =
        dynamic_cast<KisCoordinatesConverter*>(m_zoomHandler);
    KIS_ASSERT_RECOVER_RETURN(converter);

    const int oldZoomMarginSize = converter->zoomMarginSize();
    if (oldZoomMarginSize != cfg.zoomMarginSize()) {
        converter->setZoomMarginSize(cfg.zoomMarginSize());
        if (converter->zoomMode() != KoZoomMode::ZOOM_CONSTANT) {
            m_canvasController->setZoom(converter->zoomMode(), converter->zoom());
        }
    }
}


