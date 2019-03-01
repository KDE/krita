/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000-2006 David Faure <faure@kde.org>
   Copyright (C) 2007, 2009 Thomas zander <zander@kde.org>
   Copyright (C) 2010 Benjamin Port <port.benjamin@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KisMainWindow.h"

#include <KoConfig.h>

// qt includes
#include <QApplication>
#include <QByteArray>
#include <QCloseEvent>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QDialog>
#include <QDockWidget>
#include <QIcon>
#include <QInputDialog>
#include <QLabel>
#include <QLayout>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMutex>
#include <QMutexLocker>
#include <QPointer>
#include <QPrintDialog>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QToolButton>
#include <QSignalMapper>
#include <QTabBar>
#include <QMoveEvent>
#include <QUrl>
#include <QMessageBox>
#include <QStatusBar>
#include <QMenu>
#include <QMenuBar>
#include <KisMimeDatabase.h>
#include <QMimeData>
#include <QStackedWidget>
#include <QProxyStyle>
#include <QScreen>
#include <QAction>

#include <kactioncollection.h>
#include <kactionmenu.h>
#include <kis_debug.h>
#include <kedittoolbar.h>
#include <khelpmenu.h>
#include <klocalizedstring.h>
#include <kaboutdata.h>
#include <kis_workspace_resource.h>
#include <input/kis_input_manager.h>
#include "kis_selection_manager.h"
#include "kis_icon_utils.h"
#include <krecentfilesaction.h>
#include <ktoggleaction.h>
#include <ktoolbar.h>
#include <kmainwindow.h>
#include <kxmlguiwindow.h>
#include <kxmlguifactory.h>
#include <kxmlguiclient.h>
#include <kguiitem.h>
#include <kwindowconfig.h>
#include <kformat.h>

#include <KoResourcePaths.h>
#include <KoToolFactoryBase.h>
#include <KoToolRegistry.h>
#include "KoDockFactoryBase.h"
#include "KoDocumentInfoDlg.h"
#include "KoDocumentInfo.h"
#include "KoFileDialog.h"
#include <kis_icon.h>
#include <KoPageLayoutDialog.h>
#include <KoPageLayoutWidget.h>
#include <KoToolManager.h>
#include <KoZoomController.h>
#include "KoToolDocker.h"
#include "KoToolBoxDocker_p.h"
#include <KoToolBoxFactory.h>
#include <KoDockRegistry.h>
#include <KoPluginLoader.h>
#include <KoColorSpaceEngine.h>
#include <KoUpdater.h>
#include <KoResourceModel.h>

#include <brushengine/kis_paintop_settings.h>
#include "dialogs/kis_about_application.h"
#include "dialogs/kis_delayed_save_dialog.h"
#include "dialogs/kis_dlg_preferences.h"
#include "kis_action.h"
#include "kis_action_manager.h"
#include "KisApplication.h"
#include "kis_canvas2.h"
#include "kis_canvas_controller.h"
#include "kis_canvas_resource_provider.h"
#include "kis_clipboard.h"
#include "kis_config.h"
#include "kis_config_notifier.h"
#include "kis_custom_image_widget.h"
#include <KisDocument.h>
#include "kis_group_layer.h"
#include "kis_image_from_clipboard_widget.h"
#include "kis_image.h"
#include <KisImportExportFilter.h>
#include "KisImportExportManager.h"
#include "kis_mainwindow_observer.h"
#include "kis_memory_statistics_server.h"
#include "kis_node.h"
#include "KisOpenPane.h"
#include "kis_paintop_box.h"
#include "KisPart.h"
#include "KisPrintJob.h"
#include "KisResourceServerProvider.h"
#include "kis_signal_compressor_with_param.h"
#include "kis_statusbar.h"
#include "KisView.h"
#include "KisViewManager.h"
#include "thememanager.h"
#include "kis_animation_importer.h"
#include "dialogs/kis_dlg_import_image_sequence.h"
#include <KisImageConfigNotifier.h>
#include "KisWindowLayoutManager.h"
#include <KisUndoActionsUpdateManager.h>
#include "KisWelcomePageWidget.h"
#include <KritaVersionWrapper.h>
#include <kritaversion.h>
#include <mutex>

#ifdef Q_OS_WIN
  #include <QtPlatformHeaders/QWindowsWindowFunctions>
#endif

class ToolDockerFactory : public KoDockFactoryBase
{
public:
    ToolDockerFactory() : KoDockFactoryBase() { }

    QString id() const override {
        return "sharedtooldocker";
    }

    QDockWidget* createDockWidget() override {
        KoToolDocker* dockWidget = new KoToolDocker();
        return dockWidget;
    }

    DockPosition defaultDockPosition() const override {
        return DockRight;
    }
};

class Q_DECL_HIDDEN KisMainWindow::Private
{
public:
    Private(KisMainWindow *parent, QUuid id)
        : q(parent)
        , id(id)
        , dockWidgetMenu(new KActionMenu(i18nc("@action:inmenu", "&Dockers"), parent))
        , windowMenu(new KActionMenu(i18nc("@action:inmenu", "&Window"), parent))
        , documentMenu(new KActionMenu(i18nc("@action:inmenu", "New &View"), parent))
        , workspaceMenu(new KActionMenu(i18nc("@action:inmenu", "Wor&kspace"), parent))
        , welcomePage(new KisWelcomePageWidget(parent))
        , widgetStack(new QStackedWidget(parent))
        , mdiArea(new QMdiArea(parent))
        , windowMapper(new QSignalMapper(parent))
        , documentMapper(new QSignalMapper(parent))
    {
        if (id.isNull()) this->id = QUuid::createUuid();

        widgetStack->addWidget(welcomePage);
        widgetStack->addWidget(mdiArea);
        mdiArea->setTabsMovable(true);
        mdiArea->setActivationOrder(QMdiArea::ActivationHistoryOrder);
    }

    ~Private() {
        qDeleteAll(toolbarList);
    }

    KisMainWindow *q {0};
    QUuid id;

    KisViewManager *viewManager {0};

    QPointer<KisView> activeView;

    QList<QAction *> toolbarList;

    bool firstTime {true};
    bool windowSizeDirty {false};
    bool readOnly {false};

    KisAction *showDocumentInfo {0};
    KisAction *saveAction {0};
    KisAction *saveActionAs {0};
//    KisAction *printAction;
//    KisAction *printActionPreview;
//    KisAction *exportPdf {0};
    KisAction *importAnimation {0};
    KisAction *closeAll {0};
//    KisAction *reloadFile;
    KisAction *importFile {0};
    KisAction *exportFile {0};
    KisAction *undo {0};
    KisAction *redo {0};
    KisAction *newWindow {0};
    KisAction *close {0};
    KisAction *mdiCascade {0};
    KisAction *mdiTile {0};
    KisAction *mdiNextWindow {0};
    KisAction *mdiPreviousWindow {0};
    KisAction *toggleDockers {0};
    KisAction *toggleDockerTitleBars {0};
    KisAction *fullScreenMode {0};
    KisAction *showSessionManager {0};

    KisAction *expandingSpacers[2];

    KActionMenu *dockWidgetMenu;
    KActionMenu *windowMenu;
    KActionMenu *documentMenu;
    KActionMenu *workspaceMenu;

    KHelpMenu *helpMenu  {0};

    KRecentFilesAction *recentFiles {0};
    KoResourceModel *workspacemodel {0};

    QScopedPointer<KisUndoActionsUpdateManager> undoActionsUpdateManager;

    QString lastExportLocation;

    QMap<QString, QDockWidget *> dockWidgetsMap;
    QByteArray dockerStateBeforeHiding;
    KoToolDocker *toolOptionsDocker {0};

    QCloseEvent *deferredClosingEvent {0};

    Digikam::ThemeManager *themeManager {0};

    KisWelcomePageWidget *welcomePage {0};


    QStackedWidget *widgetStack {0};

    QMdiArea *mdiArea;
    QMdiSubWindow *activeSubWindow  {0};
    QSignalMapper *windowMapper;
    QSignalMapper *documentMapper;

    QByteArray lastExportedFormat;
    QScopedPointer<KisSignalCompressorWithParam<int> > tabSwitchCompressor;
    QMutex savingEntryMutex;

    KConfigGroup windowStateConfig;

    QUuid workspaceBorrowedBy;
    KisSignalAutoConnectionsStore screenConnectionsStore;

    KisActionManager * actionManager() {
        return viewManager->actionManager();
    }

    QTabBar* findTabBarHACK() {
        QObjectList objects = mdiArea->children();
        Q_FOREACH (QObject *object, objects) {
            QTabBar *bar = qobject_cast<QTabBar*>(object);
            if (bar) {
                return bar;
            }
        }
        return 0;
    }
};

KisMainWindow::KisMainWindow(QUuid uuid)
    : KXmlGuiWindow()
    , d(new Private(this, uuid))
{
    auto rserver = KisResourceServerProvider::instance()->workspaceServer();
    QSharedPointer<KoAbstractResourceServerAdapter> adapter(new KoResourceServerAdapter<KisWorkspaceResource>(rserver));
    d->workspacemodel = new KoResourceModel(adapter, this);
    connect(d->workspacemodel, &KoResourceModel::afterResourcesLayoutReset, this, [&]() { updateWindowMenu(); });


    d->viewManager = new KisViewManager(this, actionCollection());
    KConfigGroup group( KSharedConfig::openConfig(), "theme");
    d->themeManager = new Digikam::ThemeManager(group.readEntry("Theme", "Krita dark"), this);

    d->windowStateConfig = KSharedConfig::openConfig()->group("MainWindow");

    setAcceptDrops(true);
    setStandardToolBarMenuEnabled(true);
    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);
    setDockNestingEnabled(true);

    qApp->setStartDragDistance(25);     // 25 px is a distance that works well for Tablet and Mouse events

#ifdef Q_OS_OSX
    setUnifiedTitleAndToolBarOnMac(true);
