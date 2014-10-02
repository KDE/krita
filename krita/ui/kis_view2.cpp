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

#include "kis_view2.h"
#include <QPrinter>

#include <QDesktopServices>
#include <QDesktopWidget>
#include <QGridLayout>
#include <QRect>
#include <QWidget>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QApplication>
#include <QPrintDialog>
#include <QObject>
#include <QByteArray>
#include <QBuffer>
#include <QScrollBar>
#include <QMainWindow>

#include <kio/netaccess.h>
#include <kmenubar.h>
#include <ktoolbar.h>
#include <kstatusbar.h>
#include <ktoggleaction.h>
#include <kaction.h>
#include <kactionmenu.h>
#include <klocale.h>
#include <kmenu.h>
#include <kservice.h>
#include <KoServiceLocator.h>
#include <kstandardaction.h>
#include <kurl.h>
#include <kxmlguiwindow.h>
#include <kxmlguifactory.h>
#include <kmessagebox.h>
#include <kactioncollection.h>

#include <KoMainWindow.h>
#include <KoSelection.h>
#include <KoToolBoxFactory.h>
#include <KoZoomHandler.h>
#include <KoViewConverter.h>
#include <KoView.h>
#include <KoDockerManager.h>
#include <KoDockRegistry.h>
#include <KoResourceServerProvider.h>
#include <KoResourceItemChooserSync.h>
#include <KoCompositeOp.h>
#include <KoTemplateCreateDia.h>
#include <KoCanvasControllerWidget.h>
#include <KoDocumentEntry.h>
#include <KoProperties.h>
#include <KoPart.h>

#include <kis_image.h>
#include <kis_undo_adapter.h>
#include "kis_composite_progress_proxy.h"
#include <kis_layer.h>

#include "canvas/kis_canvas2.h"
#include "canvas/kis_canvas_controller.h"
#include "canvas/kis_grid_manager.h"
#include "canvas/kis_perspective_grid_manager.h"
#include "dialogs/kis_dlg_preferences.h"
#include "dialogs/kis_dlg_blacklist_cleanup.h"
#include "kis_canvas_resource_provider.h"
#include "kis_config.h"
#include "kis_config_notifier.h"
#include "kis_control_frame.h"
#include "kis_coordinates_converter.h"
#include "kis_doc2.h"
#include "kis_factory2.h"
#include "kis_filter_manager.h"
#include "kis_group_layer.h"
#include "kis_image_manager.h"
#include "kis_mask_manager.h"
#include "kis_mimedata.h"
#include "kis_node.h"
#include "kis_node_manager.h"
#include "kis_painting_assistants_decoration.h"
#include <kis_paint_layer.h>
#include "kis_paintop_box.h"
#include "kis_print_job.h"
#include "kis_progress_widget.h"
#include "kis_resource_server_provider.h"
#include "kis_selection.h"
#include "kis_selection_manager.h"
#include "kis_shape_layer.h"
#include "kis_shape_controller.h"
#include "kis_statusbar.h"
#include "kis_zoom_manager.h"
#include "kra/kis_kra_loader.h"
#include "widgets/kis_floating_message.h"

#include <QPoint>
#include <kapplication.h>
#include "kis_node_commands_adapter.h"
#include <kis_paintop_preset.h>
#include "kis_favorite_resource_manager.h"
#include "kis_action_manager.h"
#include "input/kis_input_profile_manager.h"
#include "kis_canvas_controls_manager.h"

#include "krita/gemini/ViewModeSwitchEvent.h"
#include "kis_mirror_axis.h"
#include "kis_tooltip_manager.h"
#include <kis_tool_freehand.h>

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

class KisView2::KisView2Private
{

public:

    KisView2Private()
        : canvas(0)
        , doc(0)
        , viewConverter(0)
        , canvasController(0)
        , resourceProvider(0)
        , filterManager(0)
        , statusBar(0)
        , selectionManager(0)
        , controlFrame(0)
        , nodeManager(0)
        , zoomManager(0)
        , imageManager(0)
        , gridManager(0)
        , perspectiveGridManager(0)
        , paintingAssistantsDecoration(0)
        , actionManager(0)
        , mainWindow(0)
        , tooltipManager(0)
        , showFloatingMessage(true)
    {
    }

    ~KisView2Private() {
        if (canvasController) {
            KoToolManager::instance()->removeCanvasController(canvasController);
        }

        delete resourceProvider;
        delete canvasController;
        delete canvas;
        delete filterManager;
        delete selectionManager;
        delete nodeManager;
        delete zoomManager;
        delete imageManager;
        delete gridManager;
        delete perspectiveGridManager;
        delete paintingAssistantsDecoration;
        delete viewConverter;
        delete statusBar;
        delete actionManager;
        delete canvasControlsManager;
        delete tooltipManager;

        /**
         * Push a timebomb, which will try to release the memory after
         * the document has been deleted
         */
        KisPaintDevice::createMemoryReleaseObject()->deleteLater();
    }

public:
    KisCanvas2 *canvas;
    KisDoc2 *doc;
    KisCoordinatesConverter *viewConverter;
    KisCanvasController *canvasController;
    KisCanvasResourceProvider *resourceProvider;
    KisFilterManager *filterManager;
    KisStatusBar *statusBar;
    KAction *mirrorCanvas;
    KAction *createTemplate;
    KAction *saveIncremental;
    KAction *saveIncrementalBackup;
    KAction *openResourcesDirectory;
    KisSelectionManager *selectionManager;
    KisControlFrame *controlFrame;
    KisNodeManager *nodeManager;
    KisZoomManager *zoomManager;
    KisImageManager *imageManager;
    KisGridManager *gridManager;
    KisCanvasControlsManager *canvasControlsManager;
    KisPerspectiveGridManager * perspectiveGridManager;
    KisPaintingAssistantsDecoration *paintingAssistantsDecoration;
    BlockingUserInputEventFilter blockingEventFilter;
    KisFlipbook *flipbook;
    KisActionManager* actionManager;
    QMainWindow* mainWindow;
    KisMirrorAxis* mirrorAxis;
    KisTooltipManager* tooltipManager;
    QPointer<KisFloatingMessage> savedFloatingMessage;
    bool showFloatingMessage;
};


