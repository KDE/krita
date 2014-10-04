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

#include "KoMainWindow.h"

#if defined (Q_OS_MAC) && QT_VERSION < 0x050000
#include "MacSupport.h"
#endif

#include "KoView.h"
#include "KoDocument.h"
#include "KoFilterManager.h"
#include "KoDocumentInfo.h"
#include "KoDocumentInfoDlg.h"
#include "KoFileDialog.h"
#include "KoVersionDialog.h"
#include "KoDockFactoryBase.h"
#include "KoDockWidgetTitleBar.h"
#include "KoPrintJob.h"
#include "KoDocumentEntry.h"
#include "KoDockerManager.h"
#include "KoPart.h"
#include <KoPageLayoutDialog.h>
#include "KoApplication.h"
#include <KoPageLayoutWidget.h>
#include <KoIcon.h>
#include <KoConfig.h>

#include <kdeversion.h>
#if KDE_IS_VERSION(4,6,0)
#include <krecentdirs.h>
#endif
#include <khelpmenu.h>
#include <krecentfilesaction.h>
#include <kaboutdata.h>
#include <ktoggleaction.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kedittoolbar.h>
#include <ktemporaryfile.h>
#include <krecentdocument.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kglobalsettings.h>
#include <ktoolinvocation.h>
#include <kxmlguifactory.h>
#include <kfileitem.h>
#include <ktoolbar.h>
#include <kdebug.h>
#include <kactionmenu.h>
#include <kactioncollection.h>
#include <kdiroperator.h>
#include <kmenubar.h>
#include <kmimetype.h>

#ifdef HAVE_KACTIVITIES
#include <KActivities/ResourceInstance>
#endif

//   // qt includes
#include <QDockWidget>
#include <QApplication>
#include <QLayout>
#include <QLabel>
#include <QProgressBar>
#include <QTabBar>
#include <QPrinter>
#include <QPrintDialog>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QPrintPreviewDialog>
#include <QCloseEvent>
#include <QPointer>
#include <QByteArray>
#include <QMutex>
#include <QMutexLocker>

#include "thememanager.h"

#include "calligraversion.h"

class KoMainWindowPrivate
{
public:
    KoMainWindowPrivate(const QByteArray &_nativeMimeType, KoMainWindow *w)
    {
        nativeMimeType = _nativeMimeType;
        parent = w;
        rootDocument = 0;
        rootPart = 0;
        partToOpen = 0;
        mainWindowGuiIsBuilt = false;
        forQuit = false;
        activePart = 0;
        activeView = 0;
        firstTime = true;
        progress = 0;
        showDocumentInfo = 0;
        saveAction = 0;
        saveActionAs = 0;
        printAction = 0;
        printActionPreview = 0;
        sendFileAction = 0;
        exportPdf = 0;
        closeFile = 0;
        reloadFile = 0;
        showFileVersions = 0;
        importFile = 0;
        exportFile = 0;
        encryptDocument = 0;
#ifndef NDEBUG
        uncompressToDir = 0;
#endif
        isImporting = false;
        isExporting = false;
        windowSizeDirty = false;
        lastExportSpecialOutputFlag = 0;
        readOnly = false;
        dockWidgetMenu = 0;
        dockerManager = 0;
        deferredClosingEvent = 0;
#ifdef HAVE_KACTIVITIES
        activityResource = 0;
#endif
        themeManager = 0;

        m_helpMenu = 0;

        // PartManger
        m_activeWidget = 0;
        m_activePart = 0;

        noCleanup = false;
    }

    ~KoMainWindowPrivate() {
        qDeleteAll(toolbarList);
    }

    void applyDefaultSettings(QPrinter &printer) {
        QString title = rootDocument->documentInfo()->aboutInfo("title");
        if (title.isEmpty()) {
            title = rootDocument->url().fileName();
            // strip off the native extension (I don't want foobar.kwd.ps when printing into a file)
            KMimeType::Ptr mime = KMimeType::mimeType(rootDocument->outputMimeType());
            if (mime) {
                QString extension = mime->property("X-KDE-NativeExtension").toString();

                if (title.endsWith(extension))
                    title.chop(extension.length());
            }
        }

        if (title.isEmpty()) {
            // #139905
            const QString programName = parent->componentData().aboutData() ?
                        parent->componentData().aboutData()->programName() : parent->componentData().componentName();
            title = i18n("%1 unsaved document (%2)", programName,
                         KGlobal::locale()->formatDate(QDate::currentDate(), KLocale::ShortDate));
        }
        printer.setDocName(title);
    }

    QByteArray nativeMimeType;

    KoMainWindow *parent;
    KoDocument *rootDocument;
    QList<KoView*> rootViews;

    // PartManager
    QPointer<KoPart> rootPart;
    QPointer<KoPart> partToOpen;
    QPointer<KoPart> activePart;
    QPointer<KoPart> m_activePart;
    QPointer<KoPart> m_registeredPart;

    KoView *activeView;
    QWidget *m_activeWidget;

    QPointer<QProgressBar> progress;
    QMutex progressMutex;

    QList<QAction *> toolbarList;

    bool mainWindowGuiIsBuilt;
    bool forQuit;
    bool firstTime;
    bool windowSizeDirty;
    bool readOnly;

    KAction *showDocumentInfo;
    KAction *saveAction;
    KAction *saveActionAs;
    KAction *printAction;
    KAction *printActionPreview;
    KAction *sendFileAction;
    KAction *exportPdf;
    KAction *closeFile;
    KAction *reloadFile;
    KAction *showFileVersions;
    KAction *importFile;
    KAction *exportFile;
    KAction *encryptDocument;
#ifndef NDEBUG
    KAction *uncompressToDir;
#endif
    KToggleAction *toggleDockers;
    KRecentFilesAction *recent;

    bool isImporting;
    bool isExporting;

    KUrl lastExportUrl;
    QByteArray lastExportedFormat;
    int lastExportSpecialOutputFlag;

    QMap<QString, QDockWidget *> dockWidgetsMap;
    KActionMenu *dockWidgetMenu;
    QMap<QDockWidget *, bool> dockWidgetVisibilityMap;
    KoDockerManager *dockerManager;
    QList<QDockWidget *> dockWidgets;
    QByteArray m_dockerStateBeforeHiding;

    QCloseEvent *deferredClosingEvent;

#ifdef HAVE_KACTIVITIES
    KActivities::ResourceInstance *activityResource;
#endif

    Digikam::ThemeManager *themeManager;

    KHelpMenu *m_helpMenu;

    bool noCleanup;

};

