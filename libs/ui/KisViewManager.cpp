/*
 *  This file is part of KimageShop^WKrayon^WKrita
 *
 *  SPDX-FileCopyrightText: 1999 Matthias Elter <me@kde.org>
 *  SPDX-FileCopyrightText: 1999 Michael Koch <koch@kde.org>
 *  SPDX-FileCopyrightText: 1999 Carsten Pfeiffer <pfeiffer@kde.org>
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2003-2011 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2004 Clarence Dang <dang@kde.org>
 *  SPDX-FileCopyrightText: 2011 José Luis Vergara <pentalis@gmail.com>
 *  SPDX-FileCopyrightText: 2017 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#include "KisViewManager.h"
#include <QPrinter>

#include <QAction>
#include <QApplication>
#include <QBuffer>
#include <QByteArray>
#include <QStandardPaths>
#include <QScreen>
#include <QDesktopServices>
#include <QGridLayout>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QObject>
#include <QPoint>
#include <QPrintDialog>
#include <QPushButton>
#include <QScreen>
#include <QScrollBar>
#include <QStatusBar>
#include <QToolBar>
#include <QUrl>
#include <QWidget>
#include <QActionGroup>
#include <QRegExp>

#include <kactioncollection.h>
#include <klocalizedstring.h>
#include <KoResourcePaths.h>
#include <kselectaction.h>

#include <KoCanvasController.h>
#include <KoCompositeOp.h>
#include <KoDockRegistry.h>
#include <KoDockWidgetTitleBar.h>
#include <KoFileDialog.h>
#include <KoProperties.h>
#include <KisResourceItemChooserSync.h>
#include <KoSelection.h>
#include <KoStore.h>
#include <KoToolManager.h>
#include <KoToolRegistry.h>
#include <KoViewConverter.h>
#include <KoZoomHandler.h>
#include <KoPluginLoader.h>
#include <KoDocumentInfo.h>
#include <KoColorSpaceRegistry.h>
#include <KisResourceLocator.h>

#include "input/kis_input_manager.h"
#include "canvas/kis_canvas2.h"
#include "canvas/kis_canvas_controller.h"
#include "canvas/kis_grid_manager.h"
#include "input/kis_input_profile_manager.h"
#include "kis_action_manager.h"
#include "kis_action.h"
#include "kis_canvas_controls_manager.h"
#include "kis_canvas_resource_provider.h"
#include "kis_composite_progress_proxy.h"
#include <KoProgressUpdater.h>
#include "kis_config.h"
#include "kis_config_notifier.h"
#include "kis_control_frame.h"
#include "KisDocument.h"
#include "kis_favorite_resource_manager.h"
#include "kis_filter_manager.h"
#include "kis_group_layer.h"
#include <kis_image.h>
#include "kis_image_manager.h"
#include <kis_layer.h>
#include "kis_mainwindow_observer.h"
#include "kis_mask_manager.h"
#include "kis_mirror_manager.h"
#include "kis_node_commands_adapter.h"
#include "kis_node.h"
#include "kis_node_manager.h"
#include "KisDecorationsManager.h"
#include <kis_paint_layer.h>
#include "kis_paintop_box.h"
#include <brushengine/kis_paintop_preset.h>
#include "KisPart.h"
#include <KoUpdater.h>
#include "kis_selection_mask.h"
#include "kis_selection_manager.h"
#include "kis_shape_controller.h"
#include "kis_shape_layer.h"
#include <kis_signal_compressor.h>
#include "kis_statusbar.h"
#include <KisTemplateCreateDia.h>
#include <kis_tool_freehand.h>
#include <kis_undo_adapter.h>
#include "KisView.h"
#include "kis_zoom_manager.h"
#include "widgets/kis_floating_message.h"
#include "kis_signal_auto_connection.h"
#include "kis_icon_utils.h"
#include "kis_guides_manager.h"
#include "kis_derived_resources.h"
#include "kis_abstract_resources.h"
#include "dialogs/kis_delayed_save_dialog.h"
#include <KisMainWindow.h>
#include "kis_signals_blocker.h"
#include "imagesize/imagesize.h"
#include <KoToolDocker.h>
#include <KisIdleTasksManager.h>
#include <KisImageBarrierLock.h>
#include <KisTextPropertiesManager.h>
#include <kis_selection.h>

#ifdef Q_OS_WIN
#include "KisWindowsPackageUtils.h"
#endif

class BlockingUserInputEventFilter : public QObject
{
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        Q_UNUSED(watched);
        if(dynamic_cast<QWheelEvent*>(event)
                || dynamic_cast<QKeyEvent*>(event)
                || dynamic_cast<QMouseEvent*>(event)) {
            return true;
        }
        else {
            return false;
        }
    }
};

class KisViewManager::KisViewManagerPrivate
{

public:

    KisViewManagerPrivate(KisViewManager *_q, KisKActionCollection *_actionCollection, QWidget *_q_parent)
        : filterManager(_q)
        , selectionManager(_q)
        , statusBar(_q)
        , controlFrame(_q, _q_parent)
        , nodeManager(_q)
        , imageManager(_q)
        , gridManager(_q)
        , canvasControlsManager(_q)
        , paintingAssistantsManager(_q)
        , actionManager(_q, _actionCollection)
        , canvasResourceProvider(_q)
        , canvasResourceManager()
        , guiUpdateCompressor(30, KisSignalCompressor::POSTPONE, _q)
        , actionCollection(_actionCollection)
        , mirrorManager(_q)
        , inputManager(_q)
    {
        KisViewManager::initializeResourceManager(&canvasResourceManager);
    }

public:
    KisFilterManager filterManager;
    KisAction *createTemplate {nullptr};
    KisAction *createCopy {nullptr};
    KisAction *saveIncremental {nullptr};
    KisAction *saveIncrementalBackup {nullptr};
    KisAction *openResourcesDirectory {nullptr};
    KisAction *rotateCanvasRight {nullptr};
    KisAction *rotateCanvasLeft {nullptr};
    KisAction *resetCanvasRotation {nullptr};
    KisAction *wrapAroundAction {nullptr};
    KisAction *wrapAroundHVAxisAction {nullptr};
    KisAction *wrapAroundHAxisAction {nullptr};
    KisAction *wrapAroundVAxisAction {nullptr};
    QActionGroup *wrapAroundAxisActions {nullptr};
    KisAction *levelOfDetailAction {nullptr};
    KisAction *showRulersAction {nullptr};
    KisAction *rulersTrackMouseAction {nullptr};
    KisAction *zoomTo100pct {nullptr};
    KisAction *zoomIn {nullptr};
    KisAction *zoomOut {nullptr};
    KisAction *zoomToFit {nullptr};
    KisAction *zoomToFitWidth {nullptr};
    KisAction *zoomToFitHeight {nullptr};
    KisAction *toggleZoomToFit {nullptr};
    KisAction *resetDisplay {nullptr};
    KisAction *viewPrintSize {nullptr};
    KisAction *softProof {nullptr};
    KisAction *gamutCheck {nullptr};
    KisAction *toggleFgBg {nullptr};
    KisAction *resetFgBg {nullptr};
    KisAction *toggleBrushOutline {nullptr};

    KisSelectionManager selectionManager;
    KisGuidesManager guidesManager;
    KisStatusBar statusBar;
    QPointer<KoUpdater> persistentImageProgressUpdater;

    QScopedPointer<KoProgressUpdater> persistentUnthreadedProgressUpdaterRouter;
    QPointer<KoUpdater> persistentUnthreadedProgressUpdater;

    KisControlFrame controlFrame;
    KisNodeManager nodeManager;
    KisImageManager imageManager;
    KisGridManager gridManager;
    KisCanvasControlsManager canvasControlsManager;
    KisDecorationsManager paintingAssistantsManager;
    BlockingUserInputEventFilter blockingEventFilter;
    KisActionManager actionManager;
    QMainWindow* mainWindow {nullptr};
    QPointer<KisFloatingMessage> savedFloatingMessage;
    bool showFloatingMessage {true};
    QPointer<KisView> currentImageView;
    KisCanvasResourceProvider canvasResourceProvider;
    KoCanvasResourceProvider canvasResourceManager;
    KisSignalCompressor guiUpdateCompressor;
    KisKActionCollection *actionCollection {nullptr};
    KisMirrorManager mirrorManager;
    KisInputManager inputManager;
    KisIdleTasksManager idleTasksManager;
    KisTextPropertiesManager textPropertyManager;

    KisSignalAutoConnectionsStore viewConnections;
    KSelectAction *actionAuthor {nullptr}; // Select action for author profile.
    KisAction *showPixelGrid {nullptr};

    QByteArray canvasStateInNormalMode;
    QByteArray canvasStateInCanvasOnlyMode;

    struct CanvasOnlyOptions : boost::equality_comparable<CanvasOnlyOptions>
    {
        CanvasOnlyOptions(const KisConfig &cfg)
            : hideStatusbarFullscreen(cfg.hideStatusbarFullscreen())
            , hideDockersFullscreen(cfg.hideDockersFullscreen())
            , hideTitlebarFullscreen(cfg.hideTitlebarFullscreen())
            , hideMenuFullscreen(cfg.hideMenuFullscreen())
            , hideToolbarFullscreen(cfg.hideToolbarFullscreen())
        {
        }

        bool hideStatusbarFullscreen = false;
        bool hideDockersFullscreen = false;
        bool hideTitlebarFullscreen = false;
        bool hideMenuFullscreen = false;
        bool hideToolbarFullscreen = false;

        bool operator==(const CanvasOnlyOptions &rhs) {
            return hideStatusbarFullscreen == rhs.hideStatusbarFullscreen &&
                hideDockersFullscreen == rhs.hideDockersFullscreen &&
                hideTitlebarFullscreen == rhs.hideTitlebarFullscreen &&
                hideMenuFullscreen == rhs.hideMenuFullscreen &&
                hideToolbarFullscreen == rhs.hideToolbarFullscreen;
        }
    };
    std::optional<CanvasOnlyOptions> canvasOnlyOptions;
    QPoint canvasOnlyOffsetCompensation;

    bool blockUntilOperationsFinishedImpl(KisImageSP image, bool force);
};

KisViewManager::KisViewManager(QWidget *parent, KisKActionCollection *_actionCollection)
    : d(new KisViewManagerPrivate(this, _actionCollection, parent))
{
    d->actionCollection = _actionCollection;
    d->mainWindow = dynamic_cast<QMainWindow*>(parent);
    d->canvasResourceProvider.setResourceManager(&d->canvasResourceManager);
    connect(&d->guiUpdateCompressor, SIGNAL(timeout()), this, SLOT(guiUpdateTimeout()));

    createActions();
    setupManagers();

    // These initialization functions must wait until KisViewManager ctor is complete.
    d->statusBar.setup();
    d->persistentImageProgressUpdater =
            d->statusBar.progressUpdater()->startSubtask(1, "", true);
    // reset state to "completed"
    d->persistentImageProgressUpdater->setRange(0,100);
    d->persistentImageProgressUpdater->setValue(100);

    d->persistentUnthreadedProgressUpdater =
            d->statusBar.progressUpdater()->startSubtask(1, "", true);
    // reset state to "completed"
    d->persistentUnthreadedProgressUpdater->setRange(0,100);
    d->persistentUnthreadedProgressUpdater->setValue(100);

    d->persistentUnthreadedProgressUpdaterRouter.reset(
                new KoProgressUpdater(d->persistentUnthreadedProgressUpdater,
                                      KoProgressUpdater::Unthreaded));
    d->persistentUnthreadedProgressUpdaterRouter->setAutoNestNames(true);
    d->persistentUnthreadedProgressUpdaterRouter->start();

    // just a clumsy way to mark the updater as completed, the subtask will
    // be automatically deleted on completion...
    d->persistentUnthreadedProgressUpdaterRouter->startSubtask()->setProgress(100);

    d->controlFrame.setup(parent);


    //Check to draw scrollbars after "Canvas only mode" toggle is created.
    this->showHideScrollbars();

    KoToolManager::instance()->initializeToolActions();

    connect(KoToolManager::instance(), SIGNAL(inputDeviceChanged(KoInputDevice)),
            d->controlFrame.paintopBox(), SLOT(slotInputDeviceChanged(KoInputDevice)));

    connect(KoToolManager::instance(), SIGNAL(changedTool(KoCanvasController*)),
            d->controlFrame.paintopBox(), SLOT(slotToolChanged(KoCanvasController*)));

    connect(&d->nodeManager, SIGNAL(sigNodeActivated(KisNodeSP)),
            canvasResourceProvider(), SLOT(slotNodeActivated(KisNodeSP)));

    connect(KisPart::instance(), SIGNAL(sigViewAdded(KisView*)), SLOT(slotViewAdded(KisView*)));
    connect(KisPart::instance(), SIGNAL(sigViewRemoved(KisView*)), SLOT(slotViewRemoved(KisView*)));
    connect(KisPart::instance(), SIGNAL(sigViewRemoved(KisView*)),
            d->controlFrame.paintopBox(), SLOT(updatePresetConfig()));

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotUpdateAuthorProfileActions()));
    connect(KisConfigNotifier::instance(), SIGNAL(pixelGridModeChanged()), SLOT(slotUpdatePixelGridAction()));

    connect(KoToolManager::instance(), SIGNAL(createOpacityResource(bool, KoToolBase*)), SLOT(slotCreateOpacityResource(bool, KoToolBase*)));

    KisInputProfileManager::instance()->loadProfiles();

    KisConfig cfg(true);
    d->showFloatingMessage = cfg.showCanvasMessages();
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KoColor foreground(Qt::black, cs);
    d->canvasResourceProvider.setFGColor(cfg.readKoColor("LastForeGroundColor",foreground));
    KoColor background(Qt::white, cs);
    d->canvasResourceProvider.setBGColor(cfg.readKoColor("LastBackGroundColor",background));
    d->canvasResourceProvider.setColorHistory(cfg.readKoColors("LastColorHistory"));
    d->textPropertyManager.setCanvasResourceProvider(&d->canvasResourceProvider);

    // Initialize the old imagesize plugin
    new ImageSize(this);
}


KisViewManager::~KisViewManager()
{
    KisConfig cfg(false);
    if (canvasResourceProvider() && canvasResourceProvider()->currentPreset()) {
        cfg.writeKoColor("LastForeGroundColor",canvasResourceProvider()->fgColor());
        cfg.writeKoColor("LastBackGroundColor",canvasResourceProvider()->bgColor());
    }

    if (canvasResourceProvider()) {
        cfg.writeKoColors("LastColorHistory", canvasResourceProvider()->colorHistory());
    }

    cfg.writeEntry("baseLength", KisResourceItemChooserSync::instance()->baseLength());
    cfg.writeEntry("CanvasOnlyActive", false); // We never restart in CanvasOnlyMode
    delete d;
}

#include <KoActiveCanvasResourceDependencyKoResource.h>

void KisViewManager::initializeResourceManager(KoCanvasResourceProvider *resourceManager)
{
    resourceManager->addDerivedResourceConverter(toQShared(new KisCompositeOpResourceConverter));
    resourceManager->addDerivedResourceConverter(toQShared(new KisEffectiveCompositeOpResourceConverter));
    resourceManager->addDerivedResourceConverter(toQShared(new KisFlowResourceConverter));
    resourceManager->addDerivedResourceConverter(toQShared(new KisFadeResourceConverter));
    resourceManager->addDerivedResourceConverter(toQShared(new KisScatterResourceConverter));
    resourceManager->addDerivedResourceConverter(toQShared(new KisSizeResourceConverter));
    resourceManager->addDerivedResourceConverter(toQShared(new KisBrushRotationResourceConverter));
    resourceManager->addDerivedResourceConverter(toQShared(new KisLodAvailabilityResourceConverter));
    resourceManager->addDerivedResourceConverter(toQShared(new KisLodSizeThresholdResourceConverter));
    resourceManager->addDerivedResourceConverter(toQShared(new KisLodSizeThresholdSupportedResourceConverter));
    resourceManager->addDerivedResourceConverter(toQShared(new KisEraserModeResourceConverter));
    resourceManager->addDerivedResourceConverter(toQShared(new KisPatternSizeResourceConverter));
    resourceManager->addDerivedResourceConverter(toQShared(new KisBrushNameResourceConverter));
    resourceManager->addResourceUpdateMediator(toQShared(new KisPresetUpdateMediator));

    resourceManager->addActiveCanvasResourceDependency(
        toQShared(new KoActiveCanvasResourceDependencyKoResource<KisPaintOpPreset>(
                      KoCanvasResource::CurrentPaintOpPreset,
                      KoCanvasResource::CurrentGradient)));

    resourceManager->addActiveCanvasResourceDependency(
        toQShared(new KoActiveCanvasResourceDependencyKoResource<KoAbstractGradient>(
                      KoCanvasResource::CurrentGradient,
                      KoCanvasResource::ForegroundColor)));

    resourceManager->addActiveCanvasResourceDependency(
        toQShared(new KoActiveCanvasResourceDependencyKoResource<KoAbstractGradient>(
                      KoCanvasResource::CurrentGradient,
                      KoCanvasResource::BackgroundColor)));

    KSharedConfigPtr config =  KSharedConfig::openConfig();
    KConfigGroup miscGroup = config->group("Misc");
    const uint handleRadius = miscGroup.readEntry("HandleRadius", 5);
    resourceManager->setHandleRadius(handleRadius);
}

void KisViewManager::testingInitializeOpacityToPresetResourceConverter(KoCanvasResourceProvider *resourceManager)
{
    resourceManager->addDerivedResourceConverter(toQShared(new KisOpacityToPresetOpacityResourceConverter));
}

KisKActionCollection *KisViewManager::actionCollection() const
{
    return d->actionCollection;
}

void KisViewManager::slotViewAdded(KisView *view)
{
    // WARNING: this slot is called even when a view from another main windows is added!
    //          Don't expect \p view be a child of this view manager!

    if (view->viewManager() == this && viewCount() == 0) {
        d->statusBar.showAllStatusBarItems();
    }
}

void KisViewManager::slotViewRemoved(KisView *view)
{
    // WARNING: this slot is called even when a view from another main windows is removed!
    //          Don't expect \p view be a child of this view manager!

    if (view->viewManager() == this && viewCount() == 0) {
        d->statusBar.hideAllStatusBarItems();
    }
}

void KisViewManager::setCurrentView(KisView *view)
{
    if (d->currentImageView) {
        d->currentImageView->notifyCurrentStateChanged(false);

        d->currentImageView->canvasBase()->setCursor(QCursor(Qt::ArrowCursor));
        KisDocument* doc = d->currentImageView->document();
        if (doc) {
            doc->image()->compositeProgressProxy()->removeProxy(d->persistentImageProgressUpdater);
            doc->disconnect(this);
        }
        d->currentImageView->canvasController()->proxyObject->disconnect(&d->statusBar);
        d->viewConnections.clear();
        d->idleTasksManager.setImage(0);
    }

    QPointer<KisView> imageView = qobject_cast<KisView*>(view);
    d->currentImageView = imageView;

    if (imageView) {
        /// idle tasks managed should be reconnected to the new image the first,
        /// because other dockers may request it to recalculate stuff
        d->idleTasksManager.setImage(d->currentImageView->image());

        d->softProof->setChecked(imageView->softProofing());
        d->gamutCheck->setChecked(imageView->gamutCheck());

        // Wait for the async image to have loaded
        KisDocument* doc = imageView->document();

        if (KisConfig(true).readEntry<bool>("EnablePositionLabel", false)) {
            connect(d->currentImageView->canvasController()->proxyObject,
                    SIGNAL(documentMousePositionChanged(QPointF)),
                    &d->statusBar,
                    SLOT(documentMousePositionChanged(QPointF)));
        }

        KisCanvasController *canvasController = dynamic_cast<KisCanvasController*>(d->currentImageView->canvasController());
        KIS_ASSERT(canvasController);

        d->viewConnections.addUniqueConnection(&d->nodeManager, SIGNAL(sigNodeActivated(KisNodeSP)), doc->image(), SLOT(requestStrokeEndActiveNode()));
        d->viewConnections.addUniqueConnection(d->rotateCanvasRight, SIGNAL(triggered()), canvasController, SLOT(rotateCanvasRight15()));
        d->viewConnections.addUniqueConnection(d->rotateCanvasLeft, SIGNAL(triggered()),canvasController, SLOT(rotateCanvasLeft15()));
        d->viewConnections.addUniqueConnection(d->resetCanvasRotation, SIGNAL(triggered()),canvasController, SLOT(resetCanvasRotation()));

        d->viewConnections.addUniqueConnection(d->wrapAroundAction, SIGNAL(toggled(bool)), canvasController, SLOT(slotToggleWrapAroundMode(bool)));
        d->wrapAroundAction->setChecked(canvasController->wrapAroundMode());
        d->viewConnections.addUniqueConnection(d->wrapAroundHVAxisAction, SIGNAL(triggered()), canvasController, SLOT(slotSetWrapAroundModeAxisHV()));
        d->wrapAroundHVAxisAction->setChecked(canvasController->wrapAroundModeAxis() == WRAPAROUND_BOTH);
        d->viewConnections.addUniqueConnection(d->wrapAroundHAxisAction, SIGNAL(triggered()), canvasController, SLOT(slotSetWrapAroundModeAxisH()));
        d->wrapAroundHAxisAction->setChecked(canvasController->wrapAroundModeAxis() == WRAPAROUND_HORIZONTAL);
        d->viewConnections.addUniqueConnection(d->wrapAroundVAxisAction, SIGNAL(triggered()), canvasController, SLOT(slotSetWrapAroundModeAxisV()));
        d->wrapAroundVAxisAction->setChecked(canvasController->wrapAroundModeAxis() == WRAPAROUND_VERTICAL);

        d->viewConnections.addUniqueConnection(d->levelOfDetailAction, SIGNAL(toggled(bool)), canvasController, SLOT(slotToggleLevelOfDetailMode(bool)));
        d->levelOfDetailAction->setChecked(canvasController->levelOfDetailMode());

        d->viewConnections.addUniqueConnection(d->currentImageView->image(), SIGNAL(sigColorSpaceChanged(const KoColorSpace*)), d->controlFrame.paintopBox(), SLOT(slotColorSpaceChanged(const KoColorSpace*)));
        d->viewConnections.addUniqueConnection(d->showRulersAction, SIGNAL(toggled(bool)), imageView->zoomManager(), SLOT(setShowRulers(bool)));
        d->viewConnections.addUniqueConnection(d->rulersTrackMouseAction, SIGNAL(toggled(bool)), imageView->zoomManager(), SLOT(setRulersTrackMouse(bool)));
        d->viewConnections.addUniqueConnection(d->zoomTo100pct, SIGNAL(triggered()), imageView->zoomManager(), SLOT(zoomTo100()));
        d->viewConnections.addUniqueConnection(d->zoomIn, SIGNAL(triggered()), imageView->zoomManager(), SLOT(slotZoomIn()));
        d->viewConnections.addUniqueConnection(d->zoomOut, SIGNAL(triggered()), imageView->zoomManager(), SLOT(slotZoomOut()));
        d->viewConnections.addUniqueConnection(d->zoomToFit, SIGNAL(triggered()), imageView->zoomManager(), SLOT(slotZoomToFit()));
        d->viewConnections.addUniqueConnection(d->zoomToFitWidth, SIGNAL(triggered()), imageView->zoomManager(), SLOT(slotZoomToFitWidth()));
        d->viewConnections.addUniqueConnection(d->zoomToFitHeight, SIGNAL(triggered()), imageView->zoomManager(), SLOT(slotZoomToFitHeight()));
        d->viewConnections.addUniqueConnection(d->toggleZoomToFit, SIGNAL(triggered()), imageView->zoomManager(), SLOT(slotToggleZoomToFit()));

        d->viewConnections.addUniqueConnection(d->resetDisplay, SIGNAL(triggered()), imageView->viewManager(), SLOT(slotResetDisplay()));

        d->viewConnections.addConnection(imageView->canvasController(),
                                         &KisCanvasController::sigUsePrintResolutionModeChanged,
                                         this,
                                         [this](bool value) {
                                             QSignalBlocker b(d->viewPrintSize);
                                             d->viewPrintSize->setChecked(value);
                                         });
        d->viewPrintSize->setChecked(imageView->canvasController()->usePrintResolutionMode());
        d->viewConnections.addUniqueConnection(d->viewPrintSize, &KisAction::toggled,
            imageView->canvasController(), &KisCanvasController::setUsePrintResolutionMode);

        d->viewConnections.addUniqueConnection(imageView->canvasController(),
                                               &KisCanvasController::sigUsePrintResolutionModeChanged,
                                               imageView->zoomManager()->zoomAction(),
                                               &KoZoomAction::setUsePrintResolutionMode);
        imageView->zoomManager()->zoomAction()->setUsePrintResolutionMode(imageView->canvasController()->usePrintResolutionMode());
        d->viewConnections.addUniqueConnection(imageView->zoomManager()->zoomAction(),
                                               &KoZoomAction::sigUsePrintResolutionModeChanged,
                                               imageView->canvasController(),
                                               &KisCanvasController::setUsePrintResolutionMode);

        d->viewConnections.addUniqueConnection(d->softProof, SIGNAL(toggled(bool)), view, SLOT(slotSoftProofing(bool)) );
        d->viewConnections.addUniqueConnection(d->gamutCheck, SIGNAL(toggled(bool)), view, SLOT(slotGamutCheck(bool)) );

        // set up progress reporting
        doc->image()->compositeProgressProxy()->addProxy(d->persistentImageProgressUpdater);
        d->viewConnections.addUniqueConnection(&d->statusBar, SIGNAL(sigCancellationRequested()), doc->image(), SLOT(requestStrokeCancellation()));

        d->viewConnections.addUniqueConnection(d->showPixelGrid, SIGNAL(toggled(bool)), canvasController, SLOT(slotTogglePixelGrid(bool)));

        imageView->zoomManager()->setShowRulers(d->showRulersAction->isChecked());
        imageView->zoomManager()->setRulersTrackMouse(d->rulersTrackMouseAction->isChecked());

        showHideScrollbars();
    }

    d->filterManager.setView(imageView);
    d->selectionManager.setView(imageView);
    d->guidesManager.setView(imageView);
    d->nodeManager.setView(imageView);
    d->imageManager.setView(imageView);
    d->canvasControlsManager.setView(imageView);
    d->actionManager.setView(imageView);
    d->gridManager.setView(imageView);
    d->statusBar.setView(imageView);
    d->paintingAssistantsManager.setView(imageView);
    d->mirrorManager.setView(imageView);

    if (d->currentImageView) {
        d->currentImageView->notifyCurrentStateChanged(true);
        d->currentImageView->canvasController()->activate();
        d->currentImageView->canvasController()->setFocus();

        d->viewConnections.addUniqueConnection(
                    image(), SIGNAL(sigSizeChanged(QPointF,QPointF)),
                    canvasResourceProvider(), SLOT(slotImageSizeChanged()));

        d->viewConnections.addUniqueConnection(
                    image(), SIGNAL(sigResolutionChanged(double,double)),
                    canvasResourceProvider(), SLOT(slotOnScreenResolutionChanged()));

        d->viewConnections.addUniqueConnection(
                    image(), SIGNAL(sigNodeChanged(KisNodeSP)),
                    this, SLOT(updateGUI()));

        d->viewConnections.addUniqueConnection(
                    d->currentImageView->canvasController()->proxyObject,
                    &KoCanvasControllerProxyObject::effectiveZoomChanged,
                    canvasResourceProvider(),
                    &KisCanvasResourceProvider::slotOnScreenResolutionChanged);
    }

    d->actionManager.updateGUI();

    canvasResourceProvider()->slotImageSizeChanged();
    canvasResourceProvider()->slotOnScreenResolutionChanged();

    Q_EMIT viewChanged();
}

KisImageWSP KisViewManager::image() const
{
    if (document()) {
        return document()->image();
    }
    return 0;
}

KisCanvasResourceProvider * KisViewManager::canvasResourceProvider()
{
    return &d->canvasResourceProvider;
}

KisCanvas2 * KisViewManager::canvasBase() const
{
    if (d && d->currentImageView) {
        return d->currentImageView->canvasBase();
    }
    return 0;
}

QWidget* KisViewManager::canvas() const
{
    if (d && d->currentImageView && d->currentImageView->canvasBase()->canvasWidget()) {
        return d->currentImageView->canvasBase()->canvasWidget();
    }
    return 0;
}

KisStatusBar * KisViewManager::statusBar() const
{
    return &d->statusBar;
}

KisPaintopBox* KisViewManager::paintOpBox() const
{
    return d->controlFrame.paintopBox();
}

QPointer<KoUpdater> KisViewManager::createUnthreadedUpdater(const QString &name)
{
    return d->persistentUnthreadedProgressUpdaterRouter->startSubtask(1, name, false);
}

QPointer<KoUpdater> KisViewManager::createThreadedUpdater(const QString &name)
{
    return d->statusBar.progressUpdater()->startSubtask(1, name, false);
}

KisSelectionManager * KisViewManager::selectionManager()
{
    return &d->selectionManager;
}

KisNodeSP KisViewManager::activeNode()
{
    return d->nodeManager.activeNode();
}

KisLayerSP KisViewManager::activeLayer()
{
    return d->nodeManager.activeLayer();
}

KisPaintDeviceSP KisViewManager::activeDevice()
{
    return d->nodeManager.activePaintDevice();
}

KisZoomManager * KisViewManager::zoomManager()
{
    if (d->currentImageView) {
        return d->currentImageView->zoomManager();
    }
    return 0;
}

KisFilterManager * KisViewManager::filterManager()
{
    return &d->filterManager;
}

KisImageManager * KisViewManager::imageManager()
{
    return &d->imageManager;
}

KisInputManager* KisViewManager::inputManager() const
{
    return &d->inputManager;
}

KisIdleTasksManager *KisViewManager::idleTasksManager()
{
    return &d->idleTasksManager;
}

KisTextPropertiesManager *KisViewManager::textPropertyManager() const
{
    return &d->textPropertyManager;
}

KisSelectionSP KisViewManager::selection()
{
    if (d->currentImageView) {
        return d->currentImageView->selection();
    }
    return 0;

}

bool KisViewManager::selectionEditable()
{
    KisLayerSP layer = activeLayer();
    if (layer) {
        KisSelectionMaskSP mask = layer->selectionMask();
        if (mask) {
            return mask->isEditable();
        }
    }
    // global selection is always editable
    return true;
}

KisUndoAdapter * KisViewManager::undoAdapter()
{
    if (!document()) return 0;

    KisImageWSP image = document()->image();
    Q_ASSERT(image);

    return image->undoAdapter();
}

void KisViewManager::createActions()
{
    KisConfig cfg(true);

    d->saveIncremental = actionManager()->createAction("save_incremental_version");
    connect(d->saveIncremental, SIGNAL(triggered()), this, SLOT(slotSaveIncremental()));

    d->saveIncrementalBackup = actionManager()->createAction("save_incremental_backup");
    connect(d->saveIncrementalBackup, SIGNAL(triggered()), this, SLOT(slotSaveIncrementalBackup()));

    connect(mainWindow(), SIGNAL(documentSaved()), this, SLOT(slotDocumentSaved()));

    d->saveIncremental->setEnabled(false);
    d->saveIncrementalBackup->setEnabled(false);

    KisAction *tabletDebugger = actionManager()->createAction("tablet_debugger");
    connect(tabletDebugger, SIGNAL(triggered()), this, SLOT(toggleTabletLogger()));

    d->createTemplate = actionManager()->createAction("create_template");
    connect(d->createTemplate, SIGNAL(triggered()), this, SLOT(slotCreateTemplate()));

    d->createCopy = actionManager()->createAction("create_copy");
    connect(d->createCopy, SIGNAL(triggered()), this, SLOT(slotCreateCopy()));

    d->openResourcesDirectory = actionManager()->createAction("open_resources_directory");
    connect(d->openResourcesDirectory, SIGNAL(triggered()), SLOT(openResourcesDirectory()));

    d->rotateCanvasRight   = actionManager()->createAction("rotate_canvas_right");
    d->rotateCanvasLeft    = actionManager()->createAction("rotate_canvas_left");
    d->resetCanvasRotation = actionManager()->createAction("reset_canvas_rotation");
    d->wrapAroundAction    = actionManager()->createAction("wrap_around_mode");
    d->wrapAroundHVAxisAction = actionManager()->createAction("wrap_around_hv_axis");
    d->wrapAroundHAxisAction  = actionManager()->createAction("wrap_around_h_axis");
    d->wrapAroundVAxisAction  = actionManager()->createAction("wrap_around_v_axis");
    d->wrapAroundAxisActions = new QActionGroup(this);
    d->wrapAroundAxisActions->addAction(d->wrapAroundHVAxisAction);
    d->wrapAroundAxisActions->addAction(d->wrapAroundHAxisAction);
    d->wrapAroundAxisActions->addAction(d->wrapAroundVAxisAction);
    d->levelOfDetailAction = actionManager()->createAction("level_of_detail_mode");
    d->softProof           = actionManager()->createAction("softProof");
    d->gamutCheck          = actionManager()->createAction("gamutCheck");

    KisAction *tAction = actionManager()->createAction("showStatusBar");
    tAction->setChecked(cfg.showStatusBar());
    connect(tAction, SIGNAL(toggled(bool)), this, SLOT(showStatusBar(bool)));

    tAction = actionManager()->createAction("view_show_canvas_only");
    tAction->setChecked(false);
    connect(tAction, SIGNAL(toggled(bool)), this, SLOT(switchCanvasOnly(bool)));

    //Workaround, by default has the same shortcut as mirrorCanvas
    KisAction *a = dynamic_cast<KisAction*>(actionCollection()->action("format_italic"));
    if (a) {
        a->setDefaultShortcut(QKeySequence());
    }

    actionManager()->createAction("ruler_pixel_multiple2");
    d->showRulersAction = actionManager()->createAction("view_ruler");
    d->showRulersAction->setChecked(cfg.showRulers());
    connect(d->showRulersAction, SIGNAL(toggled(bool)), SLOT(slotSaveShowRulersState(bool)));

    d->rulersTrackMouseAction = actionManager()->createAction("rulers_track_mouse");
    d->rulersTrackMouseAction->setChecked(cfg.rulersTrackMouse());
    connect(d->rulersTrackMouseAction, SIGNAL(toggled(bool)), SLOT(slotSaveRulersTrackMouseState(bool)));

    d->zoomTo100pct = actionManager()->createAction("zoom_to_100pct");

    d->zoomIn = actionManager()->createStandardAction(KStandardAction::ZoomIn, 0, "");
    d->zoomOut = actionManager()->createStandardAction(KStandardAction::ZoomOut, 0, "");

    d->zoomToFit = actionManager()->createAction("zoom_to_fit");
    d->zoomToFitWidth = actionManager()->createAction("zoom_to_fit_width");
    d->zoomToFitHeight = actionManager()->createAction("zoom_to_fit_height");
    d->toggleZoomToFit = actionManager()->createAction("toggle_zoom_to_fit");

    d->resetDisplay = actionManager()->createAction("reset_display");

    d->viewPrintSize = actionManager()->createAction("view_print_size");

    d->actionAuthor  = new KSelectAction(KisIconUtils::loadIcon("im-user"), i18n("Active Author Profile"), this);
    connect(d->actionAuthor, SIGNAL(textTriggered(QString)), this, SLOT(changeAuthorProfile(QString)));
    actionCollection()->addAction("settings_active_author", d->actionAuthor);
    slotUpdateAuthorProfileActions();

    d->showPixelGrid = actionManager()->createAction("view_pixel_grid");
    slotUpdatePixelGridAction();

    d->toggleFgBg = actionManager()->createAction("toggle_fg_bg");
    connect(d->toggleFgBg, SIGNAL(triggered(bool)), this, SLOT(slotToggleFgBg()));

    d->resetFgBg =  actionManager()->createAction("reset_fg_bg");
    connect(d->resetFgBg, SIGNAL(triggered(bool)), this, SLOT(slotResetFgBg()));

    d->toggleBrushOutline =  actionManager()->createAction("toggle_brush_outline");
    connect(d->toggleBrushOutline, SIGNAL(triggered(bool)), this, SLOT(slotToggleBrushOutline()));

}

void KisViewManager::setupManagers()
{
    // Create the managers for filters, selections, layers etc.
    // XXX: When the current layer changes, call updateGUI on all
    // managers

    d->filterManager.setup(actionCollection(), actionManager());

    d->selectionManager.setup(actionManager());

    d->guidesManager.setup(actionManager());

    d->nodeManager.setup(actionCollection(), actionManager());

    d->imageManager.setup(actionManager());

    d->gridManager.setup(actionManager());

    d->paintingAssistantsManager.setup(actionManager());

    d->canvasControlsManager.setup(actionManager());

    d->mirrorManager.setup(actionCollection());
}

void KisViewManager::updateGUI()
{
    d->guiUpdateCompressor.start();
}

KisNodeManager * KisViewManager::nodeManager() const
{
    return &d->nodeManager;
}

KisActionManager* KisViewManager::actionManager() const
{
    return &d->actionManager;
}

KisGridManager * KisViewManager::gridManager() const
{
    return &d->gridManager;
}

KisGuidesManager * KisViewManager::guidesManager() const
{
    return &d->guidesManager;
}

KisDocument *KisViewManager::document() const
{
    if (d->currentImageView && d->currentImageView->document()) {
        return d->currentImageView->document();
    }
    return 0;
}

int KisViewManager::viewCount() const
{
    KisMainWindow *mw  = qobject_cast<KisMainWindow*>(d->mainWindow);
    if (mw) {
        return mw->viewCount();
    }
    return 0;
}

bool KisViewManager::KisViewManagerPrivate::blockUntilOperationsFinishedImpl(KisImageSP image, bool force)
{
    const int busyWaitDelay = 1000;
    KisDelayedSaveDialog dialog(image, !force ? KisDelayedSaveDialog::GeneralDialog : KisDelayedSaveDialog::ForcedDialog, busyWaitDelay, mainWindow);
    dialog.blockIfImageIsBusy();

    return dialog.result() == QDialog::Accepted;
}


bool KisViewManager::blockUntilOperationsFinished(KisImageSP image)
{
    return d->blockUntilOperationsFinishedImpl(image, false);
}

void KisViewManager::blockUntilOperationsFinishedForced(KisImageSP image)
{
    d->blockUntilOperationsFinishedImpl(image, true);
}

void KisViewManager::slotCreateTemplate()
{
    if (!document()) return;
    KisTemplateCreateDia::createTemplate( QStringLiteral("templates/"), ".kra", document(), mainWindow());
}

void KisViewManager::slotCreateCopy()
{
    KisDocument *srcDoc = document();
    if (!srcDoc) return;

    if (!this->blockUntilOperationsFinished(srcDoc->image())) return;

    KisDocument *doc = 0;
    {
        KisImageReadOnlyBarrierLock l(srcDoc->image());
        doc = srcDoc->clone(true);
    }
    KIS_SAFE_ASSERT_RECOVER_RETURN(doc);

    QString name = srcDoc->documentInfo()->aboutInfo("name");
    if (name.isEmpty()) {
        name = document()->path();
    }
    name = i18n("%1 (Copy)", name);
    doc->documentInfo()->setAboutInfo("title", name);
    doc->resetPath();

    KisPart::instance()->addDocument(doc);
    KisMainWindow *mw  = qobject_cast<KisMainWindow*>(d->mainWindow);
    mw->addViewAndNotifyLoadingCompleted(doc);
}


QMainWindow* KisViewManager::qtMainWindow() const
{
    if (d->mainWindow)
        return d->mainWindow;

    //Fallback for when we have not yet set the main window.
    QMainWindow* w = qobject_cast<QMainWindow*>(qApp->activeWindow());
    if(w)
        return w;

    return mainWindow();
}

void KisViewManager::setQtMainWindow(QMainWindow* newMainWindow)
{
    d->mainWindow = newMainWindow;
}

void KisViewManager::slotDocumentSaved()
{
    d->saveIncremental->setEnabled(true);
    d->saveIncrementalBackup->setEnabled(true);
}

QString KisViewManager::canonicalPath()
{
#ifdef Q_OS_ANDROID
    QString path = QFileInfo(document()->localFilePath()).canonicalPath();
    // if the path is based on a document tree then a directory would be returned. So check if it exists and more
    // importantly check if we have permissions
    if (QDir(path).exists()) {
        return path;
    } else {
        KoFileDialog dialog(nullptr, KoFileDialog::ImportDirectory, "OpenDirectory");
        dialog.setDirectoryUrl(QUrl(document()->localFilePath()));
        return dialog.filename();
    }
#else
    return QFileInfo(document()->localFilePath()).canonicalPath();
#endif
}

void KisViewManager::slotSaveIncremental()
{
    if (!document()) return;

    if (document()->path().isEmpty()) {
        KisMainWindow *mw = qobject_cast<KisMainWindow*>(d->mainWindow);
        mw->saveDocument(document(), true, false);
        return;
    }

    bool foundVersion;
    bool fileAlreadyExists;
    bool isBackup;
    QString version = "000";
    QString newVersion;
    QString letter;
    QString path = canonicalPath();

    QString fileName = QFileInfo(document()->localFilePath()).fileName();

    // Find current version filenames
    // v v Regexp to find incremental versions in the filename, taking our backup scheme into account as well
    // Considering our incremental version and backup scheme, format is filename_001~001.ext
    QRegExp regex("_\\d{1,4}[.]|_\\d{1,4}[a-z][.]|_\\d{1,4}[~]|_\\d{1,4}[a-z][~]");
    regex.indexIn(fileName);     //  Perform the search
    QStringList matches = regex.capturedTexts();
    foundVersion = matches.at(0).isEmpty() ? false : true;

    // Ensure compatibility with Save Incremental Backup
    // If this regex is not kept separate, the entire algorithm needs modification;
    // It's simpler to just add this.
    QRegExp regexAux("_\\d{1,4}[~]|_\\d{1,4}[a-z][~]");
    regexAux.indexIn(fileName);     //  Perform the search
    QStringList matchesAux = regexAux.capturedTexts();
    isBackup = matchesAux.at(0).isEmpty() ? false : true;

    // If the filename has a version, prepare it for incrementation
    if (foundVersion) {
        version = matches.at(matches.count() - 1);     //  Look at the last index, we don't care about other matches
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        if (version.contains(QRegExp("[a-z]"))) {
#else
        if (QRegExp("[a-z]").containedIn(version)) {
#endif

            version.chop(1);             //  Trim "."
            letter = version.right(1);   //  Save letter
            version.chop(1);             //  Trim letter
        } else {
            version.chop(1);             //  Trim "."
        }
        version.remove(0, 1);            //  Trim "_"
    } else {
        // TODO: this will not work with files extensions like jp2
        // ...else, simply add a version to it so the next loop works
        QRegExp regex2("[.][a-z]{2,4}$");  //  Heuristic to find file extension
        regex2.indexIn(fileName);
        QStringList matches2 = regex2.capturedTexts();
        QString extensionPlusVersion = matches2.at(0);
        extensionPlusVersion.prepend(version);
        extensionPlusVersion.prepend("_");
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        fileName.replace(regex2, extensionPlusVersion);
#else
        regex2.replaceIn(fileName, extensionPlusVersion);
#endif
    }

    // Prepare the base for new version filename
    int intVersion = version.toInt(0);
    ++intVersion;
    QString baseNewVersion = QString::number(intVersion);
    while (baseNewVersion.length() < version.length()) {
        baseNewVersion.prepend("0");
    }

    // Check if the file exists under the new name and search until options are exhausted (test appending a to z)
    do {
        newVersion = baseNewVersion;
        newVersion.prepend("_");
        if (!letter.isNull()) newVersion.append(letter);
        if (isBackup) {
            newVersion.append("~");
        } else {
            newVersion.append(".");
        }
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        fileName.replace(regex, newVersion);
#else
        regex.replaceIn(fileName, newVersion);
#endif
        fileAlreadyExists = QFileInfo(path + '/' + fileName).exists();
        if (fileAlreadyExists) {
            if (!letter.isNull()) {
                char letterCh = letter.at(0).toLatin1();
                ++letterCh;
                letter = QString(QChar(letterCh));
            } else {
                letter = 'a';
            }
        }
    } while (fileAlreadyExists && letter != "{");  // x, y, z, {...

    if (letter == "{") {
        QMessageBox::critical(mainWindow(), i18nc("@title:window", "Couldn't save incremental version"), i18n("Alternative names exhausted, try manually saving with a higher number"));
        return;
    }
    QString newFilePath = path + '/' + fileName;
    document()->setFileBatchMode(true);
    document()->saveAs(newFilePath, document()->mimeType(), true);
    document()->setFileBatchMode(false);
    KisPart::instance()->queueAddRecentURLToAllMainWindowsOnFileSaved(QUrl::fromLocalFile(newFilePath),
                                                                      QUrl::fromLocalFile(document()->path()));
}

void KisViewManager::slotSaveIncrementalBackup()
{
    if (!document()) return;

    if (document()->path().isEmpty()) {
        KisMainWindow *mw = qobject_cast<KisMainWindow*>(d->mainWindow);
        mw->saveDocument(document(), true, false);
        return;
    }

    bool workingOnBackup;
    bool fileAlreadyExists;
    QString version = "000";
    QString newVersion;
    QString letter;
    QString path = canonicalPath();
    QString fileName = QFileInfo(document()->localFilePath()).fileName();

    // First, discover if working on a backup file, or a normal file
    QRegExp regex("~\\d{1,4}[.]|~\\d{1,4}[a-z][.]");
    regex.indexIn(fileName);     //  Perform the search
    QStringList matches = regex.capturedTexts();
    workingOnBackup = matches.at(0).isEmpty() ? false : true;

    if (workingOnBackup) {
        // Try to save incremental version (of backup), use letter for alt versions
        version = matches.at(matches.count() - 1);     //  Look at the last index, we don't care about other matches
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        if (version.contains(QRegExp("[a-z]"))) {
#else
        if (QRegExp("[a-z]").containedIn(version)) {
#endif
            version.chop(1);             //  Trim "."
            letter = version.right(1);   //  Save letter
            version.chop(1);             //  Trim letter
        } else {
            version.chop(1);             //  Trim "."
        }
        version.remove(0, 1);            //  Trim "~"

        // Prepare the base for new version filename
        int intVersion = version.toInt(0);
        ++intVersion;
        QString baseNewVersion = QString::number(intVersion);
        QString backupFileName = document()->localFilePath();
        while (baseNewVersion.length() < version.length()) {
            baseNewVersion.prepend("0");
        }

        // Check if the file exists under the new name and search until options are exhausted (test appending a to z)
        do {
            newVersion = baseNewVersion;
            newVersion.prepend("~");
            if (!letter.isNull()) newVersion.append(letter);
            newVersion.append(".");
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            backupFileName.replace(regex, newVersion);
#else
            regex.replaceIn(backupFileName, newVersion);
#endif
            fileAlreadyExists = QFile(path + '/' + backupFileName).exists();
            if (fileAlreadyExists) {
                if (!letter.isNull()) {
                    char letterCh = letter.at(0).toLatin1();
                    ++letterCh;
                    letter = QString(QChar(letterCh));
                } else {
                    letter = 'a';
                }
            }
        } while (fileAlreadyExists && letter != "{");  // x, y, z, {...

        if (letter == "{") {
            QMessageBox::critical(mainWindow(), i18nc("@title:window", "Couldn't save incremental backup"), i18n("Alternative names exhausted, try manually saving with a higher number"));
            return;
        }
        QFile::copy(path + '/' + fileName, path + '/' + backupFileName);
        document()->saveAs(path + '/' + fileName, document()->mimeType(), true);
    }
    else { // if NOT working on a backup...
        // Navigate directory searching for latest backup version, ignore letters
        const quint8 HARDCODED_DIGIT_COUNT = 3;
        QString baseNewVersion = "000";
        QString backupFileName = QFileInfo(document()->localFilePath()).fileName();
        QRegExp regex2("[.][a-z]{2,4}$");  //  Heuristic to find file extension
        regex2.indexIn(backupFileName);
        QStringList matches2 = regex2.capturedTexts();
        QString extensionPlusVersion = matches2.at(0);
        extensionPlusVersion.prepend(baseNewVersion);
        extensionPlusVersion.prepend("~");
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        backupFileName.replace(regex2, extensionPlusVersion);
#else
        regex2.replaceIn(backupFileName, extensionPlusVersion);
#endif

        // Save version with 1 number higher than the highest version found ignoring letters
        do {
            newVersion = baseNewVersion;
            newVersion.prepend("~");
            newVersion.append(".");
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            backupFileName.replace(regex, newVersion);
#else
            regex.replaceIn(backupFileName, newVersion);
#endif
            fileAlreadyExists = QFile(path + '/' + backupFileName).exists();
            if (fileAlreadyExists) {
                // Prepare the base for new version filename, increment by 1
                int intVersion = baseNewVersion.toInt(0);
                ++intVersion;
                baseNewVersion = QString::number(intVersion);
                while (baseNewVersion.length() < HARDCODED_DIGIT_COUNT) {
                    baseNewVersion.prepend("0");
                }
            }
        } while (fileAlreadyExists);

        // Save both as backup and on current file for interapplication workflow
        document()->setFileBatchMode(true);
        QFile::copy(path + '/' + fileName, path + '/' + backupFileName);
        document()->saveAs(path + '/' + fileName, document()->mimeType(), true);
        document()->setFileBatchMode(false);
    }
}

void KisViewManager::disableControls()
{
    // prevents possible crashes, if somebody changes the paintop during dragging by using the mousewheel
    // this is for Bug 250944
    // the solution blocks all wheel, mouse and key event, while dragging with the freehand tool
    // see KisToolFreehand::initPaint() and endPaint()
    d->controlFrame.paintopBox()->installEventFilter(&d->blockingEventFilter);
    Q_FOREACH (QObject* child, d->controlFrame.paintopBox()->children()) {
        child->installEventFilter(&d->blockingEventFilter);
    }
}

void KisViewManager::enableControls()
{
    d->controlFrame.paintopBox()->removeEventFilter(&d->blockingEventFilter);
    Q_FOREACH (QObject* child, d->controlFrame.paintopBox()->children()) {
        child->removeEventFilter(&d->blockingEventFilter);
    }
}

void KisViewManager::showStatusBar(bool toggled)
{
    KisMainWindow *mw = mainWindow();
    if(mw && mw->statusBar()) {
        mw->statusBar()->setVisible(toggled);
        KisConfig cfg(false);
        cfg.setShowStatusBar(toggled);
    }
}

void KisViewManager::notifyWorkspaceLoaded()
{
    d->canvasStateInNormalMode.clear();
    d->canvasStateInCanvasOnlyMode.clear();
    d->canvasOnlyOptions = std::nullopt;
}

void KisViewManager::switchCanvasOnly(bool toggled)
{
    KisConfig cfg(false);
    KisMainWindow *main = mainWindow();

    if(!main) {
        dbgUI << "Unable to switch to canvas-only mode, main window not found";
        return;
    }

    cfg.writeEntry("CanvasOnlyActive", toggled);

    KisViewManagerPrivate::CanvasOnlyOptions options(cfg);

    if (toggled) {
        d->canvasStateInNormalMode = qtMainWindow()->saveState();
    } else {
        d->canvasStateInCanvasOnlyMode = qtMainWindow()->saveState();
        d->canvasOnlyOptions = options;
    }

    const bool toggleFullscreen = (options.hideTitlebarFullscreen && !cfg.fullscreenMode());
    const bool useCanvasOffsetCompensation = d->currentImageView &&
                                             d->currentImageView->canvasController() &&
                                             d->currentImageView->isMaximized() &&
                                             !main->canvasDetached();

    if (useCanvasOffsetCompensation) {
        // The offset is calculated in two steps; this is the first step.
        if (toggled) {
            /**
             *  Capture the initial canvas position.
             *
             *  Going into a fullscreen mode is a multistage process that includes making
             *  the window fullscreen. It means that it is not easy to catch a moment,
             *  when the new state is "fully established". First, it hides the dockers, then
             *  it hides the menu, then it makes the window fullscreen (hiding titlebar
             *  and "start" menu). And these actions happen in the course of dozens of
             *  milliseconds.
             *
             *  And since we cannot catch the moment when the new state is "fully
             *  established" we use a heuristic (read "a hack") that just pre-scrolls
             *  the canves into an expected offset. The prescroll offset is saved
             *  **globally**, so that when exiting the fullscreen mode we would know
             *  how big the new offset should (since we cannot get the real value of
             *  it).
             *
             *  Here are the list of cases when this approach fails:
             *
             *  1) When the fullscreen window migrates to a different screen,
             *     potentially with a different resolution (while being fullscreen)
             *
             *  2) When dockers or toolbars are visible, and the user resizes
             *     them while the window is fullscreen.
             *
             *  In both the cases the saved offset in `d->canvasOnlyOffsetCompensation`
             *  becomes invalid and the window jumps in an offset direction on exiting
             *  the fullscreen mode.
             */
            QPoint origin;
            if (toggleFullscreen) {
                // We're windowed, so also capture the position of the window in the screen.
                origin = main->geometry().topLeft() - main->screen()->geometry().topLeft();
            }
            d->canvasOnlyOffsetCompensation = d->currentImageView->mapTo(main, origin);
        } else {
            // Restore the original canvas position. The result is more stable if we pan before showing the UI elements.
            d->currentImageView->canvasController()->pan(- d->canvasOnlyOffsetCompensation);
        }
    }

    if (options.hideStatusbarFullscreen) {
        if (main->statusBar()) {
            if (!toggled) {
                if (main->statusBar()->dynamicPropertyNames().contains("wasvisible")) {
                    if (main->statusBar()->property("wasvisible").toBool()) {
                        main->statusBar()->setVisible(true);
                    }
                }
            }
            else {
                main->statusBar()->setProperty("wasvisible", main->statusBar()->isVisible());
                main->statusBar()->setVisible(false);
            }
        }
    }

    if (options.hideDockersFullscreen) {
        KisAction* action = qobject_cast<KisAction*>(main->actionCollection()->action("view_toggledockers"));
        if (action) {
            action->setCheckable(true);
            if (toggled) {
                if (action->isChecked()) {
                    cfg.setShowDockers(action->isChecked());
                    action->setChecked(false);
                } else {
                    cfg.setShowDockers(false);
                }
            } else {
                action->setChecked(cfg.showDockers());
            }
        }
    }

    // QT in windows does not return to maximized upon 4th tab in a row
    // https://bugreports.qt.io/browse/QTBUG-57882, https://bugreports.qt.io/browse/QTBUG-52555, https://codereview.qt-project.org/#/c/185016/
    if (toggleFullscreen) {
        if(toggled) {
            main->setWindowState( main->windowState() | Qt::WindowFullScreen);
        } else {
            main->setWindowState( main->windowState() & ~Qt::WindowFullScreen);
        }
    }

    if (options.hideMenuFullscreen) {
        if (!toggled) {
            if (main->menuBar()->dynamicPropertyNames().contains("wasvisible")) {
                if (main->menuBar()->property("wasvisible").toBool()) {
                    main->menuBar()->setVisible(true);
                }
            }
        }
        else {
            main->menuBar()->setProperty("wasvisible", main->menuBar()->isVisible());
            main->menuBar()->setVisible(false);
        }
    }

    if (options.hideToolbarFullscreen) {
        QList<QToolBar*> toolBars = main->findChildren<QToolBar*>();
        Q_FOREACH (QToolBar* toolbar, toolBars) {
            if (!toggled) {
                if (toolbar->dynamicPropertyNames().contains("wasvisible")) {
                    if (toolbar->property("wasvisible").toBool()) {
                        toolbar->setVisible(true);
                    }
                }
            }
            else {
                toolbar->setProperty("wasvisible", toolbar->isVisible());
                toolbar->setVisible(false);
            }
        }
    }

    showHideScrollbars();

    if (toggled) {
        if (!d->canvasStateInCanvasOnlyMode.isEmpty() &&
            d->canvasOnlyOptions &&
            *d->canvasOnlyOptions == options) {

            /**
             * Restore state uses the current layout state of the window,
             * but removal of the menu will be backed into this state after
             * receiving of some events in the event queue. Hence we cannot
             * apply the application of the saved state directly. We need to
             * postpone that via the events queue.
             *
             * See https://bugs.kde.org/show_bug.cgi?id=475973
             */
            QTimer::singleShot(0, this, [this] () {
                this->mainWindow()->restoreState(d->canvasStateInCanvasOnlyMode);
            });
        }

               // show a fading heads-up display about the shortcut to go back
        showFloatingMessage(i18n("Going into Canvas-Only mode.\nPress %1 to go back.",
                                 actionCollection()->action("view_show_canvas_only")->shortcut().toString(QKeySequence::NativeText)), QIcon(),
                            2000,
                            KisFloatingMessage::Low);
    }
    else {
        if (!d->canvasStateInNormalMode.isEmpty()) {
            main->restoreState(d->canvasStateInNormalMode);
        }
    }

    if (useCanvasOffsetCompensation && toggled) {
        const KoZoomMode::Mode mode = d->currentImageView->canvasController()->zoomState().mode;
        
        const bool allowedZoomMode =
            (mode == KoZoomMode::ZOOM_CONSTANT) ||
            (mode == KoZoomMode::ZOOM_HEIGHT);

        if (allowedZoomMode) {
            // Defer the pan action until the layout is fully settled in (including the menu bars, etc.).
            QTimer::singleShot(0, this, [this] () {
                // Compensate by the difference of (after - before) layout.
                d->canvasOnlyOffsetCompensation = d->currentImageView->mapTo(this->mainWindow(), QPoint()) - d->canvasOnlyOffsetCompensation;
                d->currentImageView->canvasController()->pan(d->canvasOnlyOffsetCompensation);
            });
        } else {
            // Nothing to restore.
            d->canvasOnlyOffsetCompensation = QPoint();
        }
    }
}

