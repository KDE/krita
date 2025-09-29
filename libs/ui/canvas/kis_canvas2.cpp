/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2006, 2010 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_canvas2.h"

#include <functional>
#include <numeric>

#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QTime>
#include <QMouseEvent>
#include <QScreen>
#include <QScreen>
#include <QWindow>

#include <kis_debug.h>

#include <KoUnit.h>
#include <KoShapeManager.h>
#include <KisSelectedShapesProxy.h>
#include <KoColorProfile.h>
#include <KoCanvasControllerWidget.h>
#include <KisDocument.h>
#include <KoSelection.h>
#include <KoShapeController.h>
#include <KisReferenceImagesLayer.h>

#include <KisUsageLogger.h>

#include <kis_lod_transform.h>
#include "kis_tool_proxy.h"
#include "kis_coordinates_converter.h"
#include "kis_prescaled_projection.h"
#include "kis_image.h"
#include "KisImageBarrierLock.h"
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
#include "KisMainWindow.h"

#include "KisCanvasAnimationState.h"
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

#include <KisStrokeSpeedMonitor.h>
#include "opengl/kis_opengl_canvas_debugger.h"

#include "kis_wrapped_rect.h"
#include "kis_algebra_2d.h"
#include "kis_image_signal_router.h"

#include "KisSnapPixelStrategy.h"
#include "KisDisplayConfig.h"
#include "config-qt-patches-present.h"
#include <KoIcon.h>

#include <config-use-surface-color-management-api.h>
#if KRITA_USE_SURFACE_COLOR_MANAGEMENT_API

#include <surfacecolormanagement/KisSurfaceColorManagerInterface.h>
#include <KisCanvasSurfaceColorSpaceManager.h>
#include <KisPlatformPluginInterfaceFactory.h>

#endif

#include <KisMultiSurfaceStateManager.h>


class Q_DECL_HIDDEN KisCanvas2::KisCanvas2Private
{
    /**
     * Interface that can be void-ed when original canvas has been deleted
     * Providing a void interface is necessary to avoid a KisInputActionGroupsMaskGuard updating
     * an already-deleted canvas
     */
    struct CanvasInputActionGroupsMaskInterface : public KisInputActionGroupsMaskInterface
    {
        KisCanvas2Private * m_canvasPrivateRef = nullptr;
        CanvasInputActionGroupsMaskInterface() = delete;
        CanvasInputActionGroupsMaskInterface(KisCanvas2Private * canvasPrivateRef)
            :m_canvasPrivateRef(canvasPrivateRef)
        { }
        KisInputActionGroupsMask inputActionGroupsMask() const override
        {
            Q_ASSERT(m_canvasPrivateRef); // this method should only be used upon creating a KisInputActionGroupsMaskGuard
            return m_canvasPrivateRef->inputActionGroupsMask;
        }
        void setInputActionGroupsMask(KisInputActionGroupsMask mask) override
        {
            if(m_canvasPrivateRef)
                m_canvasPrivateRef->inputActionGroupsMask = mask;
        }
    }; // class CanvasInputActionGroupsMask

public:

    KisCanvas2Private(KisCanvas2 *parent, KisCoordinatesConverter* coordConverter, QPointer<KisView> view, KoCanvasResourceProvider* resourceManager)
        : q(parent)
        , coordinatesConverter(coordConverter)
        , view(view)
        , shapeManager(parent)
        , selectedShapesProxy(&shapeManager)
        , toolProxy(parent)
        , proofingConfig(new KisProofingConfiguration)
        , displayColorConverter(resourceManager, view)
        , inputActionGroupsMaskInterface(new CanvasInputActionGroupsMaskInterface(this))
        , regionOfInterestUpdateCompressor(100, KisSignalCompressor::FIRST_INACTIVE)
        , referencesBoundsUpdateCompressor(100, KisSignalCompressor::FIRST_INACTIVE)
        , multiSurfaceSetupManager(view)
    {
    }

    ~KisCanvas2Private()
    {
        inputActionGroupsMaskInterface->m_canvasPrivateRef = nullptr;

        // We need to make sure that the QScopedPointer gets freed within the scope
        // of KisCanvas2Private's lifespan. For some reason, this isn't guaranteed
        // and was causing a crash when closing a file when playback is happening
        // and there are multiple images. See Bug: 499658
        animationPlayer.reset();
    }


    KisCanvas2 *q = 0;
    KisCoordinatesConverter *coordinatesConverter = 0;
    QPointer<KisView>view;
    KisAbstractCanvasWidget *canvasWidget = 0;
    KoShapeManager shapeManager;
    KisSelectedShapesProxy selectedShapesProxy;
    bool currentCanvasIsOpenGL = true;
    int openGLFilterMode = 0;
    KisToolProxy toolProxy;
    KisPrescaledProjectionSP prescaledProjection;

#if !KRITA_QT_HAS_UPDATE_COMPRESSION_PATCH
    KisSignalCompressor canvasUpdateCompressor;
#endif
    QRect savedCanvasProjectionUpdateRect;
    QRect savedOverlayUpdateRect;
    bool updateSceneRequested = false;

    QBitArray channelFlags;
    KisProofingConfigurationSP proofingConfig;
    bool proofingConfigUpdated = false;

