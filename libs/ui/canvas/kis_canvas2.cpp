/* This file is part of the KDE project
 *
 * Copyright (C) 2006, 2010 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) Lukáš Tvrdý <lukast.dev@gmail.com>, (C) 2010
 * Copyright (C) 2011 Silvio Heinrich <plassy@web.de>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA..
 */

#include "kis_canvas2.h"

#include <functional>
#include <numeric>

#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QTime>
#include <QLabel>
#include <QMouseEvent>
#include <QDesktopWidget>

#include <kis_debug.h>

#include <KoUnit.h>
#include <KoShapeManager.h>
#include <KisSelectedShapesProxy.h>
#include <KoColorProfile.h>
#include <KoCanvasControllerWidget.h>
#include <KisDocument.h>
#include <KoSelection.h>
#include <KoShapeController.h>

#include <kis_lod_transform.h>
#include "kis_tool_proxy.h"
#include "kis_coordinates_converter.h"
#include "kis_prescaled_projection.h"
#include "kis_image.h"
#include "kis_image_barrier_locker.h"
#include "kis_undo_adapter.h"
#include "flake/kis_shape_layer.h"
#include "kis_canvas_resource_provider.h"
#include "KisViewManager.h"
#include "kis_config.h"
#include "kis_config_notifier.h"
#include "kis_abstract_canvas_widget.h"
#include "kis_qpainter_canvas.h"
#include "kis_group_layer.h"
#include "flake/kis_shape_controller.h"
#include "kis_node_manager.h"
#include "kis_selection.h"
#include "kis_selection_component.h"
#include "flake/kis_shape_selection.h"
#include "kis_selection_mask.h"
#include "kis_image_config.h"
#include "kis_infinity_manager.h"
#include "kis_signal_compressor.h"
#include "kis_display_color_converter.h"
#include "kis_exposure_gamma_correction_interface.h"
#include "KisView.h"
#include "kis_canvas_controller.h"
#include "kis_grid_config.h"

#include "kis_animation_player.h"
#include "kis_animation_frame_cache.h"
#include "opengl/kis_opengl_canvas2.h"
#include "opengl/kis_opengl.h"
#include "kis_fps_decoration.h"

#include "KoColorConversionTransformation.h"
#include "KisProofingConfiguration.h"

#include <kis_favorite_resource_manager.h>
#include <kis_popup_palette.h>

#include "input/kis_input_manager.h"
#include "kis_painting_assistants_decoration.h"

#include "kis_canvas_updates_compressor.h"
#include "KoZoomController.h"

#include <KisStrokeSpeedMonitor.h>
#include "opengl/kis_opengl_canvas_debugger.h"

#include "kis_algebra_2d.h"
#include "kis_image_signal_router.h"


class Q_DECL_HIDDEN KisCanvas2::KisCanvas2Private
{

public:

    KisCanvas2Private(KoCanvasBase *parent, KisCoordinatesConverter* coordConverter, QPointer<KisView> view, KoCanvasResourceProvider* resourceManager)
        : coordinatesConverter(coordConverter)
        , view(view)
        , shapeManager(parent)
        , selectedShapesProxy(&shapeManager)
        , toolProxy(parent)
        , displayColorConverter(resourceManager, view)
        , regionOfInterestUpdateCompressor(100, KisSignalCompressor::FIRST_INACTIVE)
    {
    }

    KisCoordinatesConverter *coordinatesConverter;
    QPointer<KisView>view;
    KisAbstractCanvasWidget *canvasWidget = 0;
    KoShapeManager shapeManager;
    KisSelectedShapesProxy selectedShapesProxy;
    bool currentCanvasIsOpenGL;
    int openGLFilterMode;
    KisToolProxy toolProxy;
    KisPrescaledProjectionSP prescaledProjection;
    bool vastScrolling;

    KisSignalCompressor canvasUpdateCompressor;
    QRect savedUpdateRect;

    QBitArray channelFlags;
    KisProofingConfigurationSP proofingConfig;
    bool softProofing = false;
    bool gamutCheck = false;
    bool proofingConfigUpdated = false;

    KisPopupPalette *popupPalette = 0;
    KisDisplayColorConverter displayColorConverter;

    KisCanvasUpdatesCompressor projectionUpdatesCompressor;
    KisAnimationPlayer *animationPlayer;
    KisAnimationFrameCacheSP frameCache;
    bool lodAllowedInImage = false;
    bool bootstrapLodBlocked;
    QPointer<KoShapeManager> currentlyActiveShapeManager;
    KisInputActionGroupsMask inputActionGroupsMask = AllActionGroup;

    KisSignalCompressor frameRenderStartCompressor;

    KisSignalCompressor regionOfInterestUpdateCompressor;
    QRect regionOfInterest;

    QRect renderingLimit;
    int isBatchUpdateActive = 0;

    bool effectiveLodAllowedInImage() {
        return lodAllowedInImage && !bootstrapLodBlocked;
    }

    void setActiveShapeManager(KoShapeManager *shapeManager);
};

namespace {
KoShapeManager* fetchShapeManagerFromNode(KisNodeSP node)
{
    KoShapeManager *shapeManager = 0;
    KisSelectionSP selection;

    if (KisLayer *layer = dynamic_cast<KisLayer*>(node.data())) {
        KisShapeLayer *shapeLayer = dynamic_cast<KisShapeLayer*>(layer);
        if (shapeLayer) {
            shapeManager = shapeLayer->shapeManager();

        }
    } else if (KisSelectionMask *mask = dynamic_cast<KisSelectionMask*>(node.data())) {
        selection = mask->selection();
    }

    if (!shapeManager && selection && selection->hasShapeSelection()) {
        KisShapeSelection *shapeSelection = dynamic_cast<KisShapeSelection*>(selection->shapeSelection());
        KIS_ASSERT_RECOVER_RETURN_VALUE(shapeSelection, 0);

        shapeManager = shapeSelection->shapeManager();
    }

    return shapeManager;
}
}