void KisViewManager::toggleTabletLogger()
{
    d->inputManager.toggleTabletLogger();
}

void KisViewManager::openResourcesDirectory()
{
    QString resourcePath = KisResourceLocator::instance()->resourceLocationBase();
#ifdef Q_OS_WIN

    QString folderInStandardAppData;
    QString folderInPrivateAppData;
    KoResourcePaths::getAllUserResourceFoldersLocationsForWindowsStore(folderInStandardAppData, folderInPrivateAppData);

    if (!folderInPrivateAppData.isEmpty()) {

        const auto pathToDisplay = [](const QString &path) {
            // Due to how Unicode word wrapping works, the string does not
            // wrap after backslashes in Qt 5.12. We don't want the path to
            // become too long, so we add a U+200B ZERO WIDTH SPACE to allow
            // wrapping. The downside is that we cannot let the user select
            // and copy the path because it now contains invisible unicode
            // code points.
            // See: https://bugreports.qt.io/browse/QTBUG-80892
            return QDir::toNativeSeparators(path).replace(QChar('\\'), QStringLiteral(u"\\\u200B"));
        };

        QMessageBox mbox(qApp->activeWindow());
        mbox.setIcon(QMessageBox::Information);
        mbox.setWindowTitle(i18nc("@title:window resource folder", "Open Resource Folder"));
        // Similar text is also used in kis_dlg_preferences.cc

        mbox.setText(i18nc("@info resource folder",
            "<p>You are using the Microsoft Store package version of Krita. "
            "Even though Krita can be configured to place resources under the "
            "user AppData location, Windows may actually store the files "
            "inside a private app location.</p>\n"
            "<p>You should check both locations to determine where "
            "the files are located.</p>\n"
            "<p><b>User AppData</b>:<br/>\n"
            "%1</p>\n"
            "<p><b>Private app location</b>:<br/>\n"
            "%2</p>",
            pathToDisplay(folderInStandardAppData),
            pathToDisplay(folderInPrivateAppData)
        ));
        mbox.setTextInteractionFlags(Qt::NoTextInteraction);

        const auto *btnOpenUserAppData = mbox.addButton(i18nc("@action:button resource folder", "Open in &user AppData"), QMessageBox::AcceptRole);
        const auto *btnOpenPrivateAppData = mbox.addButton(i18nc("@action:button resource folder", "Open in &private app location"), QMessageBox::AcceptRole);

        mbox.addButton(QMessageBox::Close);
        mbox.setDefaultButton(QMessageBox::Close);
        mbox.exec();

        if (mbox.clickedButton() == btnOpenPrivateAppData) {
            resourcePath = folderInPrivateAppData;
        } else if (mbox.clickedButton() == btnOpenUserAppData) {
            // no-op: resourcePath = resourceDir.absolutePath();
        } else {
            return;
        }


    }
#endif
    QDesktopServices::openUrl(QUrl::fromLocalFile(resourcePath));
}

