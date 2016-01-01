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
#include <QDesktopServices>
#include <QDesktopWidget>
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
#include <KoDockWidgetTitleBar.h>
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
#include <KoGlobal.h>

#include "input/kis_input_manager.h"
#include "canvas/kis_canvas2.h"
#include "canvas/kis_canvas_controller.h"
#include "canvas/kis_grid_manager.h"
#include "canvas/kis_perspective_grid_manager.h"
#include "dialogs/kis_dlg_blacklist_cleanup.h"
#include "input/kis_input_profile_manager.h"
#include "kis_action_manager.h"
#include "kis_action.h"
#include "kis_canvas_controls_manager.h"
#include "kis_canvas_resource_provider.h"
#include "kis_composite_progress_proxy.h"
#include "kis_config.h"
#include "kis_config_notifier.h"
#include "kis_control_frame.h"
#include "kis_coordinates_converter.h"
#include <KisDocumentEntry.h>
#include "KisDocument.h"
#include "kis_favorite_resource_manager.h"
#include "kis_filter_manager.h"
#include "kis_group_layer.h"
#include <kis_image.h>
#include "kis_image_manager.h"
#include <kis_layer.h>
#include "kis_mainwindow_observer.h"
#include "kis_mask_manager.h"
#include "kis_mimedata.h"
#include "kis_mirror_manager.h"
#include "kis_node_commands_adapter.h"
#include "kis_node.h"
#include "kis_node_manager.h"
#include "kis_painting_assistants_manager.h"
#include <kis_paint_layer.h>
#include "kis_paintop_box.h"
#include <kis_paintop_preset.h>
#include "KisPart.h"
#include "KisPrintJob.h"
#include "kis_progress_widget.h"
#include "kis_resource_server_provider.h"
#include "kis_selection.h"
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
#include "kra/kis_kra_loader.h"
#include "widgets/kis_floating_message.h"
#include "kis_signal_auto_connection.h"
#include "kis_icon_utils.h"


class StatusBarItem
{
public:
    StatusBarItem() // for QValueList
        : m_widget(0),
          m_connected(false),
          m_hidden(false) {}

    StatusBarItem(QWidget * widget, int stretch, bool permanent)
        : m_widget(widget),
          m_stretch(stretch),
          m_permanent(permanent),
          m_connected(false),
          m_hidden(false) {}

    bool operator==(const StatusBarItem& rhs) {
        return m_widget == rhs.m_widget;
    }

    bool operator!=(const StatusBarItem& rhs) {
        return m_widget != rhs.m_widget;
    }

    QWidget * widget() const {
        return m_widget;
    }

    void ensureItemShown(QStatusBar * sb) {
        Q_ASSERT(m_widget);
        if (!m_connected) {
            if (m_permanent)
                sb->addPermanentWidget(m_widget, m_stretch);
            else
                sb->addWidget(m_widget, m_stretch);

            if(!m_hidden)
                m_widget->show();

            m_connected = true;
        }
    }
    void ensureItemHidden(QStatusBar * sb) {
        if (m_connected && m_widget) {
            m_hidden = m_widget->isHidden();
            sb->removeWidget(m_widget);
            m_widget->hide();
            m_connected = false;
        }
    }
private:
    QPointer<QWidget> m_widget;
    int m_stretch;
    bool m_permanent;
    bool m_connected;
    bool m_hidden;
};

class BlockingUserInputEventFilter : public QObject
{
    bool eventFilter(QObject *watched, QEvent *event)
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