KoMainWindow::KoMainWindow(const QByteArray nativeMimeType, const KComponentData &componentData)
    : KXmlGuiWindow()
    , d(new KoMainWindowPrivate(nativeMimeType, this))
{
#ifdef Q_OS_MAC
    #if QT_VERSION < 0x050000
    MacSupport::addFullscreen(this);
    #endif
    #if QT_VERSION >= 0x050201
    setUnifiedTitleAndToolBarOnMac(true);
    #endif
#endif
    setStandardToolBarMenuEnabled(true);
    Q_ASSERT(componentData.isValid());

    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

    connect(this, SIGNAL(restoringDone()), this, SLOT(forceDockTabFonts()));

    // PartManager
    // End

    if (componentData.isValid()) {
        setComponentData(componentData);   // don't load plugins! we don't want
        // the part's plugins with this main window, even though we are using the
        // part's componentData! (Simon)
        KGlobal::setActiveComponent(componentData);
    }

    QString doc;
    QStringList allFiles = KGlobal::dirs()->findAllResources("data", "calligra/calligra_shell.rc");
    setXMLFile(findMostRecentXMLFile(allFiles, doc));
    setLocalXMLFile(KStandardDirs::locateLocal("data", "calligra/calligra_shell.rc"));

    actionCollection()->addAction(KStandardAction::New, "file_new", this, SLOT(slotFileNew()));
    actionCollection()->addAction(KStandardAction::Open, "file_open", this, SLOT(slotFileOpen()));
    d->recent = KStandardAction::openRecent(this, SLOT(slotFileOpenRecent(const KUrl&)), actionCollection());
    connect(d->recent, SIGNAL(recentListCleared()), this, SLOT(saveRecentFiles()));
    d->saveAction = actionCollection()->addAction(KStandardAction::Save,  "file_save", this, SLOT(slotFileSave()));
    d->saveActionAs = actionCollection()->addAction(KStandardAction::SaveAs,  "file_save_as", this, SLOT(slotFileSaveAs()));
    d->printAction = actionCollection()->addAction(KStandardAction::Print,  "file_print", this, SLOT(slotFilePrint()));
    d->printActionPreview = actionCollection()->addAction(KStandardAction::PrintPreview,  "file_print_preview", this, SLOT(slotFilePrintPreview()));

    d->exportPdf  = new KAction(i18n("Export as PDF..."), this);
    d->exportPdf->setIcon(koIcon("application-pdf"));
    actionCollection()->addAction("file_export_pdf", d->exportPdf);
    connect(d->exportPdf, SIGNAL(triggered()), this, SLOT(exportToPdf()));

    d->sendFileAction = actionCollection()->addAction(KStandardAction::Mail,  "file_send_file", this, SLOT(slotEmailFile()));

    d->closeFile = actionCollection()->addAction(KStandardAction::Close,  "file_close", this, SLOT(slotFileClose()));
    actionCollection()->addAction(KStandardAction::Quit,  "file_quit", this, SLOT(slotFileQuit()));

    d->reloadFile  = new KAction(i18n("Reload"), this);
    actionCollection()->addAction("file_reload_file", d->reloadFile);
    connect(d->reloadFile, SIGNAL(triggered(bool)), this, SLOT(slotReloadFile()));

    d->showFileVersions  = new KAction(i18n("Versions..."), this);
    actionCollection()->addAction("file_versions_file", d->showFileVersions);
    connect(d->showFileVersions, SIGNAL(triggered(bool)), this, SLOT(slotVersionsFile()));

    d->importFile  = new KAction(koIcon("document-import"), i18n("Open ex&isting Document as Untitled Document..."), this);
    actionCollection()->addAction("file_import_file", d->importFile);
    connect(d->importFile, SIGNAL(triggered(bool)), this, SLOT(slotImportFile()));

    d->exportFile  = new KAction(koIcon("document-export"), i18n("E&xport..."), this);
    actionCollection()->addAction("file_export_file", d->exportFile);
    connect(d->exportFile, SIGNAL(triggered(bool)), this, SLOT(slotExportFile()));

    d->encryptDocument = new KAction(i18n("En&crypt Document"), this);
    actionCollection()->addAction("file_encrypt_doc", d->encryptDocument);
    connect(d->encryptDocument, SIGNAL(triggered(bool)), this, SLOT(slotEncryptDocument()));

#ifndef NDEBUG
    d->uncompressToDir = new KAction(i18n("&Uncompress to Directory"), this);
    actionCollection()->addAction("file_uncompress_doc", d->uncompressToDir);
    connect(d->uncompressToDir, SIGNAL(triggered(bool)), this, SLOT(slotUncompressToDir()));
#endif

    KAction *actionNewView  = new KAction(koIcon("window-new"), i18n("&New View"), this);
    actionCollection()->addAction("view_newview", actionNewView);
    connect(actionNewView, SIGNAL(triggered(bool)), this, SLOT(newView()));

    /* The following entry opens the document information dialog.  Since the action is named so it
        intends to show data this entry should not have a trailing ellipses (...).  */
    d->showDocumentInfo  = new KAction(koIcon("document-properties"), i18n("Document Information"), this);
    actionCollection()->addAction("file_documentinfo", d->showDocumentInfo);
    connect(d->showDocumentInfo, SIGNAL(triggered(bool)), this, SLOT(slotDocumentInfo()));

    KStandardAction::keyBindings(this, SLOT(slotConfigureKeys()), actionCollection());
    KStandardAction::configureToolbars(this, SLOT(slotConfigureToolbars()), actionCollection());

    d->showDocumentInfo->setEnabled(false);
    d->saveActionAs->setEnabled(false);
    d->reloadFile->setEnabled(false);
    d->showFileVersions->setEnabled(false);
    d->importFile->setEnabled(true);    // always enabled like File --> Open
    d->exportFile->setEnabled(false);
    d->saveAction->setEnabled(false);
    d->printAction->setEnabled(false);
    d->printActionPreview->setEnabled(false);
    d->sendFileAction->setEnabled(false);
    d->exportPdf->setEnabled(false);
    d->closeFile->setEnabled(false);
    d->encryptDocument->setEnabled(false);
#ifndef NDEBUG
    d->uncompressToDir->setEnabled(false);
#endif

    // populate theme menu
    d->themeManager = new Digikam::ThemeManager(this);
    KConfigGroup group(KGlobal::config(), "theme");
    d->themeManager->setThemeMenuAction(new KActionMenu(i18n("&Themes"), this));
    d->themeManager->registerThemeActions(actionCollection());
    d->themeManager->setCurrentTheme(group.readEntry("Theme",
                                                     d->themeManager->defaultThemeName()));
    connect(d->themeManager, SIGNAL(signalThemeChanged()), this, SIGNAL(themeChanged()));

    KToggleAction *fullscreenAction  = new KToggleAction(koIcon("view-fullscreen"), i18n("Full Screen Mode"), this);
    actionCollection()->addAction("view_fullscreen", fullscreenAction);
    fullscreenAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_F));
    connect(fullscreenAction, SIGNAL(toggled(bool)), this, SLOT(viewFullscreen(bool)));

    d->toggleDockers = new KToggleAction(i18n("Show Dockers"), this);
    d->toggleDockers->setChecked(true);
    actionCollection()->addAction("view_toggledockers", d->toggleDockers);

    connect(d->toggleDockers, SIGNAL(toggled(bool)), SLOT(toggleDockersVisibility(bool)));

    d->dockWidgetMenu  = new KActionMenu(i18n("Dockers"), this);
    actionCollection()->addAction("settings_dockers_menu", d->dockWidgetMenu);
    d->dockWidgetMenu->setVisible(false);
    d->dockWidgetMenu->setDelayed(false);

    // Load list of recent files
    KSharedConfigPtr configPtr = componentData.isValid() ? componentData.config() : KGlobal::config();
    d->recent->loadEntries(configPtr->group("RecentFiles"));


    createMainwindowGUI();
    d->mainWindowGuiIsBuilt = true;

    // if the user didn's specify the geometry on the command line (does anyone do that still?),
    // we first figure out some good default size and restore the x,y position. See bug 285804Z.
    KConfigGroup cfg(KGlobal::config(), "MainWindow");
    if (!initialGeometrySet()) {
        QByteArray geom = QByteArray::fromBase64(cfg.readEntry("ko_geometry", QByteArray()));
        if (!restoreGeometry(geom)) {
            const int scnum = QApplication::desktop()->screenNumber(parentWidget());
            QRect desk = QApplication::desktop()->availableGeometry(scnum);
            // if the desktop is virtual then use virtual screen size
            if (QApplication::desktop()->isVirtualDesktop()) {
                desk = QApplication::desktop()->availableGeometry(QApplication::desktop()->screen());
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
    }
    restoreState(QByteArray::fromBase64(cfg.readEntry("ko_windowstate", QByteArray())));

    d->dockerManager = new KoDockerManager(this);
}

void KoMainWindow::setNoCleanup(bool noCleanup)
{
    d->noCleanup = noCleanup;
}

KoMainWindow::~KoMainWindow()
{
    KConfigGroup cfg(KGlobal::config(), "MainWindow");
    cfg.writeEntry("ko_geometry", saveGeometry().toBase64());
    cfg.writeEntry("ko_windowstate", saveState().toBase64());

    {
        KConfigGroup group(KGlobal::config(), "theme");
        group.writeEntry("Theme", d->themeManager->currentThemeName());
    }

    // Explicitly delete the docker manager to ensure that it is deleted before the dockers
    delete d->dockerManager;
    d->dockerManager = 0;

    // The doc and view might still exist (this is the case when closing the window)
    if (d->rootPart)
        d->rootPart->removeMainWindow(this);

    if (d->partToOpen) {
        d->partToOpen->removeMainWindow(this);
        delete d->partToOpen;
    }

    // safety first ;)
    setActivePart(0, 0);

    if (d->rootViews.indexOf(d->activeView) == -1) {
        delete d->activeView;
        d->activeView = 0;
    }
    while (!d->rootViews.isEmpty()) {
        delete d->rootViews.takeFirst();
    }

    if(d->noCleanup)
        return;
    // We have to check if this was a root document.
    // This has to be checked from queryClose, too :)
    if (d->rootPart && d->rootPart->viewCount() == 0) {
        //kDebug(30003) <<"Destructor. No more views, deleting old doc" << d->rootDoc;
        delete d->rootDocument;
    }

    delete d;
}

void KoMainWindow::setRootDocument(KoDocument *doc, KoPart *part, bool deletePrevious)
{
    if (d->rootDocument == doc)
        return;

    if (d->partToOpen && d->partToOpen->document() != doc) {
        d->partToOpen->removeMainWindow(this);
        if (deletePrevious) delete d->partToOpen;
    }
    d->partToOpen = 0;

    //kDebug(30003) <<"KoMainWindow::setRootDocument this =" << this <<" doc =" << doc;
    QList<KoView*> oldRootViews = d->rootViews;
    d->rootViews.clear();
    KoDocument *oldRootDoc = d->rootDocument;
    KoPart *oldRootPart = d->rootPart;

    if (oldRootDoc) {
        oldRootDoc->disconnect(this);
        oldRootPart->removeMainWindow(this);

        if (dockerManager()) {
            dockerManager()->resetToolDockerWidgets();
        }

        // Hide all dockwidgets and remember their old state
        d->dockWidgetVisibilityMap.clear();

        foreach(QDockWidget* dockWidget, d->dockWidgetsMap) {
            d->dockWidgetVisibilityMap.insert(dockWidget, dockWidget->isVisible());
            dockWidget->setVisible(false);
        }

        d->dockWidgetMenu->setVisible(false);
    }

    d->rootDocument = doc;
    // XXX remove this after the splitting
    if (!part && doc) {
        d->rootPart = doc->documentPart();
    }
    else {
        d->rootPart = part;
    }

    if (doc) {
        d->dockWidgetMenu->setVisible(true);
        d->m_registeredPart = d->rootPart.data();

        KoView *view = d->rootPart->createView(doc, this);
        setCentralWidget(view);
        d->rootViews.append(view);

        view->show();
        view->setFocus();

        // The addMainWindow has been done already if using openUrl
        if (!d->rootPart->mainWindows().contains(this)) {
            d->rootPart->addMainWindow(this);
        }
    }

    bool enable = d->rootDocument != 0 ? true : false;
    d->showDocumentInfo->setEnabled(enable);
    d->saveAction->setEnabled(enable);
    d->saveActionAs->setEnabled(enable);
    d->importFile->setEnabled(enable);
    d->exportFile->setEnabled(enable);
    d->encryptDocument->setEnabled(enable);
#ifndef NDEBUG
    d->uncompressToDir->setEnabled(enable);
#endif
    d->printAction->setEnabled(enable);
    d->printActionPreview->setEnabled(enable);
    d->sendFileAction->setEnabled(enable);
    d->exportPdf->setEnabled(enable);
    d->closeFile->setEnabled(enable);
    updateCaption();

    setActivePart(d->rootPart, doc ? d->rootViews.first() : 0);
    emit restoringDone();

    while(!oldRootViews.isEmpty()) {
        delete oldRootViews.takeFirst();
    }
    if (oldRootPart && oldRootPart->viewCount() == 0) {
        //kDebug(30003) <<"No more views, deleting old doc" << oldRootDoc;
        oldRootDoc->clearUndoHistory();
        if(deletePrevious)
            delete oldRootDoc;
    }

    if (doc && !d->dockWidgetVisibilityMap.isEmpty()) {
        foreach(QDockWidget* dockWidget, d->dockWidgetsMap) {
            dockWidget->setVisible(d->dockWidgetVisibilityMap.value(dockWidget));
        }
    }

    if (!d->rootDocument) {
        statusBar()->setVisible(false);
    }
    else {
#ifdef Q_OS_MAC
        statusBar()->setMaximumHeight(28);
#endif
        connect(d->rootDocument, SIGNAL(titleModified(QString,bool)), SLOT(slotDocumentTitleModified(QString,bool)));
    }
}

void KoMainWindow::updateReloadFileAction(KoDocument *doc)
{
    d->reloadFile->setEnabled(doc && !doc->url().isEmpty());
}

void KoMainWindow::updateVersionsFileAction(KoDocument *doc)
{
    //TODO activate it just when we save it in oasis file format
    d->showFileVersions->setEnabled(doc && !doc->url().isEmpty() && (doc->outputMimeType() == doc->nativeOasisMimeType() || doc->outputMimeType() == doc->nativeOasisMimeType() + "-template"));
}

void KoMainWindow::setReadWrite(bool readwrite)
{
    d->saveAction->setEnabled(readwrite);
    d->importFile->setEnabled(readwrite);
    d->readOnly =  !readwrite;
    updateCaption();
}

void KoMainWindow::addRecentURL(const KUrl& url)
{
    kDebug(30003) << "KoMainWindow::addRecentURL url=" << url.prettyUrl();
    // Add entry to recent documents list
    // (call coming from KoDocument because it must work with cmd line, template dlg, file/open, etc.)
    if (!url.isEmpty()) {
        bool ok = true;
        if (url.isLocalFile()) {
            QString path = url.toLocalFile(KUrl::RemoveTrailingSlash);
            const QStringList tmpDirs = KGlobal::dirs()->resourceDirs("tmp");
            for (QStringList::ConstIterator it = tmpDirs.begin() ; ok && it != tmpDirs.end() ; ++it)
                if (path.contains(*it))
                    ok = false; // it's in the tmp resource
            if (ok) {
                KRecentDocument::add(path);
#if KDE_IS_VERSION(4,6,0)
                KRecentDirs::add(":OpenDialog", QFileInfo(path).dir().canonicalPath());
#endif
            }
        } else {
            KRecentDocument::add(url.url(KUrl::RemoveTrailingSlash), true);
        }
        if (ok) {
            d->recent->addUrl(url);
        }
        saveRecentFiles();

#ifdef HAVE_KACTIVITIES
        if (!d->activityResource) {
            d->activityResource = new KActivities::ResourceInstance(winId(), this);
        }
        d->activityResource->setUri(url);
#endif
    }
}

void KoMainWindow::saveRecentFiles()
{
    // Save list of recent files
    KSharedConfigPtr config = componentData().isValid() ? componentData().config() : KGlobal::config();
    kDebug(30003) << this << " Saving recent files list into config. componentData()=" << componentData().componentName();
    d->recent->saveEntries(config->group("RecentFiles"));
    config->sync();

    // Tell all windows to reload their list, after saving
    // Doesn't work multi-process, but it's a start
    foreach(KMainWindow* window, KMainWindow::memberList())
        static_cast<KoMainWindow *>(window)->reloadRecentFileList();
}

void KoMainWindow::reloadRecentFileList()
{
    KSharedConfigPtr config = componentData().isValid() ? componentData().config() : KGlobal::config();
    d->recent->loadEntries(config->group("RecentFiles"));
}

KoPart* KoMainWindow::createPart() const
{
    KoDocumentEntry entry = KoDocumentEntry::queryByMimeType(d->nativeMimeType);
    QString errorMsg;
    KoPart *part = entry.createKoPart(&errorMsg);

    if (!part || !errorMsg.isEmpty()) {
        return 0;
    }
    return part;
}

void KoMainWindow::updateCaption()
{
    kDebug(30003) << "KoMainWindow::updateCaption()";
    if (!d->rootDocument) {
        updateCaption(QString(), false);
    }
    else {
        QString caption( d->rootDocument->caption() );
        if (d->readOnly) {
            caption += ' ' + i18n("(write protected)");
        }

        updateCaption(caption, d->rootDocument->isModified());
        if (!rootDocument()->url().fileName(KUrl::ObeyTrailingSlash).isEmpty())
            d->saveAction->setToolTip(i18n("Save as %1", d->rootDocument->url().fileName(KUrl::ObeyTrailingSlash)));
        else
            d->saveAction->setToolTip(i18n("Save"));
    }
}

void KoMainWindow::updateCaption(const QString & caption, bool mod)
{
    kDebug(30003) << "KoMainWindow::updateCaption(" << caption << "," << mod << ")";
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

KoDocument *KoMainWindow::rootDocument() const
{
    return d->rootDocument;
}

KoView *KoMainWindow::rootView() const
{
    if (d->rootViews.indexOf(d->activeView) != -1)
        return d->activeView;
    return d->rootViews.first();
}

bool KoMainWindow::openDocument(const KUrl & url)
{
    if (!KIO::NetAccess::exists(url, KIO::NetAccess::SourceSide, 0)) {
        KMessageBox::error(0, i18n("The file %1 does not exist.", url.url()));
        d->recent->removeUrl(url); //remove the file from the recent-opened-file-list
        saveRecentFiles();
        return false;
    }
    return openDocumentInternal(url);
}

bool KoMainWindow::openDocument(KoPart *newPart, const KUrl & url)
{
    // the part always has a document; the document doesn't know about the part.
    KoDocument *newdoc = newPart->document();
    if (!KIO::NetAccess::exists(url, KIO::NetAccess::SourceSide, 0)) {
        newdoc->initEmpty(); //create an empty document
        setRootDocument(newdoc, newPart);
        newdoc->setUrl(url);
        QString mime = KMimeType::findByUrl(url)->name();
        if (mime.isEmpty() || mime == KMimeType::defaultMimeType())
            mime = newdoc->nativeFormatMimeType();
        newdoc->setMimeTypeAfterLoading(mime);
        updateCaption();
        return true;
    }
    return openDocumentInternal(url, newPart, newdoc);
}

bool KoMainWindow::openDocumentInternal(const KUrl & url, KoPart *newpart, KoDocument *newdoc)
{
    kDebug(30003) <<"KoMainWindow::openDocument" << url.url();

    if (!newpart)
        newpart = createPart();

    if (!newpart)
        return false;

    if (!newdoc)
        newdoc = newpart->document();

    d->firstTime = true;
    connect(newdoc, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));
    connect(newdoc, SIGNAL(completed()), this, SLOT(slotLoadCompleted()));
    connect(newdoc, SIGNAL(canceled(const QString &)), this, SLOT(slotLoadCanceled(const QString &)));
    newpart->addMainWindow(this);   // used by openUrl
    bool openRet = (!isImporting()) ? newdoc->openUrl(url) : newdoc->importDocument(url);
    if (!openRet) {
        newpart->removeMainWindow(this);
        delete newdoc;
        delete newpart;
        return false;
    }
    updateReloadFileAction(newdoc);
    updateVersionsFileAction(newdoc);

    KFileItem file(url, newdoc->mimeType(), KFileItem::Unknown);
    if (!file.isWritable()) {
        setReadWrite(false);
    }
    return true;
}

// Separate from openDocument to handle async loading (remote URLs)
void KoMainWindow::slotLoadCompleted()
{
    kDebug(30003) << "KoMainWindow::slotLoadCompleted";
    KoDocument *newdoc = qobject_cast<KoDocument*>(sender());
    KoPart *newpart = newdoc->documentPart();

    if (d->rootDocument && d->rootDocument->isEmpty()) {
        // Replace current empty document
        setRootDocument(newdoc);
    } else if (d->rootDocument && !d->rootDocument->isEmpty()) {
        // Open in a new main window
        // (Note : could create the main window first and the doc next for this
        // particular case, that would give a better user feedback...)
        KoMainWindow *s = newpart->createMainWindow();
        s->show();
        newpart->removeMainWindow(this);
        s->setRootDocument(newdoc, newpart);
    } else {
        // We had no document, set the new one
        setRootDocument(newdoc);
    }
    slotProgress(-1);
    disconnect(newdoc, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));
    disconnect(newdoc, SIGNAL(completed()), this, SLOT(slotLoadCompleted()));
    disconnect(newdoc, SIGNAL(canceled(const QString &)), this, SLOT(slotLoadCanceled(const QString &)));
    emit loadCompleted();
}

