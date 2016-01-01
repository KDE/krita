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
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QDockWidget>
#include <QIcon>
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
#include <QProgressBar>
#include <QSignalMapper>
#include <QTabBar>
#include <QMoveEvent>
#include <QUrl>
#include <QMessageBox>
#include <QTemporaryFile>
#include <QStatusBar>
#include <QMenu>
#include <QMenuBar>

#include <kactioncollection.h>
#include <QAction>
#include <kactionmenu.h>
#include <kis_debug.h>
#include <kedittoolbar.h>
#include <khelpmenu.h>
#include <klocalizedstring.h>

#ifdef HAVE_KIO
#include <krecentdocument.h>
#endif
#include <krecentfilesaction.h>
#include <KoResourcePaths.h>
#include <ktoggleaction.h>
#include <ktoolbar.h>
#include <kmainwindow.h>
#include <kxmlguiwindow.h>
#include <kxmlguifactory.h>
#include <kxmlguiclient.h>
#include <kguiitem.h>
#include <kwindowconfig.h>

#include "KoDockFactoryBase.h"
#include "KoDockWidgetTitleBar.h"
#include "KoDocumentInfoDlg.h"
#include "KoDocumentInfo.h"
#include "KoFileDialog.h"
#include <kis_icon.h>
#include <KoPageLayoutDialog.h>
#include <KoPageLayoutWidget.h>
#include <KoToolManager.h>
#include <KoZoomController.h>
#include "KoToolDocker.h"
#include <KoToolBoxFactory.h>
#include <KoDockRegistry.h>
#include <KoPluginLoader.h>
#include <KoColorSpaceEngine.h>

#include "KisView.h"
#include "KisDocument.h"
#include "KisImportExportManager.h"
#include "KisPrintJob.h"
#include "KisPart.h"
#include "KisApplication.h"

#include "kis_action.h"
#include "kis_canvas_controller.h"
#include "kis_canvas2.h"
#include "KisViewManager.h"
#include "KisDocument.h"
#include "dialogs/kis_dlg_preferences.h"
#include "kis_config_notifier.h"
#include "kis_canvas_resource_provider.h"
#include "kis_node.h"
#include "kis_image.h"
#include "kis_group_layer.h"
#include "kis_paintop_settings.h"
#include "kis_paintop_box.h"
#include "kis_config.h"
#include "kis_config_notifier.h"
#include "dialogs/kis_about_application.h"
#include "kis_mainwindow_observer.h"
#include "kis_action_manager.h"
#include "thememanager.h"
#include "kis_resource_server_provider.h"
#ifdef HAVE_OPENGL
#include "kis_animation_exporter.h"
#endif
#include "kis_icon_utils.h"
#include <KisImportExportFilter.h>
#include <KisDocumentEntry.h>

class ToolDockerFactory : public KoDockFactoryBase
{
public:
    ToolDockerFactory() : KoDockFactoryBase() { }

    QString id() const {
        return "sharedtooldocker";
    }

    QDockWidget* createDockWidget() {
        KoToolDocker* dockWidget = new KoToolDocker();
        dockWidget->setTabEnabled(false);
        return dockWidget;
    }

    DockPosition defaultDockPosition() const {
        return DockRight;
    }
};

class Q_DECL_HIDDEN KisMainWindow::Private
{
public:
    Private(KisMainWindow *parent)
        : viewManager(0)
        , firstTime(true)
        , windowSizeDirty(false)
        , readOnly(false)
        , isImporting(false)
        , isExporting(false)
        , noCleanup(false)
        , showDocumentInfo(0)
        , saveAction(0)
        , saveActionAs(0)
        , printAction(0)
        , printActionPreview(0)
        , exportPdf(0)
        , closeAll(0)
//        , reloadFile(0)
        , importFile(0)
        , exportFile(0)
        , undo(0)
        , redo(0)
        , newWindow(0)
        , close(0)
        , mdiCascade(0)
        , mdiTile(0)
        , mdiNextWindow(0)
        , mdiPreviousWindow(0)
        , toggleDockers(0)
        , toggleDockerTitleBars(0)
        , dockWidgetMenu(new KActionMenu(i18nc("@action:inmenu", "&Dockers"), parent))
        , windowMenu(new KActionMenu(i18nc("@action:inmenu", "&Window"), parent))
        , documentMenu(new KActionMenu(i18nc("@action:inmenu", "New &View"), parent))
        , helpMenu(0)
        , brushesAndStuff(0)
        , recentFiles(0)
        , toolOptionsDocker(0)
        , deferredClosingEvent(0)
        , themeManager(0)
        , mdiArea(new QMdiArea(parent))
        , activeSubWindow(0)
        , windowMapper(new QSignalMapper(parent))
        , documentMapper(new QSignalMapper(parent))
        , lastExportSpecialOutputFlag(0)
    {
    }

    ~Private() {
        qDeleteAll(toolbarList);
    }

    KisViewManager *viewManager;

    QPointer<KisView> activeView;

    QPointer<QProgressBar> progress;
    QMutex progressMutex;

    QList<QAction *> toolbarList;

    bool firstTime;
    bool windowSizeDirty;
    bool readOnly;
    bool isImporting;
    bool isExporting;
    bool noCleanup;

    KisAction *showDocumentInfo;
    KisAction *saveAction;
    KisAction *saveActionAs;
    KisAction *printAction;
    KisAction *printActionPreview;
    KisAction *exportPdf;
#ifdef HAVE_OPENGL
    KisAction *exportAnimation;
#endif
    KisAction *closeAll;
//    KisAction *reloadFile;
    KisAction *importFile;
    KisAction *exportFile;
    KisAction *undo;
    KisAction *redo;
    KisAction *newWindow;
    KisAction *close;
    KisAction *mdiCascade;
    KisAction *mdiTile;
    KisAction *mdiNextWindow;
    KisAction *mdiPreviousWindow;
    KisAction *toggleDockers;
    KisAction *toggleDockerTitleBars;

    KisAction *expandingSpacers[2];

    KActionMenu *dockWidgetMenu;
    KActionMenu *windowMenu;
    KActionMenu *documentMenu;

    KHelpMenu *helpMenu;

    KToolBar *brushesAndStuff;

    KRecentFilesAction *recentFiles;