#endif

    connect(this, SIGNAL(restoringDone()), this, SLOT(forceDockTabFonts()));
    connect(this, SIGNAL(themeChanged()), d->viewManager, SLOT(updateIcons()));
    connect(KisPart::instance(), SIGNAL(documentClosed(QString)), SLOT(updateWindowMenu()));
    connect(KisPart::instance(), SIGNAL(documentOpened(QString)), SLOT(updateWindowMenu()));
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), this, SLOT(configChanged()));

    actionCollection()->addAssociatedWidget(this);
    KoPluginLoader::instance()->load("Krita/ViewPlugin", "Type == 'Service' and ([X-Krita-Version] == 28)", KoPluginLoader::PluginsConfig(), d->viewManager, false);

    // Load the per-application plugins (Right now, only Python) We do this only once, when the first mainwindow is being created.
    KoPluginLoader::instance()->load("Krita/ApplicationPlugin", "Type == 'Service' and ([X-Krita-Version] == 28)", KoPluginLoader::PluginsConfig(), qApp, true);

    KoToolBoxFactory toolBoxFactory;
    QDockWidget *toolbox = createDockWidget(&toolBoxFactory);
    toolbox->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);

    KisConfig cfg(true);
    if (cfg.toolOptionsInDocker()) {
        ToolDockerFactory toolDockerFactory;
        d->toolOptionsDocker = qobject_cast<KoToolDocker*>(createDockWidget(&toolDockerFactory));
        d->toolOptionsDocker->toggleViewAction()->setEnabled(true);
    }

    QMap<QString, QAction*> dockwidgetActions;
    dockwidgetActions[toolbox->toggleViewAction()->text()] = toolbox->toggleViewAction();
    Q_FOREACH (const QString & docker, KoDockRegistry::instance()->keys()) {
        KoDockFactoryBase *factory = KoDockRegistry::instance()->value(docker);
        QDockWidget *dw = createDockWidget(factory);
        dockwidgetActions[dw->toggleViewAction()->text()] = dw->toggleViewAction();
    }

    if (d->toolOptionsDocker) {
        dockwidgetActions[d->toolOptionsDocker->toggleViewAction()->text()] = d->toolOptionsDocker->toggleViewAction();
    }
    connect(KoToolManager::instance(), SIGNAL(toolOptionWidgetsChanged(KoCanvasController*,QList<QPointer<QWidget> >)), this, SLOT(newOptionWidgets(KoCanvasController*,QList<QPointer<QWidget> >)));

    Q_FOREACH (QString title, dockwidgetActions.keys()) {
        d->dockWidgetMenu->addAction(dockwidgetActions[title]);
    }

    Q_FOREACH (QDockWidget *wdg, dockWidgets()) {
        if ((wdg->features() & QDockWidget::DockWidgetClosable) == 0) {
            wdg->setVisible(true);
        }
    }

    Q_FOREACH (KoCanvasObserverBase* observer, canvasObservers()) {
        observer->setObservedCanvas(0);
        KisMainwindowObserver* mainwindowObserver = dynamic_cast<KisMainwindowObserver*>(observer);
        if (mainwindowObserver) {
            mainwindowObserver->setViewManager(d->viewManager);
        }
    }

    // Load all the actions from the tool plugins
    Q_FOREACH(KoToolFactoryBase *toolFactory, KoToolRegistry::instance()->values()) {
        toolFactory->createActions(actionCollection());
    }

    d->mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    d->mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    d->mdiArea->setTabPosition(QTabWidget::North);
    d->mdiArea->setTabsClosable(true);

    // Tab close button override
    // Windows just has a black X, and Ubuntu has a dark x that is hard to read
    // just switch this icon out for all OSs so it is easier to see
    d->mdiArea->setStyleSheet("QTabBar::close-button { image: url(:/pics/broken-preset.png) }");

    setCentralWidget(d->widgetStack);
    d->widgetStack->setCurrentIndex(0);

    connect(d->mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(subWindowActivated()));
    connect(d->windowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(setActiveSubWindow(QWidget*)));
    connect(d->documentMapper, SIGNAL(mapped(QObject*)), this, SLOT(newView(QObject*)));

    createActions();

    // the welcome screen needs to grab actions...so make sure this line goes after the createAction() so they exist
    d->welcomePage->setMainWindow(this);

    setAutoSaveSettings(d->windowStateConfig, false);

    subWindowActivated();
    updateWindowMenu();

    if (isHelpMenuEnabled() && !d->helpMenu) {
        // workaround for KHelpMenu (or rather KAboutData::applicationData()) internally
        // not using the Q*Application metadata ATM, which results e.g. in the bugreport wizard
        // not having the app version preset
        // fixed hopefully in KF5 5.22.0, patch pending
        QGuiApplication *app = qApp;
        KAboutData aboutData(app->applicationName(), app->applicationDisplayName(), app->applicationVersion());
        aboutData.setOrganizationDomain(app->organizationDomain().toUtf8());
        d->helpMenu = new KHelpMenu(this, aboutData, false);
        // workaround-less version:
        // d->helpMenu = new KHelpMenu(this, QString()/*unused*/, false);

        // The difference between using KActionCollection->addAction() is that
        // these actions do not get tied to the MainWindow.  What does this all do?
        KActionCollection *actions = d->viewManager->actionCollection();
        QAction *helpContentsAction = d->helpMenu->action(KHelpMenu::menuHelpContents);
        QAction *whatsThisAction = d->helpMenu->action(KHelpMenu::menuWhatsThis);
        QAction *reportBugAction = d->helpMenu->action(KHelpMenu::menuReportBug);
        QAction *switchLanguageAction = d->helpMenu->action(KHelpMenu::menuSwitchLanguage);
        QAction *aboutAppAction = d->helpMenu->action(KHelpMenu::menuAboutApp);
        QAction *aboutKdeAction = d->helpMenu->action(KHelpMenu::menuAboutKDE);

        if (helpContentsAction) {
            actions->addAction(helpContentsAction->objectName(), helpContentsAction);
        }
        if (whatsThisAction) {
            actions->addAction(whatsThisAction->objectName(), whatsThisAction);
        }
        if (reportBugAction) {
            actions->addAction(reportBugAction->objectName(), reportBugAction);
        }
        if (switchLanguageAction) {
            actions->addAction(switchLanguageAction->objectName(), switchLanguageAction);
        }
        if (aboutAppAction) {
            actions->addAction(aboutAppAction->objectName(), aboutAppAction);
        }
        if (aboutKdeAction) {
            actions->addAction(aboutKdeAction->objectName(), aboutKdeAction);
        }

        connect(d->helpMenu, SIGNAL(showAboutApplication()), SLOT(showAboutApplication()));
    }

    // KDE' libs 4''s help contents action is broken outside kde, for some reason... We can handle it just as easily ourselves
    QAction *helpAction = actionCollection()->action("help_contents");
    helpAction->disconnect();
    connect(helpAction, SIGNAL(triggered()), this, SLOT(showManual()));

#if 0
    //check for colliding shortcuts
    QSet<QKeySequence> existingShortcuts;
    Q_FOREACH (QAction* action, actionCollection()->actions()) {
        if(action->shortcut() == QKeySequence(0)) {
            continue;
        }
        dbgKrita << "shortcut " << action->text() << " " << action->shortcut();
        Q_ASSERT(!existingShortcuts.contains(action->shortcut()));
        existingShortcuts.insert(action->shortcut());
    }
#endif

    configChanged();

    // If we have customized the toolbars, load that first
    setLocalXMLFile(KoResourcePaths::locateLocal("data", "krita4.xmlgui"));
    setXMLFile(":/kxmlgui5/krita4.xmlgui");

    guiFactory()->addClient(this);

    // Create and plug toolbar list for Settings menu
    QList<QAction *> toolbarList;
    Q_FOREACH (QWidget* it, guiFactory()->containers("ToolBar")) {
        KToolBar * toolBar = ::qobject_cast<KToolBar *>(it);
        toolBar->setMovable(KisConfig(true).readEntry<bool>("LockAllDockerPanels", false));

        if (toolBar) {
            if (toolBar->objectName() == "BrushesAndStuff") {
                toolBar->setEnabled(false);
            }

            KToggleAction* act = new KToggleAction(i18n("Show %1 Toolbar", toolBar->windowTitle()), this);
            actionCollection()->addAction(toolBar->objectName().toUtf8(), act);
            act->setCheckedState(KGuiItem(i18n("Hide %1 Toolbar", toolBar->windowTitle())));
            connect(act, SIGNAL(toggled(bool)), this, SLOT(slotToolbarToggled(bool)));
            act->setChecked(!toolBar->isHidden());
            toolbarList.append(act);
        } else {
            warnUI << "Toolbar list contains a " << it->metaObject()->className() << " which is not a toolbar!";
        }
    }

    KToolBar::setToolBarsLocked(KisConfig(true).readEntry<bool>("LockAllDockerPanels", false));
    plugActionList("toolbarlist", toolbarList);
    d->toolbarList = toolbarList;

    applyToolBarLayout();

    d->viewManager->updateGUI();
    d->viewManager->updateIcons();

#ifdef Q_OS_WIN
    auto w = qApp->activeWindow();
    if (w) QWindowsWindowFunctions::setHasBorderInFullScreen(w->windowHandle(), true);
#endif

    QTimer::singleShot(1000, this, SLOT(checkSanity()));

    {
        using namespace std::placeholders; // For _1 placeholder
        std::function<void (int)> callback(
            std::bind(&KisMainWindow::switchTab, this, _1));

        d->tabSwitchCompressor.reset(
            new KisSignalCompressorWithParam<int>(500, callback, KisSignalCompressor::FIRST_INACTIVE));
    }


    if (cfg.readEntry("CanvasOnlyActive", false)) {
        QString currentWorkspace = cfg.readEntry<QString>("CurrentWorkspace", "Default");
        KoResourceServer<KisWorkspaceResource> * rserver = KisResourceServerProvider::instance()->workspaceServer();
        KisWorkspaceResource* workspace = rserver->resourceByName(currentWorkspace);
        if (workspace) {
            restoreWorkspace(workspace);
        }
        cfg.writeEntry("CanvasOnlyActive", false);
        menuBar()->setVisible(true);
    }


}

KisMainWindow::~KisMainWindow()
{
//    Q_FOREACH (QAction *ac, actionCollection()->actions()) {
//        QAction *action = qobject_cast<QAction*>(ac);
//        if (action) {
//        qDebug() << "<Action"
//                 << "\n\tname=" << action->objectName()
//                 << "\n\ticon=" << action->icon().name()
//                 << "\n\ttext="  << action->text().replace("&", "&amp;")
//                 << "\n\twhatsThis="  << action->whatsThis()
//                 << "\n\ttoolTip="  << action->toolTip().replace("<html>", "").replace("</html>", "")
//                 << "\n\ticonText="  << action->iconText().replace("&", "&amp;")
//                 << "\n\tshortcut="  << action->shortcut().toString()
//                 << "\n\tisCheckable="  << QString((action->isChecked() ? "true" : "false"))
//                 << "\n\tstatusTip=" << action->statusTip()
//                 << "\n/>\n"   ;
//        }
//        else {
//            dbgKrita << "Got a non-qaction:" << ac->objectName();
//        }
//    }

    // The doc and view might still exist (this is the case when closing the window)
    KisPart::instance()->removeMainWindow(this);

    delete d->viewManager;
    delete d;

}

QUuid KisMainWindow::id() const {
    return d->id;
}

void KisMainWindow::addView(KisView *view)
{
    if (d->activeView == view) return;

    if (d->activeView) {
        d->activeView->disconnect(this);
    }

    // register the newly created view in the input manager
    viewManager()->inputManager()->addTrackedCanvas(view->canvasBase());

    showView(view);
    updateCaption();
    emit restoringDone();

    if (d->activeView) {
        connect(d->activeView, SIGNAL(titleModified(QString,bool)), SLOT(slotDocumentTitleModified()));
        connect(d->viewManager->statusBar(), SIGNAL(memoryStatusUpdated()), this, SLOT(updateCaption()));
    }
}

void KisMainWindow::notifyChildViewDestroyed(KisView *view)
{
    viewManager()->inputManager()->removeTrackedCanvas(view->canvasBase());
    if (view->canvasBase() == viewManager()->canvasBase()) {
        viewManager()->setCurrentView(0);
    }
}


void KisMainWindow::showView(KisView *imageView)
{
    if (imageView && activeView() != imageView) {
        // XXX: find a better way to initialize this!
        imageView->setViewManager(d->viewManager);

        imageView->canvasBase()->setFavoriteResourceManager(d->viewManager->paintOpBox()->favoriteResourcesManager());
        imageView->slotLoadingFinished();

        QMdiSubWindow *subwin = d->mdiArea->addSubWindow(imageView);
        imageView->setSubWindow(subwin);
        subwin->setAttribute(Qt::WA_DeleteOnClose, true);
        connect(subwin, SIGNAL(destroyed()), SLOT(updateWindowMenu()));

        KisConfig cfg(true);
        subwin->setOption(QMdiSubWindow::RubberBandMove, cfg.readEntry<int>("mdi_rubberband", cfg.useOpenGL()));
        subwin->setOption(QMdiSubWindow::RubberBandResize, cfg.readEntry<int>("mdi_rubberband", cfg.useOpenGL()));
        subwin->setWindowIcon(qApp->windowIcon());

        /**
         * Hack alert!
         *
         * Here we explicitly request KoToolManager to emit all the tool
         * activation signals, to reinitialize the tool options docker.
         *
         * That is needed due to a design flaw we have in the
         * initialization procedure.  The tool in the KoToolManager is
         * initialized in KisView::setViewManager() calls, which
         * happens early enough. During this call the tool manager
         * requests KoCanvasControllerWidget to emit the signal to
         * update the widgets in the tool docker. *But* at that moment
         * of time the view is not yet connected to the main window,
         * because it happens in KisViewManager::setCurrentView a bit
         * later. This fact makes the widgets updating signals be lost
         * and never reach the tool docker.
         *
         * So here we just explicitly call the tool activation stub.
         */

        KoToolManager::instance()->initializeCurrentToolForCanvas();

        if (d->mdiArea->subWindowList().size() == 1) {
            imageView->showMaximized();
        }
        else {
            imageView->show();
        }

        // No, no, no: do not try to call this _before_ the show() has
        // been called on the view; only when that has happened is the
        // opengl context active, and very bad things happen if we tell
        // the dockers to update themselves with a view if the opengl
        // context is not active.
        setActiveView(imageView);

        updateWindowMenu();
        updateCaption();
    }
}

