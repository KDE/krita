/* This file is part of the KDE project
   SPDX-FileCopyrightText: 1998, 1999 Torben Weis <weis@kde.org>
   SPDX-FileCopyrightText: 2000-2006 David Faure <faure@kde.org>
   SPDX-FileCopyrightText: 2007, 2009 Thomas zander <zander@kde.org>
   SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
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
#include <QToolButton>
#include <KisSignalMapper.h>
#include <QTabBar>
#include <QMoveEvent>
#include <QUrl>
#include <QMessageBox>
#include <QStatusBar>
#include <QStyleFactory>
#include <QMenu>
#include <QMenuBar>
#include <KisMimeDatabase.h>
#include <QMimeData>
#include <QStackedWidget>
#include <QProxyStyle>
#include <QScreen>
#include <QAction>
#include <QWindow>
#include <QTemporaryDir>
#include <QScrollArea>
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
#include "krita_utils.h"
#include <ktoggleaction.h>
#include <ktoolbar.h>
#include <kmainwindow.h>
#include <kxmlguiwindow.h>
#include <kxmlguifactory.h>
#include <kxmlguiclient.h>
#include <kguiitem.h>
#include <kwindowconfig.h>
#include <kacceleratormanager.h>

#include <KoResourcePaths.h>
#include <KoToolFactoryBase.h>
#include <KoToolRegistry.h>
#include <KoDockFactoryBase.h>
#include <KoDockWidgetTitleBar.h>
#include <KoDocumentInfoDlg.h>
#include <KoDocumentInfo.h>
#include <KoFileDialog.h>
#include <kis_icon.h>
#include <KoToolManager.h>
#include <KoZoomController.h>
#include "KoToolDocker.h"
#include "toolbox/KoToolBoxFactory.h"
#include <KoDockRegistry.h>
#include <KoPluginLoader.h>
#include <KoColorSpaceEngine.h>
#include <KoUpdater.h>
#include <KisResourceModel.h>
#include <KisResourceLoaderRegistry.h>
#include <KisResourceIterator.h>
#include <KisResourceTypes.h>
#include <KisResourceCacheDb.h>
#include <KisStorageModel.h>
#include <KisStorageFilterProxyModel.h>
#include <KisPlaybackEngine.h>

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#endif

#include <KisUsageLogger.h>
#include <brushengine/kis_paintop_settings.h>
#include "dialogs/kis_about_application.h"
#include "dialogs/kis_delayed_save_dialog.h"
#include "dialogs/kis_dlg_preferences.h"
#include "kis_action_manager.h"
#include "KisApplication.h"
#include "kis_canvas2.h"
#include "kis_canvas_controller.h"
#include "kis_canvas_resource_provider.h"
#include "kis_clipboard.h"
#include "kis_config.h"
#include "kis_config_notifier.h"
#include "kis_custom_image_widget.h"
#include "animation/KisAnimationRender.h"
#include "animation/KisDlgAnimationRenderer.h"
#include <KisDocument.h>
#include "kis_group_layer.h"
#include "kis_image_from_clipboard_widget.h"
#include "kis_image.h"
#include "kis_image_animation_interface.h"
#include <KisImportExportFilter.h>
#include "KisImportExportManager.h"
#include "kis_mainwindow_observer.h"
#include "kis_node.h"
#include "KisOpenPane.h"
#include "kis_paintop_box.h"
#include "KisPart.h"
#include "KisResourceServerProvider.h"
#include "kis_signal_compressor_with_param.h"
#include "kis_statusbar.h"
#include "KisView.h"
#include "KisViewManager.h"
#include "thememanager.h"
#include "kis_animation_importer.h"
#include "dialogs/kis_dlg_import_image_sequence.h"
#include "animation/KisDlgImportVideoAnimation.h"
#include <KisImageConfigNotifier.h>
#include "KisWindowLayoutManager.h"
#include <KisUndoActionsUpdateManager.h>
#include "KisWelcomePageWidget.h"
#include "KisRecentDocumentsModelWrapper.h"
#include <KritaVersionWrapper.h>
#include "KisCanvasWindow.h"
#include "kis_action.h"
#include <katecommandbar.h>
#include "KisNodeActivationActionCreatorVisitor.h"
#include "KisUiFont.h"
#include <KisResourceUserOperations.h>
#include "KisRecentFilesManager.h"
#include <config-qmdiarea-always-show-subwindow-title.h>

#include <mutex>

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
        , styleMenu(new KActionMenu(i18nc("@action:inmenu", "Styles"), parent))
        , dockWidgetMenu(new KActionMenu(i18nc("@action:inmenu", "&Dockers"), parent))
        , windowMenu(new KActionMenu(i18nc("@action:inmenu", "&Window"), parent))
        , documentMenu(new KActionMenu(i18nc("@action:inmenu", "New &View"), parent))
        , workspaceMenu(new KActionMenu(i18nc("@action:inmenu", "Wor&kspace"), parent))
        , welcomePage(new KisWelcomePageWidget(parent))
        , widgetStack(new QStackedWidget(parent))
        , mdiArea(new QMdiArea(parent))
        , windowMapper(new KisSignalMapper(parent))
        , documentMapper(new KisSignalMapper(parent))
    {
        if (id.isNull()) this->id = QUuid::createUuid();

        welcomeScroller = new QScrollArea();
        welcomeScroller->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        welcomeScroller->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        welcomeScroller->setWidget(welcomePage);
        welcomeScroller->setWidgetResizable(true);

        widgetStack->addWidget(welcomeScroller);
        widgetStack->addWidget(mdiArea);
        mdiArea->setTabsMovable(true);
        mdiArea->setActivationOrder(QMdiArea::ActivationHistoryOrder);
        mdiArea->setDocumentMode(true);
        mdiArea->setOption(QMdiArea::DontMaximizeSubWindowOnActivation);
#ifdef HAVE_QMDIAREA_ALWAYS_SHOW_SUBWINDOW_TITLE
        mdiArea->setOption(QMdiArea::AlwaysShowSubwindowNameInTitleBar);
#endif /* HAVE_QMDIAREA_ALWAYS_SHOW_SUBWINDOW_TITLE */

        commandBar = new KateCommandBar(parent);
    }

    ~Private() {
        qDeleteAll(toolbarList);
    }

    KisMainWindow *q {nullptr};
    QUuid id;

    KisViewManager *viewManager {nullptr};

    QPointer<KisView> activeView;
    KisSignalAutoConnectionsStore activeViewConnections;

    QList<QAction *> toolbarList;

    bool firstTime {true};
    bool windowSizeDirty {false};

    KisAction *showDocumentInfo {nullptr};
    KisAction *saveAction {nullptr};
    KisAction *saveActionAs {nullptr};
    KisAction *importAnimation {nullptr};
    KisAction *importVideoAnimation {nullptr};
    KisAction *renderAnimation {nullptr};
    KisAction *renderAnimationAgain {nullptr};
    KisAction *closeAll {nullptr};
    KisAction *importFile {nullptr};
    KisAction *exportFile {nullptr};
    KisAction *exportFileAdvance {nullptr};
    KisAction *undo {nullptr};
    KisAction *redo {nullptr};
    KisAction *close {nullptr};
    KisAction *mdiCascade {nullptr};
    KisAction *mdiTile {nullptr};
    KisAction *mdiNextWindow {nullptr};
    KisAction *mdiPreviousWindow {nullptr};
    KisAction *toggleDockers {nullptr};
    KisAction *resetConfigurations {nullptr};
    KisAction *toggleDockerTitleBars {nullptr};
#ifndef Q_OS_ANDROID
    KisAction *toggleDetachCanvas {nullptr};
    KisAction *newWindow {nullptr};
#endif
    KisAction *fullScreenMode {nullptr};
    KisAction *showSessionManager {nullptr};
    KisAction *commandBarAction {nullptr};
    KisAction *expandingSpacers[2];

    KActionMenu *styleMenu {nullptr};
    QActionGroup* styleActions {nullptr};
    QMap<QString, QAction*> actionMap;

    KActionMenu *dockWidgetMenu {nullptr};
    KActionMenu *windowMenu {nullptr};
    KActionMenu *documentMenu {nullptr};
    KActionMenu *workspaceMenu {nullptr};

    KisKHelpMenu *helpMenu  {nullptr};

    KRecentFilesAction *recentFiles {nullptr};
    KisResourceModel *workspacemodel {nullptr};

    QScopedPointer<KisUndoActionsUpdateManager> undoActionsUpdateManager;

    QString lastExportLocation;

    QMap<QString, QDockWidget *> dockWidgetsMap;
    QByteArray dockerStateBeforeHiding;
    KoToolDocker *toolOptionsDocker {nullptr};

    QCloseEvent *deferredClosingEvent {nullptr};
#ifndef Q_OS_HAIKU
    Digikam::ThemeManager *themeManager {nullptr};
#endif
    QScrollArea *welcomeScroller {nullptr};
    KisWelcomePageWidget *welcomePage {nullptr};


    QStackedWidget *widgetStack {nullptr};

    QMdiArea *mdiArea {nullptr};
    QMdiSubWindow *activeSubWindow  {nullptr};
    KisSignalMapper *windowMapper {nullptr};
    KisSignalMapper *documentMapper {nullptr};
    KisCanvasWindow *canvasWindow {nullptr};

    QByteArray lastExportedFormat;
    QScopedPointer<KisSignalCompressorWithParam<int> > tabSwitchCompressor;
    QMutex savingEntryMutex;

    KConfigGroup windowStateConfig;

    QUuid workspaceBorrowedBy;

    KateCommandBar *commandBar {nullptr};

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

class ScopedWidgetDisabler
{
    QWidget *widget;
public:
    ScopedWidgetDisabler(QWidget *widget_)
        : widget(widget_)
    {
        widget->setEnabled(false);
    }

    ~ScopedWidgetDisabler()
    {
        widget->setEnabled(true);
    }
};