    KisViewManagerPrivate(KisViewManager *_q, QWidget *_q_parent)
        : filterManager(_q)
        , createTemplate(0)
        , saveIncremental(0)
        , saveIncrementalBackup(0)
        , openResourcesDirectory(0)
        , rotateCanvasRight(0)
        , rotateCanvasLeft(0)
        , wrapAroundAction(0)
        , levelOfDetailAction(0)
        , showRulersAction(0)
        , zoomTo100pct(0)
        , zoomIn(0)
        , zoomOut(0)
        , showGuidesAction(0)
        , selectionManager(_q)
        , statusBar(_q)
        , controlFrame(_q, _q_parent)
        , nodeManager(_q)
        , imageManager(_q)
        , gridManager(_q)
        , canvasControlsManager(_q)
        , perspectiveGridManager(_q)
        , paintingAssistantsManager(_q)
        , actionManager(_q)
        , mainWindow(0)
        , showFloatingMessage(true)
        , currentImageView(0)
        , canvasResourceProvider(_q)
        , canvasResourceManager()
        , guiUpdateCompressor(30, KisSignalCompressor::POSTPONE, _q)
        , mirrorManager(_q)
        , inputManager(_q)
        , actionAuthor(0)
    {
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
    KisAction *wrapAroundAction;
    KisAction *levelOfDetailAction;
    KisAction *showRulersAction;
    KisAction *zoomTo100pct;
    KisAction *zoomIn;
    KisAction *zoomOut;
    KisAction *showGuidesAction;

    KisSelectionManager selectionManager;
    KisStatusBar statusBar;

    KisControlFrame controlFrame;
    KisNodeManager nodeManager;
    KisImageManager imageManager;
    KisGridManager gridManager;
    KisCanvasControlsManager canvasControlsManager;
    KisPerspectiveGridManager  perspectiveGridManager;
    KisPaintingAssistantsManager paintingAssistantsManager;
    BlockingUserInputEventFilter blockingEventFilter;
    KisActionManager actionManager;
    QMainWindow* mainWindow;
    QPointer<KisFloatingMessage> savedFloatingMessage;
    bool showFloatingMessage;
    QPointer<KisView> currentImageView;
    KisCanvasResourceProvider canvasResourceProvider;
    KoCanvasResourceManager canvasResourceManager;
    QVector<StatusBarItem> statusBarItems;
    KisSignalCompressor guiUpdateCompressor;
    KActionCollection *actionCollection;
    KisMirrorManager mirrorManager;
    KisInputManager inputManager;

    KisSignalAutoConnectionsStore viewConnections;
    KSelectAction *actionAuthor; // Select action for author profile.

    QByteArray canvasState;
};


KisViewManager::KisViewManager(QWidget *parent, KActionCollection *_actionCollection)
    : d(new KisViewManagerPrivate(this, parent))
{
    d->actionCollection = _actionCollection;
    d->mainWindow = dynamic_cast<QMainWindow*>(parent);
    d->canvasResourceProvider.setResourceManager(&d->canvasResourceManager);
    connect(&d->guiUpdateCompressor, SIGNAL(timeout()), this, SLOT(guiUpdateTimeout()));

    createActions();
    setupManagers();

    // These initialization functions must wait until KisViewManager ctor is complete.
    d->statusBar.setup();
    d->controlFrame.setup(parent);

    //Check to draw scrollbars after "Canvas only mode" toggle is created.
    this->showHideScrollbars();

    KoCanvasController *dummy = new KoDummyCanvasController(actionCollection());
    KoToolManager::instance()->registerToolActions(actionCollection(), dummy);

    QTimer::singleShot(0, this, SLOT(makeStatusBarVisible()));

    connect(KoToolManager::instance(), SIGNAL(inputDeviceChanged(KoInputDevice)),
            d->controlFrame.paintopBox(), SLOT(slotInputDeviceChanged(KoInputDevice)));

    connect(KoToolManager::instance(), SIGNAL(changedTool(KoCanvasController*,int)),
            d->controlFrame.paintopBox(), SLOT(slotToolChanged(KoCanvasController*,int)));

    connect(&d->nodeManager, SIGNAL(sigNodeActivated(KisNodeSP)),
            resourceProvider(), SLOT(slotNodeActivated(KisNodeSP)));

    connect(resourceProvider()->resourceManager(), SIGNAL(canvasResourceChanged(int,QVariant)),
            d->controlFrame.paintopBox(), SLOT(slotCanvasResourceChanged(int,QVariant)));

    connect(KisPart::instance(), SIGNAL(sigViewAdded(KisView*)), SLOT(slotViewAdded(KisView*)));
    connect(KisPart::instance(), SIGNAL(sigViewRemoved(KisView*)), SLOT(slotViewRemoved(KisView*)));

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotUpdateAuthorProfileActions()));

    KisInputProfileManager::instance()->loadProfiles();

    KisConfig cfg;
    d->showFloatingMessage = cfg.showCanvasMessages();

}