void KisViewManager::updateIcons()
{
    if (mainWindow()) {
        QList<QDockWidget*> dockers = mainWindow()->dockWidgets();
        Q_FOREACH (QDockWidget* dock, dockers) {
            KoDockWidgetTitleBar* titlebar = dynamic_cast<KoDockWidgetTitleBar*>(dock->titleBarWidget());
            if (titlebar) {
                titlebar->updateIcons();
            }
            if (qobject_cast<KoToolDocker*>(dock)) {
                // Tool options widgets icons are updated by KoToolManager
                continue;
            }
            QObjectList objects;
            objects.append(dock);
            while (!objects.isEmpty()) {
                QObject* object = objects.takeFirst();
                objects.append(object->children());
                KisIconUtils::updateIconCommon(object);
            }
        }
    }
}

void KisViewManager::guiUpdateTimeout()
{
    d->nodeManager.updateGUI();
    d->selectionManager.updateGUI();
    d->filterManager.updateGUI();
    d->gridManager.updateGUI();
    d->actionManager.updateGUI();
}

void KisViewManager::showFloatingMessage(const QString &message, const QIcon& icon, int timeout, KisFloatingMessage::Priority priority, int alignment)
{
    if (!d->currentImageView) return;
    d->currentImageView->showFloatingMessage(message, icon, timeout, priority, alignment);

    Q_EMIT floatingMessageRequested(message, icon.name());
}