    QUrl lastExportUrl;

    QMap<QString, QDockWidget *> dockWidgetsMap;
    QMap<QDockWidget *, bool> dockWidgetVisibilityMap;
    QByteArray dockerStateBeforeHiding;
    KoToolDocker *toolOptionsDocker;

    QCloseEvent *deferredClosingEvent;

    Digikam::ThemeManager *themeManager;

    QMdiArea *mdiArea;
    QMdiSubWindow *activeSubWindow;
    QSignalMapper *windowMapper;
    QSignalMapper *documentMapper;

    QByteArray lastExportedFormat;
    int lastExportSpecialOutputFlag;

    KisActionManager * actionManager() {
        return viewManager->actionManager();
    }
};

KisMainWindow::KisMainWindow()
    : KXmlGuiWindow()
    , d(new Private(this))
{
    KisConfig cfg;

    d->viewManager = new KisViewManager(this, actionCollection());
    KConfigGroup group( KSharedConfig::openConfig(), "theme");
    d->themeManager = new Digikam::ThemeManager(group.readEntry("Theme", "Krita dark"), this);

    setAcceptDrops(true);
    setStandardToolBarMenuEnabled(true);
    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);
    setDockNestingEnabled(true);

    qApp->setStartDragDistance(25);     // 25 px is a distance that works well for Tablet and Mouse events

#ifdef Q_OS_MAC
    setUnifiedTitleAndToolBarOnMac(true);
#endif

    connect(this, SIGNAL(restoringDone()), this, SLOT(forceDockTabFonts()));
    connect(this, SIGNAL(documentSaved()), d->viewManager, SLOT(slotDocumentSaved()));
    connect(this, SIGNAL(themeChanged()), d->viewManager, SLOT(updateIcons()));
    connect(KisPart::instance(), SIGNAL(documentClosed(QString)), SLOT(updateWindowMenu()));
    connect(KisPart::instance(), SIGNAL(documentOpened(QString)), SLOT(updateWindowMenu()));
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), this, SLOT(configChanged()));

    actionCollection()->addAssociatedWidget(this);

    QMetaObject::invokeMethod(this, "initializeGeometry", Qt::QueuedConnection);

    KoToolBoxFactory toolBoxFactory;
    QDockWidget *toolbox = createDockWidget(&toolBoxFactory);
    toolbox->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);

    if (cfg.toolOptionsInDocker()) {
        ToolDockerFactory toolDockerFactory;
        d->toolOptionsDocker = qobject_cast<KoToolDocker*>(createDockWidget(&toolDockerFactory));
    }

    QMap<QString, QAction*> dockwidgetActions;
    dockwidgetActions[toolbox->toggleViewAction()->text()] = toolbox->toggleViewAction();
    Q_FOREACH (const QString & docker, KoDockRegistry::instance()->keys()) {
        KoDockFactoryBase *factory = KoDockRegistry::instance()->value(docker);
        QDockWidget *dw = createDockWidget(factory);
        dockwidgetActions[dw->toggleViewAction()->text()] = dw->toggleViewAction();
    }
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
            mainwindowObserver->setMainWindow(d->viewManager);
        }
    }

    d->mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    d->mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    d->mdiArea->setTabPosition(QTabWidget::North);
    d->mdiArea->setTabsClosable(true);

    setCentralWidget(d->mdiArea);

    connect(d->mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(subWindowActivated()));
    connect(d->windowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(setActiveSubWindow(QWidget*)));
    connect(d->documentMapper, SIGNAL(mapped(QObject*)), this, SLOT(newView(QObject*)));

    createActions();

    setAutoSaveSettings("krita", false);

    KoPluginLoader::instance()->load("Krita/ViewPlugin",
                                     "Type == 'Service' and ([X-Krita-Version] == 28)",
                                     KoPluginLoader::PluginsConfig(),
                                     viewManager());

    subWindowActivated();
    updateWindowMenu();


    if (isHelpMenuEnabled() && !d->helpMenu) {
        d->helpMenu = new KHelpMenu(this, "Dummy Text That Is Not Used In Frameworks 5", false);

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
    setLocalXMLFile(KoResourcePaths::locateLocal("data", "krita/krita.rc"));

    QString doc;
    QStringList allFiles = KoResourcePaths::findAllResources("data", "krita/krita.rc");
    KIS_ASSERT(allFiles.size() > 0); // We need at least one krita.rc file!
    setXMLFile(findMostRecentXMLFile(allFiles, doc));

    guiFactory()->addClient(this);

    // Create and plug toolbar list for Settings menu
    QList<QAction *> toolbarList;
    Q_FOREACH (QWidget* it, guiFactory()->containers("ToolBar")) {
        KToolBar * toolBar = ::qobject_cast<KToolBar *>(it);

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
        } else
            warnUI << "Toolbar list contains a " << it->metaObject()->className() << " which is not a toolbar!";
    }
    plugActionList("toolbarlist", toolbarList);
    setToolbarList(toolbarList);

    applyToolBarLayout();

    d->viewManager->updateGUI();
    d->viewManager->updateIcons();

    QTimer::singleShot(1000, this, SLOT(checkSanity()));
}

void KisMainWindow::setNoCleanup(bool noCleanup)
{
    d->noCleanup = noCleanup;
}

KisMainWindow::~KisMainWindow()
{
//    Q_FOREACH (QAction *ac, actionCollection()->actions()) {
//        QAction *action = qobject_cast<QAction*>(ac);
//        if (action) {
//        dbgKrita << "<Action"
//                 << "name=" << action->objectName()
//                 << "icon=" << action->icon().name()
//                 << "text="  << action->text().replace("&", "&amp;")
//                 << "whatsThis="  << action->whatsThis()
//                 << "toolTip="  << action->toolTip().replace("<html>", "").replace("</html>", "")
//                 << "iconText="  << action->iconText().replace("&", "&amp;")
//                 << "shortcut="  << action->shortcut(QAction::ActiveShortcut).toString()
//                 << "defaultShortcut="  << action->shortcut(QAction::DefaultShortcut).toString()
//                 << "isCheckable="  << QString((action->isChecked() ? "true" : "false"))
//                 << "statusTip=" << action->statusTip()
//                 << "/>"   ;
//        }
//        else {
//            dbgKrita << "Got a QAction:" << ac->objectName();
//        }

//    }

    KConfigGroup cfg( KSharedConfig::openConfig(), "MainWindow");
    cfg.writeEntry("ko_geometry", saveGeometry().toBase64());
    cfg.writeEntry("ko_windowstate", saveState().toBase64());

    {
        KConfigGroup group( KSharedConfig::openConfig(), "theme");
        group.writeEntry("Theme", d->themeManager->currentThemeName());
    }

    // The doc and view might still exist (this is the case when closing the window)
    KisPart::instance()->removeMainWindow(this);

    if (d->noCleanup)
        return;

    delete d->viewManager;
    delete d;

}