KisCanvas2::KisCanvas2(KisCoordinatesConverter *coordConverter, KoCanvasResourceProvider *resourceManager, KisView *view, KoShapeControllerBase *sc)
    : KoCanvasBase(sc, resourceManager)
    , m_d(new KisCanvas2Private(this, coordConverter, view, resourceManager))
{
    /**
     * While loading LoD should be blocked. Only when GUI has finished
     * loading and zoom level settled down, LoD is given a green
     * light.
     */
    m_d->bootstrapLodBlocked = true;
    connect(view->mainWindow(), SIGNAL(guiLoadingFinished()), SLOT(bootstrapFinished()));
    connect(view->mainWindow(), SIGNAL(screenChanged()), SLOT(slotConfigChanged()));

    KisImageConfig config(false);

    m_d->canvasUpdateCompressor.setDelay(1000 / config.fpsLimit());
    m_d->canvasUpdateCompressor.setMode(KisSignalCompressor::FIRST_ACTIVE);

    m_d->frameRenderStartCompressor.setDelay(1000 / config.fpsLimit());
    m_d->frameRenderStartCompressor.setMode(KisSignalCompressor::FIRST_ACTIVE);
}

void KisCanvas2::setup()
{
    // a bit of duplication from slotConfigChanged()
    KisConfig cfg(true);
    m_d->vastScrolling = cfg.vastScrolling();
    m_d->lodAllowedInImage = cfg.levelOfDetailEnabled();

    createCanvas(cfg.useOpenGL());

    setLodAllowedInCanvas(m_d->lodAllowedInImage);
    m_d->animationPlayer = new KisAnimationPlayer(this);
    connect(m_d->view->canvasController()->proxyObject, SIGNAL(moveDocumentOffset(QPoint)), SLOT(documentOffsetMoved(QPoint)));
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotConfigChanged()));

    /**
     * We switch the shape manager every time vector layer or
     * shape selection is activated. Flake does not expect this
     * and connects all the signals of the global shape manager
     * to the clients in the constructor. To workaround this we
     * forward the signals of local shape managers stored in the
     * vector layers to the signals of global shape manager. So the
     * sequence of signal deliveries is the following:
     *
     * shapeLayer.m_d.canvas.m_shapeManager.selection() ->
     * shapeLayer ->
     * shapeController ->
     * globalShapeManager.selection()
     */

    KisShapeController *kritaShapeController = static_cast<KisShapeController*>(shapeController()->documentBase());
    connect(kritaShapeController, SIGNAL(selectionChanged()),
            this, SLOT(slotSelectionChanged()));
    connect(kritaShapeController, SIGNAL(selectionContentChanged()),
            selectedShapesProxy(), SIGNAL(selectionContentChanged()));
    connect(kritaShapeController, SIGNAL(currentLayerChanged(const KoShapeLayer*)),
            selectedShapesProxy(), SIGNAL(currentLayerChanged(const KoShapeLayer*)));

    connect(&m_d->canvasUpdateCompressor, SIGNAL(timeout()), SLOT(slotDoCanvasUpdate()));

    connect(this, SIGNAL(sigCanvasCacheUpdated()), &m_d->frameRenderStartCompressor, SLOT(start()));
    connect(&m_d->frameRenderStartCompressor, SIGNAL(timeout()), SLOT(updateCanvasProjection()));

    connect(this, SIGNAL(sigContinueResizeImage(qint32,qint32)), SLOT(finishResizingImage(qint32,qint32)));

    connect(&m_d->regionOfInterestUpdateCompressor, SIGNAL(timeout()), SLOT(slotUpdateRegionOfInterest()));

    connect(m_d->view->document(), SIGNAL(sigReferenceImagesChanged()), this, SLOT(slotReferenceImagesChanged()));

    initializeFpsDecoration();
}

void KisCanvas2::initializeFpsDecoration()
{
    KisConfig cfg(true);

    const bool shouldShowDebugOverlay =
        (canvasIsOpenGL() && cfg.enableOpenGLFramerateLogging()) ||
        cfg.enableBrushSpeedLogging();

    if (shouldShowDebugOverlay && !decoration(KisFpsDecoration::idTag)) {
        addDecoration(new KisFpsDecoration(imageView()));

        if (cfg.enableBrushSpeedLogging()) {
            connect(KisStrokeSpeedMonitor::instance(), SIGNAL(sigStatsUpdated()), this, SLOT(updateCanvas()));
        }
    } else if (!shouldShowDebugOverlay && decoration(KisFpsDecoration::idTag)) {
        m_d->canvasWidget->removeDecoration(KisFpsDecoration::idTag);
        disconnect(KisStrokeSpeedMonitor::instance(), SIGNAL(sigStatsUpdated()), this, SLOT(updateCanvas()));
    }
}

KisCanvas2::~KisCanvas2()
{
    if (m_d->animationPlayer->isPlaying()) {
        m_d->animationPlayer->forcedStopOnExit();
    }
    delete m_d;
}