void KoMainWindow::slotLoadCanceled(const QString & errMsg)
{
    kDebug(30003) << "KoMainWindow::slotLoadCanceled";
    if (!errMsg.isEmpty())   // empty when canceled by user
        KMessageBox::error(this, errMsg);
    // ... can't delete the document, it's the one who emitted the signal...

    KoDocument* doc = qobject_cast<KoDocument*>(sender());
    Q_ASSERT(doc);
    disconnect(doc, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));
    disconnect(doc, SIGNAL(completed()), this, SLOT(slotLoadCompleted()));
    disconnect(doc, SIGNAL(canceled(const QString &)), this, SLOT(slotLoadCanceled(const QString &)));
}

void KoMainWindow::slotSaveCanceled(const QString &errMsg)
{
    kDebug(30003) << "KoMainWindow::slotSaveCanceled";
    if (!errMsg.isEmpty())   // empty when canceled by user
        KMessageBox::error(this, errMsg);
    slotSaveCompleted();
}

void KoMainWindow::slotSaveCompleted()
{
    kDebug(30003) << "KoMainWindow::slotSaveCompleted";
    KoDocument* doc = qobject_cast<KoDocument*>(sender());
    Q_ASSERT(doc);
    disconnect(doc, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));
    disconnect(doc, SIGNAL(completed()), this, SLOT(slotSaveCompleted()));
    disconnect(doc, SIGNAL(canceled(const QString &)), this, SLOT(slotSaveCanceled(const QString &)));

    if (d->deferredClosingEvent) {
        KXmlGuiWindow::closeEvent(d->deferredClosingEvent);
    }
}