KisMainWindow::KisMainWindow(QUuid uuid)
    : KXmlGuiWindow()
    , d(new Private(this, uuid))
{
    KAcceleratorManager::setNoAccel(this);

    d->workspacemodel = new KisResourceModel(ResourceType::Workspaces, this);
    connect(d->workspacemodel, SIGNAL(modelReset()), this, SLOT(updateWindowMenu()));
    connect(d->workspacemodel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(updateWindowMenu()));
    connect(d->workspacemodel, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(updateWindowMenu()));

    d->viewManager = new KisViewManager(this, actionCollection());
    KConfigGroup group( KSharedConfig::openConfig(), "theme");
#ifndef Q_OS_HAIKU
    d->themeManager = new Digikam::ThemeManager(group.readEntry("Theme", "Krita dark"), this);
#endif
    d->windowStateConfig = KSharedConfig::openConfig()->group("MainWindow");

    setStandardToolBarMenuEnabled(true);
    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);
    setDockNestingEnabled(true);

    qApp->setStartDragDistance(25);     // 25 px is a distance that works well for Tablet and Mouse events

#ifdef Q_OS_MACOS
    setUnifiedTitleAndToolBarOnMac(true);
#endif

    connect(this, SIGNAL(restoringDone()), this, SLOT(forceDockTabFonts()));
    connect(this, SIGNAL(themeChanged()), d->viewManager, SLOT(updateIcons()));
    connect(this, SIGNAL(themeChanged()), KoToolManager::instance(), SLOT(themeChanged()));
    connect(KisPart::instance(), SIGNAL(documentClosed(QString)), SLOT(updateWindowMenu()));
    connect(KisPart::instance(), SIGNAL(documentOpened(QString)), SLOT(updateWindowMenu()));
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), this, SLOT(configChanged()));

    actionCollection()->addAssociatedWidget(this);
    KoPluginLoader::instance()->load("Krita/ViewPlugin", "Type == 'Service' and ([X-Krita-Version] == 28)", KoPluginLoader::PluginsConfig(), d->viewManager, false);

    // Load the per-application plugins (Right now, only Python) We do this only once, when the first mainwindow is being created.
    KoPluginLoader::instance()->load("Krita/ApplicationPlugin", "Type == 'Service' and ([X-Krita-Version] == 28)", KoPluginLoader::PluginsConfig(), qApp, true);

    KoToolBoxFactory toolBoxFactory;
    QDockWidget *toolbox = createDockWidget(&toolBoxFactory);

    KisConfig cfg(true);
    if (cfg.toolOptionsInDocker()) {
        ToolDockerFactory toolDockerFactory;
        d->toolOptionsDocker = qobject_cast<KoToolDocker*>(createDockWidget(&toolDockerFactory));
        if (d->toolOptionsDocker) {
            d->toolOptionsDocker->toggleViewAction()->setEnabled(true);
        }
    }

    QMap<QString, QAction*> dockwidgetActions;

    if (toolbox) {
        dockwidgetActions[toolbox->toggleViewAction()->text()] = toolbox->toggleViewAction();
    }
    Q_FOREACH (const QString & docker, KoDockRegistry::instance()->keys()) {
        KoDockFactoryBase *factory = KoDockRegistry::instance()->value(docker);
        QDockWidget *dw = createDockWidget(factory);
        if (dw) {
            dockwidgetActions[dw->toggleViewAction()->text()] = dw->toggleViewAction();
        }
    }

    if (d->toolOptionsDocker) {
        dockwidgetActions[d->toolOptionsDocker->toggleViewAction()->text()] = d->toolOptionsDocker->toggleViewAction();
    }
    connect(KoToolManager::instance(), SIGNAL(toolOptionWidgetsChanged(KoCanvasController*,QList<QPointer<QWidget> >)), this, SLOT(newOptionWidgets(KoCanvasController*,QList<QPointer<QWidget> >)));

    Q_FOREACH (QString title, dockwidgetActions.keys()) {
        d->dockWidgetMenu->addAction(dockwidgetActions[title]);
    }


    // Style menu actions
    d->styleActions = new QActionGroup(this);
    QAction * action;

    QStringList allowableStyles = QStringList() << "macintosh" << "breeze" << "fusion";

    Q_FOREACH (QString styleName, QStyleFactory::keys()) {
#ifdef Q_OS_ANDROID
        // disable the style for android platform
        if (styleName.toLower().contains("android")) {
            continue;
        }
#endif
        if (qgetenv("KRITA_NO_STYLE_OVERRIDE").isEmpty()) {
            if (!allowableStyles.contains(styleName.toLower())) {
                continue;
            }
        }
        action = new QAction(styleName, d->styleActions);
        action->setCheckable(true);
        d->actionMap.insert(styleName, action);
        d->styleMenu->addAction(d->actionMap.value(styleName));
    }


    // select the config value, or the current style if that does not exist
    QString styleFromConfig = cfg.widgetStyle().toLower();
    QString styleToSelect = styleFromConfig == "" ? style()->objectName().toLower() : styleFromConfig;

    Q_FOREACH (auto key, d->actionMap.keys()) {
        if(key.toLower() == styleToSelect) { // does the key match selection
            d->actionMap.value(key)->setChecked(true);
        }
    }

    connect(d->styleActions, SIGNAL(triggered(QAction*)),
            this, SLOT(slotUpdateWidgetStyle()));


    // Load all the actions from the tool plugins
    // ToolBoxDocker needs them when at setViewManager()
    Q_FOREACH(KoToolFactoryBase *toolFactory, KoToolRegistry::instance()->values()) {
        toolFactory->createActions(actionCollection());
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

    d->mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    d->mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    d->mdiArea->setTabPosition(QTabWidget::North);
    d->mdiArea->setTabsClosable(true);
    d->mdiArea->setAcceptDrops(true);

    themeChanged(); // updates icon styles

    setCentralWidget(d->widgetStack);
    d->widgetStack->setCurrentIndex(0);

    connect(d->mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(subWindowActivated()));
    connect(d->windowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(setActiveSubWindow(QWidget*)));
    connect(d->documentMapper, SIGNAL(mapped(QObject*)), this, SLOT(newView(QObject*)));

    d->canvasWindow = new KisCanvasWindow(this);
    actionCollection()->addAssociatedWidget(d->canvasWindow);

    createActions();

    // the welcome screen needs to grab actions...so make sure this line goes after the createAction() so they exist
    d->welcomePage->setMainWindow(this);

    d->recentFiles->setRecentFilesModel(&KisRecentDocumentsModelWrapper::instance()->model());

    applyMainWindowSettings(d->windowStateConfig);

    subWindowActivated();
    updateWindowMenu();

    if (isHelpMenuEnabled() && !d->helpMenu) {
        QGuiApplication *app = qApp;
        KAboutData aboutData(KAboutData::applicationData());
        aboutData.setOrganizationDomain(app->organizationDomain().toUtf8());

        d->helpMenu = new KisKHelpMenu(this, aboutData, false);

        // workaround-less version:
        // d->helpMenu = new KisKHelpMenu(this, QString()/*unused*/, false);

        // The difference between using KisKActionCollection->addAction() is that
        // these actions do not get tied to the MainWindow.  What does this all do?
        KisKActionCollection *actions = d->viewManager->actionCollection();
        QAction *helpContentsAction = d->helpMenu->action(KisKHelpMenu::menuHelpContents);
        QAction *whatsThisAction = d->helpMenu->action(KisKHelpMenu::menuWhatsThis);
        QAction *reportBugAction = d->helpMenu->action(KisKHelpMenu::menuReportBug);
        QAction *switchLanguageAction = d->helpMenu->action(KisKHelpMenu::menuSwitchLanguage);
        QAction *aboutAppAction = d->helpMenu->action(KisKHelpMenu::menuAboutApp);
        QAction *aboutKdeAction = d->helpMenu->action(KisKHelpMenu::menuAboutKDE);

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

    // Make sure the python plugins create their actions in time
    KisPart::instance()->notifyMainWindowIsBeingCreated(this);

    // If we have customized the toolbars, load that first
    setLocalXMLFile(KoResourcePaths::locateLocal("data", "krita5.xmlgui"));
    setXMLFile(":/kxmlgui5/krita5.xmlgui");

    guiFactory()->addClient(this);
    connect(guiFactory(), SIGNAL(makingChanges(bool)), SLOT(slotXmlGuiMakingChanges(bool)));

    // Create and plug toolbar list for Settings menu
    QList<QAction *> toolbarList;
    Q_FOREACH (QWidget* it, guiFactory()->containers("ToolBar")) {
        KisToolBar * toolBar = ::qobject_cast<KisToolBar *>(it);
        if (toolBar) {
            toolBar->setMovable(KisConfig(true).readEntry<bool>("LockAllDockerPanels", false));

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

    plugActionList("toolbarlist", toolbarList);
    d->toolbarList = toolbarList;

    applyToolBarLayout();

    d->viewManager->updateGUI();
    d->viewManager->updateIcons();

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
        KisWorkspaceResourceSP workspace = rserver->resource("", "", currentWorkspace);
        if (workspace) {
            restoreWorkspace(workspace);
        }
        cfg.writeEntry("CanvasOnlyActive", false);
        menuBar()->setVisible(true);
    }

    this->winId(); // Ensures the native window has been created.

#ifdef Q_OS_ANDROID
    // HACK: This prevents the mainWindow from going beyond the screen with no
    // way to bring it back. Apparently the size doesn't matter here as long as
    // it remains fixed?
    setFixedSize(KisApplication::primaryScreen()->availableGeometry().size());

    QScreen *s = QGuiApplication::primaryScreen();
    s->setOrientationUpdateMask(Qt::LandscapeOrientation|Qt::InvertedLandscapeOrientation|Qt::PortraitOrientation|Qt::InvertedPortraitOrientation);
    connect(s, SIGNAL(orientationChanged(Qt::ScreenOrientation)), this, SLOT(orientationChanged()));

    // When Krita starts, Java side sends an event to set applicationState() to active. But, before
    // the event could reach KisApplication's platform integration, it is cleared by KisOpenGLModeProber::probeFormat.
    // So, we send it manually when MainWindow shows up.
    QAndroidJniObject::callStaticMethod<void>("org/qtproject/qt5/android/QtNative", "setApplicationState", "(I)V", Qt::ApplicationActive);
#endif

    setAcceptDrops(true);
    QTabBar *tabBar = d->findTabBarHACK();
    if (tabBar) {
        tabBar->setElideMode(Qt::ElideRight);
        // load customized tab style
        customizeTabBar();
        tabBar->setExpanding(true);
        tabBar->setAcceptDrops(true);
        tabBar->setChangeCurrentOnDrag(true);
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

QMenu *KisMainWindow::createPopupMenu()
{
    return 0;
}

QUuid KisMainWindow::id() const {
    return d->id;
}

void KisMainWindow::addView(KisView *view, QMdiSubWindow *subWindow)
{
    if (d->activeView == view && !subWindow) return;

    if (d->activeView) {
        d->activeView->disconnect(this);
    }

    // register the newly created view in the input manager
    viewManager()->inputManager()->addTrackedCanvas(view->canvasBase());

    showView(view, subWindow);
    Q_EMIT restoringDone();

//    QTabBar *tabBar = d->findTabBarHACK();
//    Q_FOREACH(QObject *c, tabBar->children()) {
//        if (QWidget *w = qobject_cast<QWidget*>(c)) {
//            w->setAcceptDrops(true);
//        }
//    }
}

void KisMainWindow::notifyChildViewDestroyed(KisView *view)
{
    /**
     * If we are the last view of the window, Qt will not activate another tab
     * before destroying tab/window. In this case we should clear all the dangling
     * pointers manually by setting the current view to null
     */
    viewManager()->inputManager()->removeTrackedCanvas(view->canvasBase());
    if (view->canvasBase() == viewManager()->canvasBase()) {
        viewManager()->setCurrentView(0);
    }
}


void KisMainWindow::showView(KisView *imageView, QMdiSubWindow *subwin)
{
    if (imageView && activeView() != imageView) {
        // XXX: find a better way to initialize this!
        imageView->setViewManager(d->viewManager);

        imageView->canvasBase()->setFavoriteResourceManager(d->viewManager->paintOpBox()->favoriteResourcesManager());
        imageView->slotLoadingFinished();

        if (!subwin) {
            QMdiSubWindow *currentSubWin = d->mdiArea->currentSubWindow();
            bool shouldMaximize = currentSubWin ? currentSubWin->isMaximized() : true;
            subwin = d->mdiArea->addSubWindow(imageView);
            if (shouldMaximize) {
                subwin->setWindowState(Qt::WindowMaximized);
            }
        } else {
            subwin->setWidget(imageView);
        }
        imageView->setSubWindow(subwin);
        subwin->setAttribute(Qt::WA_DeleteOnClose, true);
        connect(subwin, SIGNAL(destroyed()), SLOT(updateWindowMenu()));

        KisConfig cfg(true);
        subwin->setOption(QMdiSubWindow::RubberBandMove, cfg.readEntry<int>("mdi_rubberband", cfg.useOpenGL()));
        subwin->setOption(QMdiSubWindow::RubberBandResize, cfg.readEntry<int>("mdi_rubberband", cfg.useOpenGL()));
        subwin->setWindowIcon(qApp->windowIcon());

#ifdef Q_OS_MACOS
        connect(subwin, SIGNAL(destroyed()), SLOT(updateSubwindowFlags()));
        updateSubwindowFlags();
#endif

        if (d->mdiArea->subWindowList().size() == 1) {
            imageView->showMaximized();
        }
        else {
            imageView->show();
        }

        /**
         * Hack alert!
         *
         * Here we explicitly request KoToolManager to Q_EMIT all the tool
         * activation signals, to reinitialize the tool options docker.
         *
         * That is needed due to a design flaw we have in the
         * initialization procedure.  The tool in the KoToolManager is
         * initialized in KisView::setViewManager() calls, which
         * happens early enough. During this call the tool manager
         * requests KoCanvasControllerWidget to Q_EMIT the signal to
         * update the widgets in the tool docker. *But* at that moment
         * of time the view is not yet connected to the main window,
         * because it happens in KisViewManager::setCurrentView a bit
         * later. This fact makes the widgets updating signals be lost
         * and never reach the tool docker.
         *
         * So here we just explicitly call the tool activation stub.
         */

        KoToolManager::instance()->initializeCurrentToolForCanvas();

        // No, no, no: do not try to call this _before_ the show() has
        // been called on the view; only when that has happened is the
        // opengl context active, and very bad things happen if we tell
        // the dockers to update themselves with a view if the opengl
        // context is not active.
        setActiveView(imageView);

        updateWindowMenu();
    } else {
        unsetActiveView();
    }
}

void KisMainWindow::slotPreferences()
{
    QScopedPointer<KisDlgPreferences> dlgPreferences(new KisDlgPreferences(this));

    if (dlgPreferences->editPreferences()) {
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
                if (node) {
                    node->updateSettings();
                }
            }

        }
        updateWindowMenu();

        d->viewManager->showHideScrollbars();
    }
}

void KisMainWindow::updateTheme()
{
    // reload action icons!
    Q_FOREACH (QAction *action, actionCollection()->actions()) {
        KisIconUtils::updateIcon(action);
    }
    if (d->mdiArea) {
        d->mdiArea->setPalette(qApp->palette());
        for (int i=0; i<d->mdiArea->subWindowList().size(); i++) {
            QMdiSubWindow *window = d->mdiArea->subWindowList().at(i);
            if (window) {
                window->setPalette(qApp->palette());
                KisView *view = qobject_cast<KisView*>(window->widget());
                if (view) {
                    view->slotThemeChanged(qApp->palette());
                }
            }
        }
    }

    customizeTabBar();

    // Update toolbars
    {
        Q_FOREACH (KisToolBar* aToolBar, toolBars()) {
            QObjectList objects;
            objects.append(aToolBar);
            while (!objects.isEmpty()) {
                QWidget* widget = qobject_cast<QWidget*>(objects.takeFirst());
                if (widget) {
                    objects.append(widget->children());
                    widget->setPalette(qApp->palette());
                }
            }
        }
    }
}

void KisMainWindow::slotThemeChanged()
{
    KConfigGroup group(KSharedConfig::openConfig(), "theme");
#ifndef Q_OS_HAIKU
    if (group.readEntry("Theme", "") == d->themeManager->currentThemeName()) return;

    // save theme changes instantly
    group.writeEntry("Theme", d->themeManager->currentThemeName());
#endif
    updateTheme();

    // Make the other top level windows update as well
    Q_FOREACH (QWidget* topLevelWidget, qApp->topLevelWidgets()) {
        if (topLevelWidget == this) {
            // Skip if the current top level widget is this window
            continue;
        }
        if (topLevelWidget->isHidden()) {
            // Skip unwanted top level widgets like menus, dropdowns, tooltips, etc.
            continue;
        }
        KisMainWindow *topLevelMainWindow = qobject_cast<KisMainWindow*>(topLevelWidget);
        if (topLevelMainWindow) {
            topLevelMainWindow->updateTheme();
            Q_EMIT topLevelMainWindow->themeChanged();
        } else {
            QObjectList objects;
            objects.append(topLevelWidget);
            while (!objects.isEmpty()) {
                QWidget* widget = qobject_cast<QWidget*>(objects.takeFirst());
                if (widget) {
                    objects.append(widget->children());
                    KisIconUtils::updateIconCommon(widget);
                }
            }
        }
    }

    Q_EMIT themeChanged();
}

void KisMainWindow::customizeTabBar()
{
    // update MDI area theme
    // Tab close button override
    // just switch this icon out for all OSs so it is easier to see
    QString closeButtonImageUrl;
    QString closeButtonHoverColor;
    if (KisIconUtils::useDarkIcons()) {
        closeButtonImageUrl = QStringLiteral(":/dark_close-tab.svg");
        closeButtonHoverColor = QStringLiteral("lightcoral");
    } else {
        closeButtonImageUrl = QStringLiteral(":/light_close-tab.svg");
        closeButtonHoverColor = QStringLiteral("darkred");
    }
    QString tabStyleSheet = QStringLiteral(R"(
            QTabBar::close-button {
                image: url(%1);
                padding-top: 3px;
            }
            QTabBar::close-button:hover {
                background-color: %2;
            }
            QTabBar::close-button:pressed {
                background-color: red;
            }

            QHeaderView::section {
                padding: 7px;
            }

           )")
           .arg(closeButtonImageUrl, closeButtonHoverColor);


    QTabBar* tabBar = d->findTabBarHACK();
    if (tabBar) {
        tabBar->setStyleSheet(tabStyleSheet);
    }

}

bool KisMainWindow::canvasDetached() const
{
    return centralWidget() != d->widgetStack;
}

void KisMainWindow::setCanvasDetached(bool detach)
{
#ifdef Q_OS_ANDROID
    if (detach) {
        QMessageBox::warning(this, i18nc("@title:window", "Krita"),
                             "Detach Canvas is unsupported on Android");
    }
#else
    if (detach == canvasDetached()) return;

    QWidget *outgoingWidget = centralWidget() ? takeCentralWidget() : nullptr;
    QWidget *incomingWidget = d->canvasWindow->swapMainWidget(outgoingWidget);

    if (incomingWidget) {
        setCentralWidget(incomingWidget);
    }

    if (detach) {
        d->canvasWindow->show();
    } else {
        d->canvasWindow->hide();
    }
    d->toggleDetachCanvas->setChecked(detach);
#endif
}

QWidget * KisMainWindow::canvasWindow() const
{
    return d->canvasWindow;
}

void KisMainWindow::clearRecentFiles()
{
    KisRecentFilesManager::instance()->clear();
}

void KisMainWindow::removeRecentFile(QString url)
{
    KisRecentFilesManager::instance()->remove(QUrl::fromLocalFile(url));
}

void KisMainWindow::slotUpdateSaveActionTitle(const QString &documentPath)
{
    const QString fileName = QFileInfo(documentPath).fileName();

    if (!fileName.isEmpty()) {
        d->saveAction->setToolTip(i18n("Save as %1", fileName));
    }
    else {
        d->saveAction->setToolTip(i18n("Save"));
    }
}

void KisMainWindow::slotUpdateReadWriteMode(bool readWrite)
{
    Q_UNUSED(readWrite);
    d->actionManager()->updateGUI();
}

KisView *KisMainWindow::activeView() const
{
    if (d->activeView) {
        return d->activeView;
    }
    return 0;
}

bool KisMainWindow::openDocument(const QString &path, OpenFlags flags)
{
    ScopedWidgetDisabler disabler(d->welcomePage->dropFrameBorder);
    QApplication::processEvents(); // make UI more responsive

    if (!QFile(path).exists()) {
        if (!(flags & BatchMode)) {
            QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("The file %1 does not exist.", path));
        }
        KisRecentFilesManager::instance()->remove(QUrl::fromLocalFile(path)); //remove the file from the recent-opened-file-list
        return false;
    }
    return openDocumentInternal(path, flags);
}

bool KisMainWindow::openDocumentInternal(const QString &path, OpenFlags flags)
{
    if (!QFile(path).exists()) {
        qWarning() << "KisMainWindow::openDocumentInternal. Could not open:" << path;
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
    // XXX: Why this duplication of OpenFlags...
    if (flags & RecoveryFile) {
        openFlags |= KisDocument::RecoveryFile;
    }

    bool openRet = !(flags & Import) ? newdoc->openPath(path, openFlags) : newdoc->importDocument(path);

    if (!openRet) {
        delete newdoc;
        return false;
    }

    KisPart::instance()->addDocument(newdoc);

    // Try to determine whether this was an unnamed autosave
    if (flags & RecoveryFile &&
            (   path.startsWith(QDir::tempPath())
                || path.startsWith(QDir::homePath())
                ) &&
            (      QFileInfo(path).fileName().startsWith(".krita")
                   || QFileInfo(path).fileName().startsWith("krita")
                   )
            )
    {
        QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
        if (!QFileInfo(path).exists()) {
            path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        }
        newdoc->setPath(path + "/" + newdoc->objectName() + ".kra");
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

KisView* KisMainWindow::addViewAndNotifyLoadingCompleted(KisDocument *document,
                                                         QMdiSubWindow *subWindow)
{
    showWelcomeScreen(false); // see workaround in function header

    KisView *view = KisPart::instance()->createView(document, d->viewManager, this);
    addView(view, subWindow);

    Q_EMIT guiLoadingFinished();

#ifdef Q_OS_ANDROID
    // HACK: When opening documents on Android, the main window sometimes fails
    // to update until the application is shunted to the background and brought
    // back or the menu bar is fiddled with. Flickering the window fixes this.
    // Having a docker that uses QML somehow also fixes this, so this hack is
    // gone again in 5.3 with the introduction of the text properties docker.
    QTimer::singleShot(0, this, [this] {
        hide();
        QTimer::singleShot(0, this, [this] {
            show();
        });
    });
#endif

    return view;
}

QStringList KisMainWindow::showOpenFileDialog(bool isImporting)
{
    KoFileDialog dialog(this, KoFileDialog::ImportFiles, "OpenDocument");
    dialog.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    dialog.setMimeTypeFilters(KisImportExportManager::supportedMimeTypes(KisImportExportManager::Import));
    dialog.setCaption(isImporting ? i18n("Import Images") : i18n("Open Images"));

    QStringList filenames = dialog.filenames();
    filenames.sort();

    return filenames;
}

// Separate from openDocument to handle async loading (remote URLs)
void KisMainWindow::slotLoadCompleted()
{
    KisDocument *newdoc = qobject_cast<KisDocument*>(sender());
    if (newdoc && newdoc->image()) {
        addViewAndNotifyLoadingCompleted(newdoc);

        disconnect(newdoc, SIGNAL(completed()), this, SLOT(slotLoadCompleted()));
        disconnect(newdoc, SIGNAL(canceled(QString)), this, SLOT(slotLoadCanceled(QString)));

        Q_EMIT loadCompleted();
    }
}

void KisMainWindow::slotLoadCanceled(const QString &errMsg)
{
    KisUsageLogger::log(QString("Loading canceled: %1.").arg(errMsg));
    KisDocument* doc = qobject_cast<KisDocument*>(sender());
    Q_ASSERT(doc);
    disconnect(doc, SIGNAL(completed()), this, SLOT(slotLoadCompleted()));
    disconnect(doc, SIGNAL(canceled(QString)), this, SLOT(slotLoadCanceled(QString)));
}

void KisMainWindow::slotSaveCanceled(const QString &errMsg)
{
    if (!errMsg.isEmpty()) {   // empty when cancelled by user
        KisUsageLogger::log(QString("Saving cancelled. Error:").arg(errMsg));
        QMessageBox::critical(this, i18nc("@title:window", "Krita"), errMsg);
    }
    else {
        KisUsageLogger::log(QString("Saving cancelled by the user."));
    }
    slotSaveCompleted();
}

void KisMainWindow::slotSaveCompleted()
{
    KisUsageLogger::log(QString("Saving Completed"));
    KisDocument* doc = qobject_cast<KisDocument*>(sender());
    if (doc) {
        disconnect(doc, SIGNAL(completed()), this, SLOT(slotSaveCompleted()));
        disconnect(doc, SIGNAL(canceled(QString)), this, SLOT(slotSaveCanceled(QString)));
    }

    if (d->deferredClosingEvent) {
        KXmlGuiWindow::closeEvent(d->deferredClosingEvent);
    }
}

bool KisMainWindow::hackIsSaving() const
{
    std::unique_lock<QMutex> l(d->savingEntryMutex, std::try_to_lock);
    return !l.owns_lock();
}

bool KisMainWindow::installBundle(const QString &fileName) const
{
    QFileInfo from(fileName);
    QFileInfo to(QStringLiteral("%1/%2").arg(KoResourcePaths::getAppDataLocation(), from.fileName()));
    if (to.exists()) {
        QFile::remove(to.canonicalFilePath());
    }
    return QFile::copy(fileName, to.absoluteFilePath());
}

QImage KisMainWindow::layoutThumbnail()
{
    int size = 256;
    qreal scale = qreal(size)/qreal(qMax(geometry().width(), geometry().height()));
    QImage layoutThumbnail = QImage(qRound(geometry().width()*scale), qRound(geometry().height()*scale), QImage::Format_ARGB32);
    QPainter gc(&layoutThumbnail);
    gc.fillRect(0, 0, layoutThumbnail.width(), layoutThumbnail.height(), this->palette().dark());

    Q_FOREACH(const QObject *child, children()) {
        if (child->isWidgetType()) {
            const QWidget *w = static_cast<const QWidget *>(child);

            if (w->isVisible() && !w->property("_kis_excludeFromLayoutThumbnail").toBool()) {
                QRect wRect = QRectF(w->geometry().x()*scale
                                     , w->geometry().y()*scale
                                     , w->geometry().width()*scale
                                     , w->geometry().height()*scale
                                     ).toRect();

                wRect = wRect.intersected(layoutThumbnail.rect().adjusted(-1, -1, -1, -1));

                gc.setBrush(this->palette().window());
                if (w == d->widgetStack) {
                    gc.setBrush(d->mdiArea->background());
                }
                gc.setPen(this->palette().windowText().color());
                gc.drawRect(wRect);
            }
        }
    }
    gc.end();
    return layoutThumbnail;
}

bool KisMainWindow::saveDocument(KisDocument *document, bool saveas, bool isExporting, bool isAdvancedExporting )
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
    std::unique_lock<QMutex> l(d->savingEntryMutex, std::try_to_lock);
    if (!l.owns_lock()) return false;

    // no busy wait for saving because it is dangerous!
    KisDelayedSaveDialog dlg(document->image(), KisDelayedSaveDialog::SaveDialog, 0, this);
    dlg.blockIfImageIsBusy();

    if (dlg.result() == KisDelayedSaveDialog::Rejected) {
        return false;
    }
    else if (dlg.result() == KisDelayedSaveDialog::Ignored) {
        QMessageBox::critical(qApp->activeWindow(),
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

    if (document->path().isEmpty()) {
        saveas = true;
    }

    connect(document, SIGNAL(completed()), this, SLOT(slotSaveCompleted()));
    connect(document, SIGNAL(canceled(QString)), this, SLOT(slotSaveCanceled(QString)));

    QByteArray nativeFormat = document->nativeFormatMimeType();
    QByteArray oldMimeFormat = document->mimeType();

    QUrl suggestedURL = QUrl::fromLocalFile(document->path());

    QStringList mimeFilter = KisImportExportManager::supportedMimeTypes(KisImportExportManager::Export);

    mimeFilter = KisImportExportManager::supportedMimeTypes(KisImportExportManager::Export);
    if (!mimeFilter.contains(oldMimeFormat)) {
        dbgUI << "KisMainWindow::saveDocument no export filter for" << oldMimeFormat;

        // --- don't setOutputMimeType in case the user cancels the Save As
        // dialog and then tries to just plain Save ---

        // suggest a different filename extension (yes, we fortunately don't all live in a world of magic :))
        QString suggestedFilename = QFileInfo(suggestedURL.toLocalFile()).completeBaseName();

        if (!suggestedFilename.isEmpty()) {  // ".kra" looks strange for a name
            suggestedFilename = suggestedFilename + "." + KisMimeDatabase::suffixesForMimeType(KIS_MIME_TYPE).first();
            suggestedURL = suggestedURL.adjusted(QUrl::RemoveFilename);
            suggestedURL.setPath(suggestedURL.path() + suggestedFilename);
        }

        // force the user to choose outputMimeType
        saveas = true;
    }

    bool ret = false;

    if (document->path().isEmpty() || isExporting || saveas) {
        // if you're just File/Save As'ing to change filter options you
        // don't want to be reminded about overwriting files etc.
        bool justChangingFilterOptions = false;

        KoFileDialog dialog(this, KoFileDialog::SaveFile, "SaveAs");
        dialog.setCaption(isExporting ? i18n("Exporting") : i18n("Saving As"));

        //qDebug() << ">>>>>" << isExporting << d->lastExportLocation << d->lastExportedFormat << QString::fromLatin1(document->mimeType());

        if (isExporting && !d->lastExportLocation.isEmpty() && !d->lastExportLocation.contains(QDir::tempPath())) {

            // Use the location where we last exported to, if it's set, as the opening location for the file dialog
            QString proposedPath = QFileInfo(d->lastExportLocation).absolutePath();
            // If the document doesn't have a filename yet, use the title
            QString proposedFileName = suggestedURL.isEmpty() ? document->documentInfo()->aboutInfo("title") :  QFileInfo(suggestedURL.toLocalFile()).completeBaseName();
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
                // otherwise use the document's mimetype, or if that is empty, kra, which is the safest.
                default_mime_type = document->mimeType().isEmpty() ? nativeFormat : document->mimeType();
            }
            dialog.setMimeTypeFilters(mimeFilter, QString::fromLatin1(default_mime_type));
        }

        QString newFilePath = dialog.filename();

        if (document->documentInfo()->aboutInfo("title") == i18n("Unnamed")) {
            QString fn = newFilePath;
            QFileInfo info(fn);
            document->documentInfo()->setAboutInfo("title", info.completeBaseName());
        }

        QByteArray outputFormat = nativeFormat;

        QString outputFormatString = KisMimeDatabase::mimeTypeForFile(newFilePath, false);
        outputFormat = outputFormatString.toLatin1();


        if (!isExporting) {
            justChangingFilterOptions = (newFilePath == document->path()) && (outputFormat == document->mimeType());
        }
        else {
            QString path = QFileInfo(d->lastExportLocation).absolutePath();
            QString filename = QFileInfo(document->path()).completeBaseName();
            justChangingFilterOptions = (QFileInfo(newFilePath).absolutePath() == path)
                    && (QFileInfo(newFilePath).completeBaseName() == filename)
                    && (outputFormat == d->lastExportedFormat);
        }

        bool bOk = true;
        if (newFilePath.isEmpty()) {
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
                    ret = document->saveAs(newFilePath, outputFormat, true);
                    if (ret) {
                        dbgUI << "Successful Save As!";
                        KisPart::instance()->queueAddRecentURLToAllMainWindowsOnFileSaved(QUrl::fromLocalFile(newFilePath));
                    } else {
                        dbgUI << "Failed Save As!";

                    }
                }
                else { // Export
                    ret = document->exportDocument(newFilePath, outputFormat, isAdvancedExporting, true);
                    if (ret) {
                        d->lastExportLocation = newFilePath;
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
    if (hackIsSaving()) {
        e->setAccepted(false);
        return;
    }

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
        // TODO: that assignment looks really suspicious, because `e`
        // is destroyed right after the exit of this function
        d->deferredClosingEvent = e;
        d->canvasWindow->close();
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

        // Save state of dock widgets
        for (QMap<QString, QDockWidget*>::const_iterator i = d->dockWidgetsMap.constBegin();
             i != d->dockWidgetsMap.constEnd(); ++i) {
            if (i.value()->widget()) {
                KConfigGroup dockGroup = group.group(QString("DockWidget ") + i.key());
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
    resetAutoSaveSettings(); // Don't let KisKMainWindow override the good stuff we wrote down

}

void KisMainWindow::resizeEvent(QResizeEvent * e)
{
    d->windowSizeDirty = true;
    KXmlGuiWindow::resizeEvent(e);
}


void KisMainWindow::dragMoveEvent(QDragMoveEvent *event)
{
    dragMove(event);
    event->accept();
}

void KisMainWindow::dragLeaveEvent(QDragLeaveEvent *event)
{
    dragLeave();
    event->accept();
}

bool KisMainWindow::windowsLayoutSavingAllowed() const
{
    QAction *action= d->viewManager->actionCollection()->action("view_show_canvas_only");
    return !action || !action->isChecked();
}

void KisMainWindow::showEvent(QShowEvent *event)
{
    // we're here because, we need to make sure everything (dockers, toolbars etc) is loaded and ready before
    // we can hide it.
    if (!event->spontaneous()) {
        setMainWindowLayoutForCurrentMainWidget(d->widgetStack->currentIndex(), false);
    }
    return KXmlGuiWindow::showEvent(event);
}

void KisMainWindow::setMainWindowLayoutForCurrentMainWidget(int widgetIndex, bool widgetIndexChanged)
{

    if (widgetIndex == 0) {
        if (widgetIndexChanged) {
            /// save the state of the window which existed up-to now (this is before we stop auto-saving).
            ///
            /// saving should happen if the call s has come from a real state change, but not from
            /// on-startup initialization

            if (d->mdiArea->subWindowList().isEmpty()) {
                /**
                 * When closing the latest subwindow we should restore the
                 * canvas-only mode to save stuff properly
                 */
                saveWindowState(true);
            } else {
                saveMainWindowSettings(d->windowStateConfig);
            }
        }
        adjustLayoutForWelcomePage();
    }
    else {
        setAutoSaveSettings(d->windowStateConfig, false);
        statusBar()->setVisible(KisConfig(true).showStatusBar());
    }

    QList<QAction *> actions = d->dockWidgetMenu->menu()->actions();
    actions.append(toolBarMenuAction()->menu()->actions());
    for (QAction *action : actions) {
        if (action) {
            action->setEnabled(widgetIndex);
        }
    }
}

void KisMainWindow::adjustLayoutForWelcomePage()
{
    // This makes sure we don't save window state when we're in welcome page mode, because all the dockers
    // etc are hidden while the user is here.
    resetAutoSaveSettings();

    toggleDockersVisibility(false, true);
    if (statusBar()) {
        statusBar()->hide();
    }
    QList<QToolBar *> toolbars = findChildren<QToolBar *>();
    for (QToolBar *toolbar : toolbars) {
        if (toolbar->objectName() == "BrushesAndStuff" || toolbar->objectName() == "editToolBar") {
            toolbar->hide();
        }
    }
}

void KisMainWindow::setActiveView(KisView* view)
{
    d->activeView = view;

    if (d->undoActionsUpdateManager) {
        d->undoActionsUpdateManager->setCurrentDocument(view ? view->document() : 0);
    }

    d->viewManager->setCurrentView(view);

    d->activeViewConnections.clear();
    d->activeViewConnections.addConnection(view->document(),
                                           SIGNAL(sigPathChanged(QString)),
                                           this, SLOT(slotUpdateSaveActionTitle(QString)));
    slotUpdateSaveActionTitle(view->document()->path());
    d->activeViewConnections.addConnection(view->document(),
                                           SIGNAL(sigReadWriteChanged(bool)),
                                           this, SLOT(slotUpdateReadWriteMode(bool)));
    slotUpdateReadWriteMode(view->document()->isReadWrite());

    KisWindowLayoutManager::instance()->activeDocumentChanged(view->document());

    Q_EMIT activeViewChanged();
}

void KisMainWindow::unsetActiveView()
{
    d->activeViewConnections.clear();
    slotUpdateSaveActionTitle(QString());
    slotUpdateReadWriteMode(false);
}

void KisMainWindow::dragMove(QDragMoveEvent * event)
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

void KisMainWindow::dragLeave()
{
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
    const int currentIndex = show ? 0 : 1;
    if (d->widgetStack->currentIndex() != currentIndex) {
        setUpdatesEnabled(false);
        // These have to be done in different sequence to avoid graphical
        // layout glitch during the switch.
        if (show) {
            setMainWindowLayoutForCurrentMainWidget(currentIndex, true);
            d->widgetStack->setCurrentIndex(currentIndex);
        } else {
            d->widgetStack->setCurrentIndex(currentIndex);
            setMainWindowLayoutForCurrentMainWidget(currentIndex, true);
        }
        setUpdatesEnabled(true);
    }
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
    item.title = i18n("Custom Document");
    startupWidget->addCustomDocumentWidget(item.widget, item.title, "Custom Document", item.icon);

    item.widget = new KisImageFromClipboardWidget(startupWidget,
                                            0,
                                            0,
                                            resolution,
                                            colorModel,
                                            colorDepth,
                                            colorProfile,
                                            i18n("Unnamed"));

    item.title = i18n("Create from Clipboard");
    item.icon = "tab-new";

    startupWidget->addCustomDocumentWidget(item.widget, item.title, "Create from ClipBoard", item.icon);

    connect(startupWidget, SIGNAL(documentSelected(KisDocument*)), KisPart::instance(), SLOT(startCustomDocument(KisDocument*)));
    connect(startupWidget, SIGNAL(openTemplate(QUrl)), KisPart::instance(), SLOT(openTemplate(QUrl)));

    startupWidget->exec();
    startupWidget->deleteLater();
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
            bool res = openDocument(url, flags);
            if (!res) {
                warnKrita << "Loading" << url << "failed";
            }
        }
    }
}

void KisMainWindow::slotFileOpenRecent(const QUrl &url)
{
    (void) openDocument(url.toLocalFile(), None);
}

void KisMainWindow::slotFileSave()
{
    if (saveDocument(d->activeView->document(), false, false,false)) {
        Q_EMIT documentSaved();
    }
}

void KisMainWindow::slotFileSaveAs()
{
    if (saveDocument(d->activeView->document(), true, false,false)) {
        Q_EMIT documentSaved();
    }
}

void KisMainWindow::slotExportFile()
{
    if (saveDocument(d->activeView->document(), true, true,false)) {
        Q_EMIT documentSaved();
    }
}
void KisMainWindow::slotExportAdvance()
{
    if (saveDocument(d->activeView->document(), true, true,true)) {
        Q_EMIT documentSaved();
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
    // We don't need to save welcome page's layout
    if (d->widgetStack->currentIndex() == 0) {
        // TODO(sh_zam): We should still save position/geometry, right?
        return;
    }

    if (restoreNormalState) {
        QAction *showCanvasOnly = d->viewManager->actionCollection()->action("view_show_canvas_only");

        if (showCanvasOnly && showCanvasOnly->isChecked()) {
            showCanvasOnly->setChecked(false);
        }

        d->windowStateConfig.writeEntry("ko_geometry", saveGeometry().toBase64());
        d->windowStateConfig.writeEntry("State", saveState().toBase64());

        // if the dockers are hidden at this time, save their state.
        if (!d->toggleDockers->isChecked()) {
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
    const bool showTitlebars = KisConfig(false).showDockerTitleBars();

    // needed because otherwise the layout isn't correctly restored in some situations
    Q_FOREACH (QDockWidget *dock, dockWidgets()) {
        if (dock) {
            dock->setProperty("Locked", false); // Unlock invisible dockers
            dock->toggleViewAction()->setEnabled(true);
            dock->hide();
            if (dock->titleBarWidget() && !dock->titleBarWidget()->inherits("KisUtilityTitleBar")) {
                dock->titleBarWidget()->setVisible(showTitlebars);
            }
        }
    }

    bool success = KXmlGuiWindow::restoreState(state);

    if (!success) {
        KXmlGuiWindow::restoreState(oldState);
        Q_FOREACH (QDockWidget *dock, dockWidgets()) {
            if (dock->titleBarWidget() && !dock->titleBarWidget()->inherits("KisUtilityTitleBar")) {
                dock->titleBarWidget()->setVisible(showTitlebars || dock->isFloating());
            }
        }
        return false;
    }
    return success;
}

void KisMainWindow::restoreWorkspace()
{
    QString md5 = sender()->property("md5").toString();
    KoResourceServer<KisWorkspaceResource> *rserver = KisResourceServerProvider::instance()->workspaceServer();
    KoResourceSP resource = rserver->resource(md5, "", "");
    if (resource) {
        restoreWorkspace(resource);
    }
    else {
        qWarning() << "Could not retrieve resource for" << md5;
    }
}

void KisMainWindow::openCommandBar()
{
    QList<KisKActionCollection *> actionCollections;

    auto clients = guiFactory()->clients();
    int actionsCount = 0;
    for (const KisKXMLGUIClient *c : clients) {
        if (!c) {
            continue;
        }
        if (auto collection = c->actionCollection()) {
            actionCollections.append(collection);
            actionsCount += collection->count();
        }
    }

    if (activeKisView()) {
        KisKActionCollection *layerActionCollection = new KisKActionCollection(0, "layeractions (disposable)");
        layerActionCollection->setComponentDisplayName(i18n("Layers/Masks"));
        KisNodeActivationActionCreatorVisitor v(layerActionCollection, viewManager()->nodeManager());
        activeKisView()->image()->rootLayer()->accept(v);
        actionCollections.append(layerActionCollection);
        actionsCount += layerActionCollection->count();
    }

    d->commandBar->updateBar(actionCollections, actionsCount);

    // The following line is needed to work around input method not working
    // on Windows.
    // See https://bugs.kde.org/show_bug.cgi?id=395598
    // and https://bugs.kde.org/show_bug.cgi?id=438122
    d->commandBar->activateWindow();

    // The following line is present in Kate's version and was ported over
    // but I am sceptical of its use. I worry that it may subtly cause other
    // issues, and since the command bar appears to work fine without it, I
    // believe it may be better to leave it out.  -- Alvin
    // centralWidget()->setFocusProxy(d->commandBar);
}

void KisMainWindow::slotStoragesWarning(const QString &/*location*/)
{
    QString warning;
    if (!checkActiveBundlesAvailable()) {
        warning = i18n("You don't have any resource bundles enabled.");
    }

    if (!checkPaintOpAvailable()) {
        warning += i18n("\nThere are no brush presets available. Please enable a bundle that has presets before continuing.\n");
        QMessageBox::critical(this, i18nc("@title:window", "Krita"), warning);

        QAction *action = actionCollection()->action("manage_bundles");
        if (action) {
            action->trigger();
        }
    }

    if (!checkActiveBundlesAvailable()) {
        QMessageBox::warning(this, i18nc("@title:window", "Krita"), warning + i18n("\nOnly your local resources are available."));
    }

}

bool KisMainWindow::restoreWorkspace(KoResourceSP res)
{
    KisWorkspaceResourceSP workspace = res.dynamicCast<KisWorkspaceResource>();

    bool success = restoreWorkspaceState(workspace->dockerState());

    const bool showTitlebars = KisConfig(false).showDockerTitleBars();
    Q_FOREACH (QDockWidget *dock, dockWidgets()) {
        if (dock->titleBarWidget() && !dock->titleBarWidget()->inherits("KisUtilityTitleBar")) {
            dock->titleBarWidget()->setVisible(showTitlebars || dock->isFloating());
        }
    }

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

    return true;
}

void KisMainWindow::slotFileQuit()
{
    // Do not close while KisMainWindow has the savingEntryMutex locked, bug409395.
    // After the background saving job is initiated, KisDocument blocks closing
    // while it saves itself.
    if (hackIsSaving()) {
        return;
    }
    KisPart::instance()->closeSession();
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
        bool startFrom1 = dlg.startFrom1();
        bool autoAddHoldframes = dlg.autoAddHoldframes();


        KoUpdaterPtr updater =
                !document->fileBatchMode() ? viewManager()->createUnthreadedUpdater(i18n("Import frames")) : 0;
        KisAnimationImporter importer(document->image(), updater);
        int isAscending = dlg.isAscending();
        KisImportExportErrorCode status = importer.import(files, firstFrame, step, autoAddHoldframes, startFrom1, isAscending);  // modify here, add a flag

        if (!status.isOk() && !status.isInternalError()) {
            QString msg = status.errorMessage();
            if (!msg.isEmpty())
                QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("Could not finish import animation:\n%1", msg));
        }
        activeView()->canvasBase()->refetchDataFromImage();
    }
}

void KisMainWindow::importVideoAnimation()
{
    KisDocument *document;
    KisDlgImportVideoAnimation dlg(this, activeView());

    if (dlg.exec() == QDialog::Accepted) {
        const QTemporaryDir outputLocation(QDir::tempPath() + QDir::separator() + "krita" + QDir::separator() + "import_files");
        RenderedFrames renderedFrames = dlg.renderFrames(QDir(outputLocation.path()));
        dbgFile << "Frames rendered to directory: " << outputLocation.path();
        QStringList documentInfoList = dlg.documentInfo();

        if (renderedFrames.isEmpty()) return;
        
        dbgFile << "Animation Import options: " << documentInfoList;

        int firstFrame = 0;
        const int step = documentInfoList[0].toInt();
        const int fps = documentInfoList[1].toInt();
        const int totalFrames = renderedFrames.framesNeedRelocation() ? (renderedFrames.renderedFrameTargetTimes.last() + 1) : renderedFrames.size() * step;
        const QString name = QFileInfo(documentInfoList[3]).fileName();
        const bool useCurrentDocument = documentInfoList[4].toInt();
        bool useDocumentColorSpace = false;

        if ( useCurrentDocument ) {
            document = activeView()->document();

            dbgFile << "Current frames:" << document->image()->animationInterface()->totalLength() << "total frames:" << totalFrames;
            if ( document->image()->animationInterface()->totalLength() < totalFrames ) {
                document->image()->animationInterface()->setDocumentRangeStartFrame(0);
                document->image()->animationInterface()->setDocumentRangeEndFrame(totalFrames);
            }

        } else {

            const int width = documentInfoList[5].toInt();
            const int height = documentInfoList[6].toInt();
            const double resolution = documentInfoList[7].toDouble();

            const QString colorModel = documentInfoList[8];
            const QString colorDepth = documentInfoList[9];
            const QString profile = documentInfoList[10];
            useDocumentColorSpace = profile != "Default";

            document = KisPart::instance()->createDocument();
            document->setObjectName(name);

            KisPart::instance()->addDocument(document, false);
            const KoColorSpace *cs = KoColorSpaceRegistry::instance()->colorSpace(colorModel, colorDepth, profile);
            Q_ASSERT(cs);

            QColor qc(Qt::white);
            qc.setAlpha(0);
            KoColor bgColor(qc, cs);

            if (!document->newImage(name, width, height, cs, bgColor, KisConfig::RASTER_LAYER, 1, "", double(resolution / 72) )) {
                QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("Failed to create new document. Animation import aborted."));
                return;
            }

            document->image()->animationInterface()->setFramerate(fps);
            document->image()->animationInterface()->setDocumentRangeStartFrame(0);
            document->image()->animationInterface()->setDocumentRangeEndFrame(totalFrames);


            this->showDocument(document);

        }

        KoUpdaterPtr updater =
                !document->fileBatchMode() ? viewManager()->createUnthreadedUpdater(i18n("Import frames")) : 0;
        KisAnimationImporter importer(document->image(), updater);
        KisImportExportErrorCode status = importer.import(renderedFrames.renderedFrameFiles, firstFrame, step, false, false, 0, useDocumentColorSpace, renderedFrames.renderedFrameTargetTimes);

        if (!status.isOk() && !status.isInternalError()) {
            QString msg = status.errorMessage();
            if (!msg.isEmpty())
                QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("Could not finish import animation:\n%1", msg));
        }

        activeView()->canvasBase()->refetchDataFromImage();
        document->image()->refreshGraphAsync();
        document->image()->waitForDone();

    }
}

void KisMainWindow::renderAnimation()
{
    if (!activeView()) return;

    KisImageSP image = viewManager()->image();

    if (!image) return;
    if (!image->animationInterface()->hasAnimation()) return;

    KisDocument *doc = viewManager()->document();

    KisDlgAnimationRenderer dlgAnimationRenderer(doc, viewManager()->mainWindow());
    dlgAnimationRenderer.setCaption(i18n("Render Animation"));
    if (dlgAnimationRenderer.exec() == QDialog::Accepted) {
        KisAnimationRenderingOptions encoderOptions = dlgAnimationRenderer.getEncoderOptions();
        KisAnimationRender::render(doc, viewManager(), encoderOptions);
    }
}

void KisMainWindow::renderAnimationAgain()
{
    if (!activeView()) return;

    KisImageSP image = viewManager()->image();

    if (!image) return;
    if (!image->animationInterface()->hasAnimation()) return;

    KisDocument *doc = viewManager()->document();

    KisConfig cfg(true);

    KisPropertiesConfigurationSP settings = cfg.exportConfiguration("ANIMATION_EXPORT");

    KisAnimationRenderingOptions encoderOptions;
    encoderOptions.fromProperties(settings);

    KisAnimationRender::render(doc, viewManager(), encoderOptions);
}

void KisMainWindow::slotConfigureToolbars()
{
    saveWindowState();
    KisKEditToolBar edit(factory(), this);
    connect(&edit, SIGNAL(newToolBarConfig()), this, SLOT(slotNewToolbarConfig()));
    (void) edit.exec();
    applyToolBarLayout();
}

void KisMainWindow::slotResetConfigurations()
{
    KisApplication *kisApp = static_cast<KisApplication*>(qApp);
    kisApp->askResetConfig();
}

void KisMainWindow::slotNewToolbarConfig()
{
    applyMainWindowSettings(d->windowStateConfig);

    KisKXMLGUIFactory *factory = guiFactory();
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
    KisToolBar * bar = toolBar(sender()->objectName());
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
    d->fullScreenMode->setChecked(isFullScreen());
}

QDockWidget* KisMainWindow::createDockWidget(KoDockFactoryBase* factory)
{
    QDockWidget* dockWidget = 0;
    bool lockAllDockers = KisConfig(true).readEntry<bool>("LockAllDockerPanels", false);

    if (!d->dockWidgetsMap.contains(factory->id())) {
        dockWidget = factory->createDockWidget();
        KAcceleratorManager::setNoAccel(dockWidget);

        // It is quite possible that a dock factory cannot create the dock; don't
        // do anything in that case.
        if (!dockWidget) {
            warnKrita << "Could not create docker for" << factory->id();
            return 0;
        }

        KoDockWidgetTitleBar *titleBar = dynamic_cast<KoDockWidgetTitleBar*>(dockWidget->titleBarWidget());

        // Check if the dock widget is supposed to be collapsible
        if (!dockWidget->titleBarWidget()) {
            titleBar = new KoDockWidgetTitleBar(dockWidget);
            dockWidget->setTitleBarWidget(titleBar);
        }
        if (titleBar) {
            titleBar->setFont(KisUiFont::dockFont());
        }

        if (dockWidget->titleBarWidget() && !dockWidget->titleBarWidget()->inherits("KisUtilityTitleBar")) {
            dockWidget->titleBarWidget()->setVisible(KisConfig(true).showDockerTitleBars());
        }

        dockWidget->setObjectName(factory->id());
        dockWidget->setParent(this);
        if (lockAllDockers) {
            if (dockWidget->titleBarWidget() && !dockWidget->titleBarWidget()->inherits("KisUtilityTitleBar")) {
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

        bool locked = group.readEntry("Locked", false);
        if (titleBar && locked) {
            titleBar->setLocked(true);
        }

        d->dockWidgetsMap.insert(factory->id(), dockWidget);
    }
    else {
        dockWidget = d->dockWidgetsMap[factory->id()];
    }

#ifdef Q_OS_MACOS
    dockWidget->setAttribute(Qt::WA_MacSmallSize, true);
#endif
    dockWidget->setFont(KisUiFont::dockFont());

    connect(dockWidget, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(forceDockTabFonts()));

    return dockWidget;
}

void KisMainWindow::forceDockTabFonts()
{
    Q_FOREACH (QObject *child, children()) {
        if (child->inherits("QTabBar")) {
            ((QTabBar *)child)->setFont(KisUiFont::dockFont());
        }
    }
}

void KisMainWindow::slotUpdateWidgetStyle()
{
     KisConfig cfg(true);
     QString themeFromConfig = cfg.widgetStyle();

     Q_FOREACH (auto key, d->actionMap.keys()) { // find checked style to save to config
         if(d->actionMap.value(key)->isChecked()) {
            cfg.setWidgetStyle(key);
            qApp->setProperty(currentUnderlyingStyleNameProperty, key);
            qApp->setStyle(key);
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

    observers.append(static_cast<KoCanvasObserverBase*>(KisPart::instance()->playbackEngine()));

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


void KisMainWindow::toggleDockersVisibility(bool visible, bool onWelcomePage)
{
    if (!visible) {
        d->dockerStateBeforeHiding = saveState();

        Q_FOREACH (QObject* widget, children()) {
            if (widget->inherits("QDockWidget")) {
                QDockWidget* dw = static_cast<QDockWidget*>(widget);
                if (dw->isVisible() && !(onWelcomePage && dw->property("ShowOnWelcomePage").toBool())) {
                    dw->hide();
                }
            }
        }
    }
    else {
        restoreState(d->dockerStateBeforeHiding);
    }
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

#ifndef Q_OS_ANDROID
    menu->addAction(d->newWindow);
#endif
    menu->addAction(d->documentMenu);

    QMenu *docMenu = d->documentMenu->menu();
    docMenu->clear();

    QFontMetrics fontMetrics = docMenu->fontMetrics();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    QRect geom = this->geometry();
    QPoint p(geom.width() / 2 + geom.left(), geom.height() / 2 + geom.top());
    QScreen *screen = qApp->screenAt(p);

    int fileStringWidth = 300;
    if (screen) {
        fileStringWidth = int(screen->availableGeometry().width() * .40f);
    }
#else
    int fileStringWidth = int(QApplication::desktop()->screenGeometry(this).width() * .40f);
#endif
    Q_FOREACH (QPointer<KisDocument> doc, KisPart::instance()->documents()) {
        if (doc) {
            QString title = fontMetrics.elidedText(doc->path(), Qt::ElideMiddle, fileStringWidth);
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
    KisResourceModel resourceModel(ResourceType::Workspaces);
    KisResourceIterator resourceIterator(&resourceModel);
    KisMainWindow *m_this = this;

    while (resourceIterator.hasNext()) {
        KisResourceItemSP resource = resourceIterator.next();
        QAction *action = workspaceMenu->addAction(resource->name());
        action->setProperty("md5", QVariant::fromValue<QString>(resource->md5sum()));
        connect(action, SIGNAL(triggered()), this, SLOT(restoreWorkspace()));
    }
    workspaceMenu->addSeparator();
    connect(workspaceMenu->addAction(i18nc("@action:inmenu", "&Import Workspace...")),
            &QAction::triggered,
            this,
            [&]()
    {
        QStringList mimeTypes = KisResourceLoaderRegistry::instance()->mimeTypes(ResourceType::Workspaces);

        KoFileDialog dialog(0, KoFileDialog::OpenFile, "OpenDocument");
        dialog.setMimeTypeFilters(mimeTypes);
        dialog.setCaption(i18nc("@title:window", "Choose File to Add"));
        QString filename = dialog.filename();

        KisResourceUserOperations::importResourceFileWithUserInput(this, "", ResourceType::Workspaces, filename);
    });

    connect(workspaceMenu->addAction(i18nc("@action:inmenu", "&New Workspace...")),
            &QAction::triggered,
            [=]() {
        QString name;
        name = QInputDialog::getText(this, i18nc("@title:window", "New Workspace..."),
                                                                i18nc("@label:textbox", "Name:"));
        if (name.isEmpty()) {
            return;
        }

        KisWorkspaceResourceSP workspace(new KisWorkspaceResource(""));
        workspace->setDockerState(m_this->saveState());
        workspace->setImage(layoutThumbnail());
        workspace->setValid(true);

        // this line must happen before we save the workspace to resource folder or other places
        // because it mostly just triggers palettes to be saved into the workspace
        d->viewManager->canvasResourceProvider()->notifySavingWorkspace(workspace);
        workspace->setValid(true);

        workspace->setFilename(name.replace(" ", "_") + workspace->defaultFileExtension());
        workspace->setName(name);

        KisResourceUserOperations::addResourceWithUserInput(this, workspace);
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
                text = i18n("&%1 %2", i + 1, fontMetrics.elidedText(child->document()->path(), Qt::ElideMiddle, fileStringWidth));
            }
            else {
                text = i18n("%1 %2", i + 1, fontMetrics.elidedText(child->document()->path(), Qt::ElideMiddle, fileStringWidth));
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
}

void KisMainWindow::updateSubwindowFlags()
{
    bool onlyOne = false;
    if (d->mdiArea->subWindowList().size() == 1 && d->mdiArea->viewMode() == QMdiArea::SubWindowView) {
        onlyOne = true;
    }
    Q_FOREACH (QMdiSubWindow *subwin, d->mdiArea->subWindowList()) {
        if (onlyOne) {
            subwin->setWindowFlags(subwin->windowFlags() | Qt::FramelessWindowHint);
            subwin->showMaximized();
        } else {
            subwin->setWindowFlags((subwin->windowFlags() | Qt::FramelessWindowHint) ^ Qt::FramelessWindowHint);
        }
    }
}

void KisMainWindow::setActiveSubWindow(QWidget *window)
{
    if (!window) {
        unsetActiveView();
        return;
    }
    QMdiSubWindow *subwin = qobject_cast<QMdiSubWindow *>(window);
    //dbgKrita << "setActiveSubWindow();" << subwin << d->activeSubWindow;

    if (subwin && subwin != d->activeSubWindow) {
        KisView *view = qobject_cast<KisView *>(subwin->widget());
        //dbgKrita << "\t" << view << activeView();
        if (view && view != activeView()) {
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
        if (viewMode == QMdiArea::TabbedView) {
            subwin->setWindowState(Qt::WindowMaximized);
        }

        /**
         * Dirty workaround for a bug in Qt (checked on Qt 5.6.1):
         *
         * If you make a window "Show on top" and then switch to the tabbed mode
         * the window will continue to be painted in its initial "mid-screen"
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
#ifdef Q_OS_MACOS
    updateSubwindowFlags();
#endif

    KConfigGroup group( KSharedConfig::openConfig(), "theme");
#ifndef Q_OS_HAIKU
    d->themeManager->setCurrentTheme(group.readEntry("Theme", "Krita dark"));
#endif
    d->actionManager()->updateGUI();

    QString s = cfg.getMDIBackgroundColor();
    KoColor c = KoColor::fromXML(s);
    QBrush brush(c.toQColor());
    d->mdiArea->setBackground(brush);

    QString backgroundImage = cfg.getMDIBackgroundImage();
    if (backgroundImage != "") {
        QImage image(backgroundImage);
        QBrush brush(image);
        d->mdiArea->setBackground(brush);
    }

    d->mdiArea->update();

    qApp->setFont(KisUiFont::normalFont());

    Q_FOREACH (QObject* widget, children()) {
        if (widget->inherits("QDockWidget")) {
            QDockWidget* dw = static_cast<QDockWidget*>(widget);
            dw->setFont(KisUiFont::dockFont());
        }
    }
}

KisView* KisMainWindow::newView(QObject *document, QMdiSubWindow *subWindow)
{
    KisDocument *doc = qobject_cast<KisDocument*>(document);
    KisView *view = addViewAndNotifyLoadingCompleted(doc, subWindow);
    d->actionManager()->updateGUI();

    return view;
}

void KisMainWindow::newWindow()
{
#ifdef Q_OS_ANDROID
    // Check if current mainwindow exists, just to be sure.
    if (KisPart::instance()->currentMainwindow()) {
        QMessageBox::warning(this, i18nc("@title:window", "Krita"),
                             "Creating a New Main Window is unsupported on Android");
        return;
    }
#endif
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

    slotStoragesWarning();

    // window is created signal (used in Python)
    // there must be some asynchronous things happening in the constructor, because the window cannot
    // be referenced until after this timeout is done
    Q_EMIT KisPart::instance()->sigMainWindowCreated();
}

void KisMainWindow::showErrorAndDie()
{
    QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Installation error"), m_errorMessage);
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
#ifdef Q_OS_MACOS
        w->setAttribute(Qt::WA_MacSmallSize, true);
#endif
        w->setFont(KisUiFont::dockFont());
    }

    if (d->toolOptionsDocker) {
        d->toolOptionsDocker->setOptionWidgets(optionWidgetList);
    }
    else {
        d->viewManager->paintOpBox()->newOptionWidgets(optionWidgetList);
    }
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

    d->saveAction = actionManager->createStandardAction(KStandardAction::Save, this, SLOT(slotFileSave()));
    d->saveAction->setActivationFlags(KisAction::IMAGE_IS_WRITABLE);

    d->saveActionAs = actionManager->createStandardAction(KStandardAction::SaveAs, this, SLOT(slotFileSaveAs()));
    d->saveActionAs->setActivationFlags(KisAction::ACTIVE_IMAGE);

    d->undo = actionManager->createStandardAction(KStandardAction::Undo, this, SLOT(undo()));
    d->undo->setActivationFlags(KisAction::ACTIVE_IMAGE);

    d->redo = actionManager->createStandardAction(KStandardAction::Redo, this, SLOT(redo()));
    d->redo->setActivationFlags(KisAction::ACTIVE_IMAGE);

    d->undoActionsUpdateManager.reset(new KisUndoActionsUpdateManager(d->undo, d->redo));
    d->undoActionsUpdateManager->setCurrentDocument(d->activeView ? d->activeView->document() : 0);

    d->importAnimation  = actionManager->createAction("file_import_animation");
    connect(d->importAnimation, SIGNAL(triggered()), this, SLOT(importAnimation()));

    d->importVideoAnimation = actionManager->createAction("file_import_video_animation");
    connect(d->importVideoAnimation, SIGNAL(triggered()), this, SLOT(importVideoAnimation()));
    
    d->renderAnimation = actionManager->createAction("render_animation");
    d->renderAnimation->setActivationFlags(KisAction::IMAGE_HAS_ANIMATION);
    connect( d->renderAnimation, SIGNAL(triggered()), this, SLOT(renderAnimation()));

    d->renderAnimationAgain = actionManager->createAction("render_animation_again");
    d->renderAnimationAgain->setActivationFlags(KisAction::IMAGE_HAS_ANIMATION);
    connect( d->renderAnimationAgain, SIGNAL(triggered()), this, SLOT(renderAnimationAgain()));

    d->closeAll = actionManager->createAction("file_close_all");
    connect(d->closeAll, SIGNAL(triggered()), this, SLOT(slotFileCloseAll()));

    d->importFile  = actionManager->createAction("file_import_file");
    d->importFile->setActivationFlags(KisAction::IMAGE_IS_WRITABLE);
    connect(d->importFile, SIGNAL(triggered(bool)), this, SLOT(slotImportFile()));

    d->exportFile  = actionManager->createAction("file_export_file");
    connect(d->exportFile, SIGNAL(triggered(bool)), this, SLOT(slotExportFile()));

    d->exportFileAdvance  = actionManager->createAction("file_export_advanced");
    connect(d->exportFileAdvance, SIGNAL(triggered(bool)), this, SLOT(slotExportAdvance()));

    /* The following entry opens the document information dialog.  Since the action is named so it
        intends to show data this entry should not have a trailing ellipses (...).  */
    d->showDocumentInfo  = actionManager->createAction("file_documentinfo");
    connect(d->showDocumentInfo, SIGNAL(triggered(bool)), this, SLOT(slotDocumentInfo()));
#ifndef Q_OS_HAIKU
    d->themeManager->setThemeMenuAction(new KActionMenu(i18nc("@action:inmenu", "&Themes"), this));
    d->themeManager->registerThemeActions(actionCollection());
    connect(d->themeManager, SIGNAL(signalThemeChanged()), this, SLOT(slotThemeChanged()), Qt::QueuedConnection);
    connect(this, SIGNAL(themeChanged()), d->welcomePage, SLOT(slotUpdateThemeColors()), Qt::UniqueConnection);
#endif
    d->toggleDockers = actionManager->createAction("view_toggledockers");


    KisConfig(true).showDockers(true);
    d->toggleDockers->setChecked(true);
    connect(d->toggleDockers, SIGNAL(toggled(bool)), SLOT(toggleDockersVisibility(bool)));

    d->resetConfigurations  = actionManager->createAction("reset_configurations");
    connect(d->resetConfigurations, SIGNAL(triggered()), this, SLOT(slotResetConfigurations()));

#ifndef Q_OS_ANDROID
    d->toggleDetachCanvas = actionManager->createAction("view_detached_canvas");
    d->toggleDetachCanvas->setChecked(false);
    connect(d->toggleDetachCanvas, SIGNAL(toggled(bool)), SLOT(setCanvasDetached(bool)));
#endif
    setCanvasDetached(false);

    d->toggleDockerTitleBars = actionManager->createAction("view_toggledockertitlebars");
    d->toggleDockerTitleBars->setChecked(KisConfig(false).showDockerTitleBars());
    connect(d->toggleDockerTitleBars, SIGNAL(toggled(bool)), SLOT(showDockerTitleBars(bool)));

    actionCollection()->addAction("settings_dockers_menu", d->dockWidgetMenu);
    actionCollection()->addAction("window", d->windowMenu);

    actionCollection()->addAction("style_menu", d->styleMenu); // for widget styles: breeze, fusion, etc

    d->mdiCascade = actionManager->createAction("windows_cascade");
    connect(d->mdiCascade, SIGNAL(triggered()), d->mdiArea, SLOT(cascadeSubWindows()));

    d->mdiTile = actionManager->createAction("windows_tile");
    connect(d->mdiTile, SIGNAL(triggered()), d->mdiArea, SLOT(tileSubWindows()));

    d->mdiNextWindow = actionManager->createAction("windows_next");
    connect(d->mdiNextWindow, SIGNAL(triggered()), d->mdiArea, SLOT(activateNextSubWindow()));

    d->mdiPreviousWindow = actionManager->createAction("windows_previous");
    connect(d->mdiPreviousWindow, SIGNAL(triggered()), d->mdiArea, SLOT(activatePreviousSubWindow()));

#ifndef Q_OS_ANDROID
    d->newWindow = actionManager->createAction("view_newwindow");
    connect(d->newWindow, SIGNAL(triggered(bool)), this, SLOT(newWindow()));
#endif

    d->close = actionManager->createStandardAction(KStandardAction::Close, this, SLOT(closeCurrentWindow()));

    d->showSessionManager = actionManager->createAction("file_sessions");
    connect(d->showSessionManager, SIGNAL(triggered(bool)), this, SLOT(slotShowSessionManager()));

    d->commandBarAction = actionManager->createAction("command_bar_open");
    connect(d->commandBarAction, SIGNAL(triggered(bool)), this, SLOT(openCommandBar()));

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
    KisToolBar::setToolBarsLocked(KisConfig(true).readEntry<bool>("LockAllDockerPanels", false));

    Q_FOREACH (KisToolBar *toolBar, toolBars()) {
        toolBar->layout()->setSpacing(4);
        toolBar->setStyleSheet("QToolBar { border: none }"); // has a border in "Fusion" style that people don't like

        // Hide text for buttons with an icon in the toolbar
        Q_FOREACH (QAction *ac, toolBar->actions()){
            if (ac->icon().pixmap(QSize(1,1)).isNull() == false){
                ac->setPriority(QAction::LowPriority);
            } else {
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
        QRect desk = QGuiApplication::screens().at(scnum)->availableVirtualGeometry();

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

void KisMainWindow::showDockerTitleBars(bool show)
{
    Q_FOREACH (QDockWidget *dock, dockWidgets()) {
        if (dock->titleBarWidget() && !dock->titleBarWidget()->inherits("KisUtilityTitleBar")) {
            dock->titleBarWidget()->setVisible(show || dock->isFloating());
        }
    }

    KisConfig cfg(true);
    cfg.setShowDockerTitleBars(show);
}

void KisMainWindow::slotXmlGuiMakingChanges(bool finished)
{
    if (finished) {
        subWindowActivated();
    }
}

void KisMainWindow::orientationChanged()
{
    QScreen *screen = QGuiApplication::primaryScreen();

    for (QWindow* window: QGuiApplication::topLevelWindows()) {
        // Android: we shouldn't transform Window managers independent of its child
        if ((window->type() == Qt::Popup)
            && (window->flags() & Qt::FramelessWindowHint) == 0
            && (window->geometry().topLeft() != QPoint(0, 0))) {
            // We are using reversed values. Because geometry returned is not the updated
            // one, but the previous one.
            int screenHeight = screen->geometry().width();
            int screenWidth = screen->geometry().height();

            // scaling
            int new_x = (window->position().x() * screenWidth) / screenHeight;
            int new_y = (window->position().y() * screenHeight) / screenWidth;

            // window width or height shouldn't change
            int winWidth = window->geometry().width();
            int winHeight = window->geometry().height();

            // Try best to not let the window go beyond screen.
            if (new_x > screenWidth - winWidth) {
                new_x = screenWidth - winWidth;
                if (new_x < 0)
                    new_x = 0;
            }
            if (new_y > screenHeight - winHeight) {
                new_y = screenHeight - winHeight;
                if (new_y < 0)
                    new_y = 0;
            }

            window->setPosition(QPoint(new_x, new_y));
        }
    }
}

bool KisMainWindow::checkActiveBundlesAvailable()
{
    KisStorageFilterProxyModel proxy;
    proxy.setSourceModel(KisStorageModel::instance());
    proxy.setFilter(KisStorageFilterProxyModel::ByStorageType,
                    QStringList()
                    << KisResourceStorage::storageTypeToUntranslatedString(KisResourceStorage::StorageType::Bundle));

    return (proxy.rowCount() > 0);
}

bool KisMainWindow::checkPaintOpAvailable()
{
    KisPaintOpPresetResourceServer * rserver = KisResourceServerProvider::instance()->paintOpPresetServer();
    return (rserver->resourceCount() > 0);
}

#include <moc_KisMainWindow.cpp>