void KisMainWindow::addView(KisView *view)
{
    if (d->activeView == view) return;

    if (d->activeView) {
        d->activeView->disconnect(this);
    }

    showView(view);
    updateCaption();
    emit restoringDone();

    if (d->activeView) {
        connect(d->activeView, SIGNAL(titleModified(QString,bool)), SLOT(slotDocumentTitleModified(QString,bool)));
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
        subwin->setAttribute(Qt::WA_DeleteOnClose, true);
        connect(subwin, SIGNAL(destroyed()), SLOT(updateWindowMenu()));

        KisConfig cfg;
        subwin->setOption(QMdiSubWindow::RubberBandMove, cfg.readEntry<int>("mdi_rubberband", cfg.useOpenGL()));
        subwin->setOption(QMdiSubWindow::RubberBandResize, cfg.readEntry<int>("mdi_rubberband", cfg.useOpenGL()));
        subwin->setWindowIcon(qApp->windowIcon());

        if (d->mdiArea->subWindowList().size() == 1) {
            imageView->showMaximized();
        }
        else {
            imageView->show();
        }

        setActiveView(imageView);
        updateWindowMenu();
        updateCaption();
    }
}

void KisMainWindow::slotPreferences()
{
    if (KisDlgPreferences::editPreferences()) {
        KisConfigNotifier::instance()->notifyConfigChanged();

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
    dbgUI << "KisMainWindow::addRecentURL url=" << url.toDisplayString();
    // Add entry to recent documents list
    // (call coming from KisDocument because it must work with cmd line, template dlg, file/open, etc.)
    if (!url.isEmpty()) {
        bool ok = true;
        if (url.isLocalFile()) {
            QString path = url.adjusted(QUrl::StripTrailingSlash).toLocalFile();
            const QStringList tmpDirs = KoResourcePaths::resourceDirs("tmp");
            for (QStringList::ConstIterator it = tmpDirs.begin() ; ok && it != tmpDirs.end() ; ++it)
                if (path.contains(*it))
                    ok = false; // it's in the tmp resource
#ifdef HAVE_KIO
            if (ok) {
                KRecentDocument::add(path);
            }
#endif
        }
#ifdef HAVE_KIO
        else {
            KRecentDocument::add(url.url(QUrl::StripTrailingSlash), true);
        }
#endif
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
    Q_FOREACH (KMainWindow* window, KMainWindow::memberList())
        static_cast<KisMainWindow *>(window)->reloadRecentFileList();
}

void KisMainWindow::reloadRecentFileList()
{
    d->recentFiles->loadEntries( KSharedConfig::openConfig()->group("RecentFiles"));
}

void KisMainWindow::updateCaption()
{
    if (!d->mdiArea->activeSubWindow()) {
        updateCaption(QString(), false);
    }
    else {
        QString caption( d->activeView->document()->caption() );
        if (d->readOnly) {
            caption += ' ' + i18n("(write protected)");
        }

        d->activeView->setWindowTitle(caption);

        updateCaption(caption, d->activeView->document()->isModified());

        if (!d->activeView->document()->url().fileName().isEmpty())
            d->saveAction->setToolTip(i18n("Save as %1", d->activeView->document()->url().fileName()));
        else
            d->saveAction->setToolTip(i18n("Save"));
    }
}

void KisMainWindow::updateCaption(const QString & caption, bool mod)
{
    dbgUI << "KisMainWindow::updateCaption(" << caption << "," << mod << ")";
#ifdef CALLIGRA_ALPHA
    setCaption(QString("ALPHA %1: %2").arg(CALLIGRA_ALPHA).arg(caption), mod);
    return;
#endif
#ifdef CALLIGRA_BETA
    setCaption(QString("BETA %1: %2").arg(CALLIGRA_BETA).arg(caption), mod);
    return;
#endif
#ifdef CALLIGRA_RC
    setCaption(QString("RELEASE CANDIDATE %1: %2").arg(CALLIGRA_RC).arg(caption), mod);
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

bool KisMainWindow::openDocument(const QUrl &url)
{
    if (!QFile(url.toLocalFile()).exists()) {
        QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("The file %1 does not exist.", url.url()));
        d->recentFiles->removeUrl(url); //remove the file from the recent-opened-file-list
        saveRecentFiles();
        return false;
    }
    return openDocumentInternal(url);
}

bool KisMainWindow::openDocumentInternal(const QUrl &url, KisDocument *newdoc)
{
    if (!url.isLocalFile()) {
        qDebug() << "KisMainWindow::openDocumentInternal. Not a local file:" << url;
        return false;
    }

    if (!newdoc) {
        newdoc = KisPart::instance()->createDocument();
    }

    d->firstTime = true;
    connect(newdoc, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));
    connect(newdoc, SIGNAL(completed()), this, SLOT(slotLoadCompleted()));
    connect(newdoc, SIGNAL(canceled(const QString &)), this, SLOT(slotLoadCanceled(const QString &)));
    bool openRet = (!isImporting()) ? newdoc->openUrl(url) : newdoc->importDocument(url);
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

void KisMainWindow::addViewAndNotifyLoadingCompleted(KisDocument *document)
{
    KisView *view = KisPart::instance()->createView(document, resourceManager(), actionCollection(), this);
    addView(view);

    emit guiLoadingFinished();
}

// Separate from openDocument to handle async loading (remote URLs)
void KisMainWindow::slotLoadCompleted()
{
    KisDocument *newdoc = qobject_cast<KisDocument*>(sender());

    addViewAndNotifyLoadingCompleted(newdoc);

    disconnect(newdoc, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));
    disconnect(newdoc, SIGNAL(completed()), this, SLOT(slotLoadCompleted()));
    disconnect(newdoc, SIGNAL(canceled(const QString &)), this, SLOT(slotLoadCanceled(const QString &)));

    emit loadCompleted();
}

void KisMainWindow::slotLoadCanceled(const QString & errMsg)
{
    dbgUI << "KisMainWindow::slotLoadCanceled";
    if (!errMsg.isEmpty())   // empty when canceled by user
        QMessageBox::critical(this, i18nc("@title:window", "Krita"), errMsg);
    // ... can't delete the document, it's the one who emitted the signal...

    KisDocument* doc = qobject_cast<KisDocument*>(sender());
    Q_ASSERT(doc);
    disconnect(doc, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));
    disconnect(doc, SIGNAL(completed()), this, SLOT(slotLoadCompleted()));
    disconnect(doc, SIGNAL(canceled(const QString &)), this, SLOT(slotLoadCanceled(const QString &)));
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
    disconnect(doc, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));
    disconnect(doc, SIGNAL(completed()), this, SLOT(slotSaveCompleted()));
    disconnect(doc, SIGNAL(canceled(const QString &)), this, SLOT(slotSaveCanceled(const QString &)));

    if (d->deferredClosingEvent) {
        KXmlGuiWindow::closeEvent(d->deferredClosingEvent);
    }
}