KisMainWindow *KisViewManager::mainWindow() const
{
    return qobject_cast<KisMainWindow*>(d->mainWindow);
}

QWidget *KisViewManager::mainWindowAsQWidget() const
{
    return mainWindow();
}


void KisViewManager::showHideScrollbars()
{
    if (!d->currentImageView) return;
    if (!d->currentImageView->canvasController()) return;

    KisConfig cfg(true);
    bool toggled = actionCollection()->action("view_show_canvas_only")->isChecked();

    if ( (toggled && cfg.hideScrollbarsFullscreen()) || (!toggled && cfg.hideScrollbars()) ) {
        d->currentImageView->canvasController()->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        d->currentImageView->canvasController()->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    } else {
        d->currentImageView->canvasController()->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        d->currentImageView->canvasController()->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    }
}

void KisViewManager::slotSaveShowRulersState(bool value)
{
    KisConfig cfg(false);
    cfg.setShowRulers(value);
}

void KisViewManager::slotSaveRulersTrackMouseState(bool value)
{
    KisConfig cfg(false);
    cfg.setRulersTrackMouse(value);
}

void KisViewManager::setShowFloatingMessage(bool show)
{
    d->showFloatingMessage = show;
}

void KisViewManager::changeAuthorProfile(const QString &profileName)
{
    KConfigGroup appAuthorGroup(KSharedConfig::openConfig(), "Author");
    if (profileName.isEmpty() || profileName == i18nc("choice for author profile", "Anonymous")) {
        appAuthorGroup.writeEntry("active-profile", "");
    } else {
        appAuthorGroup.writeEntry("active-profile", profileName);
    }
    appAuthorGroup.sync();
    Q_FOREACH (KisDocument *doc, KisPart::instance()->documents()) {
        doc->documentInfo()->updateParameters();
    }
}