    KisPopupPalette *popupPalette = 0;
    KisDisplayColorConverter displayColorConverter;

    KisCanvasUpdatesCompressor projectionUpdatesCompressor;
    QScopedPointer<KisCanvasAnimationState> animationPlayer;
    KisAnimationFrameCacheSP frameCache;
    bool lodPreferredInImage = false;
    bool bootstrapLodBlocked = false;
    QPointer<KoShapeManager> currentlyActiveShapeManager;
    KisInputActionGroupsMask inputActionGroupsMask = AllActionGroup;

    QSharedPointer<CanvasInputActionGroupsMaskInterface> inputActionGroupsMaskInterface;

    KisSignalCompressor frameRenderStartCompressor;

    KisSignalCompressor regionOfInterestUpdateCompressor;
    KisSignalCompressor referencesBoundsUpdateCompressor;
    QRect regionOfInterest;
    qreal regionOfInterestMargin = 0.25;

    QRect renderingLimit;
    int isBatchUpdateActive = 0;

    KisMultiSurfaceStateManager multiSurfaceSetupManager;
    std::optional<KisMultiSurfaceStateManager::State> multiSurfaceState;
#if KRITA_USE_SURFACE_COLOR_MANAGEMENT_API
    QScopedPointer<KisCanvasSurfaceColorSpaceManager> surfaceColorManager;
#endif

    bool effectiveLodAllowedInImage() const {
        return lodPreferredInImage && !bootstrapLodBlocked;
    }

    bool lodIsSupported() const {
        return currentCanvasIsOpenGL &&
                KisOpenGL::supportsLoD() &&
                (openGLFilterMode == KisOpenGL::TrilinearFilterMode ||
                 openGLFilterMode == KisOpenGL::HighQualityFiltering);
    }

    void setActiveShapeManager(KoShapeManager *shapeManager);

    QRect docUpdateRectToWidget(const QRectF &docRect);

    KisDisplayConfig::Options overriddenWithProofingConfig(const KisDisplayConfig::Options &options) const {
        if (proofingConfig && proofingConfig->displayFlags.testFlag(KoColorConversionTransformation::SoftProofing)) {
            return { proofingConfig->determineDisplayIntent(options.first),
                     proofingConfig->determineDisplayFlags(options.second) };
        }

        return options;
    }

    KisDisplayConfig::Options effectiveDisplayConfigOptions(const KisConfig &cfg) const {
        return overriddenWithProofingConfig(KisDisplayConfig::optionsFromKisConfig(cfg));
    }

    int currentScreenId() const {
        int canvasScreenNumber = qApp->screens().indexOf(view->currentScreen());

        if (canvasScreenNumber < 0) {
            warnKrita << "Couldn't detect screen that Krita belongs to..." << ppVar(view->currentScreen());
            canvasScreenNumber = 0;
        }
        return canvasScreenNumber;
    }