bool KisMainWindow::saveDocument(KisDocument *document, bool saveas, bool silent, int specialOutputFlag)
{
    if (!document) {
        return true;
    }

    bool reset_url;

    if (document->url().isEmpty()) {
        reset_url = true;
        saveas = true;
    } else {
        reset_url = false;
    }

    connect(document, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));
    connect(document, SIGNAL(completed()), this, SLOT(slotSaveCompleted()));
    connect(document, SIGNAL(canceled(const QString &)), this, SLOT(slotSaveCanceled(const QString &)));

    QUrl oldURL = document->url();
    QString oldFile = document->localFilePath();

    QByteArray _native_format = document->nativeFormatMimeType();
    QByteArray oldOutputFormat = document->outputMimeType();

    int oldSpecialOutputFlag = document->specialOutputFlag();

    QUrl suggestedURL = document->url();

    QStringList mimeFilter;
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForName(_native_format);

    if (specialOutputFlag) {
        mimeFilter = mime.globPatterns();
    }
    else {
        mimeFilter = KisImportExportManager::mimeFilter(_native_format,
                                                 KisImportExportManager::Export,
                                                 document->extraNativeMimeTypes());
    }


    if (!mimeFilter.contains(oldOutputFormat) && !isExporting()) {
        dbgUI << "KisMainWindow::saveDocument no export filter for" << oldOutputFormat;

        // --- don't setOutputMimeType in case the user cancels the Save As
        // dialog and then tries to just plain Save ---

        // suggest a different filename extension (yes, we fortunately don't all live in a world of magic :))
        QString suggestedFilename = suggestedURL.fileName();

        if (!suggestedFilename.isEmpty()) {  // ".kra" looks strange for a name
            int c = suggestedFilename.lastIndexOf('.');

            const QString ext = mime.preferredSuffix();
            if (!ext.isEmpty()) {
                if (c < 0)
                    suggestedFilename += ext;
                else
                    suggestedFilename = suggestedFilename.left(c) + ext;
            } else { // current filename extension wrong anyway
                if (c > 0) {
                    // this assumes that a . signifies an extension, not just a .
                    suggestedFilename = suggestedFilename.left(c);
                }
            }

            suggestedURL = suggestedURL.adjusted(QUrl::RemoveFilename);
            suggestedURL.setPath(suggestedURL.path() + suggestedFilename);
        }

        // force the user to choose outputMimeType
        saveas = true;
    }

    bool ret = false;

    if (document->url().isEmpty() || saveas) {
        // if you're just File/Save As'ing to change filter options you
        // don't want to be reminded about overwriting files etc.
        bool justChangingFilterOptions = false;

        KoFileDialog dialog(this, KoFileDialog::SaveFile, "SaveDocument");
        dialog.setCaption(i18n("untitled"));
        if (isExporting() && !d->lastExportUrl.isEmpty()) {
            dialog.setDefaultDir(d->lastExportUrl.toLocalFile(), true);
        }
        else {
            dialog.setDefaultDir(suggestedURL.toLocalFile(), true);
        }
        dialog.setMimeTypeFilters(mimeFilter, KIS_MIME_TYPE);
        QUrl newURL = QUrl::fromUserInput(dialog.filename());

        if (newURL.isLocalFile()) {
            QString fn = newURL.toLocalFile();
            if (QFileInfo(fn).completeSuffix().isEmpty()) {
                QMimeDatabase db;
                QMimeType mime = db.mimeTypeForName(_native_format);
                fn.append(mime.preferredSuffix());
                newURL = QUrl::fromLocalFile(fn);
            }
        }

        if (document->documentInfo()->aboutInfo("title") == i18n("Unnamed")) {
            QString fn = newURL.toLocalFile();
            QFileInfo info(fn);
            document->documentInfo()->setAboutInfo("title", info.baseName());
        }

        QByteArray outputFormat = _native_format;

        if (!specialOutputFlag) {
            QMimeType mime = db.mimeTypeForUrl(newURL);
            QString outputFormatString = mime.name();
            outputFormat = outputFormatString.toLatin1();
        }

        if (!isExporting())
            justChangingFilterOptions = (newURL == document->url()) &&
                    (outputFormat == document->mimeType()) &&
                    (specialOutputFlag == oldSpecialOutputFlag);
        else
            justChangingFilterOptions = (newURL == d->lastExportUrl) &&
                    (outputFormat == d->lastExportedFormat) &&
                    (specialOutputFlag == d->lastExportSpecialOutputFlag);


        bool bOk = true;
        if (newURL.isEmpty()) {
            bOk = false;
        }

        // adjust URL before doing checks on whether the file exists.
        if (specialOutputFlag) {
            QString fileName = newURL.fileName();
            if ( specialOutputFlag== KisDocument::SaveAsDirectoryStore) {
                //dbgKrita << "save to directory: " << newURL.url();
            }
            else if (specialOutputFlag == KisDocument::SaveEncrypted) {
                int dot = fileName.lastIndexOf('.');
                dbgKrita << dot;
                QString ext = mime.preferredSuffix();
                if (!ext.isEmpty()) {
                    if (dot < 0) fileName += ext;
                    else fileName = fileName.left(dot) + ext;
                } else { // current filename extension wrong anyway
                    if (dot > 0) fileName = fileName.left(dot);
                }
                newURL = newURL.adjusted(QUrl::RemoveFilename);
                newURL.setPath(newURL.path() + fileName);
            }
        }

        if (bOk) {
            bool wantToSave = true;

            // don't change this line unless you know what you're doing :)
            if (!justChangingFilterOptions || document->confirmNonNativeSave(isExporting())) {
                if (!document->isNativeFormat(outputFormat))
                    wantToSave = true;
            }

            if (wantToSave) {
                //
                // Note:
                // If the user is stupid enough to Export to the current URL,
                // we do _not_ change this operation into a Save As.  Reasons
                // follow:
                //
                // 1. A check like "isExporting() && oldURL == newURL"
                //    doesn't _always_ work on case-insensitive filesystems
                //    and inconsistent behaviour is bad.
                // 2. It is probably not a good idea to change document->mimeType
                //    and friends because the next time the user File/Save's,
                //    (not Save As) they won't be expecting that they are
                //    using their File/Export settings
                //
                // As a bad side-effect of this, the modified flag will not
                // be updated and it is possible that what is currently on
                // their screen is not what is stored on disk (through loss
                // of formatting).  But if you are dumb enough to change
                // mimetype but not the filename, then arguably, _you_ are
                // the "bug" :)
                //
                // - Clarence
                //
                document->setOutputMimeType(outputFormat, specialOutputFlag);
                if (!isExporting()) {  // Save As
                    ret = document->saveAs(newURL);

                    if (ret) {
                        dbgUI << "Successful Save As!";
                        addRecentURL(newURL);
                        setReadWrite(true);
                    } else {
                        dbgUI << "Failed Save As!";
                        document->setUrl(oldURL);
                        document->setLocalFilePath(oldFile);
                        document->setOutputMimeType(oldOutputFormat, oldSpecialOutputFlag);
                    }
                } else { // Export
                    ret = document->exportDocument(newURL);

                    if (ret) {
                        // a few file dialog convenience things
                        d->lastExportUrl = newURL;
                        d->lastExportedFormat = outputFormat;
                        d->lastExportSpecialOutputFlag = specialOutputFlag;
                    }

                    // always restore output format
                    document->setOutputMimeType(oldOutputFormat, oldSpecialOutputFlag);
                }

                if (silent) // don't let the document change the window caption
                    document->setTitleModified();
            }   // if (wantToSave)  {
            else
                ret = false;
        }   // if (bOk) {
        else
            ret = false;
    } else { // saving

        bool needConfirm = document->confirmNonNativeSave(false) && !document->isNativeFormat(oldOutputFormat);

        if (!needConfirm ||
                (needConfirm && exportConfirmation(oldOutputFormat /* not so old :) */))
                ) {
            // be sure document has the correct outputMimeType!
            if (isExporting() || document->isModified()) {
                ret = document->save();
            }

            if (!ret) {
                dbgUI << "Failed Save!";
                document->setUrl(oldURL);
                document->setLocalFilePath(oldFile);
            }
        } else
            ret = false;
    }


    if (!ret && reset_url)
        document->resetURL(); //clean the suggested filename as the save dialog was rejected

    updateReloadFileAction(document);
    updateCaption();

    return ret;
}