void KisViewManager::slotUpdateAuthorProfileActions()
{
    Q_ASSERT(d->actionAuthor);
    if (!d->actionAuthor) {
        return;
    }
    d->actionAuthor->clear();
    d->actionAuthor->addAction(i18nc("choice for author profile", "Anonymous"));

    KConfigGroup authorGroup(KSharedConfig::openConfig(), "Author");
    QStringList profiles = authorGroup.readEntry("profile-names", QStringList());
    QString authorInfo = KoResourcePaths::getAppDataLocation() + "/authorinfo/";
    QStringList filters = QStringList() << "*.authorinfo";
    QDir dir(authorInfo);
    Q_FOREACH(QString entry, dir.entryList(filters)) {
        int ln = QString(".authorinfo").size();
        entry.chop(ln);
        if (!profiles.contains(entry)) {
            profiles.append(entry);
        }
    }
    Q_FOREACH (const QString &profile , profiles) {
        d->actionAuthor->addAction(profile);
    }

    KConfigGroup appAuthorGroup(KSharedConfig::openConfig(), "Author");
    QString profileName = appAuthorGroup.readEntry("active-profile", "");

    if (profileName == "anonymous" || profileName.isEmpty()) {
        d->actionAuthor->setCurrentItem(0);
    } else if (profiles.contains(profileName)) {
        d->actionAuthor->setCurrentAction(profileName);
    }
}

