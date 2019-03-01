/*
 *  This file is part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *                1999 Michael Koch    <koch@kde.org>
 *                1999 Carsten Pfeiffer <pfeiffer@kde.org>
 *                2002 Patrick Julien <freak@codepimps.org>
 *                2003-2011 Boudewijn Rempt <boud@valdyas.org>
 *                2004 Clarence Dang <dang@kde.org>
 *                2011 José Luis Vergara <pentalis@gmail.com>
 *                2017 L. E. Segovia <leo.segovia@siggraph.org>
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

#include <stdio.h>

#include "KisViewManager.h"
#include <QPrinter>

#include <QAction>
#include <QApplication>
#include <QBuffer>
#include <QByteArray>
#include <QStandardPaths>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QGridLayout>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QObject>
#include <QPoint>
#include <QPrintDialog>
#include <QRect>
#include <QScrollBar>
#include <QStatusBar>
#include <QToolBar>
#include <QUrl>
#include <QWidget>

#include <kactioncollection.h>
#include <klocalizedstring.h>
#include <KoResourcePaths.h>
#include <kselectaction.h>

#include <KoCanvasController.h>
#include <KoCompositeOp.h>
#include <KoDockRegistry.h>
#include <KoProperties.h>
#include <KoResourceItemChooserSync.h>
#include <KoSelection.h>
#include <KoStore.h>
#include <KoToolManager.h>
#include <KoToolRegistry.h>
#include <KoViewConverter.h>
#include <KoZoomHandler.h>
#include <KoPluginLoader.h>
#include <KoDocumentInfo.h>
#include <KoColorSpaceRegistry.h>

#include "input/kis_input_manager.h"
#include "canvas/kis_canvas2.h"
#include "canvas/kis_canvas_controller.h"
#include "canvas/kis_grid_manager.h"
#include "dialogs/kis_dlg_blacklist_cleanup.h"
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
#include "kis_coordinates_converter.h"
#include "KisDocument.h"
#include "kis_favorite_resource_manager.h"
#include "kis_filter_manager.h"
#include "kis_group_layer.h"
#include <kis_image.h>
#include <kis_image_barrier_locker.h>
#include "kis_image_manager.h"
#include <kis_layer.h>
#include "kis_mainwindow_observer.h"
#include "kis_mask_manager.h"
#include "kis_mimedata.h"
#include "kis_mirror_manager.h"
#include "kis_node_commands_adapter.h"
#include "kis_node.h"
#include "kis_node_manager.h"
#include "KisDecorationsManager.h"
#include <kis_paint_layer.h>
#include "kis_paintop_box.h"
#include <brushengine/kis_paintop_preset.h>
#include "KisPart.h"
#include "KisPrintJob.h"
#include <KoUpdater.h>
#include "KisResourceServerProvider.h"
#include "kis_selection.h"
#include "kis_selection_mask.h"
#include "kis_selection_manager.h"
#include "kis_shape_controller.h"
#include "kis_shape_layer.h"
#include <kis_signal_compressor.h>
#include "kis_statusbar.h"
#include <KisTemplateCreateDia.h>
#include <kis_tool_freehand.h>
#include "kis_tooltip_manager.h"
#include <kis_undo_adapter.h>
#include "KisView.h"
#include "kis_zoom_manager.h"
#include "widgets/kis_floating_message.h"
#include "kis_signal_auto_connection.h"
#include "kis_icon_utils.h"
#include "kis_guides_manager.h"
#include "kis_derived_resources.h"
#include "dialogs/kis_delayed_save_dialog.h"
#include <KisMainWindow.h>
#include "kis_signals_blocker.h"


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

    KisViewManagerPrivate(KisViewManager *_q, KActionCollection *_actionCollection, QWidget *_q_parent)
        : filterManager(_q)
        , createTemplate(0)
        , saveIncremental(0)
        , saveIncrementalBackup(0)
        , openResourcesDirectory(0)
        , rotateCanvasRight(0)
        , rotateCanvasLeft(0)
        , resetCanvasRotation(0)
        , wrapAroundAction(0)
        , levelOfDetailAction(0)
        , showRulersAction(0)
        , rulersTrackMouseAction(0)
        , zoomTo100pct(0)
        , zoomIn(0)
        , zoomOut(0)
        , selectionManager(_q)
        , statusBar(_q)
        , controlFrame(_q, _q_parent)
        , nodeManager(_q)
        , imageManager(_q)
        , gridManager(_q)
        , canvasControlsManager(_q)
        , paintingAssistantsManager(_q)
        , actionManager(_q, _actionCollection)
        , mainWindow(0)
        , showFloatingMessage(true)
        , currentImageView(0)
        , canvasResourceProvider(_q)
        , canvasResourceManager()
        , guiUpdateCompressor(30, KisSignalCompressor::POSTPONE, _q)
        , actionCollection(_actionCollection)
        , mirrorManager(_q)
        , inputManager(_q)
        , actionAuthor(0)
        , showPixelGrid(0)
    {
        KisViewManager::initializeResourceManager(&canvasResourceManager);
    }

public:
    KisFilterManager filterManager;
    KisAction *createTemplate;
    KisAction *createCopy;
    KisAction *saveIncremental;
    KisAction *saveIncrementalBackup;
    KisAction *openResourcesDirectory;
    KisAction *rotateCanvasRight;
    KisAction *rotateCanvasLeft;
    KisAction *resetCanvasRotation;
    KisAction *wrapAroundAction;
    KisAction *levelOfDetailAction;
    KisAction *showRulersAction;
    KisAction *rulersTrackMouseAction;
    KisAction *zoomTo100pct;
    KisAction *zoomIn;
    KisAction *zoomOut;
    KisAction *softProof;
    KisAction *gamutCheck;
    KisAction *toggleFgBg;
    KisAction *resetFgBg;

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
    QMainWindow* mainWindow;
    QPointer<KisFloatingMessage> savedFloatingMessage;
    bool showFloatingMessage;
    QPointer<KisView> currentImageView;
    KisCanvasResourceProvider canvasResourceProvider;
    KoCanvasResourceProvider canvasResourceManager;
    KisSignalCompressor guiUpdateCompressor;
    KActionCollection *actionCollection;
    KisMirrorManager mirrorManager;
    KisInputManager inputManager;

    KisSignalAutoConnectionsStore viewConnections;
    KSelectAction *actionAuthor; // Select action for author profile.
    KisAction *showPixelGrid;

    QByteArray canvasState;
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    QFlags<Qt::WindowState> windowFlags;
#endif

    bool blockUntilOperationsFinishedImpl(KisImageSP image, bool force);
};

KisViewManager::KisViewManager(QWidget *parent, KActionCollection *_actionCollection)
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

    d->controlFrame.setup(parent);


    //Check to draw scrollbars after "Canvas only mode" toggle is created.
    this->showHideScrollbars();

    QScopedPointer<KoDummyCanvasController> dummy(new KoDummyCanvasController(actionCollection()));
    KoToolManager::instance()->registerToolActions(actionCollection(), dummy.data());

    QTimer::singleShot(0, this, SLOT(initializeStatusBarVisibility()));

    connect(KoToolManager::instance(), SIGNAL(inputDeviceChanged(KoInputDevice)),
            d->controlFrame.paintopBox(), SLOT(slotInputDeviceChanged(KoInputDevice)));

    connect(KoToolManager::instance(), SIGNAL(changedTool(KoCanvasController*,int)),
            d->controlFrame.paintopBox(), SLOT(slotToolChanged(KoCanvasController*,int)));

    connect(&d->nodeManager, SIGNAL(sigNodeActivated(KisNodeSP)),
            canvasResourceProvider(), SLOT(slotNodeActivated(KisNodeSP)));

    connect(KisPart::instance(), SIGNAL(sigViewAdded(KisView*)), SLOT(slotViewAdded(KisView*)));
    connect(KisPart::instance(), SIGNAL(sigViewRemoved(KisView*)), SLOT(slotViewRemoved(KisView*)));

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotUpdateAuthorProfileActions()));
    connect(KisConfigNotifier::instance(), SIGNAL(pixelGridModeChanged()), SLOT(slotUpdatePixelGridAction()));

    KisInputProfileManager::instance()->loadProfiles();

    KisConfig cfg(true);
    d->showFloatingMessage = cfg.showCanvasMessages();
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KoColor foreground(Qt::black, cs);
    d->canvasResourceProvider.setFGColor(cfg.readKoColor("LastForeGroundColor",foreground));
    KoColor background(Qt::white, cs);
    d->canvasResourceProvider.setBGColor(cfg.readKoColor("LastBackGroundColor",background));
}


KisViewManager::~KisViewManager()
{
    KisConfig cfg(false);
    if (canvasResourceProvider() && canvasResourceProvider()->currentPreset()) {
        cfg.writeKoColor("LastForeGroundColor",canvasResourceProvider()->fgColor());
        cfg.writeKoColor("LastBackGroundColor",canvasResourceProvider()->bgColor());
    }

    cfg.writeEntry("baseLength", KoResourceItemChooserSync::instance()->baseLength());
    cfg.writeEntry("CanvasOnlyActive", false); // We never restart in CavnasOnlyMode
    delete d;
}

void KisViewManager::initializeResourceManager(KoCanvasResourceProvider *resourceManager)
{
    resourceManager->addDerivedResourceConverter(toQShared(new KisCompositeOpResourceConverter));
    resourceManager->addDerivedResourceConverter(toQShared(new KisEffectiveCompositeOpResourceConverter));
    resourceManager->addDerivedResourceConverter(toQShared(new KisOpacityResourceConverter));
    resourceManager->addDerivedResourceConverter(toQShared(new KisFlowResourceConverter));
    resourceManager->addDerivedResourceConverter(toQShared(new KisSizeResourceConverter));
    resourceManager->addDerivedResourceConverter(toQShared(new KisLodAvailabilityResourceConverter));
    resourceManager->addDerivedResourceConverter(toQShared(new KisLodSizeThresholdResourceConverter));
    resourceManager->addDerivedResourceConverter(toQShared(new KisLodSizeThresholdSupportedResourceConverter));
    resourceManager->addDerivedResourceConverter(toQShared(new KisEraserModeResourceConverter));
    resourceManager->addResourceUpdateMediator(toQShared(new KisPresetUpdateMediator));
}

KActionCollection *KisViewManager::actionCollection() const
{
    return d->actionCollection;
}

void KisViewManager::slotViewAdded(KisView *view)
{
    // WARNING: this slot is called even when a view from another main windows is added!
    //          Don't expect \p view be a child of this view manager!
    Q_UNUSED(view);

    if (viewCount() == 0) {
        d->statusBar.showAllStatusBarItems();
    }
}

void KisViewManager::slotViewRemoved(KisView *view)
{
    // WARNING: this slot is called even when a view from another main windows is removed!
    //          Don't expect \p view be a child of this view manager!
    Q_UNUSED(view);

    if (viewCount() == 0) {
        d->statusBar.hideAllStatusBarItems();
    }

    KisConfig cfg(false);
    if (canvasResourceProvider() && canvasResourceProvider()->currentPreset()) {
        cfg.writeEntry("LastPreset", canvasResourceProvider()->currentPreset()->name());
    }
}

void KisViewManager::setCurrentView(KisView *view)
{
    bool first = true;
    if (d->currentImageView) {
        d->currentImageView->notifyCurrentStateChanged(false);

        d->currentImageView->canvasBase()->setCursor(QCursor(Qt::ArrowCursor));
        first = false;
        KisDocument* doc = d->currentImageView->document();
        if (doc) {
            doc->image()->compositeProgressProxy()->removeProxy(d->persistentImageProgressUpdater);
            doc->disconnect(this);
        }
        d->currentImageView->canvasController()->proxyObject->disconnect(&d->statusBar);
        d->viewConnections.clear();
    }


    QPointer<KisView> imageView = qobject_cast<KisView*>(view);
    d->currentImageView = imageView;

    if (imageView) {
        d->softProof->setChecked(imageView->softProofing());
        d->gamutCheck->setChecked(imageView->gamutCheck());

        // Wait for the async image to have loaded
        KisDocument* doc = view->document();

        if (KisConfig(true).readEntry<bool>("EnablePositionLabel", false)) {
            connect(d->currentImageView->canvasController()->proxyObject,
                    SIGNAL(documentMousePositionChanged(QPointF)),
                    &d->statusBar,
                    SLOT(documentMousePositionChanged(QPointF)));
        }

        // Restore the last used brush preset, color and background color.
        if (first) {
            KisPaintOpPresetResourceServer * rserver = KisResourceServerProvider::instance()->paintOpPresetServer();
            QString defaultPresetName = "basic_tip_default";
            bool foundTip = false;
            for (int i=0; i<rserver->resourceCount(); i++) {
                KisPaintOpPresetSP resource = rserver->resources().at(i);
                if (resource->name().toLower().contains("basic_tip_default")) {
                    defaultPresetName = resource->name();
                    foundTip = true;
                } else if (foundTip == false && (resource->name().toLower().contains("default") ||
                                                 resource->filename().toLower().contains("default"))) {
                    defaultPresetName = resource->name();
                    foundTip = true;
                }
            }
            KisConfig cfg(true);
            QString lastPreset = cfg.readEntry("LastPreset", defaultPresetName);
            KisPaintOpPresetSP preset = rserver->resourceByName(lastPreset);
            if (!preset) {
                preset = rserver->resourceByName(defaultPresetName);
            }

            if (!preset && !rserver->resources().isEmpty()) {
                preset = rserver->resources().first();
            }
            if (preset) {
                paintOpBox()->restoreResource(preset.data());
            }
        }

        KisCanvasController *canvasController = dynamic_cast<KisCanvasController*>(d->currentImageView->canvasController());

        d->viewConnections.addUniqueConnection(&d->nodeManager, SIGNAL(sigNodeActivated(KisNodeSP)), doc->image(), SLOT(requestStrokeEndActiveNode()));
        d->viewConnections.addUniqueConnection(d->rotateCanvasRight, SIGNAL(triggered()), canvasController, SLOT(rotateCanvasRight15()));
        d->viewConnections.addUniqueConnection(d->rotateCanvasLeft, SIGNAL(triggered()),canvasController, SLOT(rotateCanvasLeft15()));
        d->viewConnections.addUniqueConnection(d->resetCanvasRotation, SIGNAL(triggered()),canvasController, SLOT(resetCanvasRotation()));

        d->viewConnections.addUniqueConnection(d->wrapAroundAction, SIGNAL(toggled(bool)), canvasController, SLOT(slotToggleWrapAroundMode(bool)));
        d->wrapAroundAction->setChecked(canvasController->wrapAroundMode());

        d->viewConnections.addUniqueConnection(d->levelOfDetailAction, SIGNAL(toggled(bool)), canvasController, SLOT(slotToggleLevelOfDetailMode(bool)));
        d->levelOfDetailAction->setChecked(canvasController->levelOfDetailMode());

        d->viewConnections.addUniqueConnection(d->currentImageView->image(), SIGNAL(sigColorSpaceChanged(const KoColorSpace*)), d->controlFrame.paintopBox(), SLOT(slotColorSpaceChanged(const KoColorSpace*)));
        d->viewConnections.addUniqueConnection(d->showRulersAction, SIGNAL(toggled(bool)), imageView->zoomManager(), SLOT(setShowRulers(bool)));
        d->viewConnections.addUniqueConnection(d->rulersTrackMouseAction, SIGNAL(toggled(bool)), imageView->zoomManager(), SLOT(setRulersTrackMouse(bool)));
        d->viewConnections.addUniqueConnection(d->zoomTo100pct, SIGNAL(triggered()), imageView->zoomManager(), SLOT(zoomTo100()));
        d->viewConnections.addUniqueConnection(d->zoomIn, SIGNAL(triggered()), imageView->zoomController()->zoomAction(), SLOT(zoomIn()));
        d->viewConnections.addUniqueConnection(d->zoomOut, SIGNAL(triggered()), imageView->zoomController()->zoomAction(), SLOT(zoomOut()));

        d->viewConnections.addUniqueConnection(d->softProof, SIGNAL(toggled(bool)), view, SLOT(slotSoftProofing(bool)) );
        d->viewConnections.addUniqueConnection(d->gamutCheck, SIGNAL(toggled(bool)), view, SLOT(slotGamutCheck(bool)) );

        // set up progrress reporting
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
                    d->currentImageView->zoomManager()->zoomController(),
                    SIGNAL(zoomChanged(KoZoomMode::Mode,qreal)),
                    canvasResourceProvider(), SLOT(slotOnScreenResolutionChanged()));

    }

    d->actionManager.updateGUI();

    canvasResourceProvider()->slotImageSizeChanged();
    canvasResourceProvider()->slotOnScreenResolutionChanged();

    Q_EMIT viewChanged();
}


KoZoomController *KisViewManager::zoomController() const
{
    if (d->currentImageView) {
        return d->currentImageView->zoomController();
    }
    return 0;
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

    a = actionManager()->createAction("edit_blacklist_cleanup");
    connect(a, SIGNAL(triggered()), this, SLOT(slotBlacklistCleanup()));

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

    d->actionAuthor  = new KSelectAction(KisIconUtils::loadIcon("im-user"), i18n("Active Author Profile"), this);
    connect(d->actionAuthor, SIGNAL(triggered(QString)), this, SLOT(changeAuthorProfile(QString)));
    actionCollection()->addAction("settings_active_author", d->actionAuthor);
    slotUpdateAuthorProfileActions();

    d->showPixelGrid = actionManager()->createAction("view_pixel_grid");
    slotUpdatePixelGridAction();

    d->toggleFgBg = actionManager()->createAction("toggle_fg_bg");
    connect(d->toggleFgBg, SIGNAL(triggered(bool)), this, SLOT(slotToggleFgBg()));

    d->resetFgBg =  actionManager()->createAction("reset_fg_bg");
    connect(d->resetFgBg, SIGNAL(triggered(bool)), this, SLOT(slotResetFgBg()));

}

void KisViewManager::setupManagers()
{
    // Create the managers for filters, selections, layers etc.
    // XXX: When the currentlayer changes, call updateGUI on all
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

void KisViewManager::slotBlacklistCleanup()
{
    KisDlgBlacklistCleanup dialog;
    dialog.exec();
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
        KisImageBarrierLocker l(srcDoc->image());
        doc = srcDoc->clone();
    }
    KIS_SAFE_ASSERT_RECOVER_RETURN(doc);

    QString name = srcDoc->documentInfo()->aboutInfo("name");
    if (name.isEmpty()) {
        name = document()->url().toLocalFile();
    }
    name = i18n("%1 (Copy)", name);
    doc->documentInfo()->setAboutInfo("title", name);

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

void KisViewManager::slotSaveIncremental()
{
    if (!document()) return;

    if (document()->url().isEmpty()) {
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
    QString fileName = document()->localFilePath();

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
        if (version.contains(QRegExp("[a-z]"))) {
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
        fileName.replace(regex2, extensionPlusVersion);
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
        fileName.replace(regex, newVersion);
        fileAlreadyExists = QFile(fileName).exists();
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
    document()->setFileBatchMode(true);
    document()->saveAs(QUrl::fromUserInput(fileName), document()->mimeType(), true);
    document()->setFileBatchMode(false);

    if (mainWindow()) {
        mainWindow()->updateCaption();
    }

}

void KisViewManager::slotSaveIncrementalBackup()
{
    if (!document()) return;

    if (document()->url().isEmpty()) {
        KisMainWindow *mw = qobject_cast<KisMainWindow*>(d->mainWindow);
        mw->saveDocument(document(), true, false);
        return;
    }

    bool workingOnBackup;
    bool fileAlreadyExists;
    QString version = "000";
    QString newVersion;
    QString letter;
    QString fileName = document()->localFilePath();

    // First, discover if working on a backup file, or a normal file
    QRegExp regex("~\\d{1,4}[.]|~\\d{1,4}[a-z][.]");
    regex.indexIn(fileName);     //  Perform the search
    QStringList matches = regex.capturedTexts();
    workingOnBackup = matches.at(0).isEmpty() ? false : true;

    if (workingOnBackup) {
        // Try to save incremental version (of backup), use letter for alt versions
        version = matches.at(matches.count() - 1);     //  Look at the last index, we don't care about other matches
        if (version.contains(QRegExp("[a-z]"))) {
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
            backupFileName.replace(regex, newVersion);
            fileAlreadyExists = QFile(backupFileName).exists();
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
        QFile::copy(fileName, backupFileName);
        document()->saveAs(QUrl::fromUserInput(fileName), document()->mimeType(), true);

        if (mainWindow()) mainWindow()->updateCaption();
    }
    else { // if NOT working on a backup...
        // Navigate directory searching for latest backup version, ignore letters
        const quint8 HARDCODED_DIGIT_COUNT = 3;
        QString baseNewVersion = "000";
        QString backupFileName = document()->localFilePath();
        QRegExp regex2("[.][a-z]{2,4}$");  //  Heuristic to find file extension
        regex2.indexIn(backupFileName);
        QStringList matches2 = regex2.capturedTexts();
        QString extensionPlusVersion = matches2.at(0);
        extensionPlusVersion.prepend(baseNewVersion);
        extensionPlusVersion.prepend("~");
        backupFileName.replace(regex2, extensionPlusVersion);

        // Save version with 1 number higher than the highest version found ignoring letters
        do {
            newVersion = baseNewVersion;
            newVersion.prepend("~");
            newVersion.append(".");
            backupFileName.replace(regex, newVersion);
            fileAlreadyExists = QFile(backupFileName).exists();
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
        QFile::copy(fileName, backupFileName);
        document()->saveAs(QUrl::fromUserInput(fileName), document()->mimeType(), true);
        document()->setFileBatchMode(false);

        if (mainWindow()) mainWindow()->updateCaption();
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

void KisViewManager::switchCanvasOnly(bool toggled)
{
    KisConfig cfg(false);
    KisMainWindow *main = mainWindow();

    if(!main) {
        dbgUI << "Unable to switch to canvas-only mode, main window not found";
        return;
    }

    cfg.writeEntry("CanvasOnlyActive", toggled);

    if (toggled) {
        d->canvasState = qtMainWindow()->saveState();
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
        d->windowFlags = main->windowState();
#endif
    }

    if (cfg.hideStatusbarFullscreen()) {
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

    if (cfg.hideDockersFullscreen()) {
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
    if (cfg.hideTitlebarFullscreen() && !cfg.fullscreenMode()) {
        if(toggled) {
            main->setWindowState( main->windowState() | Qt::WindowFullScreen);
        } else {
            main->setWindowState( main->windowState() & ~Qt::WindowFullScreen);
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
            // If window was maximized prior to fullscreen, restore that
            if (d->windowFlags & Qt::WindowMaximized) {
                main->setWindowState( main->windowState() | Qt::WindowMaximized);
            }
#endif
        }
    }

    if (cfg.hideMenuFullscreen()) {
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

    if (cfg.hideToolbarFullscreen()) {
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
        // show a fading heads-up display about the shortcut to go back

        showFloatingMessage(i18n("Going into Canvas-Only mode.\nPress %1 to go back.",
                                 actionCollection()->action("view_show_canvas_only")->shortcut().toString()), QIcon());
    }
    else {
        main->restoreState(d->canvasState);
    }

}

void KisViewManager::toggleTabletLogger()
{
    d->inputManager.toggleTabletLogger();
}

void KisViewManager::openResourcesDirectory()
{
    QString dir = KoResourcePaths::locateLocal("data", "");
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

void KisViewManager::updateIcons()
{
    if (mainWindow()) {
        QList<QDockWidget*> dockers = mainWindow()->dockWidgets();
        Q_FOREACH (QDockWidget* dock, dockers) {
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
void KisViewManager::initializeStatusBarVisibility()
{
    KisConfig cfg(true);
    d->mainWindow->statusBar()->setVisible(cfg.showStatusBar());
}

void KisViewManager::guiUpdateTimeout()
{
    d->nodeManager.updateGUI();
    d->selectionManager.updateGUI();
    d->filterManager.updateGUI();
    if (zoomManager()) {
        zoomManager()->updateGUI();
    }
    d->gridManager.updateGUI();
    d->actionManager.updateGUI();
}

void KisViewManager::showFloatingMessage(const QString &message, const QIcon& icon, int timeout, KisFloatingMessage::Priority priority, int alignment)
{
    if (!d->currentImageView) return;
    d->currentImageView->showFloatingMessage(message, icon, timeout, priority, alignment);

    emit floatingMessageRequested(message, icon.name());
}

KisMainWindow *KisViewManager::mainWindow() const
{
    return qobject_cast<KisMainWindow*>(d->mainWindow);
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
    QString authorInfo = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/authorinfo/";
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
        tool->activate(KoToolBase::DefaultActivation, dummy);
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