bool KisMainWindow::exportConfirmation(const QByteArray &/*outputFormat*/)
{
    return true;
}

void KisMainWindow::undo()
{
    if (activeView()) {
        activeView()->undoAction()->trigger();
        d->undo->setText(activeView()->undoAction()->text());
    }
}

void KisMainWindow::redo()
{
    if (activeView()) {
        activeView()->redoAction()->trigger();
        d->redo->setText(activeView()->redoAction()->text());
    }
}

void KisMainWindow::closeEvent(QCloseEvent *e)
{
    d->mdiArea->closeAllSubWindows();

    if(d->activeView && d->activeView->document() && d->activeView->document()->isLoading()) {
        e->setAccepted(false);
        return;
    }

    QList<QMdiSubWindow*> childrenList = d->mdiArea->subWindowList();

    if (childrenList.isEmpty()) {
        d->deferredClosingEvent = e;

        if (!d->dockerStateBeforeHiding.isEmpty()) {
            restoreState(d->dockerStateBeforeHiding);
        }
        statusBar()->setVisible(true);
        menuBar()->setVisible(true);

        saveWindowSettings();

        if (d->noCleanup)
            return;

        Q_FOREACH (QMdiSubWindow *subwin, d->mdiArea->subWindowList()) {
            KisView *view = dynamic_cast<KisView*>(subwin);
            if (view) {
                KisPart::instance()->removeView(view);
            }
        }

        if (!d->dockWidgetVisibilityMap.isEmpty()) { // re-enable dockers for persistency
            Q_FOREACH (QDockWidget* dockWidget, d->dockWidgetsMap)
                dockWidget->setVisible(d->dockWidgetVisibilityMap.value(dockWidget));
        }
    } else {
        e->setAccepted(false);
    }
}

void KisMainWindow::saveWindowSettings()
{
    KSharedConfigPtr config =  KSharedConfig::openConfig();

    if (d->windowSizeDirty ) {
        dbgUI << "KisMainWindow::saveWindowSettings";
        KConfigGroup group = config->group("MainWindow");
        KWindowConfig::saveWindowSize(windowHandle(), group);
        config->sync();
        d->windowSizeDirty = false;
    }

    if (!d->activeView || d->activeView->document()) {

        // Save toolbar position into the config file of the app, under the doc's component name
        KConfigGroup group =  KSharedConfig::openConfig()->group("krita");
        saveMainWindowSettings(group);

        // Save collapsable state of dock widgets
        for (QMap<QString, QDockWidget*>::const_iterator i = d->dockWidgetsMap.constBegin();
             i != d->dockWidgetsMap.constEnd(); ++i) {
            if (i.value()->widget()) {
                KConfigGroup dockGroup = group.group(QString("DockWidget ") + i.key());
                dockGroup.writeEntry("Collapsed", i.value()->widget()->isHidden());
                dockGroup.writeEntry("Locked", i.value()->property("Locked").toBool());
                dockGroup.writeEntry("DockArea", (int) dockWidgetArea(i.value()));
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
    actionCollection()->action("edit_undo")->setText(activeView()->undoAction()->text());
    actionCollection()->action("edit_redo")->setText(activeView()->redoAction()->text());
}

void KisMainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->accept();
    }
}

void KisMainWindow::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() > 0) {
        Q_FOREACH (const QUrl &url, event->mimeData()->urls()) {
            openDocument(url);
        }
    }
}