KisView2::KisView2(KoPart *part, KisDoc2 * doc, QWidget * parent)
    : KoView(part, doc, parent),
      m_d(new KisView2Private())
{
    Q_ASSERT(doc);
    Q_ASSERT(doc->image());

    setXMLFile(QString("%1.rc").arg(qAppName()));
    KisConfig cfg;

    KoResourceItemChooserSync::instance()->setBaseLength(cfg.readEntry("baseLength", 50));
    setFocusPolicy(Qt::NoFocus);

    if (mainWindow()) {
        mainWindow()->setDockNestingEnabled(true);
        actionCollection()->addAction(KStandardAction::KeyBindings, "keybindings", mainWindow()->guiFactory(), SLOT(configureShortcuts()));
    }

    m_d->doc = doc;
    m_d->viewConverter = new KisCoordinatesConverter();

    KisCanvasController *canvasController = new KisCanvasController(this, actionCollection());
    canvasController->setDrawShadow(false);
    canvasController->setCanvasMode(KoCanvasController::Infinite);
    canvasController->setVastScrolling(cfg.vastScrolling());

    m_d->canvasController = canvasController;

    m_d->resourceProvider = new KisCanvasResourceProvider(this);
    m_d->resourceProvider->resetDisplayProfile(QApplication::desktop()->screenNumber(this));
    m_d->canvas = new KisCanvas2(m_d->viewConverter, this, doc->shapeController());
    connect(m_d->resourceProvider, SIGNAL(sigDisplayProfileChanged(const KoColorProfile*)), m_d->canvas, SLOT(slotSetDisplayProfile(const KoColorProfile*)));

    m_d->canvasController->setCanvas(m_d->canvas);

    m_d->resourceProvider->setResourceManager(m_d->canvas->resourceManager());

    createActions();
    createManagers();

    m_d->controlFrame = new KisControlFrame(this);

    Q_ASSERT(m_d->canvasController);
    KoToolManager::instance()->addController(m_d->canvasController);
    KoToolManager::instance()->registerTools(actionCollection(), m_d->canvasController);

    // krita/krita.rc must also be modified to add actions to the menu entries

    m_d->saveIncremental = new KAction(i18n("Save Incremental &Version"), this);
    m_d->saveIncremental->setShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_S));
    actionCollection()->addAction("save_incremental_version", m_d->saveIncremental);
    connect(m_d->saveIncremental, SIGNAL(triggered()), this, SLOT(slotSaveIncremental()));

    m_d->saveIncrementalBackup = new KAction(i18n("Save Incremental Backup"), this);
    m_d->saveIncrementalBackup->setShortcut(Qt::Key_F4);
    actionCollection()->addAction("save_incremental_backup", m_d->saveIncrementalBackup);
    connect(m_d->saveIncrementalBackup, SIGNAL(triggered()), this, SLOT(slotSaveIncrementalBackup()));

    connect(qtMainWindow(), SIGNAL(documentSaved()), this, SLOT(slotDocumentSaved()));
    connect(this, SIGNAL(sigSavingFinished()), this, SLOT(slotSavingFinished()));

    if (m_d->doc->localFilePath().isNull()) {
        m_d->saveIncremental->setEnabled(false);
        m_d->saveIncrementalBackup->setEnabled(false);
    }

    KAction *tabletDebugger = new KAction(i18n("Toggle Tablet Debugger"), this);
    actionCollection()->addAction("tablet_debugger", tabletDebugger );
    tabletDebugger->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_T));
    connect(tabletDebugger, SIGNAL(triggered()), this, SLOT(toggleTabletLogger()));

    m_d->createTemplate = new KAction( i18n( "&Create Template From Image..." ), this);
    actionCollection()->addAction("createTemplate", m_d->createTemplate);
    connect(m_d->createTemplate, SIGNAL(triggered()), this, SLOT(slotCreateTemplate()));

    m_d->mirrorCanvas = new KToggleAction(i18n("Mirror View"), this);
    m_d->mirrorCanvas->setChecked(false);
    actionCollection()->addAction("mirror_canvas", m_d->mirrorCanvas);
    m_d->mirrorCanvas->setShortcut(QKeySequence(Qt::Key_M));
    connect(m_d->mirrorCanvas, SIGNAL(toggled(bool)),m_d->canvasController, SLOT(mirrorCanvas(bool)));

    m_d->openResourcesDirectory = new KAction(i18n("Open Resources Folder"), this);
    m_d->openResourcesDirectory->setToolTip(i18n("Opens a file browser at the location Krita saves resources such as brushes to."));
    m_d->openResourcesDirectory->setWhatsThis(i18n("Opens a file browser at the location Krita saves resources such as brushes to."));
    actionCollection()->addAction("open_resources_directory", m_d->openResourcesDirectory);
    connect(m_d->openResourcesDirectory, SIGNAL(triggered()), SLOT(openResourcesDirectory()));

    KAction *rotateCanvasRight = new KAction(i18n("Rotate Canvas Right"), this);
    actionCollection()->addAction("rotate_canvas_right", rotateCanvasRight);
    rotateCanvasRight->setShortcut(QKeySequence("Ctrl+]"));
    connect(rotateCanvasRight, SIGNAL(triggered()),m_d->canvasController, SLOT(rotateCanvasRight15()));

    KAction *rotateCanvasLeft = new KAction(i18n("Rotate Canvas Left"), this);
    actionCollection()->addAction("rotate_canvas_left", rotateCanvasLeft);
    rotateCanvasLeft->setShortcut(QKeySequence("Ctrl+["));
    connect(rotateCanvasLeft, SIGNAL(triggered()),m_d->canvasController, SLOT(rotateCanvasLeft15()));

    KAction *resetCanvasRotation = new KAction(i18n("Reset Canvas Rotation"), this);
    actionCollection()->addAction("reset_canvas_rotation", resetCanvasRotation);
    connect(resetCanvasRotation, SIGNAL(triggered()),m_d->canvasController, SLOT(resetCanvasRotation()));

    KToggleAction *wrapAroundAction = new KToggleAction(i18n("Wrap Around Mode"), this);
    actionCollection()->addAction("wrap_around_mode", wrapAroundAction);
    wrapAroundAction->setShortcut(QKeySequence(Qt::Key_W));
    connect(wrapAroundAction, SIGNAL(toggled(bool)), m_d->canvasController, SLOT(slotToggleWrapAroundMode(bool)));

    KToggleAction *tAction = new KToggleAction(i18n("Show Status Bar"), this);
    tAction->setCheckedState(KGuiItem(i18n("Hide Status Bar")));
    tAction->setChecked(true);
    tAction->setToolTip(i18n("Shows or hides the status bar"));
    actionCollection()->addAction("showStatusBar", tAction);
    connect(tAction, SIGNAL(toggled(bool)), this, SLOT(showStatusBar(bool)));

    tAction = new KToggleAction(i18n("Show Canvas Only"), this);
    tAction->setCheckedState(KGuiItem(i18n("Return to Window")));
    tAction->setToolTip(i18n("Shows just the canvas or the whole window"));
    QList<QKeySequence> shortcuts;
    shortcuts << QKeySequence(Qt::Key_Tab);
    tAction->setShortcuts(shortcuts);
    tAction->setChecked(false);
    actionCollection()->addAction("view_show_just_the_canvas", tAction);
    connect(tAction, SIGNAL(toggled(bool)), this, SLOT(showJustTheCanvas(bool)));

    //Check to draw scrollbars after "Canvas only mode" toggle is created.
    this->showHideScrollbars();

    //Workaround, by default has the same shortcut as mirrorCanvas
    KAction* action = dynamic_cast<KAction*>(actionCollection()->action("format_italic"));
    if (action) {
        action->setShortcut(QKeySequence(), KAction::DefaultShortcut);
        action->setShortcut(QKeySequence(), KAction::ActiveShortcut);
    }

    //Workaround, by default has the same shortcut as hide/show dockers
    if (mainWindow()) {
        connect(mainWindow(), SIGNAL(documentSaved()), this, SLOT(slotDocumentSaved()));
        action = dynamic_cast<KAction*>(mainWindow()->actionCollection()->action("view_toggledockers"));
        if (action) {
            action->setShortcut(QKeySequence(), KAction::DefaultShortcut);
            action->setShortcut(QKeySequence(), KAction::ActiveShortcut);
        }

        KoToolBoxFactory toolBoxFactory;
        mainWindow()->createDockWidget(&toolBoxFactory);

        connect(canvasController, SIGNAL(toolOptionWidgetsChanged(QList<QWidget*>)),
                mainWindow()->dockerManager(), SLOT(newOptionWidgets(QList<QWidget*>)));

        mainWindow()->dockerManager()->setIcons(false);
    }

    m_d->statusBar = new KisStatusBar(this);
    connect(m_d->canvasController->proxyObject, SIGNAL(documentMousePositionChanged(QPointF)),
            m_d->statusBar, SLOT(documentMousePositionChanged(QPointF)));

    connect(m_d->nodeManager, SIGNAL(sigNodeActivated(KisNodeSP)),
            m_d->resourceProvider, SLOT(slotNodeActivated(KisNodeSP)));

    connect(m_d->nodeManager, SIGNAL(sigNodeActivated(KisNodeSP)),
            m_d->controlFrame->paintopBox(), SLOT(slotCurrentNodeChanged(KisNodeSP)));
    connect(m_d->resourceProvider->resourceManager(), SIGNAL(canvasResourceChanged(int,QVariant)),
            m_d->controlFrame->paintopBox(), SLOT(slotCanvasResourceChanged(int,QVariant)));

    connect(m_d->nodeManager, SIGNAL(sigNodeActivated(KisNodeSP)),
            m_d->doc->image(), SLOT(requestStrokeEnd()));

    connect(KoToolManager::instance(), SIGNAL(inputDeviceChanged(KoInputDevice)),
            m_d->controlFrame->paintopBox(), SLOT(slotInputDeviceChanged(KoInputDevice)));

    connect(KoToolManager::instance(), SIGNAL(changedTool(KoCanvasController*,int)),
            m_d->controlFrame->paintopBox(), SLOT(slotToolChanged(KoCanvasController*,int)));

    // 25 px is a distance that works well for Tablet and Mouse events
    qApp->setStartDragDistance(25);

    loadPlugins();

    // Wait for the async image to have loaded
    connect(m_d->doc, SIGNAL(sigLoadingFinished()), this, SLOT(slotLoadingFinished()));
    if (!m_d->doc->isLoading() || m_d->doc->image()) {
        slotLoadingFinished();
    }

    connect(m_d->canvasController, SIGNAL(documentSizeChanged()), m_d->zoomManager, SLOT(slotScrollAreaSizeChanged()));

    setAcceptDrops(true);

    KConfigGroup group(KGlobal::config(), "krita/shortcuts");
    foreach(KActionCollection *collection, KActionCollection::allCollections()) {
        collection->setConfigGroup("krita/shortcuts");
        collection->readSettings(&group);
    }

    KisInputProfileManager::instance()->loadProfiles();