void KisCanvas2::setCanvasWidget(KisAbstractCanvasWidget *widget)
{
    if (m_d->popupPalette) {
        m_d->popupPalette->setParent(widget->widget());
    }

    if (m_d->canvasWidget != 0) {
        widget->setDecorations(m_d->canvasWidget->decorations());

        // Redundant check for the constructor case, see below
        if(viewManager() != 0)
            viewManager()->inputManager()->removeTrackedCanvas(this);
    }

    m_d->canvasWidget = widget;

    // Either tmp was null or we are being called by KisCanvas2 constructor that is called by KisView
    // constructor, so the view manager still doesn't exists.
    if(m_d->canvasWidget != 0 && viewManager() != 0)
        viewManager()->inputManager()->addTrackedCanvas(this);

    if (!m_d->canvasWidget->decoration(INFINITY_DECORATION_ID)) {
        KisInfinityManager *manager = new KisInfinityManager(m_d->view, this);
        manager->setVisible(true);
        m_d->canvasWidget->addDecoration(manager);
    }

    widget->widget()->setAutoFillBackground(false);
    widget->widget()->setAttribute(Qt::WA_OpaquePaintEvent);
    widget->widget()->setMouseTracking(true);
    widget->widget()->setAcceptDrops(true);

    KoCanvasControllerWidget *controller = dynamic_cast<KoCanvasControllerWidget*>(canvasController());
    if (controller && controller->canvas() == this) {
        controller->changeCanvasWidget(widget->widget());
    }
}

bool KisCanvas2::canvasIsOpenGL() const
{
    return m_d->currentCanvasIsOpenGL;
}

KisOpenGL::FilterMode KisCanvas2::openGLFilterMode() const
{
    return KisOpenGL::FilterMode(m_d->openGLFilterMode);
}

void KisCanvas2::gridSize(QPointF *offset, QSizeF *spacing) const
{
    QTransform transform = coordinatesConverter()->imageToDocumentTransform();

    const QPoint intSpacing = m_d->view->document()->gridConfig().spacing();
    const QPoint intOffset = m_d->view->document()->gridConfig().offset();

    QPointF size = transform.map(QPointF(intSpacing));
    spacing->rwidth() = size.x();
    spacing->rheight() = size.y();

    *offset = transform.map(QPointF(intOffset));
}

bool KisCanvas2::snapToGrid() const
{
    return m_d->view->document()->gridConfig().snapToGrid();
}

qreal KisCanvas2::rotationAngle() const
{
    return m_d->coordinatesConverter->rotationAngle();
}

bool KisCanvas2::xAxisMirrored() const
{
    return m_d->coordinatesConverter->xAxisMirrored();
}

bool KisCanvas2::yAxisMirrored() const
{
    return m_d->coordinatesConverter->yAxisMirrored();
}

void KisCanvas2::channelSelectionChanged()
{
    KisImageSP image = this->image();
    m_d->channelFlags = image->rootLayer()->channelFlags();

    m_d->view->viewManager()->blockUntilOperationsFinishedForced(image);

    image->barrierLock();
    m_d->canvasWidget->channelSelectionChanged(m_d->channelFlags);
    startUpdateInPatches(image->bounds());
    image->unlock();
}

void KisCanvas2::addCommand(KUndo2Command *command)
{
    // This method exists to support flake-related operations
    m_d->view->document()->addCommand(command);
}

void KisCanvas2::KisCanvas2Private::setActiveShapeManager(KoShapeManager *shapeManager)
{
    if (shapeManager != currentlyActiveShapeManager) {
        currentlyActiveShapeManager = shapeManager;
        selectedShapesProxy.setShapeManager(shapeManager);
    }
}

KoShapeManager* KisCanvas2::shapeManager() const
{
    KoShapeManager *localShapeManager = this->localShapeManager();

    // sanity check for consistency of the local shape manager
    KIS_SAFE_ASSERT_RECOVER (localShapeManager == m_d->currentlyActiveShapeManager) {
        localShapeManager = globalShapeManager();
    }

    return localShapeManager ? localShapeManager : globalShapeManager();
}

KoSelectedShapesProxy* KisCanvas2::selectedShapesProxy() const
{
    return &m_d->selectedShapesProxy;
}

KoShapeManager* KisCanvas2::globalShapeManager() const
{
    return &m_d->shapeManager;
}

KoShapeManager *KisCanvas2::localShapeManager() const
{
    KisNodeSP node = m_d->view->currentNode();
    KoShapeManager *localShapeManager = fetchShapeManagerFromNode(node);

    if (localShapeManager != m_d->currentlyActiveShapeManager) {
        m_d->setActiveShapeManager(localShapeManager);
    }

    return localShapeManager;
}

void KisCanvas2::updateInputMethodInfo()
{
    // TODO call (the protected) QWidget::updateMicroFocus() on the proper canvas widget...
}

const KisCoordinatesConverter* KisCanvas2::coordinatesConverter() const
{
    return m_d->coordinatesConverter;
}

KoViewConverter* KisCanvas2::viewConverter() const
{
    return m_d->coordinatesConverter;
}

KisInputManager* KisCanvas2::globalInputManager() const
{
    return m_d->view->globalInputManager();
}

KisInputActionGroupsMask KisCanvas2::inputActionGroupsMask() const
{
    return m_d->inputActionGroupsMask;
}

void KisCanvas2::setInputActionGroupsMask(KisInputActionGroupsMask mask)
{
    m_d->inputActionGroupsMask = mask;
}

QWidget* KisCanvas2::canvasWidget()
{
    return m_d->canvasWidget->widget();
}

const QWidget* KisCanvas2::canvasWidget() const
{
    return m_d->canvasWidget->widget();
}


KoUnit KisCanvas2::unit() const
{
    KoUnit unit(KoUnit::Pixel);

    KisImageWSP image = m_d->view->image();
    if (image) {
        if (!qFuzzyCompare(image->xRes(), image->yRes())) {
            warnKrita << "WARNING: resolution of the image is anisotropic"
                       << ppVar(image->xRes())
                       << ppVar(image->yRes());
        }

        const qreal resolution = image->xRes();
        unit.setFactor(resolution);
    }

    return unit;
}

KoToolProxy * KisCanvas2::toolProxy() const
{
    return &m_d->toolProxy;
}