void KisViewManager::slotUpdatePixelGridAction()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(d->showPixelGrid);

    KisSignalsBlocker b(d->showPixelGrid);

    KisConfig cfg(true);
    d->showPixelGrid->setChecked(cfg.pixelGridEnabled() && cfg.useOpenGL());
}

void KisViewManager::slotActivateTransformTool()
{
    if(KoToolManager::instance()->activeToolId() == "KisToolTransform") {
        KoToolBase* tool = KoToolManager::instance()->toolById(canvasBase(), "KisToolTransform");

        QSet<KoShape*> dummy;
        // Start a new stroke
        tool->deactivate();
        tool->activate(dummy);
    }

    KoToolManager::instance()->switchToolRequested("KisToolTransform");
}

void KisViewManager::slotToggleFgBg()
{

    KoColor newFg = d->canvasResourceManager.backgroundColor();
    KoColor newBg = d->canvasResourceManager.foregroundColor();

    /**
     * NOTE: Some of color selectors do not differentiate foreground
     *       and background colors, so if one wants them to end up
     *       being set up to foreground color, it should be set the
     *       last.
     */
    d->canvasResourceManager.setBackgroundColor(newBg);
    d->canvasResourceManager.setForegroundColor(newFg);
}

void KisViewManager::slotResetFgBg()
{
    // see a comment in slotToggleFgBg()
    d->canvasResourceManager.setBackgroundColor(KoColor(Qt::white, KoColorSpaceRegistry::instance()->rgb8()));
    d->canvasResourceManager.setForegroundColor(KoColor(Qt::black, KoColorSpaceRegistry::instance()->rgb8()));
}