    void assignChangedMultiSurfaceStateSkipCanvasSurface(const KisMultiSurfaceStateManager::State &newState);
    void assignChangedMultiSurfaceState(const KisMultiSurfaceStateManager::State &newState);
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

KisCanvas2::KisCanvas2(KisCoordinatesConverter *coordConverter, KoCanvasResourceProvider *resourceManager, KisMainWindow *mainWindow, KisView *view, KoShapeControllerBase *sc)
    : KoCanvasBase(sc, resourceManager)
    , m_d(new KisCanvas2Private(this, coordConverter, view, resourceManager))
{
    /**
     * While loading LoD should be blocked. Only when GUI has finished
     * loading and zoom level settled down, LoD is given a green
     * light.
     */
    m_d->bootstrapLodBlocked = true;
    connect(mainWindow, SIGNAL(guiLoadingFinished()), SLOT(bootstrapFinished()));

    KisImageConfig config(false);

#if !KRITA_QT_HAS_UPDATE_COMPRESSION_PATCH
    m_d->canvasUpdateCompressor.setDelay(1000 / config.fpsLimit());
    m_d->canvasUpdateCompressor.setMode(KisSignalCompressor::FIRST_ACTIVE);
#endif

    m_d->frameRenderStartCompressor.setDelay(1000 / config.fpsLimit());
    m_d->frameRenderStartCompressor.setMode(KisSignalCompressor::FIRST_ACTIVE);
    snapGuide()->overrideSnapStrategy(KoSnapGuide::PixelSnapping, new KisSnapPixelStrategy());
}

void KisCanvas2::setup()
{
    // a bit of duplication from slotConfigChanged()
    KisConfig cfg(true);
    m_d->lodPreferredInImage = cfg.levelOfDetailEnabled();
    m_d->regionOfInterestMargin = KisImageConfig(true).animationCacheRegionOfInterestMargin();

    createCanvas(cfg.useOpenGL());

    setLodPreferredInCanvas(m_d->lodPreferredInImage);

    connect(m_d->view->canvasController()->proxyObject, SIGNAL(moveViewportOffset(QPointF, QPointF)), SLOT(viewportOffsetMoved(QPointF, QPointF)));
    connect(m_d->view->canvasController()->proxyObject, SIGNAL(effectiveZoomChanged(qreal)), SLOT(slotEffectiveZoomChanged(qreal)));
    connect(m_d->view->canvasController()->proxyObject, &KoCanvasControllerProxyObject::canvasStateChanged, this, &KisCanvas2::slotCanvasStateChanged);

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

#if !KRITA_QT_HAS_UPDATE_COMPRESSION_PATCH
    connect(&m_d->canvasUpdateCompressor, SIGNAL(timeout()), SLOT(slotDoCanvasUpdate()));
#endif

    connect(this, SIGNAL(sigCanvasCacheUpdated()), &m_d->frameRenderStartCompressor, SLOT(start()));
    connect(&m_d->frameRenderStartCompressor, SIGNAL(timeout()), SLOT(updateCanvasProjection()));

    connect(this, SIGNAL(sigContinueResizeImage(qint32,qint32)), SLOT(finishResizingImage(qint32,qint32)));

    connect(&m_d->regionOfInterestUpdateCompressor, SIGNAL(timeout()), SLOT(slotUpdateRegionOfInterest()));
    connect(&m_d->referencesBoundsUpdateCompressor, SIGNAL(timeout()), SLOT(slotUpdateReferencesBounds()));

    connect(m_d->view->document(), SIGNAL(sigReferenceImagesChanged()), &m_d->referencesBoundsUpdateCompressor, SLOT(start()));

    initializeFpsDecoration();

    m_d->animationPlayer.reset(new KisCanvasAnimationState(this));
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
    delete m_d;
}

void KisCanvas2::setCanvasWidget(KisAbstractCanvasWidget *widget)
{
    if (m_d->popupPalette) {
        m_d->popupPalette->setParent(widget->widget());
    }

    if (m_d->canvasWidget) {
        /**
         * We are switching the canvas type. We should reinitialize our
         * connections to decorations and input manager
         */

        widget->setDecorations(m_d->canvasWidget->decorations());

        if(viewManager()) {
            viewManager()->inputManager()->removeTrackedCanvas(this);
            m_d->canvasWidget = widget;
            viewManager()->inputManager()->addTrackedCanvas(this);
        } else {
            m_d->canvasWidget = widget;
        }
    } else {
        m_d->canvasWidget = widget;
    }

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


#if KRITA_USE_SURFACE_COLOR_MANAGEMENT_API
    /**
     * Load platform plugin for surface color management and set
     * the surface color space to sRGB exactly
     */
    if (KisPlatformPluginInterfaceFactory::instance()->surfaceColorManagedByOS()) {
        m_d->surfaceColorManager.reset();

        QWindow *mainWindowNativeWindow = m_d->view->mainWindow()->windowHandle();
        QWindow *nativeWindow = widget->widget()->windowHandle();

        if (nativeWindow && nativeWindow != mainWindowNativeWindow) {
            std::unique_ptr<KisSurfaceColorManagerInterface> iface(
                KisPlatformPluginInterfaceFactory::instance()->createSurfaceColorManager(nativeWindow));

            // if surfaceColorManagedByOS() is true, then interface is guaranteed to
            // be present
            KIS_SAFE_ASSERT_RECOVER_NOOP(iface);

            if (iface) {
                m_d->surfaceColorManager.reset(
                    new KisCanvasSurfaceColorSpaceManager(iface.release(),
                                                          m_d->multiSurfaceState->surfaceMode,
                                                          m_d->multiSurfaceState->multiConfig.options(),
                                                          this));

                connect(m_d->surfaceColorManager.data(),
                        &KisCanvasSurfaceColorSpaceManager::sigDisplayConfigChanged,
                        this,
                        &KisCanvas2::slotSurfaceFormatChanged);
            }
        } else {
            qWarning() << "WARNING: created non-native Krita canvas on managed platform,"
                       << "its color space will be limited to sRGB";
        }
    }
#endif
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
    m_d->view->image()->undoAdapter()->addCommand(command);
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

const KisCoordinatesConverter* KisCanvas2::coordinatesConverter() const
{
    return m_d->coordinatesConverter;
}

const KoViewConverter *KisCanvas2::viewConverter() const
{
    return m_d->coordinatesConverter;
}

KoViewConverter *KisCanvas2::viewConverter()
{
    return m_d->coordinatesConverter;
}

KisInputManager* KisCanvas2::globalInputManager() const
{
    return m_d->view->globalInputManager();
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

    m_d->multiSurfaceState =
        m_d->multiSurfaceSetupManager.createInitializingConfig(false, m_d->currentScreenId(), m_d->proofingConfig);

    KisQPainterCanvas * canvasWidget = new KisQPainterCanvas(this, m_d->coordinatesConverter, m_d->view);
    m_d->prescaledProjection = new KisPrescaledProjection();
    m_d->prescaledProjection->setCoordinatesConverter(m_d->coordinatesConverter);
    m_d->prescaledProjection->setDisplayConfig(m_d->displayColorConverter.displayConfig());
    m_d->prescaledProjection->setDisplayFilter(m_d->displayColorConverter.displayFilter());
    canvasWidget->setPrescaledProjection(m_d->prescaledProjection);
    setCanvasWidget(canvasWidget);
}

void KisCanvas2::createOpenGLCanvas()
{
    KisConfig cfg(true);
    m_d->openGLFilterMode = cfg.openGLFilteringMode();
    m_d->currentCanvasIsOpenGL = true;

    m_d->multiSurfaceState =
        m_d->multiSurfaceSetupManager.createInitializingConfig(true, m_d->currentScreenId(), m_d->proofingConfig);

    auto bitDepthMode =
        cfg.effectiveCanvasSurfaceBitDepthMode(QSurfaceFormat::defaultFormat())
            == KisConfig::CanvasSurfaceBitDepthMode::Depth10Bit ?
        KisOpenGLCanvas2::BitDepthMode::Depth10Bit :
        KisOpenGLCanvas2::BitDepthMode::Depth8Bit;

    KisOpenGLCanvas2 *canvasWidget = new KisOpenGLCanvas2(this,
                                                          m_d->coordinatesConverter,
                                                          0,
                                                          m_d->view->image(),
                                                          m_d->multiSurfaceState->multiConfig.canvasDisplayConfig(),
                                                          m_d->displayColorConverter.displayFilter(),
                                                          bitDepthMode);
    m_d->frameCache = KisAnimationFrameCache::getFrameCache(canvasWidget->openGLImageTextures());

    setCanvasWidget(canvasWidget);
}

void KisCanvas2::createCanvas(bool useOpenGL)
{
    // deinitialize previous canvas structures
    m_d->prescaledProjection = 0;
    m_d->frameCache = 0;

    KisConfig cfg(true);

    if (useOpenGL && !KisOpenGL::hasOpenGL()) {
        warnKrita << "Tried to create OpenGL widget when system doesn't have OpenGL\n";
        useOpenGL = false;
    }

    if (useOpenGL) {
        createOpenGLCanvas();
        if (cfg.canvasState() == "OPENGL_FAILED") {
            // Creating the opengl canvas failed, fall back
            warnKrita << "OpenGL Canvas initialization returned OPENGL_FAILED. Falling back to QPainter.";
            createQPainterCanvas();
        }
    } else {
        createQPainterCanvas();
    }

    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->multiSurfaceState);
    m_d->displayColorConverter.setMultiSurfaceDisplayConfig(m_d->multiSurfaceState->multiConfig);

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
    fetchProofingOptions();
}

void KisCanvas2::disconnectImage()
{
    KisImageSP image = m_d->view->image();

    /**
     * We explicitly don't use barrierLock() here, because we don't care about
     * all the updates completed (we don't use image's content). We only need to
     * guarantee that the image will not try to access us in a multithreaded way,
     * while we are being destroyed.
     */

    image->immediateLockForReadOnly();
    disconnect(image.data(), 0, this, 0);
    image->unlock();
}

void KisCanvas2::connectCurrentCanvas()
{
    KisImageWSP image = m_d->view->image();

    if (!m_d->currentCanvasIsOpenGL) {
        Q_ASSERT(m_d->prescaledProjection);
        m_d->prescaledProjection->setImage(image);
    }

    startResizingImage();
    setLodPreferredInCanvas(m_d->lodPreferredInImage);

    Q_EMIT sigCanvasEngineChanged();
}

void KisCanvas2::resetCanvas(bool useOpenGL)
{
    // we cannot reset the canvas before it's created, but this method might be called,
    // for instance when setting the monitor profile.
    if (!m_d->canvasWidget) {
        return;
    }

    KisConfig cfg(true);

    const bool canvasHasNativeSurface = bool(m_d->canvasWidget->widget()->windowHandle());
    const bool canvasNeedsNativeSurface =
        cfg.enableCanvasSurfaceColorSpaceManagement() &&
        KisPlatformPluginInterfaceFactory::instance()->surfaceColorManagedByOS();

    bool needReset = (m_d->currentCanvasIsOpenGL != useOpenGL) ||
        (m_d->currentCanvasIsOpenGL &&
         m_d->openGLFilterMode != cfg.openGLFilteringMode()) ||
         canvasHasNativeSurface != canvasNeedsNativeSurface;

    if (needReset) {
        createCanvas(useOpenGL);
        connectCurrentCanvas();
        slotEffectiveZoomChanged(m_d->coordinatesConverter->effectiveZoom());
    }
    updateCanvasWidgetImpl();
}

void KisCanvas2::startUpdateInPatches(const QRect &imageRect)
{
    /**
     * We don't do patched loading for openGL canvas, because it loads
     * the tiles, which are basically "patches". Therefore, big chunks
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
    m_d->channelFlags = image->rootLayer()->channelFlags();
    m_d->canvasWidget->channelSelectionChanged(m_d->channelFlags);

    // Not all color spaces are supported by soft-proofing, so update state
    if (imageView()->softProofing()) {
        updateProofingState();
    }

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

void KisCanvas2::fetchProofingOptions()
{
    KisProofingConfigurationSP baseConfig = image()->proofingConfiguration();
    if (!baseConfig) {
        baseConfig = KisImageConfig(true).defaultProofingconfiguration();
    }
    *m_d->proofingConfig = *baseConfig;

    updateProofingState();
}

void KisCanvas2::updateProofingState()
{
    KoColorConversionTransformation::ConversionFlags displayFlags = m_d->proofingConfig->displayFlags;
    displayFlags.setFlag(KoColorConversionTransformation::SoftProofing, false);

    if (image()->colorSpace()->colorDepthId().id().contains("U")) {
        displayFlags.setFlag(KoColorConversionTransformation::SoftProofing, imageView()->softProofing());
        displayFlags.setFlag(KoColorConversionTransformation::GamutCheck, imageView()->gamutCheck());
    }
    m_d->proofingConfig->displayFlags = displayFlags;
    m_d->proofingConfigUpdated = true;

    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->multiSurfaceState);
    auto newState = m_d->multiSurfaceSetupManager.onProofingChanged(*m_d->multiSurfaceState, m_d->proofingConfig);
    m_d->assignChangedMultiSurfaceState(newState);
}

void KisCanvas2::slotSoftProofing()
{
    updateProofingState();
    refetchDataFromImage();
}

void KisCanvas2::slotGamutCheck()
{
    updateProofingState();
    if (imageView()->softProofing()) {
        refetchDataFromImage();
    }
}

void KisCanvas2::slotChangeProofingConfig()
{
    fetchProofingOptions();
    if (imageView()->softProofing()) {
        refetchDataFromImage();
    }
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
    return m_d->proofingConfig;
}

void KisCanvas2::startResizingImage()
{
    KisImageWSP image = this->image();
    qint32 w = image->width();
    qint32 h = image->height();

    Q_EMIT sigContinueResizeImage(w, h);

    QRect imageBounds(0, 0, w, h);
    startUpdateInPatches(imageBounds);
}

void KisCanvas2::finishResizingImage(qint32 w, qint32 h)
{
    m_d->canvasWidget->finishResizingImage(w, h);
}

void KisCanvas2::startUpdateCanvasProjection(const QRect & rc)
{
    KisUpdateInfoSP info = m_d->canvasWidget->startUpdateCanvasProjection(rc);
    if (m_d->projectionUpdatesCompressor.putUpdateInfo(info)) {
        Q_EMIT sigCanvasCacheUpdated();
    }
}

void KisCanvas2::updateCanvasProjection()
{
    auto tryIssueCanvasUpdates = [this](const QRect &vRect) {
        if (!m_d->isBatchUpdateActive) {
            // TODO: Implement info->dirtyViewportRect() for KisOpenGLCanvas2 to avoid updating whole canvas
            if (m_d->currentCanvasIsOpenGL) {
                m_d->savedCanvasProjectionUpdateRect |= vRect;

                // we already had a compression in frameRenderStartCompressor, so force the update directly
                slotDoCanvasUpdate();
            } else if (/* !m_d->currentCanvasIsOpenGL && */ !vRect.isEmpty()) {
                m_d->savedCanvasProjectionUpdateRect |= m_d->coordinatesConverter->viewportToWidget(vRect).toAlignedRect();

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
    Q_EMIT sigCanvasCacheUpdated();
}

void KisCanvas2::slotEndUpdatesBatch()
{
    KisUpdateInfoSP info =
        new KisMarkerUpdateInfo(KisMarkerUpdateInfo::EndBatch,
                                      m_d->coordinatesConverter->imageRectInImagePixels());
    m_d->projectionUpdatesCompressor.putUpdateInfo(info);
    Q_EMIT sigCanvasCacheUpdated();
}

void KisCanvas2::slotSetLodUpdatesBlocked(bool value)
{
    KisUpdateInfoSP info =
        new KisMarkerUpdateInfo(value ?
                                KisMarkerUpdateInfo::BlockLodUpdates :
                                KisMarkerUpdateInfo::UnblockLodUpdates,
                                m_d->coordinatesConverter->imageRectInImagePixels());
    m_d->projectionUpdatesCompressor.putUpdateInfo(info);
    Q_EMIT sigCanvasCacheUpdated();
}

void KisCanvas2::requestCanvasUpdateMaybeCompressed()
{
    /**
    * If Qt has our custom patch for global updates compression, then we shouldn't do
    * our own compression here in the canvas. Everything will be done in Qt.
    */
#if !KRITA_QT_HAS_UPDATE_COMPRESSION_PATCH
    m_d->canvasUpdateCompressor.start();
#else
    slotDoCanvasUpdate();
#endif
}

void KisCanvas2::slotDoCanvasUpdate()
{

#if !KRITA_QT_HAS_UPDATE_COMPRESSION_PATCH
    /**
     * WARNING: in isBusy() we access openGL functions without making the painting
     * context current. We hope that currently active context will be Qt's one,
     * which is shared with our own.
     */
    if (m_d->canvasWidget->isBusy()) {
        // just restarting the timer
        m_d->canvasUpdateCompressor.start();
        return;
    }
#endif

    QRect combinedUpdateRect = m_d->savedCanvasProjectionUpdateRect | m_d->savedOverlayUpdateRect;
    if (!combinedUpdateRect.isEmpty()) {
        // TODO: Remove this signal (only used by the old KisSketchView)
        Q_EMIT updateCanvasRequested(combinedUpdateRect);

        if (wrapAroundViewingMode() && !m_d->savedCanvasProjectionUpdateRect.isEmpty()) {
            const QRect widgetRect = m_d->canvasWidget->widget()->rect();
            const QRect imageRect = m_d->coordinatesConverter->imageRectInImagePixels();

            const QRect widgetRectInImagePixels =
                m_d->coordinatesConverter->widgetToImage(widgetRect).toAlignedRect();

            const QRect rc = m_d->coordinatesConverter->widgetToImage(m_d->savedCanvasProjectionUpdateRect).toAlignedRect();

            const QVector<QRect> updateRects =
                KisWrappedRect::multiplyWrappedRect(rc, imageRect, widgetRectInImagePixels, wrapAroundViewingModeAxis());

            Q_FOREACH(const QRect &rc, updateRects) {
                const QRect widgetUpdateRect =
                    m_d->coordinatesConverter->imageToWidget(rc).toAlignedRect() & widgetRect;
                m_d->canvasWidget->updateCanvasImage(widgetUpdateRect);
            }
            m_d->canvasWidget->updateCanvasDecorations(m_d->savedOverlayUpdateRect);
        } else {
            m_d->canvasWidget->updateCanvasImage(m_d->savedCanvasProjectionUpdateRect);
            m_d->canvasWidget->updateCanvasDecorations(m_d->savedOverlayUpdateRect);
        }
    } else if (m_d->updateSceneRequested) {
        m_d->canvasWidget->widget()->update();
    }

    m_d->savedCanvasProjectionUpdateRect = QRect();
    m_d->savedOverlayUpdateRect = QRect();
    m_d->updateSceneRequested = false;
}

void KisCanvas2::updateCanvasWidgetImpl(const QRect &rc)
{
    QRect rect = m_d->canvasWidget->widget()->rect();
    if (!rc.isEmpty()) {
        rect &= rc;
        if (rect.isEmpty()) {
            return;
        }
    }
    // We don't know if it's the canvas projection or the overlay that's
    // changed, so we update both.
    m_d->savedCanvasProjectionUpdateRect |= rect;
    m_d->savedOverlayUpdateRect |= rect;
    requestCanvasUpdateMaybeCompressed();
}

void KisCanvas2::updateCanvas()
{
    updateCanvasWidgetImpl();
}

QRect KisCanvas2::KisCanvas2Private::docUpdateRectToWidget(const QRectF &docRect)
{
    QRect widgetRect = coordinatesConverter->documentToWidget(docRect).toAlignedRect();
    widgetRect.adjust(-2, -2, 2, 2);
    return widgetRect & canvasWidget->widget()->rect();
}

void KisCanvas2::updateCanvas(const QRectF& documentRect)
{
    // updateCanvas is called from tools, never from the projection
    // updates, so no need to prescale!
    QRect widgetRect = m_d->docUpdateRectToWidget(documentRect);
    if (!widgetRect.isEmpty()) {
        updateCanvasWidgetImpl(widgetRect);
    }
}

void KisCanvas2::updateCanvasProjection(const QRectF &docRect)
{   
    QRect widgetRect = m_d->docUpdateRectToWidget(docRect);
    if (!widgetRect.isEmpty()) {
        m_d->savedCanvasProjectionUpdateRect |= widgetRect;
        requestCanvasUpdateMaybeCompressed();
    }
}

void KisCanvas2::updateCanvasDecorations()
{
    m_d->savedOverlayUpdateRect = m_d->canvasWidget->widget()->rect();
    requestCanvasUpdateMaybeCompressed();
}

void KisCanvas2::updateCanvasDecorations(const QRectF &docRect)
{
    QRect widgetRect = m_d->docUpdateRectToWidget(docRect);
    if (!widgetRect.isEmpty()) {
        m_d->savedOverlayUpdateRect |= widgetRect;
        requestCanvasUpdateMaybeCompressed();
    }
}

void KisCanvas2::updateCanvasToolOutlineDoc(const QRectF &docRect)
{
    QRect widgetRect = m_d->docUpdateRectToWidget(docRect);
    if (!widgetRect.isEmpty()) {
        updateCanvasToolOutlineWdg(widgetRect);
    }
}

void KisCanvas2::updateCanvasToolOutlineWdg(const QRect &widgetRect)
{
    QRect rect = widgetRect & m_d->canvasWidget->widget()->rect();
    if (!rect.isEmpty()) {
        m_d->savedOverlayUpdateRect |= rect;
#ifdef HAVE_NO_QT_UPDATE_COMPRESSIO
        m_d->canvasUpdateCompressor.start();
#else
        slotDoCanvasUpdate();
#endif
    }
}

void KisCanvas2::updateCanvasScene()
{
    m_d->updateSceneRequested = true;
    requestCanvasUpdateMaybeCompressed();
}

void KisCanvas2::disconnectCanvasObserver(QObject *object)
{
    KoCanvasBase::disconnectCanvasObserver(object);
    m_d->view->disconnect(object);
}

void KisCanvas2::slotEffectiveZoomChanged(qreal newZoom)
{
    Q_UNUSED(newZoom)

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

    const qreal ratio = m_d->regionOfInterestMargin;
    const QRect proposedRoi = KisAlgebra2D::blowRect(m_d->coordinatesConverter->widgetRectInImagePixels(), ratio).toAlignedRect();

    const QRect imageRect = m_d->coordinatesConverter->imageRectInImagePixels();

    m_d->regionOfInterest = proposedRoi & imageRect;

    if (m_d->regionOfInterest != oldRegionOfInterest) {
        Q_EMIT sigRegionOfInterestChanged(m_d->regionOfInterest);
    }
}

void KisCanvas2::slotUpdateReferencesBounds()
{
    QRectF referencesRect;
    KisReferenceImagesLayerSP layer = m_d->view->document()->referenceImagesLayer();
    if (layer) {
        referencesRect = layer->boundingImageRect();
    }

    m_d->view->canvasController()->syncOnReferencesChange(referencesRect);
}

void KisCanvas2::setRenderingLimit(const QRect &rc)
{
    m_d->renderingLimit = rc;
}

QRect KisCanvas2::renderingLimit() const
{
    return m_d->renderingLimit;
}

KisPopupPalette *KisCanvas2::popupPalette()
{
    return m_d->popupPalette;
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
    KisImageSP image = this->image();

    if (m_d->bootstrapLodBlocked || !m_d->lodIsSupported()) {
        image->setLodPreferences(KisLodPreferences(KisLodPreferences::None, 0));
    } else {
        const qreal effectiveZoom = m_d->coordinatesConverter->effectiveZoom();

        KisConfig cfg(true);
        const int maxLod = cfg.numMipmapLevels();
        const int lod = KisLodTransform::scaleToLod(effectiveZoom, maxLod);
        KisLodPreferences::PreferenceFlags flags = KisLodPreferences::LodSupported;

        if (m_d->lodPreferredInImage) {
            flags |= KisLodPreferences::LodPreferred;
        }
        image->setLodPreferences(KisLodPreferences(flags, lod));
    }
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

void KisCanvas2::viewportOffsetMoved(const QPointF &oldOffset, const QPointF &newOffset)
{
    if (!m_d->currentCanvasIsOpenGL) {
        const QPointF moveOffset = oldOffset - newOffset;
        m_d->prescaledProjection->viewportMoved(-moveOffset);
        // we don't emit updateCanvas() here, because it will be emitted later
        // in documentOffsetMoved()
    }
}

void KisCanvas2::slotCanvasStateChanged()
{
    updateCanvas();
    m_d->regionOfInterestUpdateCompressor.start();

    Q_EMIT sigCanvasStateChanged();
}

void KisCanvas2::slotConfigChanged()
{
    KisConfig cfg(true);
    m_d->regionOfInterestMargin = KisImageConfig(true).animationCacheRegionOfInterestMargin();

    resetCanvas(cfg.useOpenGL());

    QWidget *mainWindow = m_d->view->mainWindow();
    KIS_SAFE_ASSERT_RECOVER_RETURN(mainWindow);

    QWidget *topLevelWidget = mainWindow->topLevelWidget();
    KIS_SAFE_ASSERT_RECOVER_RETURN(topLevelWidget);

    auto options = KisDisplayConfig::optionsFromKisConfig(cfg);
    ENTER_FUNCTION() << ppVar(options);

    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->multiSurfaceState);
    auto newState = m_d->multiSurfaceSetupManager.onConfigChanged(*m_d->multiSurfaceState,
                                                                  m_d->currentScreenId(),
                                                                  cfg.canvasSurfaceColorSpaceManagementMode(),
                                                                  KisDisplayConfig::optionsFromKisConfig(cfg));
    m_d->assignChangedMultiSurfaceState(newState);

    initializeFpsDecoration();
}

void KisCanvas2::slotScreenChanged(QScreen *screen)
{
    const int screenId = qApp->screens().indexOf(screen);

    if (screenId < 0) {
        warnUI << "Failed to get screenNumber for updating display profile.";
        return;
    }

    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->multiSurfaceState);
    auto newState = m_d->multiSurfaceSetupManager.onScreenChanged(*m_d->multiSurfaceState,
                                                                  screenId);
    m_d->assignChangedMultiSurfaceState(newState);
}