#if 0
    //check for colliding shortcuts
    QSet<QKeySequence> existingShortcuts;
    foreach(QAction* action, actionCollection()->actions()) {
        if(action->shortcut() == QKeySequence(0)) {
            continue;
        }
        dbgUI << "shortcut " << action->text() << " " << action->shortcut();
        Q_ASSERT(!existingShortcuts.contains(action->shortcut()));
        existingShortcuts.insert(action->shortcut());
    }
#endif

    QString lastPreset = cfg.readEntry("LastPreset", QString("Basic_tip_default"));
    KoResourceServer<KisPaintOpPreset> * rserver = KisResourceServerProvider::instance()->paintOpPresetServer();
    KisPaintOpPreset *preset = rserver->resourceByName(lastPreset);
    if (!preset) {
        preset = rserver->resourceByName("Basic_tip_default");
    }
    if (!preset) {
        if (rserver->resources().isEmpty()) {
            KMessageBox::error(this, i18n("Krita cannot find any brush presets and will close now. Please check your installation.", i18n("Critical Error")));
            exit(0);
        }
        preset = rserver->resources().first();
    }
    if (preset) {
        paintOpBox()->resourceSelected(preset);
    }
    connect(mainWindow(), SIGNAL(themeChanged()), this, SLOT(updateIcons()));
    updateIcons();

    /**
     * FIXME: for now enable tooltip manager only in non-sketch mode!
     */
    if (mainWindow()) {
        m_d->tooltipManager = new KisTooltipManager(this);
        // m_d->tooltipManager->record();
    }
}


KisView2::~KisView2()
{
    KisConfig cfg;
    cfg.writeEntry("LastPreset", m_d->resourceProvider->currentPreset()->name());
    cfg.writeEntry("baseLength", KoResourceItemChooserSync::instance()->baseLength());

    if (m_d->filterManager->isStrokeRunning()) {
        m_d->filterManager->cancel();
    }

    // The reason for this is to ensure the shortcuts are saved at the right time,
    // and only the right shortcuts. Gemini has two views at all times, and shortcuts
    // must be handled by the desktopview, but if we use the logic as below, we
    // overwrite the desktop view's settings with the sketch view's
    if(qApp->applicationName() == QLatin1String("kritagemini")) {
        KConfigGroup group(KGlobal::config(), "krita/shortcuts");
        foreach(KActionCollection *collection, KActionCollection::allCollections()) {
            const QObject* obj = dynamic_cast<const QObject*>(collection->parentGUIClient());
            if(obj && qobject_cast<const KisView2*>(obj) && !obj->objectName().startsWith("view_0"))
                break;
            collection->setConfigGroup("krita/shortcuts");
            collection->writeSettings(&group);
        }
    }
    else {
        KConfigGroup group(KGlobal::config(), "krita/shortcuts");
        foreach(KActionCollection *collection, KActionCollection::allCollections()) {
            collection->setConfigGroup("krita/shortcuts");
            collection->writeSettings(&group);
        }
    }
    delete m_d;
}


void KisView2::dragEnterEvent(QDragEnterEvent *event)
{
    dbgUI << "KisView2::dragEnterEvent";
    // Only accept drag if we're not busy, particularly as we may
    // be showing a progress bar and calling qApp->processEvents().
    if (event->mimeData()->hasImage()
            || event->mimeData()->hasUrls()
            || event->mimeData()->hasFormat("application/x-krita-node")) {
        event->accept();
    } else {
        event->ignore();
    }
}