void KisCanvas2::createQPainterCanvas()
{
    m_d->currentCanvasIsOpenGL = false;

    KisQPainterCanvas * canvasWidget = new KisQPainterCanvas(this, m_d->coordinatesConverter, m_d->view);
    m_d->prescaledProjection = new KisPrescaledProjection();
    m_d->prescaledProjection->setCoordinatesConverter(m_d->coordinatesConverter);
    m_d->prescaledProjection->setMonitorProfile(m_d->displayColorConverter.monitorProfile(),
                                                m_d->displayColorConverter.renderingIntent(),
                                                m_d->displayColorConverter.conversionFlags());
    m_d->prescaledProjection->setDisplayFilter(m_d->displayColorConverter.displayFilter());
    canvasWidget->setPrescaledProjection(m_d->prescaledProjection);
    setCanvasWidget(canvasWidget);
}

void KisCanvas2::createOpenGLCanvas()
{
    KisConfig cfg(true);
    m_d->openGLFilterMode = cfg.openGLFilteringMode();
    m_d->currentCanvasIsOpenGL = true;

    KisOpenGLCanvas2 *canvasWidget = new KisOpenGLCanvas2(this, m_d->coordinatesConverter, 0, m_d->view->image(), &m_d->displayColorConverter);
    m_d->frameCache = KisAnimationFrameCache::getFrameCache(canvasWidget->openGLImageTextures());

    setCanvasWidget(canvasWidget);
}

void KisCanvas2::createCanvas(bool useOpenGL)
{
    // deinitialize previous canvas structures
    m_d->prescaledProjection = 0;
    m_d->frameCache = 0;

    KisConfig cfg(true);
    QDesktopWidget dw;
    const KoColorProfile *profile = cfg.displayProfile(dw.screenNumber(imageView()));
    m_d->displayColorConverter.notifyOpenGLCanvasIsActive(useOpenGL && KisOpenGL::hasOpenGL());
    m_d->displayColorConverter.setMonitorProfile(profile);

    if (useOpenGL && !KisOpenGL::hasOpenGL()) {
        warnKrita << "Tried to create OpenGL widget when system doesn't have OpenGL\n";
        useOpenGL = false;
    }

    m_d->displayColorConverter.notifyOpenGLCanvasIsActive(useOpenGL);

    if (useOpenGL) {
        createOpenGLCanvas();
        if (cfg.canvasState() == "OPENGL_FAILED") {
            // Creating the opengl canvas failed, fall back
            warnKrita << "OpenGL Canvas initialization returned OPENGL_FAILED. Falling back to QPainter.";
            m_d->displayColorConverter.notifyOpenGLCanvasIsActive(false);
            createQPainterCanvas();
        }
    } else {
        createQPainterCanvas();
    }

    if (m_d->popupPalette) {
        m_d->popupPalette->setParent(m_d->canvasWidget->widget());
    }

}

void KisCanvas2::initializeImage()
{
    KisImageSP image = m_d->view->image();

    m_d->displayColorConverter.setImageColorSpace(image->colorSpace());
    m_d->coordinatesConverter->setImage(image);
    m_d->toolProxy.initializeImage(image);

    connect(image, SIGNAL(sigImageUpdated(QRect)), SLOT(startUpdateCanvasProjection(QRect)), Qt::DirectConnection);
    connect(image->signalRouter(), SIGNAL(sigNotifyBatchUpdateStarted()), SLOT(slotBeginUpdatesBatch()), Qt::DirectConnection);
    connect(image->signalRouter(), SIGNAL(sigNotifyBatchUpdateEnded()), SLOT(slotEndUpdatesBatch()), Qt::DirectConnection);
    connect(image->signalRouter(), SIGNAL(sigRequestLodPlanesSyncBlocked(bool)), SLOT(slotSetLodUpdatesBlocked(bool)), Qt::DirectConnection);

    connect(image, SIGNAL(sigProofingConfigChanged()), SLOT(slotChangeProofingConfig()));
    connect(image, SIGNAL(sigSizeChanged(QPointF,QPointF)), SLOT(startResizingImage()), Qt::DirectConnection);
    connect(image->undoAdapter(), SIGNAL(selectionChanged()), SLOT(slotTrySwitchShapeManager()));

    connect(image, SIGNAL(sigColorSpaceChanged(const KoColorSpace*)), SLOT(slotImageColorSpaceChanged()));
    connect(image, SIGNAL(sigProfileChanged(const KoColorProfile*)), SLOT(slotImageColorSpaceChanged()));

    connectCurrentCanvas();
}

void KisCanvas2::connectCurrentCanvas()
{
    KisImageWSP image = m_d->view->image();

    if (!m_d->currentCanvasIsOpenGL) {
        Q_ASSERT(m_d->prescaledProjection);
        m_d->prescaledProjection->setImage(image);
    }

    startResizingImage();
    setLodAllowedInCanvas(m_d->lodAllowedInImage);

    emit sigCanvasEngineChanged();
}

void KisCanvas2::resetCanvas(bool useOpenGL)
{
    // we cannot reset the canvas before it's created, but this method might be called,
    // for instance when setting the monitor profile.
    if (!m_d->canvasWidget) {
        return;
    }
    KisConfig cfg(true);
    bool needReset = (m_d->currentCanvasIsOpenGL != useOpenGL) ||
        (m_d->currentCanvasIsOpenGL &&
         m_d->openGLFilterMode != cfg.openGLFilteringMode());

    if (needReset) {
        createCanvas(useOpenGL);
        connectCurrentCanvas();
        notifyZoomChanged();
    }
    updateCanvasWidgetImpl();
}