// returns true if we should save, false otherwise.
bool KoMainWindow::exportConfirmation(const QByteArray &outputFormat)
{
    KConfigGroup group = KGlobal::config()->group(d->rootPart->componentData().componentName());
    if (!group.readEntry("WantExportConfirmation", true)) {
        return true;
    }

    KMimeType::Ptr mime = KMimeType::mimeType(outputFormat);
    QString comment = mime ? mime->comment() : i18n("%1 (unknown file type)", QString::fromLatin1(outputFormat));

    // Warn the user
    int ret;
    if (!isExporting()) { // File --> Save
        ret = KMessageBox::warningContinueCancel
                (
                    this,
                    i18n("<qt>Saving as a %1 may result in some loss of formatting."
                         "<p>Do you still want to save in this format?</qt>",
                         QString("<b>%1</b>").arg(comment)),      // in case we want to remove the bold later
                    i18n("Confirm Save"),
                    KStandardGuiItem::save(),
                    KStandardGuiItem::cancel(),
                    "NonNativeSaveConfirmation"
                    );
    } else { // File --> Export
        ret = KMessageBox::warningContinueCancel
                (
                    this,
                    i18n("<qt>Exporting as a %1 may result in some loss of formatting."
                         "<p>Do you still want to export to this format?</qt>",
                         QString("<b>%1</b>").arg(comment)),      // in case we want to remove the bold later
                    i18n("Confirm Export"),
                    KGuiItem(i18n("Export")),
                    KStandardGuiItem::cancel(),
                    "NonNativeExportConfirmation" // different to the one used for Save (above)
                    );
    }

    return (ret == KMessageBox::Continue);
}