void KisView2::dropEvent(QDropEvent *event)
{
    KisImageSP kisimage = image();
    Q_ASSERT(kisimage);

    QPoint cursorPos = canvasBase()->coordinatesConverter()->widgetToImage(event->pos()).toPoint();
    QRect imageBounds = kisimage->bounds();
    QPoint pasteCenter;
    bool forceRecenter;

    if (event->keyboardModifiers() & Qt::ShiftModifier &&
        imageBounds.contains(cursorPos)) {

        pasteCenter = cursorPos;
        forceRecenter = true;
    } else {
        pasteCenter = imageBounds.center();
        forceRecenter = false;
    }

    if (event->mimeData()->hasFormat("application/x-krita-node") ||
        event->mimeData()->hasImage())
    {
        KisShapeController *kritaShapeController =
            dynamic_cast<KisShapeController*>(m_d->doc->shapeController());

        QList<KisNodeSP> nodes =
            KisMimeData::loadNodes(event->mimeData(), imageBounds,
                                  pasteCenter, forceRecenter,
                                  kisimage, kritaShapeController);

        foreach(KisNodeSP node, nodes) {
            if (node) {
                KisNodeCommandsAdapter adapter(this);
                if (!m_d->nodeManager->activeLayer()) {
                    adapter.addNode(node, kisimage->rootLayer() , 0);
                } else {
                    adapter.addNode(node,
                                    m_d->nodeManager->activeLayer()->parent(),
                                    m_d->nodeManager->activeLayer());
                }
            }
        }

    } else if (event->mimeData()->hasUrls()) {

        QList<QUrl> urls = event->mimeData()->urls();
        if (urls.length() > 0) {

            KMenu popup;
            popup.setObjectName("drop_popup");

            QAction *insertAsNewLayer = new KAction(i18n("Insert as New Layer"), &popup);
            QAction *insertManyLayers = new KAction(i18n("Insert Many Layers"), &popup);

            QAction *openInNewDocument = new KAction(i18n("Open in New Document"), &popup);
            QAction *openManyDocuments = new KAction(i18n("Open Many Documents"), &popup);

            QAction *replaceCurrentDocument = new KAction(i18n("Replace Current Document"), &popup);

            QAction *cancel = new KAction(i18n("Cancel"), &popup);

            popup.addAction(insertAsNewLayer);
            popup.addAction(openInNewDocument);
            popup.addAction(replaceCurrentDocument);
            popup.addAction(insertManyLayers);
            popup.addAction(openManyDocuments);

            insertAsNewLayer->setEnabled(image() && urls.count() == 1);
            openInNewDocument->setEnabled(urls.count() == 1);
            replaceCurrentDocument->setEnabled(image() && urls.count() == 1);
            insertManyLayers->setEnabled(image() && urls.count() > 1);
            openManyDocuments->setEnabled(urls.count() > 1);

            popup.addSeparator();
            popup.addAction(cancel);

            QAction *action = popup.exec(QCursor::pos());

            if (action != 0 && action != cancel) {
                foreach(const QUrl &url, urls) {

                    if (action == insertAsNewLayer || action == insertManyLayers) {
                        m_d->imageManager->importImage(KUrl(url));
                        activateWindow();
                    }
                    else if (action == replaceCurrentDocument) {
                        if (m_d->doc->isModified()) {
                            m_d->doc->save();
                        }
                        if (mainWindow() != 0) {
                            /**
                             * NOTE: this is effectively deferred self-destruction
                             */
                            connect(mainWindow(), SIGNAL(loadCompleted()),
                                    mainWindow(), SLOT(close()));

                            mainWindow()->openDocument(url);
                        }
                    } else {
                        Q_ASSERT(action == openInNewDocument || action == openManyDocuments);
                        if (mainWindow()) {
                            mainWindow()->openDocument(url);
                        }
                    }
                }
            }
        }
    }
}