void KisMainWindow::slotPreferences()
{
    if (KisDlgPreferences::editPreferences()) {
        KisConfigNotifier::instance()->notifyConfigChanged();
        KisConfigNotifier::instance()->notifyPixelGridModeChanged();
        KisImageConfigNotifier::instance()->notifyConfigChanged();

        // XXX: should this be changed for the views in other windows as well?
        Q_FOREACH (QPointer<KisView> koview, KisPart::instance()->views()) {
            KisViewManager *view = qobject_cast<KisViewManager*>(koview);
            if (view) {
                // Update the settings for all nodes -- they don't query
                // KisConfig directly because they need the settings during
                // compositing, and they don't connect to the config notifier
                // because nodes are not QObjects (because only one base class
                // can be a QObject).
                KisNode* node = dynamic_cast<KisNode*>(view->image()->rootLayer().data());
                node->updateSettings();
            }

        }

        d->viewManager->showHideScrollbars();
    }
}

void KisMainWindow::slotThemeChanged()
{
    // save theme changes instantly
    KConfigGroup group( KSharedConfig::openConfig(), "theme");
    group.writeEntry("Theme", d->themeManager->currentThemeName());

    // reload action icons!
    Q_FOREACH (QAction *action, actionCollection()->actions()) {
        KisIconUtils::updateIcon(action);
    }

    emit themeChanged();
}

void KisMainWindow::updateReloadFileAction(KisDocument *doc)
{
    Q_UNUSED(doc);
//    d->reloadFile->setEnabled(doc && !doc->url().isEmpty());
}

void KisMainWindow::setReadWrite(bool readwrite)
{
    d->saveAction->setEnabled(readwrite);
    d->importFile->setEnabled(readwrite);
    d->readOnly = !readwrite;
    updateCaption();
}

void KisMainWindow::addRecentURL(const QUrl &url)
{
    // Add entry to recent documents list
    // (call coming from KisDocument because it must work with cmd line, template dlg, file/open, etc.)
    if (!url.isEmpty()) {
        bool ok = true;
        if (url.isLocalFile()) {
            QString path = url.adjusted(QUrl::StripTrailingSlash).toLocalFile();
            const QStringList tmpDirs = KoResourcePaths::resourceDirs("tmp");
            for (QStringList::ConstIterator it = tmpDirs.begin() ; ok && it != tmpDirs.end() ; ++it) {
                if (path.contains(*it)) {
                    ok = false; // it's in the tmp resource
                }
            }

            const QStringList templateDirs = KoResourcePaths::findDirs("templates");
            for (QStringList::ConstIterator it = templateDirs.begin() ; ok && it != templateDirs.end() ; ++it) {
                if (path.contains(*it)) {
                    ok = false; // it's in the templates directory.
                    break;
                }
            }
        }
        if (ok) {
            d->recentFiles->addUrl(url);
        }
        saveRecentFiles();

    }
}

void KisMainWindow::saveRecentFiles()
{
    // Save list of recent files
    KSharedConfigPtr config =  KSharedConfig::openConfig();
    d->recentFiles->saveEntries(config->group("RecentFiles"));
    config->sync();

    // Tell all windows to reload their list, after saving
    // Doesn't work multi-process, but it's a start
    Q_FOREACH (KisMainWindow *mw, KisPart::instance()->mainWindows()) {
        if (mw != this) {
            mw->reloadRecentFileList();
        }
    }
}

QList<QUrl> KisMainWindow::recentFilesUrls()
{
    return d->recentFiles->urls();
}

void KisMainWindow::clearRecentFiles()
{
    d->recentFiles->clear();
}


void KisMainWindow::reloadRecentFileList()
{
    d->recentFiles->loadEntries(KSharedConfig::openConfig()->group("RecentFiles"));
}



void KisMainWindow::updateCaption()
{
    if (!d->mdiArea->activeSubWindow()) {
        updateCaption(QString(), false);
    }
    else if (d->activeView && d->activeView->document() && d->activeView->image()){
        KisDocument *doc = d->activeView->document();

        QString caption(doc->caption());
        if (d->readOnly) {
            caption += " [" + i18n("Write Protected") + "] ";
        }

        if (doc->isRecovered()) {
            caption += " [" + i18n("Recovered") + "] ";
        }

        // show the file size for the document
        KisMemoryStatisticsServer::Statistics m_fileSizeStats = KisMemoryStatisticsServer::instance()->fetchMemoryStatistics(d->activeView ? d->activeView->image() : 0);

        if (m_fileSizeStats.imageSize) {
            caption += QString(" (").append( KFormat().formatByteSize(m_fileSizeStats.imageSize)).append( ")");
        }

        d->activeView->setWindowTitle(caption);
        d->activeView->setWindowModified(doc->isModified());

        updateCaption(caption, doc->isModified());

        if (!doc->url().fileName().isEmpty()) {
            d->saveAction->setToolTip(i18n("Save as %1", doc->url().fileName()));
        }
        else {
            d->saveAction->setToolTip(i18n("Save"));
        }
    }
}

void KisMainWindow::updateCaption(const QString & caption, bool mod)
{
    dbgUI << "KisMainWindow::updateCaption(" << caption << "," << mod << ")";
    QString versionString = KritaVersionWrapper::versionString(true);
#if defined(KRITA_ALPHA) || defined (KRITA_BETA) || defined (KRITA_RC)
    setCaption(QString("%1: %2").arg(versionString).arg(caption), mod);
    return;
#endif

    setCaption(caption, mod);
}


KisView *KisMainWindow::activeView() const
{
    if (d->activeView) {
        return d->activeView;
    }
    return 0;
}

bool KisMainWindow::openDocument(const QUrl &url, OpenFlags flags)
{
    if (!QFile(url.toLocalFile()).exists()) {
        if (!(flags & BatchMode)) {
            QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("The file %1 does not exist.", url.url()));
        }
        d->recentFiles->removeUrl(url); //remove the file from the recent-opened-file-list
        saveRecentFiles();
        return false;
    }
    return openDocumentInternal(url, flags);
}

bool KisMainWindow::openDocumentInternal(const QUrl &url, OpenFlags flags)
{
    if (!url.isLocalFile()) {
        qWarning() << "KisMainWindow::openDocumentInternal. Not a local file:" << url;
        return false;
    }

    KisDocument *newdoc = KisPart::instance()->createDocument();

    if (flags & BatchMode) {
        newdoc->setFileBatchMode(true);
    }

    d->firstTime = true;
    connect(newdoc, SIGNAL(completed()), this, SLOT(slotLoadCompleted()));
    connect(newdoc, SIGNAL(canceled(QString)), this, SLOT(slotLoadCanceled(QString)));

    KisDocument::OpenFlags openFlags = KisDocument::None;
    if (flags & RecoveryFile) {
        openFlags |= KisDocument::RecoveryFile;
    }

    bool openRet = !(flags & Import) ? newdoc->openUrl(url, openFlags) : newdoc->importDocument(url);


    if (!openRet) {
        delete newdoc;
        return false;
    }

    KisPart::instance()->addDocument(newdoc);
    updateReloadFileAction(newdoc);

    if (!QFileInfo(url.toLocalFile()).isWritable()) {
        setReadWrite(false);
    }
    return true;
}

void KisMainWindow::showDocument(KisDocument *document) {
    Q_FOREACH(QMdiSubWindow *subwindow, d->mdiArea->subWindowList()) {
        KisView *view = qobject_cast<KisView*>(subwindow->widget());
        KIS_SAFE_ASSERT_RECOVER_NOOP(view);

        if (view) {
            if (view->document() == document) {
                setActiveSubWindow(subwindow);
                return;
            }
        }
    }

    addViewAndNotifyLoadingCompleted(document);
}

KisView* KisMainWindow::addViewAndNotifyLoadingCompleted(KisDocument *document)
{
    showWelcomeScreen(false); // see workaround in function header

    KisView *view = KisPart::instance()->createView(document, resourceManager(), actionCollection(), this);
    addView(view);

    emit guiLoadingFinished();

    return view;
}

QStringList KisMainWindow::showOpenFileDialog(bool isImporting)
{
    KoFileDialog dialog(this, KoFileDialog::ImportFiles, "OpenDocument");
    dialog.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    dialog.setMimeTypeFilters(KisImportExportManager::supportedMimeTypes(KisImportExportManager::Import));
    dialog.setCaption(isImporting ? i18n("Import Images") : i18n("Open Images"));

    return dialog.filenames();
}

// Separate from openDocument to handle async loading (remote URLs)
void KisMainWindow::slotLoadCompleted()
{
    KisDocument *newdoc = qobject_cast<KisDocument*>(sender());
    if (newdoc && newdoc->image()) {
        addViewAndNotifyLoadingCompleted(newdoc);

        disconnect(newdoc, SIGNAL(completed()), this, SLOT(slotLoadCompleted()));
        disconnect(newdoc, SIGNAL(canceled(QString)), this, SLOT(slotLoadCanceled(QString)));

        emit loadCompleted();
    }
}

void KisMainWindow::slotLoadCanceled(const QString & errMsg)
{
    dbgUI << "KisMainWindow::slotLoadCanceled";
    if (!errMsg.isEmpty())   // empty when canceled by user
        QMessageBox::critical(this, i18nc("@title:window", "Krita"), errMsg);
    // ... can't delete the document, it's the one who emitted the signal...

    KisDocument* doc = qobject_cast<KisDocument*>(sender());
    Q_ASSERT(doc);
    disconnect(doc, SIGNAL(completed()), this, SLOT(slotLoadCompleted()));
    disconnect(doc, SIGNAL(canceled(QString)), this, SLOT(slotLoadCanceled(QString)));
}

void KisMainWindow::slotSaveCanceled(const QString &errMsg)
{
    dbgUI << "KisMainWindow::slotSaveCanceled";
    if (!errMsg.isEmpty())   // empty when canceled by user
        QMessageBox::critical(this, i18nc("@title:window", "Krita"), errMsg);
    slotSaveCompleted();
}

void KisMainWindow::slotSaveCompleted()
{
    dbgUI << "KisMainWindow::slotSaveCompleted";
    KisDocument* doc = qobject_cast<KisDocument*>(sender());
    Q_ASSERT(doc);
    disconnect(doc, SIGNAL(completed()), this, SLOT(slotSaveCompleted()));
    disconnect(doc, SIGNAL(canceled(QString)), this, SLOT(slotSaveCanceled(QString)));

    if (d->deferredClosingEvent) {
        KXmlGuiWindow::closeEvent(d->deferredClosingEvent);
    }
}

bool KisMainWindow::hackIsSaving() const
{
    StdLockableWrapper<QMutex> wrapper(&d->savingEntryMutex);
    std::unique_lock<StdLockableWrapper<QMutex>> l(wrapper, std::try_to_lock);
    return !l.owns_lock();
}