void KisCanvas2::startUpdateInPatches(const QRect &imageRect)
{
    /**
     * We don't do patched loading for openGL canvas, becasue it loads
     * the tiles, which are bascially "patches". Therefore, big chunks
     * of memory are never allocated.
     */
    if (m_d->currentCanvasIsOpenGL) {
        startUpdateCanvasProjection(imageRect);
    } else {
        KisImageConfig imageConfig(true);
        int patchWidth = imageConfig.updatePatchWidth();
        int patchHeight = imageConfig.updatePatchHeight();

        for (int y = 0; y < imageRect.height(); y += patchHeight) {
            for (int x = 0; x < imageRect.width(); x += patchWidth) {
                QRect patchRect(x, y, patchWidth, patchHeight);
                startUpdateCanvasProjection(patchRect);
            }
        }
    }
}

void KisCanvas2::setDisplayFilter(QSharedPointer<KisDisplayFilter> displayFilter)
{
    m_d->displayColorConverter.setDisplayFilter(displayFilter);
    KisImageSP image = this->image();

    m_d->view->viewManager()->blockUntilOperationsFinishedForced(image);

    image->barrierLock();
    m_d->canvasWidget->setDisplayFilter(displayFilter);
    image->unlock();
}

QSharedPointer<KisDisplayFilter> KisCanvas2::displayFilter() const
{
    return m_d->displayColorConverter.displayFilter();
}

void KisCanvas2::slotImageColorSpaceChanged()
{
    KisImageSP image = this->image();

    m_d->view->viewManager()->blockUntilOperationsFinishedForced(image);

    m_d->displayColorConverter.setImageColorSpace(image->colorSpace());

    image->barrierLock();
    m_d->canvasWidget->notifyImageColorSpaceChanged(image->colorSpace());
    image->unlock();
}

KisDisplayColorConverter* KisCanvas2::displayColorConverter() const
{
    return &m_d->displayColorConverter;
}

KisExposureGammaCorrectionInterface* KisCanvas2::exposureGammaCorrectionInterface() const
{
    QSharedPointer<KisDisplayFilter> displayFilter = m_d->displayColorConverter.displayFilter();

    return displayFilter ?
        displayFilter->correctionInterface() :
        KisDumbExposureGammaCorrectionInterface::instance();
}

void KisCanvas2::setProofingOptions(bool softProof, bool gamutCheck)
{
    m_d->proofingConfig = this->image()->proofingConfiguration();
    if (!m_d->proofingConfig) {
        KisImageConfig cfg(false);
        m_d->proofingConfig = cfg.defaultProofingconfiguration();
    }
    KoColorConversionTransformation::ConversionFlags conversionFlags = m_d->proofingConfig->conversionFlags;
#if QT_VERSION >= 0x050700

    if (this->image()->colorSpace()->colorDepthId().id().contains("U")) {
        conversionFlags.setFlag(KoColorConversionTransformation::SoftProofing, softProof);
        if (softProof) {
            conversionFlags.setFlag(KoColorConversionTransformation::GamutCheck, gamutCheck);
        }
    }
#else
    if (this->image()->colorSpace()->colorDepthId().id().contains("U")) {
        conversionFlags |= KoColorConversionTransformation::SoftProofing;
    } else {
        conversionFlags = conversionFlags & ~KoColorConversionTransformation::SoftProofing;
    }
    if (gamutCheck && softProof && this->image()->colorSpace()->colorDepthId().id().contains("U")) {
        conversionFlags |= KoColorConversionTransformation::GamutCheck;
    } else {
        conversionFlags = conversionFlags & ~KoColorConversionTransformation::GamutCheck;
    }
#endif
    m_d->proofingConfig->conversionFlags = conversionFlags;

    m_d->proofingConfigUpdated = true;
    startUpdateInPatches(this->image()->bounds());

}

void KisCanvas2::slotSoftProofing(bool softProofing)
{
    m_d->softProofing = softProofing;
    setProofingOptions(m_d->softProofing, m_d->gamutCheck);
}

void KisCanvas2::slotGamutCheck(bool gamutCheck)
{
    m_d->gamutCheck = gamutCheck;
    setProofingOptions(m_d->softProofing, m_d->gamutCheck);
}

void KisCanvas2::slotChangeProofingConfig()
{
    setProofingOptions(m_d->softProofing, m_d->gamutCheck);
}

void KisCanvas2::setProofingConfigUpdated(bool updated)
{
    m_d->proofingConfigUpdated = updated;
}

bool KisCanvas2::proofingConfigUpdated()
{
    return m_d->proofingConfigUpdated;
}

KisProofingConfigurationSP KisCanvas2::proofingConfiguration() const
{
    if (!m_d->proofingConfig) {
        m_d->proofingConfig = this->image()->proofingConfiguration();
        if (!m_d->proofingConfig) {
            m_d->proofingConfig = KisImageConfig(true).defaultProofingconfiguration();
        }
    }
    return m_d->proofingConfig;
}

void KisCanvas2::startResizingImage()
{
    KisImageWSP image = this->image();
    qint32 w = image->width();
    qint32 h = image->height();

    emit sigContinueResizeImage(w, h);

    QRect imageBounds(0, 0, w, h);
    startUpdateInPatches(imageBounds);
}

void KisCanvas2::finishResizingImage(qint32 w, qint32 h)
{
    m_d->canvasWidget->finishResizingImage(w, h);
}

void KisCanvas2::startUpdateCanvasProjection(const QRect & rc)
{
    KisUpdateInfoSP info = m_d->canvasWidget->startUpdateCanvasProjection(rc, m_d->channelFlags);
    if (m_d->projectionUpdatesCompressor.putUpdateInfo(info)) {
        emit sigCanvasCacheUpdated();
    }
}