bool KoMainWindow::saveDocument(bool saveas, bool silent, int specialOutputFlag)
{
    if (!d->rootDocument || !d->rootPart) {
        return true;
    }

    bool reset_url;

    if (d->rootDocument->url().isEmpty()) {
        emit saveDialogShown();
        reset_url = true;
        saveas = true;
    } else {
        reset_url = false;
    }

    connect(d->rootDocument, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));
    connect(d->rootDocument, SIGNAL(completed()), this, SLOT(slotSaveCompleted()));
    connect(d->rootDocument, SIGNAL(canceled(const QString &)), this, SLOT(slotSaveCanceled(const QString &)));

    KUrl oldURL = d->rootDocument->url();
    QString oldFile = d->rootDocument->localFilePath();

    QByteArray _native_format = d->rootDocument->nativeFormatMimeType();
    QByteArray oldOutputFormat = d->rootDocument->outputMimeType();

    int oldSpecialOutputFlag = d->rootDocument->specialOutputFlag();

    KUrl suggestedURL = d->rootDocument->url();

    QStringList mimeFilter;
    KMimeType::Ptr mime = KMimeType::mimeType(_native_format);
    if (! mime)
        mime = KMimeType::defaultMimeTypePtr();
    if (specialOutputFlag)
        mimeFilter = mime->patterns();
    else
        mimeFilter = KoFilterManager::mimeFilter(_native_format,
                                                 KoFilterManager::Export,
                                                 d->rootDocument->extraNativeMimeTypes());


    if (!mimeFilter.contains(oldOutputFormat) && !isExporting()) {
        kDebug(30003) << "KoMainWindow::saveDocument no export filter for" << oldOutputFormat;

        // --- don't setOutputMimeType in case the user cancels the Save As
        // dialog and then tries to just plain Save ---

        // suggest a different filename extension (yes, we fortunately don't all live in a world of magic :))
        QString suggestedFilename = suggestedURL.fileName();
        if (!suggestedFilename.isEmpty()) {  // ".kra" looks strange for a name
            int c = suggestedFilename.lastIndexOf('.');

            QString ext = mime->property("X-KDE-NativeExtension").toString();
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

            suggestedURL.setFileName(suggestedFilename);
        }

        // force the user to choose outputMimeType
        saveas = true;
    }

    bool ret = false;

    if (d->rootDocument->url().isEmpty() || saveas) {
        // if you're just File/Save As'ing to change filter options you
        // don't want to be reminded about overwriting files etc.
        bool justChangingFilterOptions = false;

        KoFileDialog dialog(this, KoFileDialog::SaveFile, "SaveDocument");
        dialog.setCaption(i18n("untitled"));
        dialog.setDefaultDir((isExporting() && !d->lastExportUrl.isEmpty()) ?
                                d->lastExportUrl.toLocalFile() : suggestedURL.toLocalFile());
        dialog.setMimeTypeFilters(mimeFilter);
        KUrl newURL = dialog.url();

        if (newURL.isLocalFile()) {
            QString fn = newURL.toLocalFile();
            if (QFileInfo(fn).completeSuffix().isEmpty()) {
                KMimeType::Ptr mime = KMimeType::mimeType(_native_format);
                fn.append(mime->mainExtension());
                newURL = KUrl::fromPath(fn);
            }
        }

        QByteArray outputFormat = _native_format;

        if (!specialOutputFlag) {
            KMimeType::Ptr mime = KMimeType::findByUrl(newURL);
            QString outputFormatString = mime->name();
            outputFormat = outputFormatString.toLatin1();
        }


        if (!isExporting())
            justChangingFilterOptions = (newURL == d->rootDocument->url()) &&
                    (outputFormat == d->rootDocument->mimeType()) &&
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
            if ( specialOutputFlag== KoDocument::SaveAsDirectoryStore) {
                // Do nothing
            }
            else if (specialOutputFlag == KoDocument::SaveEncrypted) {
                int dot = fileName.lastIndexOf('.');
                QString ext = mime->mainExtension();
                if (!ext.isEmpty()) {
                    if (dot < 0) fileName += ext;
                    else fileName = fileName.left(dot) + ext;
                } else { // current filename extension wrong anyway
                    if (dot > 0) fileName = fileName.left(dot);
                }
                newURL.setFileName(fileName);
            }
        }

        if (bOk) {
            bool wantToSave = true;

            // don't change this line unless you know what you're doing :)
            if (!justChangingFilterOptions || d->rootDocument->confirmNonNativeSave(isExporting())) {
                if (!d->rootDocument->isNativeFormat(outputFormat))
                    wantToSave = exportConfirmation(outputFormat);
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
                // 2. It is probably not a good idea to change d->rootDocument->mimeType
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


                d->rootDocument->setOutputMimeType(outputFormat, specialOutputFlag);
                if (!isExporting()) {  // Save As
                    ret = d->rootDocument->saveAs(newURL);

                    if (ret) {
                        kDebug(30003) << "Successful Save As!";
                        addRecentURL(newURL);
                        setReadWrite(true);
                    } else {
                        kDebug(30003) << "Failed Save As!";
                        d->rootDocument->setUrl(oldURL);
                        d->rootDocument->setLocalFilePath(oldFile);
                        d->rootDocument->setOutputMimeType(oldOutputFormat, oldSpecialOutputFlag);
                    }
                } else { // Export
                    ret = d->rootDocument->exportDocument(newURL);

                    if (ret) {
                        // a few file dialog convenience things
                        d->lastExportUrl = newURL;
                        d->lastExportedFormat = outputFormat;
                        d->lastExportSpecialOutputFlag = specialOutputFlag;
                    }

                    // always restore output format
                    d->rootDocument->setOutputMimeType(oldOutputFormat, oldSpecialOutputFlag);
                }

                if (silent) // don't let the document change the window caption
                    d->rootDocument->setTitleModified();
            }   // if (wantToSave)  {
            else
                ret = false;
        }   // if (bOk) {
        else
            ret = false;
    } else { // saving

        bool needConfirm = d->rootDocument->confirmNonNativeSave(false) && !d->rootDocument->isNativeFormat(oldOutputFormat);

        if (!needConfirm ||
                (needConfirm && exportConfirmation(oldOutputFormat /* not so old :) */))
                ) {
            // be sure d->rootDocument has the correct outputMimeType!
            if (isExporting() || d->rootDocument->isModified()) {
                ret = d->rootDocument->save();
            }

            if (!ret) {
                kDebug(30003) << "Failed Save!";
                d->rootDocument->setUrl(oldURL);
                d->rootDocument->setLocalFilePath(oldFile);
            }
        } else
            ret = false;
    }


    if (!ret && reset_url)
        d->rootDocument->resetURL(); //clean the suggested filename as the save dialog was rejected

    updateReloadFileAction(d->rootDocument);
    updateCaption();

    return ret;
}

void KoMainWindow::closeEvent(QCloseEvent *e)
{
    if(rootDocument() && rootDocument()->isLoading()) {
        e->setAccepted(false);
        return;
    }
    if (queryClose()) {
        d->deferredClosingEvent = e;
        if (d->partToOpen) {
            // The open pane is visible
            d->partToOpen->deleteOpenPane(true);
        }
        if (!d->m_dockerStateBeforeHiding.isEmpty()) {
            restoreState(d->m_dockerStateBeforeHiding);
        }
        statusBar()->setVisible(true);
        menuBar()->setVisible(true);

        saveWindowSettings();
        if(d->noCleanup)
            return;
        setRootDocument(0);
        if (!d->dockWidgetVisibilityMap.isEmpty()) { // re-enable dockers for persistency
            foreach(QDockWidget* dockWidget, d->dockWidgetsMap)
                dockWidget->setVisible(d->dockWidgetVisibilityMap.value(dockWidget));
        }
    } else {
        e->setAccepted(false);
    }
}

void KoMainWindow::saveWindowSettings()
{
    KSharedConfigPtr config = componentData().config();

    if (d->windowSizeDirty ) {

        // Save window size into the config file of our componentData
        kDebug(30003) << "KoMainWindow::saveWindowSettings";
        saveWindowSize(config->group("MainWindow"));
        config->sync();
        d->windowSizeDirty = false;
    }

    if ( rootDocument()) {

        // Save toolbar position into the config file of the app, under the doc's component name
        KConfigGroup group = KGlobal::config()->group(d->rootPart->componentData().componentName());
        //kDebug(30003) <<"KoMainWindow::closeEvent -> saveMainWindowSettings rootdoc's componentData=" << d->rootPart->componentData().componentName();
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

    KGlobal::config()->sync();
    resetAutoSaveSettings(); // Don't let KMainWindow override the good stuff we wrote down

}

void KoMainWindow::resizeEvent(QResizeEvent * e)
{
    d->windowSizeDirty = true;
    KXmlGuiWindow::resizeEvent(e);
}

bool KoMainWindow::queryClose()
{
    if (rootDocument() == 0)
        return true;
    //kDebug(30003) <<"KoMainWindow::queryClose() viewcount=" << rootDocument()->viewCount()
    //               << " mainWindowCount=" << rootDocument()->mainWindowCount() << endl;
    if (!d->forQuit && d->rootPart->mainwindowCount() > 1)
        // there are more open, and we are closing just one, so no problem for closing
        return true;

    // main doc + internally stored child documents
    if (d->rootDocument->isModified()) {
        QString name;
        if (rootDocument()->documentInfo()) {
            name = rootDocument()->documentInfo()->aboutInfo("title");
        }
        if (name.isEmpty())
            name = rootDocument()->url().fileName();

        if (name.isEmpty())
            name = i18n("Untitled");

        int res = KMessageBox::warningYesNoCancel(this,
                                                  i18n("<p>The document <b>'%1'</b> has been modified.</p><p>Do you want to save it?</p>", name),
                                                  QString(),
                                                  KStandardGuiItem::save(),
                                                  KStandardGuiItem::discard());

        switch (res) {
        case KMessageBox::Yes : {
            bool isNative = (d->rootDocument->outputMimeType() == d->rootDocument->nativeFormatMimeType());
            if (!saveDocument(!isNative))
                return false;
            break;
        }
        case KMessageBox::No :
            rootDocument()->removeAutoSaveFiles();
            rootDocument()->setModified(false);   // Now when queryClose() is called by closeEvent it won't do anything.
            break;
        default : // case KMessageBox::Cancel :
            return false;
        }
    }

    return true;
}

// Helper method for slotFileNew and slotFileClose
void KoMainWindow::chooseNewDocument(InitDocFlags initDocFlags)
{
    KoDocument* doc = rootDocument();
    KoPart *newpart = createPart();
    KoDocument *newdoc = newpart->document();

    if (!newdoc)
        return;

    disconnect(newdoc, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));

    if ((!doc && initDocFlags == InitDocFileNew) || (doc && !doc->isEmpty())) {
        KoMainWindow *s = newpart->createMainWindow();
        s->show();
        newpart->addMainWindow(s);
        newpart->showStartUpWidget(s, true /*Always show widget*/);
        return;
    }

    if (doc) {
        setRootDocument(0);
        if(d->rootDocument)
            d->rootDocument->clearUndoHistory();
        delete d->rootDocument;
        d->rootDocument = 0;
    }

    newpart->addMainWindow(this);
    newpart->showStartUpWidget(this, true /*Always show widget*/);
}