bool KisMainWindow::installBundle(const QString &fileName) const
{
    QFileInfo from(fileName);
    QFileInfo to(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/bundles/" + from.fileName());
    if (to.exists()) {
        QFile::remove(to.canonicalFilePath());
    }
    return QFile::copy(fileName, QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/bundles/" + from.fileName());
}

bool KisMainWindow::saveDocument(KisDocument *document, bool saveas, bool isExporting)
{
    if (!document) {
        return true;
    }

    /**
     * Make sure that we cannot enter this method twice!
     *
     * The lower level functions may call processEvents() so
     * double-entry is quite possible to achieve. Here we try to lock
     * the mutex, and if it is failed, just cancel saving.
     */
    StdLockableWrapper<QMutex> wrapper(&d->savingEntryMutex);
    std::unique_lock<StdLockableWrapper<QMutex>> l(wrapper, std::try_to_lock);
    if (!l.owns_lock()) return false;

    // no busy wait for saving because it is dangerous!
    KisDelayedSaveDialog dlg(document->image(), KisDelayedSaveDialog::SaveDialog, 0, this);
    dlg.blockIfImageIsBusy();

    if (dlg.result() == KisDelayedSaveDialog::Rejected) {
        return false;
    }
    else if (dlg.result() == KisDelayedSaveDialog::Ignored) {
        QMessageBox::critical(0,
                              i18nc("@title:window", "Krita"),
                              i18n("You are saving a file while the image is "
                                   "still rendering. The saved file may be "
                                   "incomplete or corrupted.\n\n"
                                   "Please select a location where the original "
                                   "file will not be overridden!"));


        saveas = true;
    }

    if (document->isRecovered()) {
        saveas = true;
    }

    if (document->url().isEmpty()) {
        saveas = true;
    }

    connect(document, SIGNAL(completed()), this, SLOT(slotSaveCompleted()));
    connect(document, SIGNAL(canceled(QString)), this, SLOT(slotSaveCanceled(QString)));

    QByteArray nativeFormat = document->nativeFormatMimeType();
    QByteArray oldMimeFormat = document->mimeType();

    QUrl suggestedURL = document->url();

    QStringList mimeFilter = KisImportExportManager::supportedMimeTypes(KisImportExportManager::Export);

    mimeFilter = KisImportExportManager::supportedMimeTypes(KisImportExportManager::Export);
    if (!mimeFilter.contains(oldMimeFormat)) {
        dbgUI << "KisMainWindow::saveDocument no export filter for" << oldMimeFormat;

        // --- don't setOutputMimeType in case the user cancels the Save As
        // dialog and then tries to just plain Save ---

        // suggest a different filename extension (yes, we fortunately don't all live in a world of magic :))
        QString suggestedFilename = QFileInfo(suggestedURL.toLocalFile()).baseName();

        if (!suggestedFilename.isEmpty()) {  // ".kra" looks strange for a name
            suggestedFilename = suggestedFilename + "." + KisMimeDatabase::suffixesForMimeType(KIS_MIME_TYPE).first();
            suggestedURL = suggestedURL.adjusted(QUrl::RemoveFilename);
            suggestedURL.setPath(suggestedURL.path() + suggestedFilename);
        }

        // force the user to choose outputMimeType
        saveas = true;
    }

    bool ret = false;

    if (document->url().isEmpty() || isExporting || saveas) {
        // if you're just File/Save As'ing to change filter options you
        // don't want to be reminded about overwriting files etc.
        bool justChangingFilterOptions = false;

        KoFileDialog dialog(this, KoFileDialog::SaveFile, "SaveAs");
        dialog.setCaption(isExporting ? i18n("Exporting") : i18n("Saving As"));

        //qDebug() << ">>>>>" << isExporting << d->lastExportLocation << d->lastExportedFormat << QString::fromLatin1(document->mimeType());

        if (isExporting && !d->lastExportLocation.isEmpty()) {

            // Use the location where we last exported to, if it's set, as the opening location for the file dialog
            QString proposedPath = QFileInfo(d->lastExportLocation).absolutePath();
            // If the document doesn't have a filename yet, use the title
            QString proposedFileName = suggestedURL.isEmpty() ? document->documentInfo()->aboutInfo("title") :  QFileInfo(suggestedURL.toLocalFile()).baseName();
            // Use the last mimetype we exported to by default
            QString proposedMimeType =  d->lastExportedFormat.isEmpty() ? "" : d->lastExportedFormat;
            QString proposedExtension = KisMimeDatabase::suffixesForMimeType(proposedMimeType).first().remove("*,");

            // Set the default dir: this overrides the one loaded from the config file, since we're exporting and the lastExportLocation is not empty
            dialog.setDefaultDir(proposedPath + "/" + proposedFileName + "." + proposedExtension, true);
            dialog.setMimeTypeFilters(mimeFilter, proposedMimeType);
        }
        else {
            // Get the last used location for saving
            KConfigGroup group =  KSharedConfig::openConfig()->group("File Dialogs");
            QString proposedPath = group.readEntry("SaveAs", "");
            // if that is empty, get the last used location for loading
            if (proposedPath.isEmpty()) {
                proposedPath = group.readEntry("OpenDocument", "");
            }
            // If that is empty, too, use the Pictures location.
            if (proposedPath.isEmpty()) {
                proposedPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
            }
            // But only use that if the suggestedUrl, that is, the document's own url is empty, otherwise
            // open the location where the document currently is.
            dialog.setDefaultDir(suggestedURL.isEmpty() ? proposedPath : suggestedURL.toLocalFile(), true);

            // If exporting, default to all supported file types if user is exporting
            QByteArray default_mime_type = "";
            if (!isExporting) {
                // otherwise use the document's mimetype, or if that is empty, kra, which is the savest.
                default_mime_type = document->mimeType().isEmpty() ? nativeFormat : document->mimeType();
            }
            dialog.setMimeTypeFilters(mimeFilter, QString::fromLatin1(default_mime_type));
        }

        QUrl newURL = QUrl::fromUserInput(dialog.filename());

        if (newURL.isLocalFile()) {
            QString fn = newURL.toLocalFile();
            if (QFileInfo(fn).completeSuffix().isEmpty()) {
                fn.append(KisMimeDatabase::suffixesForMimeType(nativeFormat).first());
                newURL = QUrl::fromLocalFile(fn);
            }
        }

        if (document->documentInfo()->aboutInfo("title") == i18n("Unnamed")) {
            QString fn = newURL.toLocalFile();
            QFileInfo info(fn);
            document->documentInfo()->setAboutInfo("title", info.baseName());
        }

        QByteArray outputFormat = nativeFormat;

        QString outputFormatString = KisMimeDatabase::mimeTypeForFile(newURL.toLocalFile(), false);
        outputFormat = outputFormatString.toLatin1();


        if (!isExporting) {
            justChangingFilterOptions = (newURL == document->url()) && (outputFormat == document->mimeType());
        }
        else {
            QString path = QFileInfo(d->lastExportLocation).absolutePath();
            QString filename = QFileInfo(document->url().toLocalFile()).baseName();
            justChangingFilterOptions = (QFileInfo(newURL.toLocalFile()).absolutePath() == path)
                    && (QFileInfo(newURL.toLocalFile()).baseName() == filename)
                    && (outputFormat == d->lastExportedFormat);
        }

        bool bOk = true;
        if (newURL.isEmpty()) {
            bOk = false;
        }

        if (bOk) {
            bool wantToSave = true;

            // don't change this line unless you know what you're doing :)
            if (!justChangingFilterOptions) {
                if (!document->isNativeFormat(outputFormat))
                    wantToSave = true;
            }

            if (wantToSave) {
                if (!isExporting) {  // Save As
                    ret = document->saveAs(newURL, outputFormat, true);
                    if (ret) {
                        dbgUI << "Successful Save As!";
                        KisPart::instance()->addRecentURLToAllMainWindows(newURL);
                        setReadWrite(true);
                    } else {
                        dbgUI << "Failed Save As!";
                    }
                }
                else { // Export
                    ret = document->exportDocument(newURL, outputFormat);

                    if (ret) {
                        d->lastExportLocation = newURL.toLocalFile();
                        d->lastExportedFormat = outputFormat;
                    }
                }

            }   // if (wantToSave)  {
            else
                ret = false;
        }   // if (bOk) {
        else
            ret = false;
    } else { // saving
        // We cannot "export" into the currently
        // opened document. We are not Gimp.
        KIS_ASSERT_RECOVER_NOOP(!isExporting);

        // be sure document has the correct outputMimeType!
        if (document->isModified()) {
            ret = document->save(true, 0);
        }

        if (!ret) {
            dbgUI << "Failed Save!";
        }
    }

    updateReloadFileAction(document);
    updateCaption();

    return ret;
}

void KisMainWindow::undo()
{
    if (activeView()) {
        activeView()->document()->undoStack()->undo();
    }
}

void KisMainWindow::redo()
{
    if (activeView()) {
        activeView()->document()->undoStack()->redo();
    }
}

void KisMainWindow::closeEvent(QCloseEvent *e)
{
    if (!KisPart::instance()->closingSession()) {
        QAction *action= d->viewManager->actionCollection()->action("view_show_canvas_only");
        if ((action) && (action->isChecked())) {
            action->setChecked(false);
        }

        // Save session when last window is closed
        if (KisPart::instance()->mainwindowCount() == 1) {
            bool closeAllowed = KisPart::instance()->closeSession();

            if (!closeAllowed) {
                e->setAccepted(false);
                return;
            }
        }
    }

    d->mdiArea->closeAllSubWindows();

    QList<QMdiSubWindow*> childrenList = d->mdiArea->subWindowList();

    if (childrenList.isEmpty()) {
        d->deferredClosingEvent = e;
        saveWindowState(true);
    } else {
        e->setAccepted(false);
    }
}

void KisMainWindow::saveWindowSettings()
{
    KSharedConfigPtr config =  KSharedConfig::openConfig();

    if (d->windowSizeDirty ) {
        dbgUI << "KisMainWindow::saveWindowSettings";
        KConfigGroup group = d->windowStateConfig;
        KWindowConfig::saveWindowSize(windowHandle(), group);
        config->sync();
        d->windowSizeDirty = false;
    }

    if (!d->activeView || d->activeView->document()) {

        // Save toolbar position into the config file of the app, under the doc's component name
        KConfigGroup group = d->windowStateConfig;
        saveMainWindowSettings(group);

        // Save collapsible state of dock widgets
        for (QMap<QString, QDockWidget*>::const_iterator i = d->dockWidgetsMap.constBegin();
             i != d->dockWidgetsMap.constEnd(); ++i) {
            if (i.value()->widget()) {
                KConfigGroup dockGroup = group.group(QString("DockWidget ") + i.key());
                dockGroup.writeEntry("Collapsed", i.value()->widget()->isHidden());
                dockGroup.writeEntry("Locked", i.value()->property("Locked").toBool());
                dockGroup.writeEntry("DockArea", (int) dockWidgetArea(i.value()));
                dockGroup.writeEntry("xPosition", (int) i.value()->widget()->x());
                dockGroup.writeEntry("yPosition", (int) i.value()->widget()->y());

                dockGroup.writeEntry("width", (int) i.value()->widget()->width());
                dockGroup.writeEntry("height", (int) i.value()->widget()->height());
            }
        }

    }

     KSharedConfig::openConfig()->sync();
    resetAutoSaveSettings(); // Don't let KMainWindow override the good stuff we wrote down

}

void KisMainWindow::resizeEvent(QResizeEvent * e)
{
    d->windowSizeDirty = true;
    KXmlGuiWindow::resizeEvent(e);
}

void KisMainWindow::setActiveView(KisView* view)
{
    d->activeView = view;
    updateCaption();

    if (d->undoActionsUpdateManager) {
        d->undoActionsUpdateManager->setCurrentDocument(view ? view->document() : 0);
    }

    d->viewManager->setCurrentView(view);

    KisWindowLayoutManager::instance()->activeDocumentChanged(view->document());
}

void KisMainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    d->welcomePage->showDropAreaIndicator(true);


    if (event->mimeData()->hasUrls() ||
        event->mimeData()->hasFormat("application/x-krita-node") ||
        event->mimeData()->hasFormat("application/x-qt-image")) {

        event->accept();
    }
}

void KisMainWindow::dropEvent(QDropEvent *event)
{
    d->welcomePage->showDropAreaIndicator(false);

    if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() > 0) {
        Q_FOREACH (const QUrl &url, event->mimeData()->urls()) {
            if (url.toLocalFile().endsWith(".bundle")) {
                bool r = installBundle(url.toLocalFile());
                if (!r) {
                    qWarning() << "Could not install bundle" << url.toLocalFile();
                }
            }
            else {
                openDocument(url, None);
            }
        }
    }
}