void KisMainWindow::slotFileNew()
{
    KisPart::instance()->showStartUpWidget(this, true /*Always show widget*/);
}

void KisMainWindow::slotFileOpen()
{
    QStringList urls;
    KoFileDialog dialog(this, KoFileDialog::ImportFiles, "OpenDocument");
    dialog.setDefaultDir(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation));
    dialog.setMimeTypeFilters(KisImportExportManager::mimeFilter(KIS_MIME_TYPE,
                                                                 KisImportExportManager::Import,
                                                                 KisDocumentEntry::extraNativeMimeTypes()));
    QStringList filters = dialog.nameFilters();
    filters << i18n("All files (*.*)");
    dialog.setNameFilters(filters);
    dialog.setHideNameFilterDetailsOption();
    dialog.setCaption(isImporting() ? i18n("Import Images") : i18n("Open Images"));

    urls = dialog.filenames();

    if (urls.isEmpty())
        return;

    Q_FOREACH (const QString& url, urls) {

        if (!url.isEmpty()) {
            bool res = openDocument(QUrl::fromLocalFile(url));
            if (!res) {
                warnKrita << "Loading" << url << "failed";
            }
        }
    }
}

void KisMainWindow::slotFileOpenRecent(const QUrl &url)
{
    (void) openDocument(QUrl(url));
}

void KisMainWindow::slotFileSave()
{
    if (saveDocument(d->activeView->document()))
        emit documentSaved();
}

void KisMainWindow::slotFileSaveAs()
{
    if (saveDocument(d->activeView->document(), true))
        emit documentSaved();
}

KoCanvasResourceManager *KisMainWindow::resourceManager() const
{
    return d->viewManager->resourceProvider()->resourceManager();
}

int KisMainWindow::viewCount() const
{
    return d->mdiArea->subWindowList().size();
}