bool KisView2::event( QEvent* event )
{
    switch(static_cast<int>(event->type()))
    {
        case ViewModeSwitchEvent::AboutToSwitchViewModeEvent: {
            ViewModeSynchronisationObject* syncObject = static_cast<ViewModeSwitchEvent*>(event)->synchronisationObject();

            canvasControllerWidget()->setFocus();
            qApp->processEvents();

            KisCanvasResourceProvider* provider = resourceProvider();
            syncObject->backgroundColor = provider->bgColor();
            syncObject->foregroundColor = provider->fgColor();
            syncObject->exposure = provider->HDRExposure();
            syncObject->gamma = provider->HDRGamma();
            syncObject->compositeOp = provider->currentCompositeOp();
            syncObject->pattern = provider->currentPattern();
            syncObject->gradient = provider->currentGradient();
            syncObject->node = provider->currentNode();
            syncObject->paintOp = provider->currentPreset();
            syncObject->opacity = provider->opacity();
            syncObject->globalAlphaLock = provider->globalAlphaLock();

            syncObject->documentOffset = canvasControllerWidget()->scrollBarValue() - pos();
            syncObject->zoomLevel = zoomController()->zoomAction()->effectiveZoom();
            syncObject->rotationAngle = canvasBase()->rotationAngle();

            syncObject->activeToolId = KoToolManager::instance()->activeToolId();

            syncObject->gridData = &document()->gridData();

            syncObject->mirrorHorizontal = provider->mirrorHorizontal();
            syncObject->mirrorVertical = provider->mirrorVertical();
            syncObject->mirrorAxesCenter = provider->resourceManager()->resource(KisCanvasResourceProvider::MirrorAxesCenter).toPointF();

            KisToolFreehand* tool = qobject_cast<KisToolFreehand*>(KoToolManager::instance()->toolById(canvasBase(), syncObject->activeToolId));
            if(tool) {
                syncObject->smoothingOptions = tool->smoothingOptions();
            }

            syncObject->initialized = true;

            QMainWindow* mainWindow = qobject_cast<QMainWindow*>(qApp->activeWindow());
            if(mainWindow) {
                QList<QDockWidget*> dockWidgets = mainWindow->findChildren<QDockWidget*>();
                foreach(QDockWidget* widget, dockWidgets) {
                    if (widget->isFloating()) {
                        widget->hide();
                    }
                }
            }

            return true;
        }
        case ViewModeSwitchEvent::SwitchedToDesktopModeEvent: {
            ViewModeSynchronisationObject* syncObject = static_cast<ViewModeSwitchEvent*>(event)->synchronisationObject();
            canvasControllerWidget()->setFocus();
            qApp->processEvents();

            if(syncObject->initialized) {
                KisCanvasResourceProvider* provider = resourceProvider();

                provider->resourceManager()->setResource(KisCanvasResourceProvider::MirrorAxesCenter, syncObject->mirrorAxesCenter);
                if (provider->mirrorHorizontal() != syncObject->mirrorHorizontal) {
                    QAction* mirrorAction = actionCollection()->action("hmirror_action");
                    mirrorAction->setChecked(syncObject->mirrorHorizontal);
                    provider->setMirrorHorizontal(syncObject->mirrorHorizontal);
                }
                if (provider->mirrorVertical() != syncObject->mirrorVertical) {
                    QAction* mirrorAction = actionCollection()->action("vmirror_action");
                    mirrorAction->setChecked(syncObject->mirrorVertical);
                    provider->setMirrorVertical(syncObject->mirrorVertical);
                }

                provider->setPaintOpPreset(syncObject->paintOp);
                qApp->processEvents();

                KoToolManager::instance()->switchToolRequested(syncObject->activeToolId);
                qApp->processEvents();

                KisPaintOpPresetSP preset = canvasBase()->resourceManager()->resource(KisCanvasResourceProvider::CurrentPaintOpPreset).value<KisPaintOpPresetSP>();
                preset->settings()->setProperty("CompositeOp", syncObject->compositeOp);
                if(preset->settings()->hasProperty("OpacityValue"))
                    preset->settings()->setProperty("OpacityValue", syncObject->opacity);
                provider->setPaintOpPreset(preset);

                provider->setBGColor(syncObject->backgroundColor);
                provider->setFGColor(syncObject->foregroundColor);
                provider->setHDRExposure(syncObject->exposure);
                provider->setHDRGamma(syncObject->gamma);
                provider->slotPatternActivated(syncObject->pattern);
                provider->slotGradientActivated(syncObject->gradient);
                provider->slotNodeActivated(syncObject->node);
                provider->setOpacity(syncObject->opacity);
                provider->setGlobalAlphaLock(syncObject->globalAlphaLock);
                provider->setCurrentCompositeOp(syncObject->compositeOp);

                document()->gridData().setGrid(syncObject->gridData->gridX(), syncObject->gridData->gridY());
                document()->gridData().setGridColor(syncObject->gridData->gridColor());
                document()->gridData().setPaintGridInBackground(syncObject->gridData->paintGridInBackground());
                document()->gridData().setShowGrid(syncObject->gridData->showGrid());
                document()->gridData().setSnapToGrid(syncObject->gridData->snapToGrid());

                actionCollection()->action("zoom_in")->trigger();
                qApp->processEvents();


                QMainWindow* mainWindow = qobject_cast<QMainWindow*>(qApp->activeWindow());
                if(mainWindow) {
                    QList<QDockWidget*> dockWidgets = mainWindow->findChildren<QDockWidget*>();
                    foreach(QDockWidget* widget, dockWidgets) {
                        if (widget->isFloating()) {
                            widget->show();
                        }
                    }
                }

                zoomController()->setZoom(KoZoomMode::ZOOM_CONSTANT, syncObject->zoomLevel);
                canvasControllerWidget()->rotateCanvas(syncObject->rotationAngle - canvasBase()->rotationAngle());

                QPoint newOffset = syncObject->documentOffset + pos();
                qApp->processEvents();
                canvasControllerWidget()->setScrollBarValue(newOffset);

                KisToolFreehand* tool = qobject_cast<KisToolFreehand*>(KoToolManager::instance()->toolById(canvasBase(), syncObject->activeToolId));
                if(tool && syncObject->smoothingOptions) {
                    tool->smoothingOptions()->setSmoothingType(syncObject->smoothingOptions->smoothingType());
                    tool->smoothingOptions()->setSmoothPressure(syncObject->smoothingOptions->smoothPressure());
                    tool->smoothingOptions()->setTailAggressiveness(syncObject->smoothingOptions->tailAggressiveness());
                    tool->smoothingOptions()->setUseScalableDistance(syncObject->smoothingOptions->useScalableDistance());
                    tool->smoothingOptions()->setSmoothnessDistance(syncObject->smoothingOptions->smoothnessDistance());
                    tool->smoothingOptions()->setUseDelayDistance(syncObject->smoothingOptions->useDelayDistance());
                    tool->smoothingOptions()->setDelayDistance(syncObject->smoothingOptions->delayDistance());
                    tool->smoothingOptions()->setFinishStabilizedCurve(syncObject->smoothingOptions->finishStabilizedCurve());
                    tool->smoothingOptions()->setStabilizeSensors(syncObject->smoothingOptions->stabilizeSensors());
                    tool->updateSettingsViews();
                }
            }

            return true;
        }
        default:
            break;
    }

    return QWidget::event( event );
}

KoZoomController *KisView2::zoomController() const
{
    return m_d->zoomManager->zoomController();
}

KisImageWSP KisView2::image() const
{
    if (m_d && m_d->doc) {
        return m_d->doc->image();
    }
    return 0;
}

KisCanvasResourceProvider * KisView2::resourceProvider()
{
    return m_d->resourceProvider;
}

KisCanvas2 * KisView2::canvasBase() const
{
    return m_d->canvas;
}

QWidget* KisView2::canvas() const
{
    return m_d->canvas->canvasWidget();
}

KisStatusBar * KisView2::statusBar() const
{
    return m_d->statusBar;
}

KisPaintopBox* KisView2::paintOpBox() const
{
    return m_d->controlFrame->paintopBox();
}

KoProgressUpdater* KisView2::createProgressUpdater(KoProgressUpdater::Mode mode)
{
    return new KisProgressUpdater(m_d->statusBar->progress(), document()->progressProxy(), mode);
}

KisSelectionManager * KisView2::selectionManager()
{
    return m_d->selectionManager;
}

KoCanvasController * KisView2::canvasController()
{
    return m_d->canvasController;
}

KisCanvasController *KisView2::canvasControllerWidget()
{
    return m_d->canvasController;
}

KisNodeSP KisView2::activeNode()
{
    if (m_d->nodeManager)
        return m_d->nodeManager->activeNode();
    else
        return 0;
}

KisLayerSP KisView2::activeLayer()
{
    if (m_d->nodeManager)
        return m_d->nodeManager->activeLayer();
    else
        return 0;
}

KisPaintDeviceSP KisView2::activeDevice()
{
    if (m_d->nodeManager)
        return m_d->nodeManager->activePaintDevice();
    else
        return 0;
}

KisZoomManager * KisView2::zoomManager()
{
    return m_d->zoomManager;
}

KisFilterManager * KisView2::filterManager()
{
    return m_d->filterManager;
}

KisImageManager * KisView2::imageManager()
{
    return m_d->imageManager;
}

KisSelectionSP KisView2::selection()
{
    KisLayerSP layer = activeLayer();
    if (layer)
        return layer->selection(); // falls through to the global
    // selection, or 0 in the end
    return image()->globalSelection();
}

bool KisView2::selectionEditable()
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

KisUndoAdapter * KisView2::undoAdapter()
{
    KisImageWSP image = m_d->doc->image();
    Q_ASSERT(image);

    return image->undoAdapter();
}