void KoMainWindow::slotFileNew()
{
    chooseNewDocument(InitDocFileNew);
}

void KoMainWindow::slotFileOpen()
{
    KUrl url;
    if (!isImporting()) {
        KoFileDialog dialog(this, KoFileDialog::OpenFile, "OpenDocument");
        dialog.setCaption(i18n("Open Document"));
        dialog.setDefaultDir(qApp->applicationName().contains("krita") || qApp->applicationName().contains("karbon")
                               ? QDesktopServices::storageLocation(QDesktopServices::PicturesLocation)
                               : QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation));
        dialog.setMimeTypeFilters(koApp->mimeFilter(KoFilterManager::Import));
        dialog.setHideNameFilterDetailsOption();
        url = dialog.url();
    } else {
        KoFileDialog dialog(this, KoFileDialog::ImportFile, "OpenDocument");
        dialog.setCaption(i18n("Import Document"));
        dialog.setDefaultDir(qApp->applicationName().contains("krita") || qApp->applicationName().contains("karbon")
                                ? QDesktopServices::storageLocation(QDesktopServices::PicturesLocation)
                                : QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation));
        dialog.setMimeTypeFilters(koApp->mimeFilter(KoFilterManager::Import));
        dialog.setHideNameFilterDetailsOption();
        url = dialog.url();
    }

    if (url.isEmpty())
        return;

    (void) openDocument(url);
}

void KoMainWindow::slotFileOpenRecent(const KUrl & url)
{
    // Create a copy, because the original KUrl in the map of recent files in
    // KRecentFilesAction may get deleted.
    (void) openDocument(KUrl(url));
}

void KoMainWindow::slotFileSave()
{
    if (saveDocument())
        emit documentSaved();
}

void KoMainWindow::slotFileSaveAs()
{
    if (saveDocument(true))
        emit documentSaved();
}

void KoMainWindow::slotEncryptDocument()
{
    if (saveDocument(false, false, KoDocument::SaveEncrypted))
        emit documentSaved();
}

void KoMainWindow::slotUncompressToDir()
{
    if (saveDocument(true, false, KoDocument::SaveAsDirectoryStore))
        emit documentSaved();
}

void KoMainWindow::slotDocumentInfo()
{
    if (!rootDocument())
        return;

    KoDocumentInfo *docInfo = rootDocument()->documentInfo();

    if (!docInfo)
        return;

    KoDocumentInfoDlg *dlg = d->rootDocument->createDocumentInfoDialog(this, docInfo);

    if (dlg->exec()) {
        if (dlg->isDocumentSaved()) {
            rootDocument()->setModified(false);
        } else {
            rootDocument()->setModified(true);
        }
        rootDocument()->setTitleModified();
    }

    delete dlg;
}

void KoMainWindow::slotFileClose()
{
    if (queryClose()) {
        saveWindowSettings();
        setRootDocument(0);   // don't delete this main window when deleting the document
        if(d->rootDocument)
            d->rootDocument->clearUndoHistory();
        delete d->rootDocument;
        d->rootDocument = 0;
        chooseNewDocument(InitDocFileClose);
    }
}

void KoMainWindow::slotFileQuit()
{
    close();
}

void KoMainWindow::slotFilePrint()
{
    if (!rootView())
        return;
    KoPrintJob *printJob = rootView()->createPrintJob();
    if (printJob == 0)
        return;
    d->applyDefaultSettings(printJob->printer());
    QPrintDialog *printDialog = rootView()->createPrintDialog( printJob, this );
    if (printDialog && printDialog->exec() == QDialog::Accepted)
        printJob->startPrinting(KoPrintJob::DeleteWhenDone);
    else
        delete printJob;
    delete printDialog;
}

void KoMainWindow::slotFilePrintPreview()
{
    if (!rootView())
        return;
    KoPrintJob *printJob = rootView()->createPrintJob();
    if (printJob == 0)
        return;

    /* Sets the startPrinting() slot to be blocking.
     The Qt print-preview dialog requires the printing to be completely blocking
     and only return when the full document has been printed.
     By default the KoPrintingDialog is non-blocking and
     multithreading, setting blocking to true will allow it to be used in the preview dialog */
    printJob->setProperty("blocking", true);
    QPrintPreviewDialog *preview = new QPrintPreviewDialog(&printJob->printer(), this);
    printJob->setParent(preview); // will take care of deleting the job
    connect(preview, SIGNAL(paintRequested(QPrinter*)), printJob, SLOT(startPrinting()));
    preview->exec();
    delete preview;
}

KoPrintJob* KoMainWindow::exportToPdf(const QString &pdfFileName)
{
    if (!rootView())
        return 0;
    KoPageLayout pageLayout;
    pageLayout = rootView()->pageLayout();
    return exportToPdf(pageLayout, pdfFileName);
}