bool KisMainWindow::restoreWorkspace(const QByteArray &state)
{
    QByteArray oldState = saveState();
    const bool showTitlebars = KisConfig().showDockerTitleBars();

    // needed because otherwise the layout isn't correctly restored in some situations
    Q_FOREACH (QDockWidget *dock, dockWidgets()) {
        dock->hide();
        dock->titleBarWidget()->setVisible(showTitlebars);
    }

    bool success = KXmlGuiWindow::restoreState(state);

    if (!success) {
        KXmlGuiWindow::restoreState(oldState);
        Q_FOREACH (QDockWidget *dock, dockWidgets()) {
            if (dock->titleBarWidget()) {
                dock->titleBarWidget()->setVisible(showTitlebars || dock->isFloating());
            }
        }
        return false;
    }


    Q_FOREACH (QDockWidget *dock, dockWidgets()) {
        if (dock->titleBarWidget()) {
            const bool isCollapsed = (dock->widget() && dock->widget()->isHidden()) || !dock->widget();
            dock->titleBarWidget()->setVisible(showTitlebars || (dock->isFloating() && isCollapsed));
        }
    }

    return success;
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
    if(!slotFileCloseAll())
        return;

    close();

    Q_FOREACH (QPointer<KisMainWindow> mainWin, KisPart::instance()->mainWindows()) {
        if (mainWin != this) {
            if(!mainWin->slotFileCloseAll())
                return;

            mainWin->close();
        }
    }
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

KisPrintJob* KisMainWindow::exportToPdf(const QString &pdfFileName)
{
    if (!activeView())
        return 0;
    KoPageLayout pageLayout;
    pageLayout = activeView()->pageLayout();
    return exportToPdf(pageLayout, pdfFileName);
}

KisPrintJob* KisMainWindow::exportToPdf(KoPageLayout pageLayout, QString pdfFileName)
{
    if (!activeView())
        return 0;
    if (!activeView()->document())
        return 0;

    if (pdfFileName.isEmpty()) {
        KConfigGroup group =  KSharedConfig::openConfig()->group("File Dialogs");
        QString defaultDir = group.readEntry("SavePdfDialog");
        if (defaultDir.isEmpty())
            defaultDir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
        QUrl startUrl = QUrl::fromLocalFile(defaultDir);
        KisDocument* pDoc = d->activeView->document();
        /** if document has a file name, take file name and replace extension with .pdf */
        if (pDoc && pDoc->url().isValid()) {
            startUrl = pDoc->url();
            QString fileName = startUrl.fileName();
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

        KoFileDialog dialog(this, KoFileDialog::SaveFile, "SaveDocument");
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

void KisMainWindow::exportAnimation()
{
#ifdef HAVE_OPENGL
    if (!activeView()) return;

    KisDocument *document = activeView()->document();
    if (!document) return;

    KisAnimationExporterUI exporter(this);
    exporter.exportSequence(document);

    activeView()->canvasBase()->refetchDataFromImage();
#endif
}

void KisMainWindow::slotConfigureKeys()
{
    KisPart::instance()->configureShortcuts();
    emit keyBindingsChanged();
}

void KisMainWindow::slotConfigureToolbars()
{
    KConfigGroup group =  KSharedConfig::openConfig()->group("krita");
    saveMainWindowSettings(group);
    KEditToolBar edit(factory(), this);
    connect(&edit, SIGNAL(newToolBarConfig()), this, SLOT(slotNewToolbarConfig()));
    (void) edit.exec();
    applyToolBarLayout();
}

void KisMainWindow::slotNewToolbarConfig()
{
    applyMainWindowSettings(KSharedConfig::openConfig()->group("krita"));

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
            KConfigGroup group =  KSharedConfig::openConfig()->group("krita");
            saveMainWindowSettings(group);
        }
    } else
        warnUI << "slotToolbarToggled : Toolbar " << sender()->objectName() << " not found!";
}

void KisMainWindow::viewFullscreen(bool fullScreen)
{
    KisConfig cfg;
    cfg.setFullscreenMode(fullScreen);

    if (fullScreen) {
        setWindowState(windowState() | Qt::WindowFullScreen);   // set
    } else {
        setWindowState(windowState() & ~Qt::WindowFullScreen);   // reset
    }
}

void KisMainWindow::slotProgress(int value)
{
    qApp->processEvents();

    if (!d->progressMutex.tryLock()) return;

    dbgUI << "KisMainWindow::slotProgress" << value;
    if (value <= -1 || value >= 100) {
        if (d->progress) {
            statusBar()->removeWidget(d->progress);
            delete d->progress;
            d->progress = 0;
        }
        d->firstTime = true;
        d->progressMutex.unlock();
        return;
    }
    if (d->firstTime || !d->progress) {
        // The statusbar might not even be created yet.
        // So check for that first, and create it if necessary
        QStatusBar *bar = findChild<QStatusBar *>();
        if (!bar) {
            statusBar()->show();
            QApplication::sendPostedEvents(this, QEvent::ChildAdded);
        }

        if (d->progress) {
            statusBar()->removeWidget(d->progress);
            delete d->progress;
            d->progress = 0;
        }

        d->progress = new QProgressBar(statusBar());
        d->progress->setMaximumHeight(statusBar()->fontMetrics().height());
        d->progress->setRange(0, 100);
        statusBar()->addPermanentWidget(d->progress);
        d->progress->show();
        d->firstTime = false;
    }
    if (!d->progress.isNull()) {
        d->progress->setValue(value);
    }
    qApp->processEvents();

    d->progressMutex.unlock();
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

void KisMainWindow::slotImportFile()
{
    dbgUI << "slotImportFile()";

    d->isImporting = true;
    slotFileOpen();
    d->isImporting = false;
}

void KisMainWindow::slotExportFile()
{
    dbgUI << "slotExportFile()";

    d->isExporting = true;
    slotFileSaveAs();
    d->isExporting = false;
}

bool KisMainWindow::isImporting() const
{
    return d->isImporting;
}

bool KisMainWindow::isExporting() const
{
    return d->isExporting;
}

QDockWidget* KisMainWindow::createDockWidget(KoDockFactoryBase* factory)
{
    QDockWidget* dockWidget = 0;

    if (!d->dockWidgetsMap.contains(factory->id())) {
        dockWidget = factory->createDockWidget();

        // It is quite possible that a dock factory cannot create the dock; don't
        // do anything in that case.
        if (!dockWidget) {
            warnKrita << "Could not create docker for" << factory->id();
            return 0;
        }

        KoDockWidgetTitleBar *titleBar = dynamic_cast<KoDockWidgetTitleBar*>(dockWidget->titleBarWidget());

        // Check if the dock widget is supposed to be collapsable
        if (!dockWidget->titleBarWidget()) {
            titleBar = new KoDockWidgetTitleBar(dockWidget);
            dockWidget->setTitleBarWidget(titleBar);
            titleBar->setCollapsable(factory->isCollapsable());
        }
        titleBar->setFont(KoDockRegistry::dockFont());

        dockWidget->setObjectName(factory->id());
        dockWidget->setParent(this);

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

        KConfigGroup group =  KSharedConfig::openConfig()->group("krita").group("DockWidget " + factory->id());
        side = static_cast<Qt::DockWidgetArea>(group.readEntry("DockArea", static_cast<int>(side)));
        if (side == Qt::NoDockWidgetArea) side = Qt::RightDockWidgetArea;

        addDockWidget(side, dockWidget);
        if (!visible) {
            dockWidget->hide();
        }
        bool collapsed = factory->defaultCollapsed();

        bool locked = false;
        group =  KSharedConfig::openConfig()->group("krita").group("DockWidget " + factory->id());
        collapsed = group.readEntry("Collapsed", collapsed);
        locked = group.readEntry("Locked", locked);

        //dbgKrita << "docker" << factory->id() << dockWidget << "collapsed" << collapsed << "locked" << locked << "titlebar" << titleBar;

        if (titleBar && collapsed)
            titleBar->setCollapsed(true);

        if (titleBar && locked)
            titleBar->setLocked(true);

        d->dockWidgetsMap.insert(factory->id(), dockWidget);
    }
    else {
        dockWidget = d->dockWidgetsMap[factory->id()];
    }

#ifdef Q_OS_MAC
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

void KisMainWindow::setToolbarList(QList<QAction *> toolbarList)
{
    qDeleteAll(d->toolbarList);
    d->toolbarList = toolbarList;
}

void KisMainWindow::slotDocumentTitleModified(const QString &caption, bool mod)
{
    updateCaption(caption, mod);
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

    updateCaption();
    d->actionManager()->updateGUI();
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
            if (title.isEmpty()) title = doc->image()->objectName();
            QAction *action = docMenu->addAction(title);
            action->setIcon(qApp->windowIcon());
            connect(action, SIGNAL(triggered()), d->documentMapper, SLOT(map()));
            d->documentMapper->setMapping(action, doc);
        }
    }

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
        if (child) {
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
            d->viewManager->setCurrentView(view);
            setActiveView(view);
        }
        d->activeSubWindow = subwin;
    }
    updateWindowMenu();
    d->actionManager()->updateGUI();
}

void KisMainWindow::configChanged()
{
    KisConfig cfg;
    QMdiArea::ViewMode viewMode = (QMdiArea::ViewMode)cfg.readEntry<int>("mdi_viewmode", (int)QMdiArea::TabbedView);
    d->mdiArea->setViewMode(viewMode);
    Q_FOREACH (QMdiSubWindow *subwin, d->mdiArea->subWindowList()) {
        subwin->setOption(QMdiSubWindow::RubberBandMove, cfg.readEntry<int>("mdi_rubberband", cfg.useOpenGL()));
        subwin->setOption(QMdiSubWindow::RubberBandResize, cfg.readEntry<int>("mdi_rubberband", cfg.useOpenGL()));
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

void KisMainWindow::newView(QObject *document)
{
    KisDocument *doc = qobject_cast<KisDocument*>(document);
    KisView *view = KisPart::instance()->createView(doc, resourceManager(), actionCollection(), this);
    addView(view);
    d->actionManager()->updateGUI();
}

void KisMainWindow::newWindow()
{
    KisPart::instance()->createMainWindow()->show();
}

void KisMainWindow::closeCurrentWindow()
{
    d->mdiArea->currentSubWindow()->close();
    d->actionManager()->updateGUI();
}

void KisMainWindow::checkSanity()
{
    // print error if the lcms engine is not available
    if (!KoColorSpaceEngineRegistry::instance()->contains("icc")) {
        // need to wait 1 event since exiting here would not work.
        m_errorMessage = i18n("The Calligra LittleCMS color management plugin is not installed. Krita will quit now.");
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

QPointer<KisView>KisMainWindow::activeKisView()
{
    if (!d->mdiArea) return 0;
    QMdiSubWindow *activeSubWindow = d->mdiArea->activeSubWindow();
    //dbgKrita << "activeKisView" << activeSubWindow;
    if (!activeSubWindow) return 0;
    return qobject_cast<KisView*>(activeSubWindow->widget());
}


void KisMainWindow::newOptionWidgets(const QList<QPointer<QWidget> > &optionWidgetList)
{
    Q_FOREACH (QWidget *w, optionWidgetList) {
#ifdef Q_OS_MAC
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
        title = d->activeView->document()->url().fileName();
        // strip off the native extension (I don't want foobar.kwd.ps when printing into a file)
        QMimeDatabase db;
        QMimeType mime = db.mimeTypeForName(d->activeView->document()->outputMimeType());
        if (mime.isValid()) {
            QString extension = mime.preferredSuffix();

            if (title.endsWith(extension))
                title.chop(extension.length());
        }
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
    actionManager->createStandardAction(KStandardAction::KeyBindings, this, SLOT(slotConfigureKeys()));
    actionManager->createStandardAction(KStandardAction::ConfigureToolbars, this, SLOT(slotConfigureToolbars()));
    actionManager->createStandardAction(KStandardAction::FullScreen, this, SLOT(viewFullscreen(bool)));

    d->recentFiles = KStandardAction::openRecent(this, SLOT(slotFileOpenRecent(QUrl)), actionCollection());
    connect(d->recentFiles, SIGNAL(recentListCleared()), this, SLOT(saveRecentFiles()));
    KSharedConfigPtr configPtr =  KSharedConfig::openConfig();
    d->recentFiles->loadEntries(configPtr->group("RecentFiles"));

    d->saveAction = actionManager->createStandardAction(KStandardAction::Save, this, SLOT(slotFileSave()));
    d->saveAction->setActivationFlags(KisAction::ACTIVE_IMAGE);

    d->saveActionAs = actionManager->createStandardAction(KStandardAction::SaveAs, this, SLOT(slotFileSaveAs()));
    d->saveActionAs->setActivationFlags(KisAction::ACTIVE_IMAGE);

    d->printAction = actionManager->createStandardAction(KStandardAction::Print, this, SLOT(slotFilePrint()));
    d->printAction->setActivationFlags(KisAction::ACTIVE_IMAGE);

    d->printActionPreview = actionManager->createStandardAction(KStandardAction::PrintPreview, this, SLOT(slotFilePrintPreview()));
    d->printActionPreview->setActivationFlags(KisAction::ACTIVE_IMAGE);

    d->undo = actionManager->createStandardAction(KStandardAction::Undo, this, SLOT(undo()));
    d->undo ->setActivationFlags(KisAction::ACTIVE_IMAGE);

    d->redo = actionManager->createStandardAction(KStandardAction::Redo, this, SLOT(redo()));
    d->redo->setActivationFlags(KisAction::ACTIVE_IMAGE);

    d->exportPdf  = actionManager->createAction("file_export_pdf");
    connect(d->exportPdf, SIGNAL(triggered()), this, SLOT(exportToPdf()));
#ifdef HAVE_OPENGL
    d->exportAnimation  = actionManager->createAction("file_export_animation");
    connect(d->exportAnimation, SIGNAL(triggered()), this, SLOT(exportAnimation()));
#endif

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


    d->toggleDockers = actionManager->createAction("view_toggledockers");
    d->toggleDockers->setChecked(true);
    connect(d->toggleDockers, SIGNAL(toggled(bool)), SLOT(toggleDockersVisibility(bool)));

    d->toggleDockerTitleBars = actionManager->createAction("view_toggledockertitlebars");
    {
        KisConfig cfg;
        d->toggleDockerTitleBars->setChecked(cfg.showDockerTitleBars());
    }
    connect(d->toggleDockerTitleBars, SIGNAL(toggled(bool)), SLOT(showDockerTitleBars(bool)));

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

    d->close = actionManager->createAction("file_close");
    connect(d->close, SIGNAL(triggered()), SLOT(closeCurrentWindow()));

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
    }
}

void KisMainWindow::initializeGeometry()
{
    // if the user didn's specify the geometry on the command line (does anyone do that still?),
    // we first figure out some good default size and restore the x,y position. See bug 285804Z.
    KConfigGroup cfg( KSharedConfig::openConfig(), "MainWindow");
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
            // height to componensate for the window decs
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
    restoreWorkspace(QByteArray::fromBase64(cfg.readEntry("ko_windowstate", QByteArray())));
}

void KisMainWindow::showManual()
{
    QDesktopServices::openUrl(QUrl("https://userbase.kde.org/Special:MyLanguage/Krita/Manual"));
}

void KisMainWindow::showDockerTitleBars(bool show)
{
    Q_FOREACH (QDockWidget *dock, dockWidgets()) {
        if (dock->titleBarWidget()) {
            const bool isCollapsed = (dock->widget() && dock->widget()->isHidden()) || !dock->widget();
            dock->titleBarWidget()->setVisible(show || (dock->isFloating() && isCollapsed));
        }
    }

    KisConfig cfg;
    cfg.setShowDockerTitleBars(show);
}

void KisMainWindow::moveEvent(QMoveEvent *e)
{
    if (qApp->desktop()->screenNumber(this) != qApp->desktop()->screenNumber(e->oldPos())) {
        KisConfigNotifier::instance()->notifyConfigChanged();
    }
}



#include <moc_KisMainWindow.cpp>
#include <QMimeDatabase>
#include <QMimeType>