void KisCanvas2::refetchDataFromImage()
{
    KisImageSP image = this->image();
    KisImageReadOnlyBarrierLock l(image);
    startUpdateInPatches(image->bounds());
}

void KisCanvas2::KisCanvas2Private::assignChangedMultiSurfaceStateSkipCanvasSurface(const KisMultiSurfaceStateManager::State &newState)
{
    // the surface state is supposed to be initialized on canvas creation
    KIS_SAFE_ASSERT_RECOVER_RETURN(multiSurfaceState);

    if (*multiSurfaceState == newState) return;

    const KisMultiSurfaceStateManager::State oldState = *this->multiSurfaceState;
    this->multiSurfaceState = newState;

    displayColorConverter.setMultiSurfaceDisplayConfig(newState.multiConfig);

    if (oldState.multiConfig.canvasDisplayConfig() != newState.multiConfig.canvasDisplayConfig()) {
        KisImageSP image = view->image();
        KisImageReadOnlyBarrierLock l(image);
        canvasWidget->setDisplayConfig(multiSurfaceState->multiConfig.canvasDisplayConfig());

        q->refetchDataFromImage();

        // we changed the canvas conversion mode, so the canvas background color
        // has changed as well
        q->updateCanvas();
    }
}

void KisCanvas2::KisCanvas2Private::assignChangedMultiSurfaceState(const KisMultiSurfaceStateManager::State &newState)
{
    assignChangedMultiSurfaceStateSkipCanvasSurface(newState);

#if KRITA_USE_SURFACE_COLOR_MANAGEMENT_API

    if (surfaceColorManager) {
        surfaceColorManager->setDisplayConfigOptions(newState.surfaceMode, newState.multiConfig.options());
    }

#endif
}