void KisCanvas2::updateCanvasProjection()
{
    auto tryIssueCanvasUpdates = [this](const QRect &vRect) {
        if (!m_d->isBatchUpdateActive) {
            // TODO: Implement info->dirtyViewportRect() for KisOpenGLCanvas2 to avoid updating whole canvas
            if (m_d->currentCanvasIsOpenGL) {
                m_d->savedUpdateRect = QRect();

                // we already had a compression in frameRenderStartCompressor, so force the update directly
                slotDoCanvasUpdate();
            } else if (/* !m_d->currentCanvasIsOpenGL && */ !vRect.isEmpty()) {
                m_d->savedUpdateRect = m_d->coordinatesConverter->viewportToWidget(vRect).toAlignedRect();

                // we already had a compression in frameRenderStartCompressor, so force the update directly
                slotDoCanvasUpdate();
            }
        }
    };

    auto uploadData = [this, tryIssueCanvasUpdates](const QVector<KisUpdateInfoSP> &infoObjects) {
        QVector<QRect> viewportRects = m_d->canvasWidget->updateCanvasProjection(infoObjects);
        const QRect vRect = std::accumulate(viewportRects.constBegin(), viewportRects.constEnd(),
                                            QRect(), std::bit_or<QRect>());

        tryIssueCanvasUpdates(vRect);
    };

    bool shouldExplicitlyIssueUpdates = false;

    QVector<KisUpdateInfoSP> infoObjects;
    KisUpdateInfoList originalInfoObjects;
    m_d->projectionUpdatesCompressor.takeUpdateInfo(originalInfoObjects);

    for (auto it = originalInfoObjects.constBegin();
         it != originalInfoObjects.constEnd();
         ++it) {

        KisUpdateInfoSP info = *it;

        const KisMarkerUpdateInfo *batchInfo = dynamic_cast<const KisMarkerUpdateInfo*>(info.data());
        if (batchInfo) {
            if (!infoObjects.isEmpty()) {
                uploadData(infoObjects);
                infoObjects.clear();
            }

            if (batchInfo->type() == KisMarkerUpdateInfo::StartBatch) {
                m_d->isBatchUpdateActive++;
            } else if (batchInfo->type() == KisMarkerUpdateInfo::EndBatch) {
                m_d->isBatchUpdateActive--;
                KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->isBatchUpdateActive >= 0);
                if (m_d->isBatchUpdateActive == 0) {
                    shouldExplicitlyIssueUpdates = true;
                }
            } else if (batchInfo->type() == KisMarkerUpdateInfo::BlockLodUpdates) {
                m_d->canvasWidget->setLodResetInProgress(true);
            } else if (batchInfo->type() == KisMarkerUpdateInfo::UnblockLodUpdates) {
                m_d->canvasWidget->setLodResetInProgress(false);
                shouldExplicitlyIssueUpdates = true;
            }
        } else {
            infoObjects << info;
        }
    }

    if (!infoObjects.isEmpty()) {
        uploadData(infoObjects);
    } else if (shouldExplicitlyIssueUpdates) {
        tryIssueCanvasUpdates(m_d->coordinatesConverter->imageRectInImagePixels());
    }
}

void KisCanvas2::slotBeginUpdatesBatch()
{
    KisUpdateInfoSP info =
        new KisMarkerUpdateInfo(KisMarkerUpdateInfo::StartBatch,
                                      m_d->coordinatesConverter->imageRectInImagePixels());
    m_d->projectionUpdatesCompressor.putUpdateInfo(info);
    emit sigCanvasCacheUpdated();
}

void KisCanvas2::slotEndUpdatesBatch()
{
    KisUpdateInfoSP info =
        new KisMarkerUpdateInfo(KisMarkerUpdateInfo::EndBatch,
                                      m_d->coordinatesConverter->imageRectInImagePixels());
    m_d->projectionUpdatesCompressor.putUpdateInfo(info);
    emit sigCanvasCacheUpdated();
}

void KisCanvas2::slotSetLodUpdatesBlocked(bool value)
{
    KisUpdateInfoSP info =
        new KisMarkerUpdateInfo(value ?
                                KisMarkerUpdateInfo::BlockLodUpdates :
                                KisMarkerUpdateInfo::UnblockLodUpdates,
                                m_d->coordinatesConverter->imageRectInImagePixels());
    m_d->projectionUpdatesCompressor.putUpdateInfo(info);
    emit sigCanvasCacheUpdated();
}

void KisCanvas2::slotDoCanvasUpdate()
{
    /**
     * WARNING: in isBusy() we access openGL functions without making the painting
     * context current. We hope that currently active context will be Qt's one,
     * which is shared with our own.
     */
    if (m_d->canvasWidget->isBusy()) {
        // just restarting the timer
        updateCanvasWidgetImpl(m_d->savedUpdateRect);
        return;
    }

    if (m_d->savedUpdateRect.isEmpty()) {
        m_d->canvasWidget->widget()->update();
        emit updateCanvasRequested(m_d->canvasWidget->widget()->rect());
    } else {
        emit updateCanvasRequested(m_d->savedUpdateRect);
        m_d->canvasWidget->widget()->update(m_d->savedUpdateRect);
    }

    m_d->savedUpdateRect = QRect();
}

void KisCanvas2::updateCanvasWidgetImpl(const QRect &rc)
{
    if (!m_d->canvasUpdateCompressor.isActive() ||
        !m_d->savedUpdateRect.isEmpty()) {
        m_d->savedUpdateRect |= rc;
    }
    m_d->canvasUpdateCompressor.start();
}

void KisCanvas2::updateCanvas()
{
    updateCanvasWidgetImpl();
}

void KisCanvas2::updateCanvas(const QRectF& documentRect)
{
    if (m_d->currentCanvasIsOpenGL && m_d->canvasWidget->decorations().size() > 0) {
        updateCanvasWidgetImpl();
    }
    else {
        // updateCanvas is called from tools, never from the projection
        // updates, so no need to prescale!
        QRect widgetRect = m_d->coordinatesConverter->documentToWidget(documentRect).toAlignedRect();
        widgetRect.adjust(-2, -2, 2, 2);
        if (!widgetRect.isEmpty()) {
            updateCanvasWidgetImpl(widgetRect);
        }
    }
}