void KisView2::slotLoadingFinished()
{
    /**
     * Cold-start of image size/resolution signals
     */
    slotImageResolutionChanged();
    if (m_d->statusBar) {
        m_d->statusBar->imageSizeChanged();
    }
    if (m_d->resourceProvider) {
        m_d->resourceProvider->slotImageSizeChanged();
    }
    if (m_d->nodeManager) {
        m_d->nodeManager->nodesUpdated();
    }


    if (m_d->statusBar) {
        connect(image(), SIGNAL(sigColorSpaceChanged(const KoColorSpace*)), m_d->statusBar, SLOT(updateStatusBarProfileLabel()));
        connect(image(), SIGNAL(sigProfileChanged(const KoColorProfile*)), m_d->statusBar, SLOT(updateStatusBarProfileLabel()));
        connect(image(), SIGNAL(sigSizeChanged(const QPointF&, const QPointF&)), m_d->statusBar, SLOT(imageSizeChanged()));

    }
    connect(image(), SIGNAL(sigSizeChanged(const QPointF&, const QPointF&)), m_d->resourceProvider, SLOT(slotImageSizeChanged()));

    connect(image(), SIGNAL(sigResolutionChanged(double,double)),
            m_d->resourceProvider, SLOT(slotOnScreenResolutionChanged()));
    connect(zoomManager()->zoomController(), SIGNAL(zoomChanged(KoZoomMode::Mode,qreal)),
            m_d->resourceProvider, SLOT(slotOnScreenResolutionChanged()));

    connect(image(), SIGNAL(sigSizeChanged(const QPointF&, const QPointF&)), this, SLOT(slotImageSizeChanged(const QPointF&, const QPointF&)));
    connect(image(), SIGNAL(sigResolutionChanged(double,double)), this, SLOT(slotImageResolutionChanged()));
    connect(image(), SIGNAL(sigNodeChanged(KisNodeSP)), this, SLOT(slotNodeChanged()));
    connect(image()->undoAdapter(), SIGNAL(selectionChanged()), selectionManager(), SLOT(selectionChanged()));

    /*
     * WARNING: Currently we access the global progress bar in two ways:
     * connecting to composite progress proxy (strokes) and creating
     * progress updaters. The latter way should be deprecated in favour
     * of displaying the status of the global strokes queue
     */
    image()->compositeProgressProxy()->addProxy(m_d->statusBar->progress()->progressProxy());
    connect(m_d->statusBar->progress(), SIGNAL(sigCancellationRequested()),
            image(), SLOT(requestStrokeCancellation()));

    m_d->canvas->initializeImage();

    if (m_d->controlFrame) {
        connect(image(), SIGNAL(sigColorSpaceChanged(const KoColorSpace*)), m_d->controlFrame->paintopBox(), SLOT(slotColorSpaceChanged(const KoColorSpace*)));
    }

    if (image()->locked()) {
        // If this is the first view on the image, the image will have been locked
        // so unlock it.
        image()->blockSignals(false);
        image()->unlock();
    }

    KisNodeSP activeNode = m_d->doc->preActivatedNode();
    m_d->doc->setPreActivatedNode(0); // to make sure that we don't keep a reference to a layer the user can later delete.

    if (!activeNode) {
        activeNode = image()->rootLayer()->firstChild();
    }

    while (activeNode && !activeNode->inherits("KisLayer")) {
        activeNode = activeNode->nextSibling();
    }

    if (activeNode) {
        m_d->nodeManager->slotNonUiActivatedNode(activeNode);
    }


    // get the assistants and push them to the manager
    QList<KisPaintingAssistant*> paintingAssistants = m_d->doc->preLoadedAssistants();
    foreach (KisPaintingAssistant* assistant, paintingAssistants) {
        m_d->paintingAssistantsDecoration->addAssistant(assistant);
    }

    /**
     * Dirty hack alert
     */
    if (mainWindow() && m_d->zoomManager && m_d->zoomManager->zoomController())
        m_d->zoomManager->zoomController()->setAspectMode(true);
    if (m_d->viewConverter)
        m_d->viewConverter->setZoomMode(KoZoomMode::ZOOM_PAGE);
    if (m_d->paintingAssistantsDecoration){
        foreach(KisPaintingAssistant* assist, m_d->doc->preLoadedAssistants()){
            m_d->paintingAssistantsDecoration->addAssistant(assist);
        }
        m_d->paintingAssistantsDecoration->setVisible(true);
    }
    updateGUI();

    emit sigLoadingFinished();
}

void KisView2::slotSavingFinished()
{
    if(mainWindow())
        mainWindow()->updateCaption();
}

void KisView2::createActions()
{
    actionCollection()->addAction(KStandardAction::Preferences,  "preferences", this, SLOT(slotPreferences()));

    KAction *action = new KAction(i18n("Cleanup removed files..."), this);
    actionCollection()->addAction("edit_blacklist_cleanup", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotBlacklistCleanup()));
}



void KisView2::createManagers()
{
    // Create the managers for filters, selections, layers etc.
    // XXX: When the currentlayer changes, call updateGUI on all
    // managers

    m_d->actionManager = new KisActionManager(this);

    m_d->filterManager = new KisFilterManager(this, m_d->doc);
    m_d->filterManager->setup(actionCollection());

    m_d->selectionManager = new KisSelectionManager(this, m_d->doc);
    m_d->selectionManager->setup(actionCollection(), actionManager());

    m_d->nodeManager = new KisNodeManager(this, m_d->doc);
    m_d->nodeManager->setup(actionCollection(), actionManager());

    // the following cast is not really safe, but better here than in the zoomManager
    // best place would be outside kisview too
    m_d->zoomManager = new KisZoomManager(this, m_d->viewConverter, m_d->canvasController);
    m_d->zoomManager->setup(actionCollection());

    m_d->imageManager = new KisImageManager(this);
    m_d->imageManager->setup(actionCollection());

    m_d->gridManager = new KisGridManager(this);
    m_d->gridManager->setup(actionCollection());
    m_d->canvas->addDecoration(m_d->gridManager);

    m_d->perspectiveGridManager = new KisPerspectiveGridManager(this);
    m_d->perspectiveGridManager->setup(actionCollection());
    m_d->canvas->addDecoration(m_d->perspectiveGridManager);

    m_d->paintingAssistantsDecoration = new KisPaintingAssistantsDecoration(this);
    m_d->paintingAssistantsDecoration->setup(actionCollection());
    m_d->canvas->addDecoration(m_d->paintingAssistantsDecoration);

    m_d->canvasControlsManager = new KisCanvasControlsManager(this);
    m_d->canvasControlsManager->setup(actionCollection());

    m_d->mirrorAxis = new KisMirrorAxis(m_d->resourceProvider, this);
    m_d->canvas->addDecoration(m_d->mirrorAxis);
}

void KisView2::updateGUI()
{
    m_d->nodeManager->updateGUI();
    m_d->selectionManager->updateGUI();
    m_d->filterManager->updateGUI();
    m_d->zoomManager->updateGUI();
    m_d->gridManager->updateGUI();
    m_d->perspectiveGridManager->updateGUI();
    m_d->actionManager->updateGUI();
}

void KisView2::slotPreferences()
{
    if (KisDlgPreferences::editPreferences()) {
        KisConfigNotifier::instance()->notifyConfigChanged();
        m_d->resourceProvider->resetDisplayProfile(QApplication::desktop()->screenNumber(this));

        showHideScrollbars();

        // Update the settings for all nodes -- they don't query
        // KisConfig directly because they need the settings during
        // compositing, and they don't connect to the confignotifier
        // because nodes are not QObjects (because only one base class
        // can be a QObject).
        KisNode* node = dynamic_cast<KisNode*>(image()->rootLayer().data());
        node->updateSettings();
    }
}

void KisView2::slotBlacklistCleanup()
{
    KisDlgBlacklistCleanup dialog;
    dialog.exec();
}