KisViewManager::~KisViewManager()
{
    KisConfig cfg;
    if (resourceProvider() && resourceProvider()->currentPreset()) {
        cfg.writeEntry("LastPreset", resourceProvider()->currentPreset()->name());
    }
    cfg.writeEntry("baseLength", KoResourceItemChooserSync::instance()->baseLength());

    delete d;
}

KActionCollection *KisViewManager::actionCollection() const
{
    return d->actionCollection;
}

void KisViewManager::slotViewAdded(KisView *view)
{
    d->inputManager.addTrackedCanvas(view->canvasBase());
}

void KisViewManager::slotViewRemoved(KisView *view)
{
    d->inputManager.removeTrackedCanvas(view->canvasBase());
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
            doc->disconnect(this);
        }
        d->currentImageView->canvasController()->proxyObject->disconnect(&d->statusBar);
        d->viewConnections.clear();
    }

    QPointer<KisView>imageView = qobject_cast<KisView*>(view);

    if (imageView) {
        // Wait for the async image to have loaded
        KisDocument* doc = view->document();
        //        connect(canvasController()->proxyObject, SIGNAL(documentMousePositionChanged(QPointF)), d->statusBar, SLOT(documentMousePositionChanged(QPointF)));

        d->currentImageView = imageView;
        KisCanvasController *canvasController = dynamic_cast<KisCanvasController*>(d->currentImageView->canvasController());

        d->viewConnections.addUniqueConnection(&d->nodeManager, SIGNAL(sigNodeActivated(KisNodeSP)), doc->image(), SLOT(requestStrokeEndActiveNode()));
        d->viewConnections.addUniqueConnection(d->rotateCanvasRight, SIGNAL(triggered()), canvasController, SLOT(rotateCanvasRight15()));
        d->viewConnections.addUniqueConnection(d->rotateCanvasLeft, SIGNAL(triggered()),canvasController, SLOT(rotateCanvasLeft15()));

        d->viewConnections.addUniqueConnection(d->wrapAroundAction, SIGNAL(toggled(bool)), canvasController, SLOT(slotToggleWrapAroundMode(bool)));
        d->wrapAroundAction->setChecked(canvasController->wrapAroundMode());

        d->viewConnections.addUniqueConnection(d->levelOfDetailAction, SIGNAL(toggled(bool)), canvasController, SLOT(slotToggleLevelOfDetailMode(bool)));
        d->levelOfDetailAction->setChecked(canvasController->levelOfDetailMode());

        d->viewConnections.addUniqueConnection(d->currentImageView->canvasController(), SIGNAL(toolOptionWidgetsChanged(QList<QPointer<QWidget> >)), mainWindow(), SLOT(newOptionWidgets(QList<QPointer<QWidget> >)));
        d->viewConnections.addUniqueConnection(d->currentImageView->image(), SIGNAL(sigColorSpaceChanged(const KoColorSpace*)), d->controlFrame.paintopBox(), SLOT(slotColorSpaceChanged(const KoColorSpace*)));
        d->viewConnections.addUniqueConnection(d->showRulersAction, SIGNAL(toggled(bool)), imageView->zoomManager(), SLOT(toggleShowRulers(bool)));
        d->showRulersAction->setChecked(imageView->zoomManager()->horizontalRulerVisible() && imageView->zoomManager()->verticalRulerVisible());
        d->viewConnections.addUniqueConnection(d->zoomTo100pct, SIGNAL(triggered()), imageView->zoomManager(), SLOT(zoomTo100()));
        d->viewConnections.addUniqueConnection(d->zoomIn, SIGNAL(triggered()), imageView->zoomController()->zoomAction(), SLOT(zoomIn()));
        d->viewConnections.addUniqueConnection(d->zoomOut, SIGNAL(triggered()), imageView->zoomController()->zoomAction(), SLOT(zoomOut()));
        d->viewConnections.addUniqueConnection(d->showGuidesAction, SIGNAL(triggered(bool)), imageView->zoomManager(), SLOT(showGuides(bool)));

        showHideScrollbars();
    }

    d->filterManager.setView(imageView);
    d->selectionManager.setView(imageView);
    d->nodeManager.setView(imageView);
    d->imageManager.setView(imageView);
    d->canvasControlsManager.setView(imageView);
    d->actionManager.setView(imageView);
    d->gridManager.setView(imageView);
    d->statusBar.setView(imageView);
    d->paintingAssistantsManager.setView(imageView);
    d->perspectiveGridManager.setView(imageView);
    d->mirrorManager.setView(imageView);

    if (d->currentImageView) {
        d->currentImageView->notifyCurrentStateChanged(true);
        d->currentImageView->canvasController()->activate();
        d->currentImageView->canvasController()->setFocus();
    }

    d->actionManager.updateGUI();

    d->viewConnections.addUniqueConnection(
        image(), SIGNAL(sigSizeChanged(const QPointF&, const QPointF&)),
        resourceProvider(), SLOT(slotImageSizeChanged()));

    d->viewConnections.addUniqueConnection(
        image(), SIGNAL(sigResolutionChanged(double,double)),
        resourceProvider(), SLOT(slotOnScreenResolutionChanged()));

    d->viewConnections.addUniqueConnection(
        image(), SIGNAL(sigNodeChanged(KisNodeSP)),
        this, SLOT(updateGUI()));

    d->viewConnections.addUniqueConnection(
        view->zoomManager()->zoomController(),
        SIGNAL(zoomChanged(KoZoomMode::Mode,qreal)),
        resourceProvider(), SLOT(slotOnScreenResolutionChanged()));

    resourceProvider()->slotImageSizeChanged();
    resourceProvider()->slotOnScreenResolutionChanged();

    // Restore the last used brush preset
    if (first) {
        KisConfig cfg;
        KisPaintOpPresetResourceServer * rserver = KisResourceServerProvider::instance()->paintOpPresetServer();
        QString lastPreset = cfg.readEntry("LastPreset", QString("Basic_tip_default"));
        KisPaintOpPresetSP preset = rserver->resourceByName(lastPreset);
        if (!preset) {
            preset = rserver->resourceByName("Basic_tip_default");
        }

        if (!preset) {
            preset = rserver->resources().first();
        }
        if (preset) {
            paintOpBox()->restoreResource(preset.data());
        }

    }
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