void KisCanvas2::disconnectCanvasObserver(QObject *object)
{
    KoCanvasBase::disconnectCanvasObserver(object);
    m_d->view->disconnect(object);
}

void KisCanvas2::notifyZoomChanged()
{
    if (!m_d->currentCanvasIsOpenGL) {
        Q_ASSERT(m_d->prescaledProjection);
        m_d->prescaledProjection->notifyZoomChanged();
    }

    notifyLevelOfDetailChange();
    updateCanvas(); // update the canvas, because that isn't done when zooming using KoZoomAction

    m_d->regionOfInterestUpdateCompressor.start();
}

QRect KisCanvas2::regionOfInterest() const
{
    return m_d->regionOfInterest;
}

void KisCanvas2::slotUpdateRegionOfInterest()
{
    const QRect oldRegionOfInterest = m_d->regionOfInterest;

    const qreal ratio = 0.25;
    const QRect proposedRoi = KisAlgebra2D::blowRect(m_d->coordinatesConverter->widgetRectInImagePixels(), ratio).toAlignedRect();

    const QRect imageRect = m_d->coordinatesConverter->imageRectInImagePixels();

    m_d->regionOfInterest = imageRect.contains(proposedRoi) ? proposedRoi : imageRect;

    if (m_d->regionOfInterest != oldRegionOfInterest) {
        emit sigRegionOfInterestChanged(m_d->regionOfInterest);
    }
}

void KisCanvas2::slotReferenceImagesChanged()
{
    canvasController()->resetScrollBars();
}

void KisCanvas2::setRenderingLimit(const QRect &rc)
{
    m_d->renderingLimit = rc;
}

QRect KisCanvas2::renderingLimit() const
{
    return m_d->renderingLimit;
}

void KisCanvas2::slotTrySwitchShapeManager()
{
    KisNodeSP node = m_d->view->currentNode();

    QPointer<KoShapeManager> newManager;
    newManager = fetchShapeManagerFromNode(node);

    m_d->setActiveShapeManager(newManager);
}

void KisCanvas2::notifyLevelOfDetailChange()
{
    if (!m_d->effectiveLodAllowedInImage()) return;

    const qreal effectiveZoom = m_d->coordinatesConverter->effectiveZoom();

    KisConfig cfg(true);
    const int maxLod = cfg.numMipmapLevels();

    const int lod = KisLodTransform::scaleToLod(effectiveZoom, maxLod);

    if (m_d->effectiveLodAllowedInImage()) {
        KisImageSP image = this->image();
        image->setDesiredLevelOfDetail(lod);
    }
}

const KoColorProfile *  KisCanvas2::monitorProfile()
{
    return m_d->displayColorConverter.monitorProfile();
}

KisViewManager* KisCanvas2::viewManager() const
{
    if (m_d->view) {
        return m_d->view->viewManager();
    }
    return 0;
}

QPointer<KisView>KisCanvas2::imageView() const
{
    return m_d->view;
}

KisImageWSP KisCanvas2::image() const
{
    return m_d->view->image();

}

KisImageWSP KisCanvas2::currentImage() const
{
    return m_d->view->image();
}

void KisCanvas2::documentOffsetMoved(const QPoint &documentOffset)
{
    QPointF offsetBefore = m_d->coordinatesConverter->imageRectInViewportPixels().topLeft();

    qreal devicePixelRatio = m_d->coordinatesConverter->devicePixelRatio();
    // The given offset is in widget logical pixels. In order to prevent fuzzy
    // canvas rendering at 100% pixel-perfect zoom level when devicePixelRatio
    // is not integral, we adjusts the offset to map to whole device pixels.
    // We use qFloor here since the offset can be negative.
    int deviceOffsetX = qFloor(documentOffset.x() * devicePixelRatio);
    int deviceOffsetY = qFloor(documentOffset.y() * devicePixelRatio);
    // These adjusted offsets will be in logical pixel but is aligned in device
    // pixel space for pixel-perfect rendering.
    qreal pixelPerfectOffsetX = deviceOffsetX / devicePixelRatio;
    qreal pixelPerfectOffsetY = deviceOffsetY / devicePixelRatio;
    // FIXME: This is a temporary hack for fixing the canvas under fractional
    //        DPI scaling before a new coordinate system is introduced.
    QPointF offsetAdjusted(pixelPerfectOffsetX, pixelPerfectOffsetY);

    m_d->coordinatesConverter->setDocumentOffset(offsetAdjusted);
    QPointF offsetAfter = m_d->coordinatesConverter->imageRectInViewportPixels().topLeft();

    QPointF moveOffset = offsetAfter - offsetBefore;

    if (!m_d->currentCanvasIsOpenGL)
        m_d->prescaledProjection->viewportMoved(moveOffset);

    emit documentOffsetUpdateFinished();

    updateCanvas();

    m_d->regionOfInterestUpdateCompressor.start();
}

void KisCanvas2::slotConfigChanged()
{
    KisConfig cfg(true);
    m_d->vastScrolling = cfg.vastScrolling();

    resetCanvas(cfg.useOpenGL());
    setDisplayProfile(cfg.displayProfile(QApplication::desktop()->screenNumber(this->canvasWidget())));

    initializeFpsDecoration();
}

void KisCanvas2::refetchDataFromImage()
{
    KisImageSP image = this->image();
    KisImageBarrierLocker l(image);
    startUpdateInPatches(image->bounds());
}