void KisCanvas2::slotSurfaceFormatChanged(const KisDisplayConfig &config)
{
#if KRITA_USE_SURFACE_COLOR_MANAGEMENT_API

    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->multiSurfaceState);
    if (m_d->multiSurfaceState->multiConfig.canvasDisplayConfig() == config) return;

    if (m_d->multiSurfaceState->isCanvasOpenGL) {
        if (config.isHDR &&
            m_d->canvasWidget->currentBitDepthMode() < KisOpenGLCanvas2::BitDepthMode::Depth10Bit) {

            const QString warningMessage = i18n(
                "WARNING: HDR mode was activated on surface working in 8-bit mode!\n"
                "Please activate 10-bit mode in Krita's Preferences dialog and restart"
                "Krita to avoid color banding!");

            m_d->view->showFloatingMessage(warningMessage, koIcon("warning"), 7000, KisFloatingMessage::High);
            warnOpenGL.noquote() << QString(warningMessage).replace('\n', ' ');
            warnOpenGL << ppVar(QSurfaceFormat::defaultFormat());
        }
    }

    auto newState = m_d->multiSurfaceSetupManager.onCanvasSurfaceFormatChanged(*m_d->multiSurfaceState, config);
    m_d->assignChangedMultiSurfaceStateSkipCanvasSurface(newState);