void KisMainWindow::dragMoveEvent(QDragMoveEvent * event)
{
    QTabBar *tabBar = d->findTabBarHACK();

    if (!tabBar && d->mdiArea->viewMode() == QMdiArea::TabbedView) {
        qWarning() << "WARNING!!! Cannot find QTabBar in the main window! Looks like Qt has changed behavior. Drag & Drop between multiple tabs might not work properly (tabs will not switch automatically)!";
    }

    if (tabBar && tabBar->isVisible()) {
        QPoint pos = tabBar->mapFromGlobal(mapToGlobal(event->pos()));
        if (tabBar->rect().contains(pos)) {
            const int tabIndex = tabBar->tabAt(pos);

            if (tabIndex >= 0 && tabBar->currentIndex() != tabIndex) {
                d->tabSwitchCompressor->start(tabIndex);
            }
        } else if (d->tabSwitchCompressor->isActive()) {
            d->tabSwitchCompressor->stop();
        }
    }
}

void KisMainWindow::dragLeaveEvent(QDragLeaveEvent * /*event*/)
{
        d->welcomePage->showDropAreaIndicator(false);

    if (d->tabSwitchCompressor->isActive()) {
        d->tabSwitchCompressor->stop();
    }
}


void KisMainWindow::switchTab(int index)
{
    QTabBar *tabBar = d->findTabBarHACK();
    if (!tabBar) return;

    tabBar->setCurrentIndex(index);
}

void KisMainWindow::showWelcomeScreen(bool show)
{
     d->widgetStack->setCurrentIndex(!show);
}

void KisMainWindow::slotFileNew()
{
    const QStringList mimeFilter = KisImportExportManager::supportedMimeTypes(KisImportExportManager::Import);

    KisOpenPane *startupWidget = new KisOpenPane(this, mimeFilter, QStringLiteral("templates/"));
    startupWidget->setWindowModality(Qt::WindowModal);
    startupWidget->setWindowTitle(i18n("Create new document"));


    KisConfig cfg(true);

    int w = cfg.defImageWidth();
    int h = cfg.defImageHeight();
    const double resolution = cfg.defImageResolution();
    const QString colorModel = cfg.defColorModel();
    const QString colorDepth = cfg.defaultColorDepth();
    const QString colorProfile = cfg.defColorProfile();


    CustomDocumentWidgetItem item;
    item.widget = new KisCustomImageWidget(startupWidget,
                                           w,
                                           h,
                                           resolution,
                                           colorModel,
                                           colorDepth,
                                           colorProfile,
                                           i18n("Unnamed"));

    item.icon = "document-new";

    startupWidget->addCustomDocumentWidget(item.widget, item.title, item.icon);

    QSize sz = KisClipboard::instance()->clipSize();
    if (sz.isValid() && sz.width() != 0 && sz.height() != 0) {
        w = sz.width();
        h = sz.height();
    }

    item.widget = new KisImageFromClipboard(startupWidget,
                                            w,
                                            h,
                                            resolution,
                                            colorModel,
                                            colorDepth,
                                            colorProfile,
                                            i18n("Unnamed"));

    item.title = i18n("Create from Clipboard");
    item.icon = "tab-new";

    startupWidget->addCustomDocumentWidget(item.widget, item.title, item.icon);

    // calls deleteLater
    connect(startupWidget, SIGNAL(documentSelected(KisDocument*)), KisPart::instance(), SLOT(startCustomDocument(KisDocument*)));
    // calls deleteLater
    connect(startupWidget, SIGNAL(openTemplate(QUrl)), KisPart::instance(), SLOT(openTemplate(QUrl)));

    startupWidget->exec();

    // Cancel calls deleteLater...

}

void KisMainWindow::slotImportFile()
{
    dbgUI << "slotImportFile()";
    slotFileOpen(true);
}


void KisMainWindow::slotFileOpen(bool isImporting)
{
    QStringList urls = showOpenFileDialog(isImporting);

    if (urls.isEmpty())
        return;

    Q_FOREACH (const QString& url, urls) {

        if (!url.isEmpty()) {
            OpenFlags flags = isImporting ? Import : None;
            bool res = openDocument(QUrl::fromLocalFile(url), flags);
            if (!res) {
                warnKrita << "Loading" << url << "failed";
            }
        }
    }
}

void KisMainWindow::slotFileOpenRecent(const QUrl &url)
{
    (void) openDocument(QUrl::fromLocalFile(url.toLocalFile()), None);
}

void KisMainWindow::slotFileSave()
{
    if (saveDocument(d->activeView->document(), false, false)) {
        emit documentSaved();
    }
}

void KisMainWindow::slotFileSaveAs()
{
    if (saveDocument(d->activeView->document(), true, false)) {
        emit documentSaved();
    }
}

void KisMainWindow::slotExportFile()
{
    if (saveDocument(d->activeView->document(), true, true)) {
        emit documentSaved();
    }
}

void KisMainWindow::slotShowSessionManager() {
    KisPart::instance()->showSessionManager();
}

KoCanvasResourceProvider *KisMainWindow::resourceManager() const
{
    return d->viewManager->canvasResourceProvider()->resourceManager();
}

int KisMainWindow::viewCount() const
{
    return d->mdiArea->subWindowList().size();
}

const KConfigGroup &KisMainWindow::windowStateConfig() const
{
    return d->windowStateConfig;
}

void KisMainWindow::saveWindowState(bool restoreNormalState)
{
    if (restoreNormalState) {
        QAction *showCanvasOnly = d->viewManager->actionCollection()->action("view_show_canvas_only");

        if (showCanvasOnly && showCanvasOnly->isChecked()) {
            showCanvasOnly->setChecked(false);
        }

        d->windowStateConfig.writeEntry("ko_geometry", saveGeometry().toBase64());
        d->windowStateConfig.writeEntry("State", saveState().toBase64());

        if (!d->dockerStateBeforeHiding.isEmpty()) {
            restoreState(d->dockerStateBeforeHiding);
        }

        statusBar()->setVisible(true);
        menuBar()->setVisible(true);

        saveWindowSettings();

    } else {
        saveMainWindowSettings(d->windowStateConfig);
    }

}

bool KisMainWindow::restoreWorkspaceState(const QByteArray &state)
{
    QByteArray oldState = saveState();

    // needed because otherwise the layout isn't correctly restored in some situations
    Q_FOREACH (QDockWidget *dock, dockWidgets()) {
        dock->toggleViewAction()->setEnabled(true);
        dock->hide();
    }

    bool success = KXmlGuiWindow::restoreState(state);

    if (!success) {
        KXmlGuiWindow::restoreState(oldState);
        return false;
    }

    return success;
}

bool KisMainWindow::restoreWorkspace(KisWorkspaceResource *workspace)
{
    bool success = restoreWorkspaceState(workspace->dockerState());

    if (activeKisView()) {
        activeKisView()->resourceProvider()->notifyLoadingWorkspace(workspace);
    }

    return success;
}

QByteArray KisMainWindow::borrowWorkspace(KisMainWindow *other)
{
    QByteArray currentWorkspace = saveState();

    if (!d->workspaceBorrowedBy.isNull()) {
        if (other->id() == d->workspaceBorrowedBy) {
            // We're swapping our original workspace back
            d->workspaceBorrowedBy = QUuid();
            return currentWorkspace;
        } else {
            // Get our original workspace back before swapping with a third window
            KisMainWindow *borrower = KisPart::instance()->windowById(d->workspaceBorrowedBy);
            if (borrower) {
                QByteArray originalLayout = borrower->borrowWorkspace(this);
                borrower->restoreWorkspaceState(currentWorkspace);

                d->workspaceBorrowedBy = other->id();
                return originalLayout;
            }
        }
    }

    d->workspaceBorrowedBy = other->id();
    return currentWorkspace;
}

void KisMainWindow::swapWorkspaces(KisMainWindow *a, KisMainWindow *b)
{
    QByteArray workspaceA = a->borrowWorkspace(b);
    QByteArray workspaceB = b->borrowWorkspace(a);

    a->restoreWorkspaceState(workspaceB);
    b->restoreWorkspaceState(workspaceA);
}

KisViewManager *KisMainWindow::viewManager() const
{
    return d->viewManager;
}

void KisMainWindow::slotDocumentInfo()
{
    if (!d->activeView->document())
        return;

    KoDocumentInfo *docInfo = d->activeView->document()->documentInfo();

    if (!docInfo)
        return;

    KoDocumentInfoDlg *dlg = d->activeView->document()->createDocumentInfoDialog(this, docInfo);

    if (dlg->exec()) {
        if (dlg->isDocumentSaved()) {
            d->activeView->document()->setModified(false);
        } else {
            d->activeView->document()->setModified(true);
        }
        d->activeView->document()->setTitleModified();
    }

    delete dlg;
}

bool KisMainWindow::slotFileCloseAll()
{
    Q_FOREACH (QMdiSubWindow *subwin, d->mdiArea->subWindowList()) {
        if (subwin) {
            if(!subwin->close())
                return false;
        }
    }

    updateCaption();
    return true;
}

void KisMainWindow::slotFileQuit()
{
    KisPart::instance()->closeSession();
}

void KisMainWindow::slotFilePrint()
{
    if (!activeView())
        return;
    KisPrintJob *printJob = activeView()->createPrintJob();
    if (printJob == 0)
        return;
    applyDefaultSettings(printJob->printer());
    QPrintDialog *printDialog = activeView()->createPrintDialog( printJob, this );
    if (printDialog && printDialog->exec() == QDialog::Accepted) {
        printJob->printer().setPageMargins(0.0, 0.0, 0.0, 0.0, QPrinter::Point);
        printJob->printer().setPaperSize(QSizeF(activeView()->image()->width() / (72.0 * activeView()->image()->xRes()),
                                                activeView()->image()->height()/ (72.0 * activeView()->image()->yRes())),
                                         QPrinter::Inch);
        printJob->startPrinting(KisPrintJob::DeleteWhenDone);
    }
    else {
        delete printJob;
    }
    delete printDialog;
}

void KisMainWindow::slotFilePrintPreview()
{
    if (!activeView())
        return;
    KisPrintJob *printJob = activeView()->createPrintJob();
    if (printJob == 0)
        return;

    /* Sets the startPrinting() slot to be blocking.
     The Qt print-preview dialog requires the printing to be completely blocking
     and only return when the full document has been printed.
     By default the KisPrintingDialog is non-blocking and
     multithreading, setting blocking to true will allow it to be used in the preview dialog */
    printJob->setProperty("blocking", true);
    QPrintPreviewDialog *preview = new QPrintPreviewDialog(&printJob->printer(), this);
    printJob->setParent(preview); // will take care of deleting the job
    connect(preview, SIGNAL(paintRequested(QPrinter*)), printJob, SLOT(startPrinting()));
    preview->exec();
    delete preview;
}