KoPrintJob* KoMainWindow::exportToPdf(KoPageLayout pageLayout, QString pdfFileName)
{
    if (!rootView())
        return 0;
    if (pdfFileName.isEmpty()) {
        KConfigGroup group = KGlobal::config()->group("File Dialogs");
        QString defaultDir = group.readEntry("SavePdfDialog");
        if (defaultDir.isEmpty())
            defaultDir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
        KUrl startUrl = KUrl(defaultDir);
        KoDocument* pDoc = rootDocument();
        /** if document has a file name, take file name and replace extension with .pdf */
        if (pDoc && pDoc->url().isValid()) {
            startUrl = pDoc->url();
            QString fileName = startUrl.fileName();
            fileName = fileName.replace( QRegExp( "\\.\\w{2,5}$", Qt::CaseInsensitive ), ".pdf" );
            startUrl.setFileName( fileName );
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
        KUrl url = dialog.url();

        pdfFileName = url.toLocalFile();
        if (pdfFileName.isEmpty())
            return 0;
    }

    KoPrintJob *printJob = rootView()->createPdfPrintJob();
    if (printJob == 0)
        return 0;
    if (isHidden()) {
        printJob->setProperty("noprogressdialog", true);
    }

    d->applyDefaultSettings(printJob->printer());
    // TODO for remote files we have to first save locally and then upload.
    printJob->printer().setOutputFileName(pdfFileName);
    printJob->printer().setColorMode(QPrinter::Color);

    if (pageLayout.format == KoPageFormat::CustomSize) {
        printJob->printer().setPaperSize(QSizeF(pageLayout.width, pageLayout.height), QPrinter::Millimeter);
    } else {
        printJob->printer().setPaperSize(KoPageFormat::printerPageSize(pageLayout.format));
    }

    switch (pageLayout.orientation) {
    case KoPageFormat::Portrait: printJob->printer().setOrientation(QPrinter::Portrait); break;
    case KoPageFormat::Landscape: printJob->printer().setOrientation(QPrinter::Landscape); break;
    }

    printJob->printer().setPageMargins(pageLayout.leftMargin, pageLayout.topMargin, pageLayout.rightMargin, pageLayout.bottomMargin, QPrinter::Millimeter);

    //before printing check if the printer can handle printing
    if (!printJob->canPrint()) {
        KMessageBox::error(this, i18n("Cannot export to the specified file"));
    }

    printJob->startPrinting(KoPrintJob::DeleteWhenDone);
    return printJob;
}

void KoMainWindow::slotConfigureKeys()
{
    QAction* undoAction=0;
    QAction* redoAction=0;
    QString oldUndoText;
    QString oldRedoText;
    if(currentView()) {
        //The undo/redo action text is "undo" + command, replace by simple text while inside editor
        undoAction = currentView()->actionCollection()->action("edit_undo");
        redoAction = currentView()->actionCollection()->action("edit_redo");
        oldUndoText = undoAction->text();
        oldRedoText = redoAction->text();
        undoAction->setText(i18n("Undo"));
        redoAction->setText(i18n("Redo"));
    }

    guiFactory()->configureShortcuts();

    if(currentView()) {
        undoAction->setText(oldUndoText);
        redoAction->setText(oldRedoText);
    }

    emit keyBindingsChanged();
}

void KoMainWindow::slotConfigureToolbars()
{
    if (rootDocument())
        saveMainWindowSettings(KGlobal::config()->group(d->rootPart->componentData().componentName()));
    KEditToolBar edit(factory(), this);
    connect(&edit, SIGNAL(newToolBarConfig()), this, SLOT(slotNewToolbarConfig()));
    (void) edit.exec();
}

void KoMainWindow::slotNewToolbarConfig()
{
    if (rootDocument()) {
        applyMainWindowSettings(KGlobal::config()->group(d->rootPart->componentData().componentName()));
    }

    KXMLGUIFactory *factory = guiFactory();
    Q_UNUSED(factory);

    // Check if there's an active view
    if (!d->activeView)
        return;

    plugActionList("toolbarlist", d->toolbarList);
}

void KoMainWindow::slotToolbarToggled(bool toggle)
{
    //kDebug(30003) <<"KoMainWindow::slotToolbarToggled" << sender()->name() <<" toggle=" << true;
    // The action (sender) and the toolbar have the same name
    KToolBar * bar = toolBar(sender()->objectName());
    if (bar) {
        if (toggle)
            bar->show();
        else
            bar->hide();

        if (rootDocument())
            saveMainWindowSettings(KGlobal::config()->group(d->rootPart->componentData().componentName()));
    } else
        kWarning(30003) << "slotToolbarToggled : Toolbar " << sender()->objectName() << " not found!";
}

bool KoMainWindow::toolbarIsVisible(const char *tbName)
{
    QWidget *tb = toolBar(tbName);
    return !tb->isHidden();
}

void KoMainWindow::showToolbar(const char * tbName, bool shown)
{
    QWidget * tb = toolBar(tbName);
    if (!tb) {
        kWarning(30003) << "KoMainWindow: toolbar " << tbName << " not found.";
        return;
    }
    if (shown)
        tb->show();
    else
        tb->hide();

    // Update the action appropriately
    foreach(QAction* action, d->toolbarList) {
        if (action->objectName() != tbName) {
            //kDebug(30003) <<"KoMainWindow::showToolbar setChecked" << shown;
            static_cast<KToggleAction *>(action)->setChecked(shown);
            break;
        }
    }
}

void KoMainWindow::viewFullscreen(bool fullScreen)
{
    if (fullScreen) {
        setWindowState(windowState() | Qt::WindowFullScreen);   // set
    } else {
        setWindowState(windowState() & ~Qt::WindowFullScreen);   // reset
    }
}

void KoMainWindow::slotProgress(int value)
{
    QMutexLocker locker(&d->progressMutex);
    kDebug(30003) << "KoMainWindow::slotProgress" << value;
    if (value <= -1 || value >= 100) {
        if (d->progress) {
            statusBar()->removeWidget(d->progress);
            delete d->progress;
            d->progress = 0;
        }
        d->firstTime = true;
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
}

void KoMainWindow::setMaxRecentItems(uint _number)
{
    d->recent->setMaxItems(_number);
}

void KoMainWindow::slotEmailFile()
{
    if (!rootDocument())
        return;

    // Subject = Document file name
    // Attachment = The current file
    // Message Body = The current document in HTML export? <-- This may be an option.
    QString theSubject;
    QStringList urls;
    QString fileURL;
    if (rootDocument()->url().isEmpty() ||
            rootDocument()->isModified()) {
        //Save the file as a temporary file
        bool const tmp_modified = rootDocument()->isModified();
        KUrl const tmp_url = rootDocument()->url();
        QByteArray const tmp_mimetype = rootDocument()->outputMimeType();

        // a little open, close, delete dance to make sure we have a nice filename
        // to use, but won't block windows from creating a new file with this name.
        KTemporaryFile *tmpfile = new KTemporaryFile();
        tmpfile->open();
        QString fileName = tmpfile->fileName();
        tmpfile->close();
        delete tmpfile;

        KUrl u;
        u.setPath(fileName);
        rootDocument()->setUrl(u);
        rootDocument()->setModified(true);
        rootDocument()->setOutputMimeType(rootDocument()->nativeFormatMimeType());

        saveDocument(false, true);

        fileURL = fileName;
        theSubject = i18n("Document");
        urls.append(fileURL);

        rootDocument()->setUrl(tmp_url);
        rootDocument()->setModified(tmp_modified);
        rootDocument()->setOutputMimeType(tmp_mimetype);
    } else {
        fileURL = rootDocument()->url().url();
        theSubject = i18n("Document - %1", rootDocument()->url().fileName(KUrl::ObeyTrailingSlash));
        urls.append(fileURL);
    }

    kDebug(30003) << "(" << fileURL << ")";

    if (!fileURL.isEmpty()) {
        KToolInvocation::invokeMailer(QString(), QString(), QString(), theSubject,
                                      QString(), //body
                                      QString(),
                                      urls); // attachments
    }
}

void KoMainWindow::slotVersionsFile()
{
    if (!rootDocument())
        return;
    KoVersionDialog *dlg = new KoVersionDialog(this, rootDocument());
    dlg->exec();
    delete dlg;
}

void KoMainWindow::slotReloadFile()
{
    KoDocument* pDoc = rootDocument();
    if (!pDoc || pDoc->url().isEmpty() || !pDoc->isModified())
        return;

    bool bOk = KMessageBox::questionYesNo(this,
                                          i18n("You will lose all changes made since your last save\n"
                                               "Do you want to continue?"),
                                          i18n("Warning")) == KMessageBox::Yes;
    if (!bOk)
        return;

    KUrl url = pDoc->url();
    if (!pDoc->isEmpty()) {
        saveWindowSettings();
        setRootDocument(0);   // don't delete this main window when deleting the document
        if(d->rootDocument)
            d->rootDocument->clearUndoHistory();
        delete d->rootDocument;
        d->rootDocument = 0;
    }
    openDocument(url);
    return;

}

void KoMainWindow::slotImportFile()
{
    kDebug(30003) << "slotImportFile()";

    d->isImporting = true;
    slotFileOpen();
    d->isImporting = false;
}

void KoMainWindow::slotExportFile()
{
    kDebug(30003) << "slotExportFile()";

    d->isExporting = true;
    slotFileSaveAs();
    d->isExporting = false;
}

bool KoMainWindow::isImporting() const
{
    return d->isImporting;
}

bool KoMainWindow::isExporting() const
{
    return d->isExporting;
}

void KoMainWindow::setPartToOpen(KoPart *part)
{
    d->partToOpen = part;
}

QDockWidget* KoMainWindow::createDockWidget(KoDockFactoryBase* factory)
{
    QDockWidget* dockWidget = 0;

    if (!d->dockWidgetsMap.contains(factory->id())) {
        dockWidget = factory->createDockWidget();

        // It is quite possible that a dock factory cannot create the dock; don't
        // do anything in that case.
        if (!dockWidget) return 0;
        d->dockWidgets.push_back(dockWidget);

        KoDockWidgetTitleBar *titleBar = 0;
        // Check if the dock widget is supposed to be collapsable
        if (!dockWidget->titleBarWidget()) {
            titleBar = new KoDockWidgetTitleBar(dockWidget);
            dockWidget->setTitleBarWidget(titleBar);
            titleBar->setCollapsable(factory->isCollapsable());
        }

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

        if (rootDocument()) {
            KConfigGroup group = KGlobal::config()->group(d->rootPart->componentData().componentName()).group("DockWidget " + factory->id());
            side = static_cast<Qt::DockWidgetArea>(group.readEntry("DockArea", static_cast<int>(side)));
            if (side == Qt::NoDockWidgetArea) side = Qt::RightDockWidgetArea;
        }
        addDockWidget(side, dockWidget);
        if (dockWidget->features() & QDockWidget::DockWidgetClosable) {
            d->dockWidgetMenu->addAction(dockWidget->toggleViewAction());
            if (!visible)
                dockWidget->hide();
        }

        bool collapsed = factory->defaultCollapsed();
        bool locked = false;
        if (rootDocument()) {
            KConfigGroup group = KGlobal::config()->group(d->rootPart->componentData().componentName()).group("DockWidget " + factory->id());
            collapsed = group.readEntry("Collapsed", collapsed);
            locked = group.readEntry("Locked", locked);
        }
        if (titleBar && collapsed)
            titleBar->setCollapsed(true);
        if (titleBar && locked)
            titleBar->setLocked(true);

        d->dockWidgetsMap.insert(factory->id(), dockWidget);
    } else {
        dockWidget = d->dockWidgetsMap[ factory->id()];
    }

    KConfigGroup group(KGlobal::config(), "GUI");
    QFont dockWidgetFont  = KGlobalSettings::generalFont();
    qreal pointSize = group.readEntry("palettefontsize", dockWidgetFont.pointSize() * 0.75);
    pointSize = qMax(pointSize, KGlobalSettings::smallestReadableFont().pointSizeF());
    dockWidgetFont.setPointSizeF(pointSize);
#ifdef Q_OS_MAC
    dockWidget->setAttribute(Qt::WA_MacSmallSize, true);
#endif
    dockWidget->setFont(dockWidgetFont);

    connect(dockWidget, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(forceDockTabFonts()));

    return dockWidget;
}

void KoMainWindow::forceDockTabFonts()
{
    QObjectList chis = children();
    for (int i = 0; i < chis.size(); ++i) {
        if (chis.at(i)->inherits("QTabBar")) {
            QFont dockWidgetFont  = KGlobalSettings::generalFont();
            qreal pointSize = KGlobalSettings::smallestReadableFont().pointSizeF();
            dockWidgetFont.setPointSizeF(pointSize);
            ((QTabBar *)chis.at(i))->setFont(dockWidgetFont);
        }
    }
}

QList<QDockWidget*> KoMainWindow::dockWidgets()
{
    return d->dockWidgetsMap.values();
}

QList<KoCanvasObserverBase*> KoMainWindow::canvasObservers()
{

    QList<KoCanvasObserverBase*> observers;

    foreach(QDockWidget *docker, dockWidgets()) {
        KoCanvasObserverBase *observer = dynamic_cast<KoCanvasObserverBase*>(docker);
        if (observer) {
            observers << observer;
        }
    }
    return observers;
}


KoDockerManager * KoMainWindow::dockerManager() const
{
    return d->dockerManager;
}

void KoMainWindow::toggleDockersVisibility(bool visible)
{
    if (!visible) {
        d->m_dockerStateBeforeHiding = saveState();

        foreach(QObject* widget, children()) {
            if (widget->inherits("QDockWidget")) {
                QDockWidget* dw = static_cast<QDockWidget*>(widget);
                if (dw->isVisible()) {
                    dw->hide();
                }
            }
        }
    }
    else {
        restoreState(d->m_dockerStateBeforeHiding);
    }
}

KRecentFilesAction *KoMainWindow::recentAction() const
{
    return d->recent;
}

KoView* KoMainWindow::currentView() const
{
    // XXX
    if (d->activeView) {
        return d->activeView;
    }
    else if (!d->rootViews.isEmpty()) {
        return d->rootViews.first();
    }
    return 0;
}

void KoMainWindow::newView()
{
    Q_ASSERT((d != 0 && d->activeView && d->activePart && d->activeView->koDocument()));

    KoMainWindow *mainWindow = d->activePart->createMainWindow();
    mainWindow->setRootDocument(d->activeView->koDocument(), d->activePart);
    mainWindow->show();
}

void KoMainWindow::createMainwindowGUI()
{
    if ( isHelpMenuEnabled() && !d->m_helpMenu )
        d->m_helpMenu = new KHelpMenu( this, componentData().aboutData(), false, actionCollection() );

    QString f = xmlFile();
    setXMLFile( KStandardDirs::locate( "config", "ui/ui_standards.rc", componentData() ) );
    if ( !f.isEmpty() )
        setXMLFile( f, true );
    else
    {
        QString auto_file( componentData().componentName() + "ui.rc" );
        setXMLFile( auto_file, true );
    }

    guiFactory()->addClient( this );

}

// PartManager

void KoMainWindow::removePart( KoPart *part )
{
    if (d->m_registeredPart.data() != part) {
        return;
    }
    d->m_registeredPart = 0;
    if ( part == d->m_activePart ) {
        setActivePart(0, 0);
    }
}

void KoMainWindow::setActivePart(KoPart *part, QWidget *widget )
{
    if (part && d->m_registeredPart.data() != part) {
        kWarning(1000) << "trying to activate a non-registered part!" << part->objectName();
        return; // don't allow someone call setActivePart with a part we don't know about
    }

    // don't activate twice
    if ( d->m_activePart && part && d->m_activePart == part &&
         (!widget || d->m_activeWidget == widget) )
        return;

    KoPart *oldActivePart = d->m_activePart;
    QWidget *oldActiveWidget = d->m_activeWidget;

    d->m_activePart = part;
    d->m_activeWidget = widget;

    if (oldActivePart) {
        KoPart *savedActivePart = part;
        QWidget *savedActiveWidget = widget;

        if ( oldActiveWidget ) {
            disconnect( oldActiveWidget, SIGNAL(destroyed()), this, SLOT(slotWidgetDestroyed()) );
        }

        d->m_activePart = savedActivePart;
        d->m_activeWidget = savedActiveWidget;
    }

    if (d->m_activePart && d->m_activeWidget ) {
        connect( d->m_activeWidget, SIGNAL(destroyed()), this, SLOT(slotWidgetDestroyed()) );
    }
    // Set the new active instance in KGlobal
    KGlobal::setActiveComponent(d->m_activePart ? d->m_activePart->componentData() : KGlobal::mainComponent());

    // old slot called from part manager
    KoPart *newPart = static_cast<KoPart*>(d->m_activePart.data());

    if (d->activePart && d->activePart == newPart) {
        //kDebug(30003) <<"no need to change the GUI";
        return;
    }

    KXMLGUIFactory *factory = guiFactory();

    if (d->activeView) {

        factory->removeClient(d->activeView);

        unplugActionList("toolbarlist");
        qDeleteAll(d->toolbarList);
        d->toolbarList.clear();
    }

    if (!d->mainWindowGuiIsBuilt) {
        createMainwindowGUI();
    }

    if (newPart && d->m_activeWidget && d->m_activeWidget->inherits("KoView")) {
        d->activeView = qobject_cast<KoView *>(d->m_activeWidget);
        d->activeView->actionCollection()->addAction("view_newview", actionCollection()->action("view_newview"));
        d->activePart = newPart;
        //kDebug(30003) <<"new active part is" << d->activePart;

        factory->addClient(d->activeView);

        // Position and show toolbars according to user's preference
        setAutoSaveSettings(newPart->componentData().componentName(), false);

        foreach (QDockWidget *wdg, d->dockWidgets) {
            if ((wdg->features() & QDockWidget::DockWidgetClosable) == 0) {
                wdg->setVisible(true);
            }
        }

        // Create and plug toolbar list for Settings menu
        foreach(QWidget* it, factory->containers("ToolBar")) {
            KToolBar * toolBar = ::qobject_cast<KToolBar *>(it);
            if (toolBar) {
                KToggleAction * act = new KToggleAction(i18n("Show %1 Toolbar", toolBar->windowTitle()), this);
                actionCollection()->addAction(toolBar->objectName().toUtf8(), act);
                act->setCheckedState(KGuiItem(i18n("Hide %1 Toolbar", toolBar->windowTitle())));
                connect(act, SIGNAL(toggled(bool)), this, SLOT(slotToolbarToggled(bool)));
                act->setChecked(!toolBar->isHidden());
                d->toolbarList.append(act);
            } else
                kWarning(30003) << "Toolbar list contains a " << it->metaObject()->className() << " which is not a toolbar!";
        }
        plugActionList("toolbarlist", d->toolbarList);

    }
    else {
        d->activeView = 0;
        d->activePart = 0;
    }

    if (d->activeView) {
        d->activeView->guiActivateEvent(true);
    }
}

void KoMainWindow::slotWidgetDestroyed()
{
    kDebug(1000);
    if ( static_cast<const QWidget *>( sender() ) == d->m_activeWidget )
        setActivePart(0, 0); //do not remove the part because if the part's widget dies, then the
    //part will delete itself anyway, invoking removePart() in its destructor
}

void KoMainWindow::slotDocumentTitleModified(const QString &caption, bool mod)
{
    updateCaption(caption, mod);
    updateReloadFileAction(d->rootDocument);
    updateVersionsFileAction(d->rootDocument);

}





#include <KoMainWindow.moc>