#endif
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
                                            m_d->canvasWidget->widget());
    connect(m_d->popupPalette, SIGNAL(zoomLevelChanged(int)), this, SLOT(slotPopupPaletteRequestedZoomChange(int)));
    connect(m_d->popupPalette, SIGNAL(sigUpdateCanvas()), this, SLOT(updateCanvas()));
    connect(m_d->view->mainWindow(), SIGNAL(themeChanged()), m_d->popupPalette, SLOT(slotUpdateIcons()));
}

void KisCanvas2::slotPopupPaletteRequestedZoomChange(int zoom ) {
    m_d->view->canvasController()->setZoom(KoZoomMode::ZOOM_CONSTANT, (qreal)(zoom/100.0)); // 1.0 is 100% zoom
}

void KisCanvas2::setCursor(const QCursor &cursor)
{
    canvasWidget()->setCursor(cursor);
}

KisAnimationFrameCacheSP KisCanvas2::frameCache() const
{
    return m_d->frameCache;
}

KisCanvasAnimationState *KisCanvas2::animationState() const
{
    return m_d->animationPlayer.data();
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
    return m_d->canvasWidget->wrapAroundViewingMode();
}

void KisCanvas2::setWrapAroundViewingModeAxis(WrapAroundAxis value)
{
    m_d->canvasWidget->setWrapAroundViewingModeAxis(value);
    updateCanvas();
}