KisPrintJob* KisMainWindow::exportToPdf(QString pdfFileName)
{
    if (!activeView())
        return 0;

    if (!activeView()->document())
        return 0;

    KoPageLayout pageLayout;
    pageLayout.width = 0;
    pageLayout.height = 0;
    pageLayout.topMargin = 0;
    pageLayout.bottomMargin = 0;
    pageLayout.leftMargin = 0;
    pageLayout.rightMargin = 0;

    if (pdfFileName.isEmpty()) {
        KConfigGroup group =  KSharedConfig::openConfig()->group("File Dialogs");
        QString defaultDir = group.readEntry("SavePdfDialog");
        if (defaultDir.isEmpty())
            defaultDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        QUrl startUrl = QUrl::fromLocalFile(defaultDir);
        KisDocument* pDoc = d->activeView->document();
        /** if document has a file name, take file name and replace extension with .pdf */
        if (pDoc && pDoc->url().isValid()) {
            startUrl = pDoc->url();
            QString fileName = startUrl.toLocalFile();
            fileName = fileName.replace( QRegExp( "\\.\\w{2,5}$", Qt::CaseInsensitive ), ".pdf" );
            startUrl = startUrl.adjusted(QUrl::RemoveFilename);
            startUrl.setPath(startUrl.path() +  fileName );
        }

        QPointer<KoPageLayoutDialog> layoutDlg(new KoPageLayoutDialog(this, pageLayout));
        layoutDlg->setWindowModality(Qt::WindowModal);
        if (layoutDlg->exec() != QDialog::Accepted || !layoutDlg) {
            delete layoutDlg;
            return 0;
        }
        pageLayout = layoutDlg->pageLayout();
        delete layoutDlg;

        KoFileDialog dialog(this, KoFileDialog::SaveFile, "OpenDocument");
        dialog.setCaption(i18n("Export as PDF"));
        dialog.setDefaultDir(startUrl.toLocalFile());
        dialog.setMimeTypeFilters(QStringList() << "application/pdf");
        QUrl url = QUrl::fromUserInput(dialog.filename());

        pdfFileName = url.toLocalFile();
        if (pdfFileName.isEmpty())
            return 0;
    }

    KisPrintJob *printJob = activeView()->createPrintJob();
    if (printJob == 0)
        return 0;
    if (isHidden()) {
        printJob->setProperty("noprogressdialog", true);
    }

    applyDefaultSettings(printJob->printer());
    // TODO for remote files we have to first save locally and then upload.
    printJob->printer().setOutputFileName(pdfFileName);
    printJob->printer().setDocName(pdfFileName);
    printJob->printer().setColorMode(QPrinter::Color);

    if (pageLayout.format == KoPageFormat::CustomSize) {
        printJob->printer().setPaperSize(QSizeF(pageLayout.width, pageLayout.height), QPrinter::Millimeter);
    } else {
        printJob->printer().setPaperSize(KoPageFormat::printerPageSize(pageLayout.format));
    }

    printJob->printer().setPageMargins(pageLayout.leftMargin, pageLayout.topMargin, pageLayout.rightMargin, pageLayout.bottomMargin, QPrinter::Millimeter);

    switch (pageLayout.orientation) {
    case KoPageFormat::Portrait:
        printJob->printer().setOrientation(QPrinter::Portrait);
        break;
    case KoPageFormat::Landscape:
        printJob->printer().setOrientation(QPrinter::Landscape);
        break;
    }

    //before printing check if the printer can handle printing
    if (!printJob->canPrint()) {
        QMessageBox::critical(this, i18nc("@title:window", "Krita"), i18n("Cannot export to the specified file"));
    }

    printJob->startPrinting(KisPrintJob::DeleteWhenDone);
    return printJob;
}

void KisMainWindow::importAnimation()
{
    if (!activeView()) return;

    KisDocument *document = activeView()->document();
    if (!document) return;

    KisDlgImportImageSequence dlg(this, document);

    if (dlg.exec() == QDialog::Accepted) {
        QStringList files = dlg.files();
        int firstFrame = dlg.firstFrame();
        int step = dlg.step();

        KoUpdaterPtr updater =
            !document->fileBatchMode() ? viewManager()->createUnthreadedUpdater(i18n("Import frames")) : 0;
        KisAnimationImporter importer(document->image(), updater);
        KisImportExportFilter::ConversionStatus status = importer.import(files, firstFrame, step);

        if (status != KisImportExportFilter::OK && status != KisImportExportFilter::InternalError) {
            QString msg = KisImportExportFilter::conversionStatusString(status);

            if (!msg.isEmpty())
                QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("Could not finish import animation:\n%1", msg));
        }
        activeView()->canvasBase()->refetchDataFromImage();
    }
}

void KisMainWindow::slotConfigureToolbars()
{
    saveWindowState();
    KEditToolBar edit(factory(), this);
    connect(&edit, SIGNAL(newToolBarConfig()), this, SLOT(slotNewToolbarConfig()));
    (void) edit.exec();
    applyToolBarLayout();
}

void KisMainWindow::slotNewToolbarConfig()
{
    applyMainWindowSettings(d->windowStateConfig);

    KXMLGUIFactory *factory = guiFactory();
    Q_UNUSED(factory);

    // Check if there's an active view
    if (!d->activeView)
        return;

    plugActionList("toolbarlist", d->toolbarList);
    applyToolBarLayout();
}

void KisMainWindow::slotToolbarToggled(bool toggle)
{
    //dbgUI <<"KisMainWindow::slotToolbarToggled" << sender()->name() <<" toggle=" << true;
    // The action (sender) and the toolbar have the same name
    KToolBar * bar = toolBar(sender()->objectName());
    if (bar) {
        if (toggle) {
            bar->show();
        }
        else {
            bar->hide();
        }

        if (d->activeView && d->activeView->document()) {
            saveWindowState();
        }
    } else
        warnUI << "slotToolbarToggled : Toolbar " << sender()->objectName() << " not found!";
}

void KisMainWindow::viewFullscreen(bool fullScreen)
{
    KisConfig cfg(false);
    cfg.setFullscreenMode(fullScreen);

    if (fullScreen) {
        setWindowState(windowState() | Qt::WindowFullScreen);   // set
    } else {
        setWindowState(windowState() & ~Qt::WindowFullScreen);   // reset
    }
}

void KisMainWindow::setMaxRecentItems(uint _number)
{
    d->recentFiles->setMaxItems(_number);
}