void KisView2::resetImageSizeAndScroll(bool changeCentering,
                                       const QPointF oldImageStillPoint,
                                       const QPointF newImageStillPoint) {

    const KisCoordinatesConverter *converter =
        canvasBase()->coordinatesConverter();

    QPointF oldPreferredCenter =
        m_d->canvasController->preferredCenter();

    /**
     * Calculating the still point in old coordinates depending on the
     * parameters given
     */

    QPointF oldStillPoint;

    if (changeCentering) {
        oldStillPoint =
            converter->imageToWidget(oldImageStillPoint) +
            converter->documentOffset();
    } else {
        QSize oldDocumentSize = m_d->canvasController->documentSize();
        oldStillPoint = QPointF(0.5 * oldDocumentSize.width(), 0.5 * oldDocumentSize.height());
    }

    /**
     * Updating the document size
     */

    QSizeF size(image()->width() / image()->xRes(), image()->height() / image()->yRes());
    KoZoomController *zc = m_d->zoomManager->zoomController();
    zc->setZoom(KoZoomMode::ZOOM_CONSTANT, zc->zoomAction()->effectiveZoom());
    zc->setPageSize(size);
    zc->setDocumentSize(size, true);

    /**
     * Calculating the still point in new coordinates depending on the
     * parameters given
     */

    QPointF newStillPoint;

    if (changeCentering) {
        newStillPoint =
            converter->imageToWidget(newImageStillPoint) +
            converter->documentOffset();
    } else {
        QSize newDocumentSize = m_d->canvasController->documentSize();
        newStillPoint = QPointF(0.5 * newDocumentSize.width(), 0.5 * newDocumentSize.height());
    }

    m_d->canvasController->setPreferredCenter(oldPreferredCenter - oldStillPoint + newStillPoint);
}

void KisView2::slotImageSizeChanged(const QPointF &oldStillPoint, const QPointF &newStillPoint)
{
    resetImageSizeAndScroll(true, oldStillPoint, newStillPoint);
    m_d->zoomManager->updateGUI();
}

void KisView2::slotImageResolutionChanged()
{
    resetImageSizeAndScroll(false);
    m_d->zoomManager->updateGUI();
}

void KisView2::slotNodeChanged()
{
    updateGUI();
}

void KisView2::loadPlugins()
{
    // Load all plugins
    const KService::List offers = KoServiceLocator::instance()->entries("Krita/ViewPlugin");
    KService::List::ConstIterator iter;
    for (iter = offers.constBegin(); iter != offers.constEnd(); ++iter) {
        KService::Ptr service = *iter;
        dbgUI << "Load plugin " << service->name();
        QString error;

        KXMLGUIClient* plugin =
                dynamic_cast<KXMLGUIClient*>(service->createInstance<QObject>(this, QVariantList(), &error));
        if (plugin) {
            insertChildClient(plugin);
        } else {
            errKrita << "Fail to create an instance for " << service->name() << " " << error;
        }
    }
}

KisDoc2 * KisView2::document() const
{
    return m_d->doc;
}

KoPrintJob * KisView2::createPrintJob()
{
    return new KisPrintJob(image());
}

KisNodeManager * KisView2::nodeManager()
{
    return m_d->nodeManager;
}

KisActionManager* KisView2::actionManager()
{
    return m_d->actionManager;
}

KisPerspectiveGridManager* KisView2::perspectiveGridManager()
{
    return m_d->perspectiveGridManager;
}

KisGridManager * KisView2::gridManager()
{
    return m_d->gridManager;
}

KisPaintingAssistantsDecoration* KisView2::paintingAssistantsDecoration()
{
    return m_d->paintingAssistantsDecoration;
}

QMainWindow* KisView2::qtMainWindow()
{
    if(m_d->mainWindow)
        return m_d->mainWindow;

    //Fallback for when we have not yet set the main window.
    QMainWindow* w = qobject_cast<QMainWindow*>(qApp->activeWindow());
    if(w)
        return w;

    return mainWindow();
}

void KisView2::setQtMainWindow(QMainWindow* newMainWindow)
{
    m_d->mainWindow = newMainWindow;
}

void KisView2::slotCreateTemplate()
{
    KoTemplateCreateDia::createTemplate("krita_template", ".kra",
                                        KisFactory2::componentData(), m_d->doc, this);
}

void KisView2::slotDocumentSaved()
{
    m_d->saveIncremental->setEnabled(true);
    m_d->saveIncrementalBackup->setEnabled(true);
}

void KisView2::slotSaveIncremental()
{
    if (!m_d->doc) return;

    bool foundVersion;
    bool fileAlreadyExists;
    bool isBackup;
    QString version = "000";
    QString newVersion;
    QString letter;
    QString fileName = m_d->doc->localFilePath();

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
        fileAlreadyExists = KIO::NetAccess::exists(fileName, KIO::NetAccess::DestinationSide, this);
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
        KMessageBox::error(this, "Alternative names exhausted, try manually saving with a higher number", "Couldn't save incremental version");
        return;
    }
    m_d->doc->setSaveInBatchMode(true);
    m_d->doc->saveAs(fileName);
    m_d->doc->setSaveInBatchMode(false);

    emit sigSavingFinished();
}

void KisView2::slotSaveIncrementalBackup()
{
    if (!m_d->doc) return;

    bool workingOnBackup;
    bool fileAlreadyExists;
    QString version = "000";
    QString newVersion;
    QString letter;
    QString fileName = m_d->doc->localFilePath();

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
        QString backupFileName = m_d->doc->localFilePath();
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
            fileAlreadyExists = KIO::NetAccess::exists(backupFileName, KIO::NetAccess::DestinationSide, this);
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
            KMessageBox::error(this, "Alternative names exhausted, try manually saving with a higher number", "Couldn't save incremental backup");
            return;
        }
        QFile::copy(fileName, backupFileName);
        m_d->doc->saveAs(fileName);

        emit sigSavingFinished();
    }
    else { // if NOT working on a backup...
        // Navigate directory searching for latest backup version, ignore letters
        const quint8 HARDCODED_DIGIT_COUNT = 3;
        QString baseNewVersion = "000";
        QString backupFileName = m_d->doc->localFilePath();
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
            fileAlreadyExists = KIO::NetAccess::exists(backupFileName, KIO::NetAccess::DestinationSide, this);
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
        m_d->doc->setSaveInBatchMode(true);
        QFile::copy(fileName, backupFileName);
        m_d->doc->saveAs(fileName);
        m_d->doc->setSaveInBatchMode(false);

        emit sigSavingFinished();
    }
}

void KisView2::disableControls()
{
    // prevents possible crashes, if somebody changes the paintop during dragging by using the mousewheel
    // this is for Bug 250944
    // the solution blocks all wheel, mouse and key event, while dragging with the freehand tool
    // see KisToolFreehand::initPaint() and endPaint()
    m_d->controlFrame->paintopBox()->installEventFilter(&m_d->blockingEventFilter);
    foreach(QObject* child, m_d->controlFrame->paintopBox()->children()) {
        child->installEventFilter(&m_d->blockingEventFilter);
    }
}