WrapAroundAxis KisCanvas2::wrapAroundViewingModeAxis() const
{
    return m_d->canvasWidget->wrapAroundViewingModeAxis();
}

void KisCanvas2::bootstrapFinished()
{
    if (!m_d->bootstrapLodBlocked) return;

    m_d->bootstrapLodBlocked = false;
    setLodPreferredInCanvas(m_d->lodPreferredInImage);

    // Initialization of audio tracks is deferred until after canvas has been completely constructed.
    m_d->animationPlayer->setupAudioTracks();
}

void KisCanvas2::setLodPreferredInCanvas(bool value)
{
    if (!KisOpenGL::supportsLoD()) {
        qWarning() << "WARNING: Level of Detail functionality is available only with openGL + GLSL 1.3 support";
    }

    m_d->lodPreferredInImage =
        value && m_d->lodIsSupported();

    notifyLevelOfDetailChange();

    KisConfig cfg(false);
    cfg.setLevelOfDetailEnabled(m_d->lodPreferredInImage);
}

bool KisCanvas2::lodPreferredInCanvas() const
{
    return m_d->lodPreferredInImage;
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

KisInputActionGroupsMaskInterface::SharedInterface KisCanvas2::inputActionGroupsMaskInterface()
{
    return m_d->inputActionGroupsMaskInterface;
}

QString KisCanvas2::colorManagementReport() const
{
#if KRITA_USE_SURFACE_COLOR_MANAGEMENT_API
    QString report;
    QDebug str(&report);

    if (m_d->canvasWidget) {
        str << "(canvas bit depth report)" << Qt::endl;
        str << Qt::endl;
        str.noquote().nospace() << m_d->canvasWidget->currentBitDepthUserReport();
    }

    str << Qt::endl;

    if (m_d->surfaceColorManager) {
        str.noquote().nospace() << QString("(canvas surface color manager)\n");
        str.noquote().nospace() << QString("\n");
        str.noquote().nospace() << m_d->surfaceColorManager->colorManagementReport();
    } else {
        str.noquote().nospace() << QString("Surface color management is not supported on this platform\n");
    }

    return report;
#else
    return "Surface color management is disabled\n";
#endif
}