void KisViewManager::slotToggleBrushOutline()
{
    KisConfig cfg(true);

    OutlineStyle style;

    if (cfg.newOutlineStyle() != OUTLINE_NONE) {
        style = OUTLINE_NONE;
        cfg.setLastUsedOutlineStyle(cfg.newOutlineStyle());
    } else {
        style = cfg.lastUsedOutlineStyle();
        cfg.setLastUsedOutlineStyle(OUTLINE_NONE);
    }

    cfg.setNewOutlineStyle(style);

    Q_EMIT brushOutlineToggled();
}

void KisViewManager::slotResetRotation()
{
    KisCanvasController *canvasController = d->currentImageView->canvasController();
    canvasController->resetCanvasRotation();
}

void KisViewManager::slotResetDisplay()
{
    KisCanvasController *canvasController = d->currentImageView->canvasController();
    canvasController->resetCanvasRotation();
    canvasController->mirrorCanvas(false);
    zoomManager()->slotZoomToFit();
}

void KisViewManager::slotToggleFullscreen()
{
    KisConfig cfg(false);
    KisMainWindow *main = mainWindow();
    main->viewFullscreen(!main->isFullScreen());
    cfg.fullscreenMode(main->isFullScreen());
}

void KisViewManager::slotCreateOpacityResource(bool isOpacityPresetMode, KoToolBase *tool)
{
    if (isOpacityPresetMode) {
        KoToolManager::instance()->setConverter(toQShared(new KisOpacityToPresetOpacityResourceConverter), tool);
    }
    else {
        KoToolManager::instance()->setAbstractResource(toQShared(new ToolOpacityAbstractResource(KoCanvasResource::Opacity, 1.0)), tool);
    }
}