void KisView2::enableControls()
{
    m_d->controlFrame->paintopBox()->removeEventFilter(&m_d->blockingEventFilter);
    foreach(QObject* child, m_d->controlFrame->paintopBox()->children()) {
        child->removeEventFilter(&m_d->blockingEventFilter);
    }
}

void KisView2::showStatusBar(bool toggled)
{
    if (KoView::statusBar()) {
        KoView::statusBar()->setVisible(toggled);
    }
}

#if defined HAVE_OPENGL && defined Q_OS_WIN32
#include <QGLContext>
#endif

void KisView2::showJustTheCanvas(bool toggled)
{
    KisConfig cfg;

/**
 * Workaround for a broken Intel video driver on Windows :(
 * See bug 330040
 */
#if defined HAVE_OPENGL && defined Q_OS_WIN32

    if (toggled && cfg.useOpenGL()) {
        QString renderer((const char*)glGetString(GL_RENDERER));
        bool failingDriver = renderer.startsWith("Intel(R) HD Graphics");

        if (failingDriver &&
            cfg.hideStatusbarFullscreen() &&
            cfg.hideDockersFullscreen() &&
            cfg.hideTitlebarFullscreen() &&
            cfg.hideMenuFullscreen() &&
            cfg.hideToolbarFullscreen() &&
            cfg.hideScrollbarsFullscreen()) {

            int result =
                KMessageBox::warningYesNo(this,
                                          "Intel(R) HD Graphics video adapters "
                                          "are known to have problems with running "
                                          "Krita in pure canvas only mode. At least "
                                          "one UI control must be shown to "
                                          "workaround it.\n\nShow the scroll bars?",
                                          "Failing video adapter",
                                          KStandardGuiItem::yes(),
                                          KStandardGuiItem::no(),
                                          "messagebox_WorkaroundIntelVideoOnWindows");

            if (result == KMessageBox::Yes) {
                cfg.setHideScrollbarsFullscreen(false);
            }
        }
    }

#endif /* defined HAVE_OPENGL && defined Q_OS_WIN32 */

    KoMainWindow* main = mainWindow();
    if(!main) {
        main = window()->findChildren<KoMainWindow*>().value(0);
    }
    if(!main) {
        dbgUI << "Unable to switch to canvas-only mode, main window not found";
        return;
    }

    if (cfg.hideStatusbarFullscreen()) {
        if(main->statusBar() && main->statusBar()->isVisible() == toggled) {
            main->statusBar()->setVisible(!toggled);
        }
    }

    if (cfg.hideDockersFullscreen()) {
        KToggleAction* action = qobject_cast<KToggleAction*>(main->actionCollection()->action("view_toggledockers"));
        if (action && action->isChecked() == toggled) {
            action->setChecked(!toggled);
        }
    }

    if (cfg.hideTitlebarFullscreen()) {
        if(toggled) {
            window()->setWindowState( window()->windowState() | Qt::WindowFullScreen);
        } else {
            window()->setWindowState( window()->windowState() & ~Qt::WindowFullScreen);
        }
    }

    if (cfg.hideMenuFullscreen()) {
        if (main->menuBar()->isVisible() == toggled) {
            main->menuBar()->setVisible(!toggled);
        }
    }

    if (cfg.hideToolbarFullscreen()) {
        QList<QToolBar*> toolBars = main->findChildren<QToolBar*>();
        foreach(QToolBar* toolbar, toolBars) {
            if (toolbar->isVisible() == toggled) {
                toolbar->setVisible(!toggled);
            }
        }
    }

    showHideScrollbars();

    if (toggled) {
        // show a fading heads-up display about the shortcut to go back

        showFloatingMessage(i18n("Going into Canvas-Only mode.\nPress %1 to go back.",
                                 actionCollection()->action("view_show_just_the_canvas")->shortcut().toString()), QIcon());
    }
}

void KisView2::toggleTabletLogger()
{
    m_d->canvas->toggleTabletLogger();
}

void KisView2::openResourcesDirectory()
{
    QString dir = KStandardDirs::locateLocal("data", "krita");
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

void KisView2::updateIcons()
{
#if QT_VERSION >= 0x040700
    QColor background = palette().background().color();
    bool useDarkIcons = background.value() > 100;
    QString prefix = useDarkIcons ? QString("dark_") : QString("light_");

    QStringList whitelist;
    whitelist << "ToolBox" << "KisLayerBox";

    QStringList blacklistedIcons;
    blacklistedIcons << "editpath" << "artistictext-tool" << "view-choose";

    if (mainWindow()) {
        QList<QDockWidget*> dockers = mainWindow()->dockWidgets();
        foreach(QDockWidget* dock, dockers) {
            kDebug() << "name " << dock->objectName();
            if (!whitelist.contains(dock->objectName())) {
                continue;
            }

            QObjectList objects;
            objects.append(dock);
            while (!objects.isEmpty()) {
                QObject* object = objects.takeFirst();
                objects.append(object->children());

                QAbstractButton* button = dynamic_cast<QAbstractButton*>(object);
                if (button && !button->icon().name().isEmpty()) {
                    QString name = button->icon().name();
                    name = name.remove("dark_").remove("light_");

                    if (!blacklistedIcons.contains(name)) {
                        QString iconName = prefix + name;
                        KIcon icon = koIcon(iconName.toLatin1());
                        button->setIcon(icon);
                    }
                }
            }
        }
    }
#endif
}

void KisView2::showFloatingMessage(const QString message, const QIcon& icon, int timeout, KisFloatingMessage::Priority priority, int alignment)
{
    if(m_d->showFloatingMessage && qtMainWindow()) {
        if (m_d->savedFloatingMessage) {
            m_d->savedFloatingMessage->tryOverrideMessage(message, icon, timeout, priority, alignment);
        } else {
            m_d->savedFloatingMessage = new KisFloatingMessage(message, qtMainWindow()->centralWidget(), false, timeout, priority, alignment);
            m_d->savedFloatingMessage->setShowOverParent(true);
            m_d->savedFloatingMessage->setIcon(icon);
            m_d->savedFloatingMessage->showMessage();
        }
    }
#if QT_VERSION >= 0x040700
    emit floatingMessageRequested(message, icon.name());
#endif
}

void KisView2::showHideScrollbars()
{
    KisConfig cfg;
    bool toggled = actionCollection()->action("view_show_just_the_canvas")->isChecked();

    if ( (toggled && cfg.hideScrollbarsFullscreen()) || (!toggled && cfg.hideScrollbars()) ) {
        dynamic_cast<KoCanvasControllerWidget*>(canvasController())->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        dynamic_cast<KoCanvasControllerWidget*>(canvasController())->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    } else {
        dynamic_cast<KoCanvasControllerWidget*>(canvasController())->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        dynamic_cast<KoCanvasControllerWidget*>(canvasController())->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    }
}

void KisView2::setShowFloatingMessage(bool show)
{
    m_d->showFloatingMessage = show;
}

#include "kis_view2.moc"