void KisMainWindow::slotReloadFile()
{
    KisDocument* document = d->activeView->document();
    if (!document || document->url().isEmpty())
        return;

    if (document->isModified()) {
        bool ok = QMessageBox::question(this,
                                        i18nc("@title:window", "Krita"),
                                        i18n("You will lose all changes made since your last save\n"
                                             "Do you want to continue?"),
                                        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes;
        if (!ok)
            return;
    }

    QUrl url = document->url();

    saveWindowSettings();
    if (!document->reload()) {
        QMessageBox::critical(this, i18nc("@title:window", "Krita"), i18n("Error: Could not reload this document"));
    }

    return;

}

QDockWidget* KisMainWindow::createDockWidget(KoDockFactoryBase* factory)
{
    QDockWidget* dockWidget = 0;
    bool lockAllDockers = KisConfig(true).readEntry<bool>("LockAllDockerPanels", false);

    if (!d->dockWidgetsMap.contains(factory->id())) {
        dockWidget = factory->createDockWidget();

        // It is quite possible that a dock factory cannot create the dock; don't
        // do anything in that case.
        if (!dockWidget) {
            warnKrita << "Could not create docker for" << factory->id();
            return 0;
        }

        dockWidget->setFont(KoDockRegistry::dockFont());
        dockWidget->setObjectName(factory->id());
        dockWidget->setParent(this);
        if (lockAllDockers) {
            if (dockWidget->titleBarWidget()) {
                dockWidget->titleBarWidget()->setVisible(false);
            }
            dockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
        }
        if (dockWidget->widget() && dockWidget->widget()->layout())
            dockWidget->widget()->layout()->setContentsMargins(1, 1, 1, 1);

        Qt::DockWidgetArea side = Qt::RightDockWidgetArea;
        bool visible = true;

        switch (factory->defaultDockPosition()) {
        case KoDockFactoryBase::DockTornOff:
            dockWidget->setFloating(true); // position nicely?
            break;
        case KoDockFactoryBase::DockTop:
            side = Qt::TopDockWidgetArea; break;
        case KoDockFactoryBase::DockLeft:
            side = Qt::LeftDockWidgetArea; break;
        case KoDockFactoryBase::DockBottom:
            side = Qt::BottomDockWidgetArea; break;
        case KoDockFactoryBase::DockRight:
            side = Qt::RightDockWidgetArea; break;
        case KoDockFactoryBase::DockMinimized:
        default:
            side = Qt::RightDockWidgetArea;
            visible = false;
        }

        KConfigGroup group = d->windowStateConfig.group("DockWidget " + factory->id());
        side = static_cast<Qt::DockWidgetArea>(group.readEntry("DockArea", static_cast<int>(side)));
        if (side == Qt::NoDockWidgetArea) side = Qt::RightDockWidgetArea;

        addDockWidget(side, dockWidget);
        if (!visible) {
            dockWidget->hide();
        }

        d->dockWidgetsMap.insert(factory->id(), dockWidget);
    }
    else {
        dockWidget = d->dockWidgetsMap[factory->id()];
    }

#ifdef Q_OS_OSX
    dockWidget->setAttribute(Qt::WA_MacSmallSize, true);
#endif
    dockWidget->setFont(KoDockRegistry::dockFont());

    connect(dockWidget, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(forceDockTabFonts()));

    return dockWidget;
}

void KisMainWindow::forceDockTabFonts()
{
    Q_FOREACH (QObject *child, children()) {
        if (child->inherits("QTabBar")) {
            ((QTabBar *)child)->setFont(KoDockRegistry::dockFont());
        }
    }
}

QList<QDockWidget*> KisMainWindow::dockWidgets() const
{
    return d->dockWidgetsMap.values();
}

QDockWidget* KisMainWindow::dockWidget(const QString &id)
{
    if (!d->dockWidgetsMap.contains(id)) return 0;
    return d->dockWidgetsMap[id];
}

QList<KoCanvasObserverBase*> KisMainWindow::canvasObservers() const
{
    QList<KoCanvasObserverBase*> observers;

    Q_FOREACH (QDockWidget *docker, dockWidgets()) {
        KoCanvasObserverBase *observer = dynamic_cast<KoCanvasObserverBase*>(docker);
        if (observer) {
            observers << observer;
        }
        else {
            warnKrita << docker << "is not a canvas observer";
        }
    }
    return observers;
}


void KisMainWindow::toggleDockersVisibility(bool visible)
{
    if (!visible) {
        d->dockerStateBeforeHiding = saveState();

        Q_FOREACH (QObject* widget, children()) {
            if (widget->inherits("QDockWidget")) {
                QDockWidget* dw = static_cast<QDockWidget*>(widget);
                if (dw->isVisible()) {
                    dw->hide();
                }
            }
        }
    }
    else {
        restoreState(d->dockerStateBeforeHiding);
    }
}

void KisMainWindow::slotDocumentTitleModified()
{
    updateCaption();
    updateReloadFileAction(d->activeView ? d->activeView->document() : 0);
}


void KisMainWindow::subWindowActivated()
{
    bool enabled = (activeKisView() != 0);

    d->mdiCascade->setEnabled(enabled);
    d->mdiNextWindow->setEnabled(enabled);
    d->mdiPreviousWindow->setEnabled(enabled);
    d->mdiTile->setEnabled(enabled);
    d->close->setEnabled(enabled);
    d->closeAll->setEnabled(enabled);

    setActiveSubWindow(d->mdiArea->activeSubWindow());
    Q_FOREACH (QToolBar *tb, toolBars()) {
        if (tb->objectName() == "BrushesAndStuff") {
            tb->setEnabled(enabled);
        }
    }

    /**
     * Qt has a weirdness, it has hardcoded shortcuts added to an action
     * in the window menu. We need to reset the shortcuts for that menu
     * to nothing, otherwise the shortcuts cannot be made configurable.
     *
     * See: https://bugs.kde.org/show_bug.cgi?id=352205
     *      https://bugs.kde.org/show_bug.cgi?id=375524
     *      https://bugs.kde.org/show_bug.cgi?id=398729
     */
    QMdiSubWindow *subWindow = d->mdiArea->currentSubWindow();
    if (subWindow) {
        QMenu *menu = subWindow->systemMenu();
        if (menu && menu->actions().size() == 8) {
            Q_FOREACH (QAction *action, menu->actions()) {
                action->setShortcut(QKeySequence());

            }
            menu->actions().last()->deleteLater();
        }
    }

    updateCaption();
    d->actionManager()->updateGUI();
}

void KisMainWindow::windowFocused()
{
    /**
     * Notify selection manager so that it could update selection mask overlay
     */
    if (viewManager() && viewManager()->selectionManager()) {
        viewManager()->selectionManager()->selectionChanged();
    }

    KisPart *kisPart = KisPart::instance();
    KisWindowLayoutManager *layoutManager = KisWindowLayoutManager::instance();
    if (!layoutManager->primaryWorkspaceFollowsFocus()) return;

    QUuid primary = layoutManager->primaryWindowId();
    if (primary.isNull()) return;

    if (d->id == primary) {
        if (!d->workspaceBorrowedBy.isNull()) {
            KisMainWindow *borrower = kisPart->windowById(d->workspaceBorrowedBy);
            if (!borrower) return;
            swapWorkspaces(this, borrower);
        }
    } else {
        if (d->workspaceBorrowedBy == primary) return;

        KisMainWindow *primaryWindow = kisPart->windowById(primary);
        if (!primaryWindow) return;
        swapWorkspaces(this, primaryWindow);
    }
}


void KisMainWindow::updateWindowMenu()
{
    QMenu *menu = d->windowMenu->menu();
    menu->clear();

    menu->addAction(d->newWindow);
    menu->addAction(d->documentMenu);

    QMenu *docMenu = d->documentMenu->menu();
    docMenu->clear();

    Q_FOREACH (QPointer<KisDocument> doc, KisPart::instance()->documents()) {
        if (doc) {
            QString title = doc->url().toDisplayString();
            if (title.isEmpty() && doc->image()) {
                title = doc->image()->objectName();
            }
            QAction *action = docMenu->addAction(title);
            action->setIcon(qApp->windowIcon());
            connect(action, SIGNAL(triggered()), d->documentMapper, SLOT(map()));
            d->documentMapper->setMapping(action, doc);
        }
    }

    menu->addAction(d->workspaceMenu);
    QMenu *workspaceMenu = d->workspaceMenu->menu();
    workspaceMenu->clear();

    auto workspaces = KisResourceServerProvider::instance()->workspaceServer()->resources();
    auto m_this = this;
    for (auto &w : workspaces) {
        auto action = workspaceMenu->addAction(w->name());
        connect(action, &QAction::triggered, this, [=]() {
            m_this->restoreWorkspace(w);
        });
    }
    workspaceMenu->addSeparator();
    connect(workspaceMenu->addAction(i18nc("@action:inmenu", "&Import Workspace...")),
            &QAction::triggered,
            this,
            [&]() {
        QString extensions = d->workspacemodel->extensions();
        QStringList mimeTypes;
        for(const QString &suffix : extensions.split(":")) {
            mimeTypes << KisMimeDatabase::mimeTypeForSuffix(suffix);
        }

        KoFileDialog dialog(0, KoFileDialog::OpenFile, "OpenDocument");
        dialog.setMimeTypeFilters(mimeTypes);
        dialog.setCaption(i18nc("@title:window", "Choose File to Add"));
        QString filename = dialog.filename();

        d->workspacemodel->importResourceFile(filename);
    });

    connect(workspaceMenu->addAction(i18nc("@action:inmenu", "&New Workspace...")),
            &QAction::triggered,
            [=]() {
        QString name = QInputDialog::getText(this, i18nc("@title:window", "New Workspace..."),
                                             i18nc("@label:textbox", "Name:"));
        if (name.isEmpty()) return;
        auto rserver = KisResourceServerProvider::instance()->workspaceServer();

        KisWorkspaceResource* workspace = new KisWorkspaceResource("");
        workspace->setDockerState(m_this->saveState());
        d->viewManager->canvasResourceProvider()->notifySavingWorkspace(workspace);
        workspace->setValid(true);
        QString saveLocation = rserver->saveLocation();

        bool newName = false;
        if(name.isEmpty()) {
            newName = true;
            name = i18n("Workspace");
        }
        QFileInfo fileInfo(saveLocation + name + workspace->defaultFileExtension());

        int i = 1;
        while (fileInfo.exists()) {
            fileInfo.setFile(saveLocation + name + QString("%1").arg(i) + workspace->defaultFileExtension());
            i++;
        }
        workspace->setFilename(fileInfo.filePath());
        if(newName) {
            name = i18n("Workspace %1", i);
        }
        workspace->setName(name);
        rserver->addResource(workspace);
    });

    // TODO: What to do about delete?
//    workspaceMenu->addAction(i18nc("@action:inmenu", "&Delete Workspace..."));

    menu->addSeparator();
    menu->addAction(d->close);
    menu->addAction(d->closeAll);
    if (d->mdiArea->viewMode() == QMdiArea::SubWindowView) {
        menu->addSeparator();
        menu->addAction(d->mdiTile);
        menu->addAction(d->mdiCascade);
    }
    menu->addSeparator();
    menu->addAction(d->mdiNextWindow);
    menu->addAction(d->mdiPreviousWindow);
    menu->addSeparator();

    QList<QMdiSubWindow *> windows = d->mdiArea->subWindowList();
    for (int i = 0; i < windows.size(); ++i) {
        QPointer<KisView>child = qobject_cast<KisView*>(windows.at(i)->widget());
        if (child && child->document()) {
            QString text;
            if (i < 9) {
                text = i18n("&%1 %2", i + 1, child->document()->url().toDisplayString());
            }
            else {
                text = i18n("%1 %2", i + 1, child->document()->url().toDisplayString());
            }

            QAction *action  = menu->addAction(text);
            action->setIcon(qApp->windowIcon());
            action->setCheckable(true);
            action->setChecked(child == activeKisView());
            connect(action, SIGNAL(triggered()), d->windowMapper, SLOT(map()));
            d->windowMapper->setMapping(action, windows.at(i));
        }
    }

    bool showMdiArea = windows.count( ) > 0;
    if (!showMdiArea) {
        showWelcomeScreen(true); // see workaround in function in header
        // keep the recent file list updated when going back to welcome screen
        reloadRecentFileList();
        d->welcomePage->populateRecentDocuments();
    }

    // enable/disable the toolbox docker if there are no documents open
    Q_FOREACH (QObject* widget, children()) {
        if (widget->inherits("QDockWidget")) {
            QDockWidget* dw = static_cast<QDockWidget*>(widget);

            if ( dw->objectName() == "ToolBox") {
                dw->setEnabled(showMdiArea);
            }
        }
    }

    updateCaption();
}

void KisMainWindow::setActiveSubWindow(QWidget *window)
{
    if (!window) return;
    QMdiSubWindow *subwin = qobject_cast<QMdiSubWindow *>(window);
    //dbgKrita << "setActiveSubWindow();" << subwin << d->activeSubWindow;

    if (subwin && subwin != d->activeSubWindow) {
        KisView *view = qobject_cast<KisView *>(subwin->widget());
        //dbgKrita << "\t" << view << activeView();
        if (view && view != activeView()) {
             d->mdiArea->setActiveSubWindow(subwin);
            setActiveView(view);
        }
        d->activeSubWindow = subwin;
    }
    updateWindowMenu();
    d->actionManager()->updateGUI();
}

void KisMainWindow::configChanged()
{
    KisConfig cfg(true);
    QMdiArea::ViewMode viewMode = (QMdiArea::ViewMode)cfg.readEntry<int>("mdi_viewmode", (int)QMdiArea::TabbedView);
    d->mdiArea->setViewMode(viewMode);
    Q_FOREACH (QMdiSubWindow *subwin, d->mdiArea->subWindowList()) {
        subwin->setOption(QMdiSubWindow::RubberBandMove, cfg.readEntry<int>("mdi_rubberband", cfg.useOpenGL()));
        subwin->setOption(QMdiSubWindow::RubberBandResize, cfg.readEntry<int>("mdi_rubberband", cfg.useOpenGL()));

        /**
         * Dirty workaround for a bug in Qt (checked on Qt 5.6.1):
         *
         * If you make a window "Show on top" and then switch to the tabbed mode
         * the window will contiue to be painted in its initial "mid-screen"
         * position. It will persist here until you explicitly switch to its tab.
         */
        if (viewMode == QMdiArea::TabbedView) {
            Qt::WindowFlags oldFlags = subwin->windowFlags();
            Qt::WindowFlags flags = oldFlags;

            flags &= ~Qt::WindowStaysOnTopHint;
            flags &= ~Qt::WindowStaysOnBottomHint;

            if (flags != oldFlags) {
                subwin->setWindowFlags(flags);
                subwin->showMaximized();
            }
        }

    }

    KConfigGroup group( KSharedConfig::openConfig(), "theme");
    d->themeManager->setCurrentTheme(group.readEntry("Theme", "Krita dark"));
    d->actionManager()->updateGUI();

    QBrush brush(cfg.getMDIBackgroundColor());
    d->mdiArea->setBackground(brush);

    QString backgroundImage = cfg.getMDIBackgroundImage();
    if (backgroundImage != "") {
        QImage image(backgroundImage);
        QBrush brush(image);
        d->mdiArea->setBackground(brush);
    }

    d->mdiArea->update();
}

KisView* KisMainWindow::newView(QObject *document)
{
    KisDocument *doc = qobject_cast<KisDocument*>(document);
    KisView *view = addViewAndNotifyLoadingCompleted(doc);
    d->actionManager()->updateGUI();

    return view;
}

void KisMainWindow::newWindow()
{
    KisMainWindow *mainWindow = KisPart::instance()->createMainWindow();
    mainWindow->initializeGeometry();
    mainWindow->show();
}

void KisMainWindow::closeCurrentWindow()
{
    if (d->mdiArea->currentSubWindow()) {
        d->mdiArea->currentSubWindow()->close();
        d->actionManager()->updateGUI();
    }
}

void KisMainWindow::checkSanity()
{
    // print error if the lcms engine is not available
    if (!KoColorSpaceEngineRegistry::instance()->contains("icc")) {
        // need to wait 1 event since exiting here would not work.
        m_errorMessage = i18n("The Krita LittleCMS color management plugin is not installed. Krita will quit now.");
        m_dieOnError = true;
        QTimer::singleShot(0, this, SLOT(showErrorAndDie()));
        return;
    }

    KisPaintOpPresetResourceServer * rserver = KisResourceServerProvider::instance()->paintOpPresetServer();
    if (rserver->resources().isEmpty()) {
        m_errorMessage = i18n("Krita cannot find any brush presets! Krita will quit now.");
        m_dieOnError = true;
        QTimer::singleShot(0, this, SLOT(showErrorAndDie()));
        return;
    }
}

void KisMainWindow::showErrorAndDie()
{
    QMessageBox::critical(0, i18nc("@title:window", "Installation error"), m_errorMessage);
    if (m_dieOnError) {
        exit(10);
    }
}

void KisMainWindow::showAboutApplication()
{
    KisAboutApplication dlg(this);
    dlg.exec();
}

QPointer<KisView> KisMainWindow::activeKisView()
{
    if (!d->mdiArea) return 0;
    QMdiSubWindow *activeSubWindow = d->mdiArea->activeSubWindow();
    //dbgKrita << "activeKisView" << activeSubWindow;
    if (!activeSubWindow) return 0;
    return qobject_cast<KisView*>(activeSubWindow->widget());
}

void KisMainWindow::newOptionWidgets(KoCanvasController *controller, const QList<QPointer<QWidget> > &optionWidgetList)
{
    KIS_ASSERT_RECOVER_NOOP(controller == KoToolManager::instance()->activeCanvasController());
    bool isOurOwnView = false;

    Q_FOREACH (QPointer<KisView> view, KisPart::instance()->views()) {
        if (view && view->canvasController() == controller) {
            isOurOwnView = view->mainWindow() == this;
        }
    }

    if (!isOurOwnView) return;

    Q_FOREACH (QWidget *w, optionWidgetList) {
#ifdef Q_OS_OSX
        w->setAttribute(Qt::WA_MacSmallSize, true);
#endif
        w->setFont(KoDockRegistry::dockFont());
    }

    if (d->toolOptionsDocker) {
        d->toolOptionsDocker->setOptionWidgets(optionWidgetList);
    }
    else {
        d->viewManager->paintOpBox()->newOptionWidgets(optionWidgetList);
    }
}

void KisMainWindow::applyDefaultSettings(QPrinter &printer) {

    if (!d->activeView) return;

    QString title = d->activeView->document()->documentInfo()->aboutInfo("title");
    if (title.isEmpty()) {
        QFileInfo info(d->activeView->document()->url().fileName());
        title = info.baseName();
    }

    if (title.isEmpty()) {
        // #139905
        title = i18n("%1 unsaved document (%2)", qApp->applicationDisplayName(),
                     QLocale().toString(QDate::currentDate(), QLocale::ShortFormat));
    }
    printer.setDocName(title);
}

void KisMainWindow::createActions()
{
    KisActionManager *actionManager = d->actionManager();



    actionManager->createStandardAction(KStandardAction::New, this, SLOT(slotFileNew()));
    actionManager->createStandardAction(KStandardAction::Open, this, SLOT(slotFileOpen()));
    actionManager->createStandardAction(KStandardAction::Quit, this, SLOT(slotFileQuit()));
    actionManager->createStandardAction(KStandardAction::ConfigureToolbars, this, SLOT(slotConfigureToolbars()));
    d->fullScreenMode = actionManager->createStandardAction(KStandardAction::FullScreen, this, SLOT(viewFullscreen(bool)));

    d->recentFiles = KStandardAction::openRecent(this, SLOT(slotFileOpenRecent(QUrl)), actionCollection());
    connect(d->recentFiles, SIGNAL(recentListCleared()), this, SLOT(saveRecentFiles()));
    KSharedConfigPtr configPtr =  KSharedConfig::openConfig();
    d->recentFiles->loadEntries(configPtr->group("RecentFiles"));

    d->saveAction = actionManager->createStandardAction(KStandardAction::Save, this, SLOT(slotFileSave()));
    d->saveAction->setActivationFlags(KisAction::ACTIVE_IMAGE);

    d->saveActionAs = actionManager->createStandardAction(KStandardAction::SaveAs, this, SLOT(slotFileSaveAs()));
    d->saveActionAs->setActivationFlags(KisAction::ACTIVE_IMAGE);

//    d->printAction = actionManager->createStandardAction(KStandardAction::Print, this, SLOT(slotFilePrint()));
//    d->printAction->setActivationFlags(KisAction::ACTIVE_IMAGE);

//    d->printActionPreview = actionManager->createStandardAction(KStandardAction::PrintPreview, this, SLOT(slotFilePrintPreview()));
//    d->printActionPreview->setActivationFlags(KisAction::ACTIVE_IMAGE);

    d->undo = actionManager->createStandardAction(KStandardAction::Undo, this, SLOT(undo()));
    d->undo->setActivationFlags(KisAction::ACTIVE_IMAGE);

    d->redo = actionManager->createStandardAction(KStandardAction::Redo, this, SLOT(redo()));
    d->redo->setActivationFlags(KisAction::ACTIVE_IMAGE);

    d->undoActionsUpdateManager.reset(new KisUndoActionsUpdateManager(d->undo, d->redo));
    d->undoActionsUpdateManager->setCurrentDocument(d->activeView ? d->activeView->document() : 0);

//    d->exportPdf  = actionManager->createAction("file_export_pdf");
//    connect(d->exportPdf, SIGNAL(triggered()), this, SLOT(exportToPdf()));

    d->importAnimation  = actionManager->createAction("file_import_animation");
    connect(d->importAnimation, SIGNAL(triggered()), this, SLOT(importAnimation()));

    d->closeAll = actionManager->createAction("file_close_all");
    connect(d->closeAll, SIGNAL(triggered()), this, SLOT(slotFileCloseAll()));

//    d->reloadFile  = actionManager->createAction("file_reload_file");
//    d->reloadFile->setActivationFlags(KisAction::CURRENT_IMAGE_MODIFIED);
//    connect(d->reloadFile, SIGNAL(triggered(bool)), this, SLOT(slotReloadFile()));

    d->importFile  = actionManager->createAction("file_import_file");
    connect(d->importFile, SIGNAL(triggered(bool)), this, SLOT(slotImportFile()));

    d->exportFile  = actionManager->createAction("file_export_file");
    connect(d->exportFile, SIGNAL(triggered(bool)), this, SLOT(slotExportFile()));

    /* The following entry opens the document information dialog.  Since the action is named so it
        intends to show data this entry should not have a trailing ellipses (...).  */
    d->showDocumentInfo  = actionManager->createAction("file_documentinfo");
    connect(d->showDocumentInfo, SIGNAL(triggered(bool)), this, SLOT(slotDocumentInfo()));


    d->themeManager->setThemeMenuAction(new KActionMenu(i18nc("@action:inmenu", "&Themes"), this));
    d->themeManager->registerThemeActions(actionCollection());
    connect(d->themeManager, SIGNAL(signalThemeChanged()), this, SLOT(slotThemeChanged()));


    connect(d->themeManager, SIGNAL(signalThemeChanged()), d->welcomePage, SLOT(slotUpdateThemeColors()));

    d->toggleDockers = actionManager->createAction("view_toggledockers");
    KisConfig(true).showDockers(true);
    d->toggleDockers->setChecked(true);
    connect(d->toggleDockers, SIGNAL(toggled(bool)), SLOT(toggleDockersVisibility(bool)));

    actionCollection()->addAction("settings_dockers_menu", d->dockWidgetMenu);
    actionCollection()->addAction("window", d->windowMenu);

    d->mdiCascade = actionManager->createAction("windows_cascade");
    connect(d->mdiCascade, SIGNAL(triggered()), d->mdiArea, SLOT(cascadeSubWindows()));

    d->mdiTile = actionManager->createAction("windows_tile");
    connect(d->mdiTile, SIGNAL(triggered()), d->mdiArea, SLOT(tileSubWindows()));

    d->mdiNextWindow = actionManager->createAction("windows_next");
    connect(d->mdiNextWindow, SIGNAL(triggered()), d->mdiArea, SLOT(activateNextSubWindow()));

    d->mdiPreviousWindow = actionManager->createAction("windows_previous");
    connect(d->mdiPreviousWindow, SIGNAL(triggered()), d->mdiArea, SLOT(activatePreviousSubWindow()));

    d->newWindow = actionManager->createAction("view_newwindow");
    connect(d->newWindow, SIGNAL(triggered(bool)), this, SLOT(newWindow()));

    d->close = actionManager->createStandardAction(KStandardAction::Close, this, SLOT(closeCurrentWindow()));

    d->showSessionManager = actionManager->createAction("file_sessions");
    connect(d->showSessionManager, SIGNAL(triggered(bool)), this, SLOT(slotShowSessionManager()));

    actionManager->createStandardAction(KStandardAction::Preferences, this, SLOT(slotPreferences()));

    for (int i = 0; i < 2; i++) {
        d->expandingSpacers[i] = new KisAction(i18n("Expanding Spacer"));
        d->expandingSpacers[i]->setDefaultWidget(new QWidget(this));
        d->expandingSpacers[i]->defaultWidget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        actionManager->addAction(QString("expanding_spacer_%1").arg(i), d->expandingSpacers[i]);
    }
}

void KisMainWindow::applyToolBarLayout()
{
    const bool isPlastiqueStyle = style()->objectName() == "plastique";

    Q_FOREACH (KToolBar *toolBar, toolBars()) {
        toolBar->layout()->setSpacing(4);
        if (isPlastiqueStyle) {
            toolBar->setContentsMargins(0, 0, 0, 2);
        }
        //Hide text for buttons with an icon in the toolbar
        Q_FOREACH (QAction *ac, toolBar->actions()){
            if (ac->icon().pixmap(QSize(1,1)).isNull() == false){
                ac->setPriority(QAction::LowPriority);
            }else {
                ac->setIcon(QIcon());
            }
        }
    }
}

void KisMainWindow::initializeGeometry()
{
    // if the user didn's specify the geometry on the command line (does anyone do that still?),
    // we first figure out some good default size and restore the x,y position. See bug 285804Z.
    KConfigGroup cfg = d->windowStateConfig;
    QByteArray geom = QByteArray::fromBase64(cfg.readEntry("ko_geometry", QByteArray()));
    if (!restoreGeometry(geom)) {
        const int scnum = QApplication::desktop()->screenNumber(parentWidget());
        QRect desk = QApplication::desktop()->availableGeometry(scnum);
        // if the desktop is virtual then use virtual screen size
        if (QApplication::desktop()->isVirtualDesktop()) {
            desk = QApplication::desktop()->availableGeometry(QApplication::desktop()->screen(scnum));
        }

        quint32 x = desk.x();
        quint32 y = desk.y();
        quint32 w = 0;
        quint32 h = 0;

        // Default size -- maximize on small screens, something useful on big screens
        const int deskWidth = desk.width();
        if (deskWidth > 1024) {
            // a nice width, and slightly less than total available
            // height to compensate for the window decs
            w = (deskWidth / 3) * 2;
            h = (desk.height() / 3) * 2;
        }
        else {
            w = desk.width();
            h = desk.height();
        }

        x += (desk.width() - w) / 2;
        y += (desk.height() - h) / 2;

        move(x,y);
        setGeometry(geometry().x(), geometry().y(), w, h);
    }
    d->fullScreenMode->setChecked(isFullScreen());
}

void KisMainWindow::showManual()
{
    QDesktopServices::openUrl(QUrl("https://docs.krita.org"));
}

void KisMainWindow::moveEvent(QMoveEvent *e)
{
    /**
     * For checking if the display number has changed or not we should always use
     * positional overload, not using QWidget overload. Otherwise we might get
     * inconsistency, because screenNumber(widget) can return -1, but screenNumber(pos)
     * will always return the nearest screen.
     */

    const int oldScreen = qApp->desktop()->screenNumber(e->oldPos());
    const int newScreen = qApp->desktop()->screenNumber(e->pos());

    if (oldScreen != newScreen) {
        emit screenChanged();
    }

    if (d->screenConnectionsStore.isEmpty() || oldScreen != newScreen) {

        d->screenConnectionsStore.clear();

        QScreen *newScreenObject = 0;

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
        newScreenObject = qApp->screenAt(e->pos());
#else
        // TODO: i'm not sure if this pointer already has a correct value
        // by the moment we get the event. It might not work on older
        // versions of Qt
        newScreenObject = qApp->primaryScreen();
#endif

        if (newScreenObject) {
            d->screenConnectionsStore.addConnection(newScreenObject, SIGNAL(physicalDotsPerInchChanged(qreal)),
                                                    this, SIGNAL(screenChanged()));
        }
    }
}



#include <moc_KisMainWindow.cpp>