void KisCanvas2::setDisplayProfile(const KoColorProfile *monitorProfile)
{
    if (m_d->displayColorConverter.monitorProfile() == monitorProfile) return;

    m_d->displayColorConverter.setMonitorProfile(monitorProfile);

    {
        KisImageSP image = this->image();
        KisImageBarrierLocker l(image);
        m_d->canvasWidget->setDisplayColorConverter(&m_d->displayColorConverter);
    }

    refetchDataFromImage();
}

void KisCanvas2::addDecoration(KisCanvasDecorationSP deco)
{
    m_d->canvasWidget->addDecoration(deco);
}

KisCanvasDecorationSP KisCanvas2::decoration(const QString& id) const
{
    return m_d->canvasWidget->decoration(id);
}


QPoint KisCanvas2::documentOrigin() const
{
    /**
     * In Krita we don't use document origin anymore.
     * All the centering when needed (vastScrolling < 0.5) is done
     * automatically by the KisCoordinatesConverter.
     */

    return QPoint();
}

QPoint KisCanvas2::documentOffset() const
{
    return m_d->coordinatesConverter->documentOffset();
}

void KisCanvas2::setFavoriteResourceManager(KisFavoriteResourceManager* favoriteResourceManager)
{
    m_d->popupPalette = new KisPopupPalette(viewManager(), m_d->coordinatesConverter, favoriteResourceManager, displayColorConverter()->displayRendererInterface(),
                                            m_d->view->resourceProvider(), m_d->canvasWidget->widget());
    connect(m_d->popupPalette, SIGNAL(zoomLevelChanged(int)), this, SLOT(slotPopupPaletteRequestedZoomChange(int)));
    connect(m_d->popupPalette, SIGNAL(sigUpdateCanvas()), this, SLOT(updateCanvas()));
    connect(m_d->view->mainWindow(), SIGNAL(themeChanged()), m_d->popupPalette, SLOT(slotUpdateIcons()));

    m_d->popupPalette->showPopupPalette(false);
}

void KisCanvas2::slotPopupPaletteRequestedZoomChange(int zoom ) {
    m_d->view->viewManager()->zoomController()->setZoom(KoZoomMode::ZOOM_CONSTANT, (qreal)(zoom/100.0)); // 1.0 is 100% zoom
    notifyZoomChanged();
}

void KisCanvas2::setCursor(const QCursor &cursor)
{
    canvasWidget()->setCursor(cursor);
}

KisAnimationFrameCacheSP KisCanvas2::frameCache() const
{
    return m_d->frameCache;
}

KisAnimationPlayer *KisCanvas2::animationPlayer() const
{
    return m_d->animationPlayer;
}

void KisCanvas2::slotSelectionChanged()
{
    KisShapeLayer* shapeLayer = dynamic_cast<KisShapeLayer*>(viewManager()->activeLayer().data());
    if (!shapeLayer) {
        return;
    }
    m_d->shapeManager.selection()->deselectAll();
    Q_FOREACH (KoShape* shape, shapeLayer->shapeManager()->selection()->selectedShapes()) {
        m_d->shapeManager.selection()->select(shape);
    }
}

bool KisCanvas2::isPopupPaletteVisible() const
{
    if (!m_d->popupPalette) {
        return false;
    }
    return m_d->popupPalette->isVisible();
}


void KisCanvas2::setWrapAroundViewingMode(bool value)
{
    KisCanvasDecorationSP infinityDecoration =
        m_d->canvasWidget->decoration(INFINITY_DECORATION_ID);

    if (infinityDecoration) {
        infinityDecoration->setVisible(!value);
    }

    m_d->canvasWidget->setWrapAroundViewingMode(value);
}

bool KisCanvas2::wrapAroundViewingMode() const
{
    KisCanvasDecorationSP infinityDecoration =
        m_d->canvasWidget->decoration(INFINITY_DECORATION_ID);

    if (infinityDecoration) {
        return !(infinityDecoration->visible());
    }
    return false;
}

void KisCanvas2::bootstrapFinished()
{
    if (!m_d->bootstrapLodBlocked) return;

    m_d->bootstrapLodBlocked = false;
    setLodAllowedInCanvas(m_d->lodAllowedInImage);
}

void KisCanvas2::setLodAllowedInCanvas(bool value)
{
    if (!KisOpenGL::supportsLoD()) {
        qWarning() << "WARNING: Level of Detail functionality is available only with openGL + GLSL 1.3 support";
    }

    m_d->lodAllowedInImage =
        value &&
        m_d->currentCanvasIsOpenGL &&
        KisOpenGL::supportsLoD() &&
        (m_d->openGLFilterMode == KisOpenGL::TrilinearFilterMode ||
         m_d->openGLFilterMode == KisOpenGL::HighQualityFiltering);

    KisImageSP image = this->image();

    if (m_d->effectiveLodAllowedInImage() != !image->levelOfDetailBlocked()) {
        image->setLevelOfDetailBlocked(!m_d->effectiveLodAllowedInImage());
    }

    notifyLevelOfDetailChange();

    KisConfig cfg(false);
    cfg.setLevelOfDetailEnabled(m_d->lodAllowedInImage);
}

bool KisCanvas2::lodAllowedInCanvas() const
{
    return m_d->lodAllowedInImage;
}

void KisCanvas2::slotShowPopupPalette(const QPoint &p)
{
    if (!m_d->popupPalette) {
        return;
    }

    m_d->popupPalette->showPopupPalette(p);
}

KisPaintingAssistantsDecorationSP KisCanvas2::paintingAssistantsDecoration() const
{
    KisCanvasDecorationSP deco = decoration("paintingAssistantsDecoration");
    return qobject_cast<KisPaintingAssistantsDecoration*>(deco.data());
}

KisReferenceImagesDecorationSP KisCanvas2::referenceImagesDecoration() const
{
    KisCanvasDecorationSP deco = decoration("referenceImagesDecoration");
    return qobject_cast<KisReferenceImagesDecoration*>(deco.data());
}