KisCanvasResourceProvider * KisViewManager::resourceProvider()
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

void KisViewManager::addStatusBarItem(QWidget * widget, int stretch, bool permanent)
{
    if (!mainWindow()) return;

    StatusBarItem item(widget, stretch, permanent);
    QStatusBar * sb = mainWindow()->statusBar();
    if (sb) {
        item.ensureItemShown(sb);
    }
    d->statusBarItems.append(item);
}

void KisViewManager::removeStatusBarItem(QWidget * widget)
{
    QStatusBar *sb = mainWindow()->statusBar();

    int i = 0;
    Q_FOREACH(const StatusBarItem& sbItem, d->statusBarItems) {
        if (sbItem.widget() == widget) {
            break;
        }
        i++;
    }
    d->statusBarItems[i].ensureItemHidden(sb);
    d->statusBarItems.remove(i);
}

KisPaintopBox* KisViewManager::paintOpBox() const
{
    return d->controlFrame.paintopBox();
}

KoProgressUpdater* KisViewManager::createProgressUpdater(KoProgressUpdater::Mode mode)
{
    return new KisProgressUpdater(d->statusBar.progress(), document()->progressProxy(), mode);
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
        KoProperties properties;
        QList<KisNodeSP> masks = layer->childNodes(QStringList("KisSelectionMask"), properties);
        if (masks.size() == 1) {
            return masks[0]->isEditable();
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
    d->wrapAroundAction    = actionManager()->createAction("wrap_around_mode");
    d->levelOfDetailAction = actionManager()->createAction("level_of_detail_mode");

    KisAction *tAction = actionManager()->createAction("showStatusBar");
    tAction->setChecked(true);
    connect(tAction, SIGNAL(toggled(bool)), this, SLOT(showStatusBar(bool)));

    tAction = actionManager()->createAction("view_show_just_the_canvas");
    tAction->setChecked(false);
    connect(tAction, SIGNAL(toggled(bool)), this, SLOT(showJustTheCanvas(bool)));

    //Workaround, by default has the same shortcut as mirrorCanvas
    KisAction *a = dynamic_cast<KisAction*>(actionCollection()->action("format_italic"));
    if (a) {
        a->setDefaultShortcut(QKeySequence());
    }

    a = actionManager()->createAction("edit_blacklist_cleanup");
    connect(a, SIGNAL(triggered()), this, SLOT(slotBlacklistCleanup()));

    KisConfig cfg;
    d->showRulersAction = actionManager()->createAction("view_ruler");
    d->showRulersAction->setChecked(cfg.showRulers());

    d->showGuidesAction = actionManager()->createAction("view_show_guides");

    d->zoomTo100pct = actionManager()->createAction("zoom_to_100pct");

    d->zoomIn = actionManager()->createStandardAction(KStandardAction::ZoomIn, 0, "");
    d->zoomOut = actionManager()->createStandardAction(KStandardAction::ZoomOut, 0, "");

    d->actionAuthor  = new KSelectAction(KisIconUtils::loadIcon("im-user"), i18n("Active Author Profile"), this);
    connect(d->actionAuthor, SIGNAL(triggered(const QString &)), this, SLOT(changeAuthorProfile(const QString &)));
    actionCollection()->addAction("settings_active_author", d->actionAuthor);
    slotUpdateAuthorProfileActions();

}



void KisViewManager::setupManagers()
{
    // Create the managers for filters, selections, layers etc.
    // XXX: When the currentlayer changes, call updateGUI on all
    // managers

    d->filterManager.setup(actionCollection(), actionManager());

    d->selectionManager.setup(actionManager());

    d->nodeManager.setup(actionCollection(), actionManager());

    d->imageManager.setup(actionManager());

    d->gridManager.setup(actionManager());

    d->perspectiveGridManager.setup(actionCollection());

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

KisPerspectiveGridManager* KisViewManager::perspectiveGridManager() const
{
    return &d->perspectiveGridManager;
}

KisGridManager * KisViewManager::gridManager() const
{
    return &d->gridManager;
}

KisPaintingAssistantsManager* KisViewManager::paintingAssistantsManager() const
{
    return &d->paintingAssistantsManager;
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

void KisViewManager::slotCreateTemplate()
{
    if (!document()) return;
    KisTemplateCreateDia::createTemplate(KisPart::instance()->templatesResourcePath(), ".kra", document(), mainWindow());
}

void KisViewManager::slotCreateCopy()
{
    if (!document()) return;
    KisDocument *doc = KisPart::instance()->createDocument();

    QString name = document()->documentInfo()->aboutInfo("name");
    if (name.isEmpty()) {
        name = document()->url().toLocalFile();
    }
    name = i18n("%1 (Copy)", name);
    doc->documentInfo()->setAboutInfo("title", name);
    KisImageWSP image = document()->image();
    KisImageSP newImage = new KisImage(doc->createUndoStore(), image->width(), image->height(), image->colorSpace(), name);
    newImage->setRootLayer(dynamic_cast<KisGroupLayer*>(image->rootLayer()->clone().data()));
    doc->setCurrentImage(newImage);
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
    document()->setSaveInBatchMode(true);
    document()->saveAs(QUrl::fromUserInput(fileName));
    document()->setSaveInBatchMode(false);

    if (mainWindow()) {
        mainWindow()->updateCaption();
    }

}

void KisViewManager::slotSaveIncrementalBackup()
{
    if (!document()) return;

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
        document()->saveAs(QUrl::fromUserInput(fileName));

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
        document()->setSaveInBatchMode(true);
        QFile::copy(fileName, backupFileName);
        document()->saveAs(QUrl::fromUserInput(fileName));
        document()->setSaveInBatchMode(false);

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
    if (d->currentImageView && d->currentImageView->statusBar()) {
        d->currentImageView->statusBar()->setVisible(toggled);
    }
}

void KisViewManager::showJustTheCanvas(bool toggled)
{
    KisConfig cfg;
    KisMainWindow* main = mainWindow();

    if(!main) {
        dbgUI << "Unable to switch to canvas-only mode, main window not found";
        return;
    }

    if (toggled) {
        d->canvasState = qtMainWindow()->saveState();
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
        action->setCheckable(true);
        if (action && action->isChecked() == toggled) {
            action->setChecked(!toggled);
        }
    }

    if (cfg.hideTitlebarFullscreen() && !cfg.fullscreenMode()) {
        if(toggled) {
            main->setWindowState( main->windowState() | Qt::WindowFullScreen);
        } else {
            main->setWindowState( main->windowState() & ~Qt::WindowFullScreen);
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
                                 actionCollection()->action("view_show_just_the_canvas")->shortcut().toString()), QIcon());
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
    QString dir = KoResourcePaths::locateLocal("data", "krita");
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

void KisViewManager::updateIcons()
{
    if (mainWindow()) {
        QList<QDockWidget*> dockers = mainWindow()->dockWidgets();
        Q_FOREACH (QDockWidget* dock, dockers) {
            dbgKrita << "name " << dock->objectName();
            KoDockWidgetTitleBar* titlebar = dynamic_cast<KoDockWidgetTitleBar*>(dock->titleBarWidget());
            if (titlebar) {
                titlebar->updateIcons();
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
void KisViewManager::makeStatusBarVisible()
{
    d->mainWindow->statusBar()->setVisible(true);
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
    d->perspectiveGridManager.updateGUI();
    d->actionManager.updateGUI();
}

void KisViewManager::showFloatingMessage(const QString &message, const QIcon& icon, int timeout, KisFloatingMessage::Priority priority, int alignment)
{
    if (!d->currentImageView) return;
    d->currentImageView->showFloatingMessageImpl(message, icon, timeout, priority, alignment);

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

    KisConfig cfg;
    bool toggled = actionCollection()->action("view_show_just_the_canvas")->isChecked();

    if ( (toggled && cfg.hideScrollbarsFullscreen()) || (!toggled && cfg.hideScrollbars()) ) {
        d->currentImageView->canvasController()->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        d->currentImageView->canvasController()->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    } else {
        d->currentImageView->canvasController()->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        d->currentImageView->canvasController()->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    }
}

void KisViewManager::setShowFloatingMessage(bool show)
{
    d->showFloatingMessage = show;
}

void KisViewManager::changeAuthorProfile(const QString &profileName)
{
    KConfigGroup appAuthorGroup(KoGlobal::calligraConfig(), "Author");
    if (profileName.isEmpty()) {
        appAuthorGroup.writeEntry("active-profile", "");
    } else if (profileName == i18nc("choice for author profile", "Anonymous")) {
        appAuthorGroup.writeEntry("active-profile", "anonymous");
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
    d->actionAuthor->addAction(i18n("Default Author Profile"));
    d->actionAuthor->addAction(i18nc("choice for author profile", "Anonymous"));

    KConfigGroup authorGroup(KoGlobal::calligraConfig(), "Author");
    QStringList profiles = authorGroup.readEntry("profile-names", QStringList());
    Q_FOREACH (const QString &profile , profiles) {
        d->actionAuthor->addAction(profile);
    }

    KConfigGroup appAuthorGroup(KoGlobal::calligraConfig(), "Author");
    QString profileName = appAuthorGroup.readEntry("active-profile", "");

    if (profileName == "anonymous") {
        d->actionAuthor->setCurrentItem(1);
    } else if (profiles.contains(profileName)) {
        d->actionAuthor->setCurrentAction(profileName);
    } else {
        d->actionAuthor->setCurrentItem(0);
    }
}


