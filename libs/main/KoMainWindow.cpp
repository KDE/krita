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
#include "KoServiceProvider.h"
#include "KoPart.h"
#include <KoPageLayoutWidget.h>
#include <KoIcon.h>

#include <kdeversion.h>
#if KDE_IS_VERSION(4,6,0)
#include <krecentdirs.h>
#endif
#include <krecentfilesaction.h>
#include <kaboutdata.h>
#include <ktoggleaction.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kedittoolbar.h>
#include <ktemporaryfile.h>
#include <krecentdocument.h>
#include <kparts/partmanager.h>
#include <kparts/plugin.h>
#include <kparts/event.h>
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
#include <kfilewidget.h>
#include <kurlcombobox.h>
#include <kdiroperator.h>
#include <kmenubar.h>

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
#include <QPrintPreviewDialog>

#include "calligraversion.h"

class KoPartManager : public KParts::PartManager
{
public:
    KoPartManager(QWidget * parent)
            : KParts::PartManager(parent) {
        setSelectionPolicy(KParts::PartManager::TriState);
        setAllowNestedParts(false);
        setIgnoreScrollBars(true);
    }
    virtual bool eventFilter(QObject *obj, QEvent *ev) {
        if (!obj || !ev || !obj->isWidgetType())
            return false;
        return KParts::PartManager::eventFilter(obj, ev);
    }
};

class KoMainWindowPrivate
{
public:
    KoMainWindowPrivate(KoMainWindow *w) {
        parent = w;
        rootDocument = 0;
        rootPart = 0;
        partToOpen = 0;
        manager = 0;
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
        statusBarLabel = 0;
        sendFileAction = 0;
        exportPdf = 0;
        closeFile = 0;
        reloadFile = 0;
        showFileVersions = 0;
        importFile = 0;
        exportFile = 0;
        isImporting = false;
        isExporting = false;
        windowSizeDirty = false;
        lastExportSpecialOutputFlag = 0;
        readOnly = false;
        dockWidgetMenu = 0;
        dockerManager = 0;
        deferredClosingEvent = 0;
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
                    title.truncate(title.length() - extension.length());
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

    KoMainWindow *parent;
    KoDocument *rootDocument;
    KoPart *rootPart;
    KoPart *partToOpen;
    QList<KoView*> rootViews;
    KParts::PartManager *manager;

    KParts::Part *activePart;
    KoView *activeView;

    QLabel * statusBarLabel;
    QProgressBar *progress;

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

};

KoMainWindow::KoMainWindow(const KComponentData &componentData)
        : KParts::MainWindow()
        , d(new KoMainWindowPrivate(this))
{
    setStandardToolBarMenuEnabled(true);
    Q_ASSERT(componentData.isValid());

    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

    connect(this, SIGNAL(restoringDone()), this, SLOT(forceDockTabFonts()));

    d->manager = new KoPartManager(this);

    connect(d->manager, SIGNAL(activePartChanged(KParts::Part *)),
            this, SLOT(slotActivePartChanged(KParts::Part *)));

    if (componentData.isValid()) {
        setComponentData(componentData, false);   // don't load plugins! we don't want
        // the part's plugins with this shell, even though we are using the
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

    d->importFile  = new KAction(koIcon("document-import"), i18n("I&mport..."), this);
    actionCollection()->addAction("file_import_file", d->importFile);
    connect(d->importFile, SIGNAL(triggered(bool)), this, SLOT(slotImportFile()));

    d->exportFile  = new KAction(koIcon("document-export"), i18n("E&xport..."), this);
    actionCollection()->addAction("file_export_file", d->exportFile);
    connect(d->exportFile, SIGNAL(triggered(bool)), this, SLOT(slotExportFile()));

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

    // set up the action "list" for "Close all Views" (hacky :) (Werner)
    KToggleAction *fullscreenAction  = new KToggleAction(koIcon("view-fullscreen"), i18n("Full Screen Mode"), this);
    actionCollection()->addAction("view_fullscreen", fullscreenAction);
    fullscreenAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_F));
    connect(fullscreenAction, SIGNAL(toggled(bool)), this, SLOT(viewFullscreen(bool)));

    d->toggleDockers = new KToggleAction(i18n("Show Dockers"), this);
    d->toggleDockers->setChecked(true);
    actionCollection()->addAction("view_toggledockers", d->toggleDockers);

    d->toggleDockers->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_H));
    connect(d->toggleDockers, SIGNAL(toggled(bool)), SLOT(toggleDockersVisibility(bool)));

    d->dockWidgetMenu  = new KActionMenu(i18n("Dockers"), this);
    actionCollection()->addAction("settings_dockers_menu", d->dockWidgetMenu);
    d->dockWidgetMenu->setVisible(false);
    d->dockWidgetMenu->setDelayed(false);

    // Load list of recent files
    KSharedConfigPtr configPtr = componentData.isValid() ? componentData.config() : KGlobal::config();
    d->recent->loadEntries(configPtr->group("RecentFiles"));


    createShellGUI();
    d->mainWindowGuiIsBuilt = true;

    // if the user didn's specify the geometry on the command line (does anyone do that still?),
    // we first figure out some good default size and restore the x,y position. See bug 285804Z.
    if (!initialGeometrySet()) {

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
            w = ( deskWidth / 3 ) * 2;
            h = desk.height();
        }
        else {
            w = desk.width();
            h = desk.height();
        }
        // KDE doesn't restore the x,y position, so let's do that ourselves
        KConfigGroup cfg(KGlobal::config(), "MainWindow");
        x = cfg.readEntry("ko_x", x);
        y = cfg.readEntry("ko_y", y);
        setGeometry(x, y, w, h);
    }

    // Now ask kde to restore the size of the window; this could probably be replaced by
    // QWidget::saveGeometry asnd QWidget::restoreGeometry, but let's stay with the KDE
    // way of doing things.
    KConfigGroup config(KGlobal::config(), "MainWindow");
    restoreWindowSize( config );

    d->dockerManager = new KoDockerManager(this);
}

KoMainWindow::~KoMainWindow()
{
    KConfigGroup cfg(KGlobal::config(), "MainWindow");
    cfg.writeEntry("ko_x", frameGeometry().x());
    cfg.writeEntry("ko_y", frameGeometry().y());

    // Explicitly delete the docker manager to ensure that it is deleted before the dockers
    delete d->dockerManager;
    d->dockerManager = 0;
    // The doc and view might still exist (this is the case when closing the window)
    if (d->rootPart)
        d->rootPart->removeShell(this);

    if (d->partToOpen) {
        d->partToOpen->removeShell(this);
        delete d->partToOpen;
    }

    // safety first ;)
    d->manager->setActivePart(0);

    if (d->rootViews.indexOf(d->activeView) == -1) {
        delete d->activeView;
        d->activeView = 0;
    }
    while(!d->rootViews.isEmpty()) {
        delete d->rootViews.takeFirst();
    }

    // We have to check if this was a root document.
    // This has to be checked from queryClose, too :)
    if (d->rootPart && d->rootPart->viewCount() == 0) {
        //kDebug(30003) <<"Destructor. No more views, deleting old doc" << d->rootDoc;
        delete d->rootDocument;
        delete d->rootPart;
    }

    delete d->manager;
    delete d;
}

void KoMainWindow::setRootDocument(KoDocument *doc, KoPart *rootPart)
{
    if (d->rootDocument == doc)
        return;

    if (d->partToOpen && d->partToOpen->document() != doc) {
        d->partToOpen->removeShell(this);
        delete d->partToOpen;
    }
    d->partToOpen = 0;

    //kDebug(30003) <<"KoMainWindow::setRootDocument this =" << this <<" doc =" << doc;
    QList<KoView*> oldRootViews = d->rootViews;
    d->rootViews.clear();
    KoDocument *oldRootDoc = d->rootDocument;
    KoPart *oldRootPart = d->rootPart;

    if (oldRootDoc) {
        oldRootPart->removeShell(this);

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
    if (!rootPart && doc) {
        d->rootPart = doc->documentPart();
    }
    else {
        d->rootPart = rootPart;
    }

    if (doc) {
        d->dockWidgetMenu->setVisible(true);
        //d->manager->addPart( doc, false ); // done by KoView::setPartManager
        KoView *view = d->rootPart->createView(this);
        setCentralWidget(view);
        d->rootViews.append(view);
        view->setPartManager(d->manager);
        view->show();
        view->setFocus();

        // The addShell has been done already if using openUrl
        if (!d->rootPart->shells().contains(this)) {
            d->rootPart->addShell(this);
        }
    }

    bool enable = d->rootDocument != 0 ? true : false;
    d->showDocumentInfo->setEnabled(enable);
    d->saveAction->setEnabled(enable);
    d->saveActionAs->setEnabled(enable);
    d->importFile->setEnabled(enable);
    d->exportFile->setEnabled(enable);
    d->printAction->setEnabled(enable);
    d->printActionPreview->setEnabled(enable);
    d->sendFileAction->setEnabled(enable);
    d->exportPdf->setEnabled(enable);
    d->closeFile->setEnabled(enable);
    updateCaption();

    d->manager->setActivePart(d->rootPart, doc ? d->rootViews.first() : 0);
    emit restoringDone();

    while(!oldRootViews.isEmpty()) {
        delete oldRootViews.takeFirst();
    }
    if (oldRootPart && oldRootPart->viewCount() == 0) {
        //kDebug(30003) <<"No more views, deleting old doc" << oldRootDoc;
        oldRootDoc->clearUndoHistory();
        delete oldRootDoc;
    }

    if (doc && !d->dockWidgetVisibilityMap.isEmpty()) {
        foreach(QDockWidget* dockWidget, d->dockWidgetsMap) {
            dockWidget->setVisible(d->dockWidgetVisibilityMap.value(dockWidget));
        }
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
    KoDocumentEntry entry = KoDocumentEntry(KoServiceProvider::readNativeService());
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
        if (d->readOnly)
            caption += ' ' + i18n("(write protected)");

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

bool KoMainWindow::openDocument(KoPart *newPart, KoDocument *newdoc, const KUrl & url)
{
    if (!KIO::NetAccess::exists(url, KIO::NetAccess::SourceSide, 0)) {
        // the part always has a document; the document doesn't know about the part.
        if (!newdoc) {
            newdoc = newPart->document();
        }
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
    //kDebug(30003) <<"KoMainWindow::openDocument" << url.url();

    if (!newpart)
        newpart = createPart();

    if (!newpart)
        return false;

    if (!newdoc)
        newdoc = newpart->document();

    d->firstTime = true;
    connect(newdoc, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));
    connect(newpart, SIGNAL(completed()), this, SLOT(slotLoadCompleted()));
    connect(newpart, SIGNAL(canceled(const QString &)), this, SLOT(slotLoadCanceled(const QString &)));
    newpart->addShell(this);   // used by openUrl
    bool openRet = (!isImporting()) ? newdoc->openUrl(url) : newdoc->importDocument(url);
    if (!openRet) {
        newpart->removeShell(this);
        delete newdoc;
        delete newpart;
        return false;
    }
    updateReloadFileAction(newdoc);
    updateVersionsFileAction(newdoc);

    KFileItem file(url, newdoc->mimeType(), KFileItem::Unknown);
    if (!file.isWritable())
        newdoc->setReadWrite(false);
    return true;
}

// Separate from openDocument to handle async loading (remote URLs)
void KoMainWindow::slotLoadCompleted()
{
    kDebug(30003) << "KoMainWindow::slotLoadCompleted";
    KoPart *newpart = qobject_cast<KoPart*>(sender());
    KoDocument *newdoc = newpart->document();

    if (d->rootDocument && d->rootDocument->isEmpty()) {
        // Replace current empty document
        setRootDocument(newdoc);
    } else if (d->rootDocument && !d->rootDocument->isEmpty()) {
        // Open in a new shell
        // (Note : could create the shell first and the doc next for this
        // particular case, that would give a better user feedback...)
        KoMainWindow *s = new KoMainWindow(newpart->componentData());
        s->show();
        newpart->removeShell(this);
        s->setRootDocument(newdoc, newpart);
    } else {
        // We had no document, set the new one
        setRootDocument(newdoc);
    }
    disconnect(newdoc, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));
    disconnect(newpart, SIGNAL(completed()), this, SLOT(slotLoadCompleted()));
    disconnect(newpart, SIGNAL(canceled(const QString &)), this, SLOT(slotLoadCanceled(const QString &)));
}

void KoMainWindow::slotLoadCanceled(const QString & errMsg)
{
    kDebug(30003) << "KoMainWindow::slotLoadCanceled";
    if (!errMsg.isEmpty())   // empty when canceled by user
        KMessageBox::error(this, errMsg);
    // ... can't delete the document, it's the one who emitted the signal...

    KoPart* newpart = qobject_cast<KoPart*>(sender());
    Q_ASSERT(newpart);
    disconnect(newpart->document(), SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));
    disconnect(newpart, SIGNAL(completed()), this, SLOT(slotLoadCompleted()));
    disconnect(newpart, SIGNAL(canceled(const QString &)), this, SLOT(slotLoadCanceled(const QString &)));
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
    KoDocument* pDoc = (KoDocument *)(sender());
    disconnect(pDoc, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));
    disconnect(pDoc, SIGNAL(completed()), this, SLOT(slotSaveCompleted()));
    disconnect(pDoc, SIGNAL(canceled(const QString &)),
               this, SLOT(slotSaveCanceled(const QString &)));

    if (d->deferredClosingEvent) {
        KParts::MainWindow::closeEvent(d->deferredClosingEvent);
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

bool KoMainWindow::saveDocument(bool saveas, bool silent)
{
    if (!d->rootDocument || !d->rootPart) {
        return true;
    }

    bool reset_url;

    if (d->rootPart->url().isEmpty()) {
        emit saveDialogShown();
        reset_url = true;
        saveas = true;
    } else {
        reset_url = false;
    }

    connect(d->rootDocument, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));
    connect(d->rootPart, SIGNAL(completed()), this, SLOT(slotSaveCompleted()));
    connect(d->rootPart, SIGNAL(canceled(const QString &)), this, SLOT(slotSaveCanceled(const QString &)));

    KUrl oldURL = d->rootPart->url();
    QString oldFile = d->rootPart->localFilePath();

    QByteArray _native_format = d->rootDocument->nativeFormatMimeType();
    QByteArray oldOutputFormat = d->rootDocument->outputMimeType();

    int oldSpecialOutputFlag = d->rootDocument->specialOutputFlag();

    KUrl suggestedURL = d->rootPart->url();

    QStringList mimeFilter = KoFilterManager::mimeFilter(_native_format,
            KoFilterManager::Export, d->rootDocument->extraNativeMimeTypes(KoDocument::ForExport));

    if (!mimeFilter.contains(oldOutputFormat) && !isExporting()) {
        kDebug(30003) << "KoMainWindow::saveDocument no export filter for" << oldOutputFormat;

        // --- don't setOutputMimeType in case the user cancels the Save As
        // dialog and then tries to just plain Save ---

        // suggest a different filename extension (yes, we fortunately don't all live in a world of magic :))
        QString suggestedFilename = suggestedURL.fileName();
        if (!suggestedFilename.isEmpty()) {  // ".kra" looks strange for a name
            int c = suggestedFilename.lastIndexOf('.');

            KMimeType::Ptr mime = KMimeType::mimeType(_native_format);
            if (! mime)
                mime = KMimeType::defaultMimeTypePtr();
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

    if (d->rootPart->url().isEmpty() || saveas) {
        // if you're just File/Save As'ing to change filter options you
        // don't want to be reminded about overwriting files etc.
        bool justChangingFilterOptions = false;

        KoFileDialog *dialog = new KoFileDialog(
            (isExporting() && !d->lastExportUrl.isEmpty()) ?
            d->lastExportUrl.url() : suggestedURL.url(), this);

        if (!isExporting())
            dialog->setCaption(i18n("Save Document As"));
        else
            dialog->setCaption(i18n("Export Document As"));

        dialog->setOperationMode(KFileDialog::Saving);
        dialog->setMode(KFile::File);
        dialog->setSpecialMimeFilter(mimeFilter,
                                     isExporting() ? d->lastExportedFormat : d->rootDocument->mimeType(),
                                     isExporting() ? d->lastExportSpecialOutputFlag : oldSpecialOutputFlag,
                                     _native_format,
                                     d->rootDocument->supportedSpecialFormats());

        KUrl newURL;
        QByteArray outputFormat = _native_format;
        int specialOutputFlag = 0;
        bool bOk;
        do {
            bOk = true;
            if (dialog->exec() == QDialog::Accepted) {
                newURL = dialog->selectedUrl();
                QString outputFormatString = dialog->currentMimeFilter().toLatin1();
                if (outputFormatString.isNull()) {
                    KMimeType::Ptr mime = KMimeType::findByUrl(newURL);
                    outputFormatString = mime->name();
                }
                outputFormat = outputFormatString.toLatin1();

                specialOutputFlag = dialog->specialEntrySelected();
                kDebug(30003) << "KoMainWindow::saveDocument outputFormat =" << outputFormat;

                if (!isExporting())
                    justChangingFilterOptions = (newURL == d->rootPart->url()) &&
                                                (outputFormat == d->rootDocument->mimeType()) &&
                                                (specialOutputFlag == oldSpecialOutputFlag);
                else
                    justChangingFilterOptions = (newURL == d->lastExportUrl) &&
                                                (outputFormat == d->lastExportedFormat) &&
                                                (specialOutputFlag == d->lastExportSpecialOutputFlag);
            } else {
                bOk = false;
                break;
            }

            if (newURL.isEmpty()) {
                bOk = false;
                break;
            }

            // adjust URL before doing checks on whether the file exists.
            if (specialOutputFlag == KoDocument::SaveAsDirectoryStore) {
                QString fileName = newURL.fileName();
                if (fileName != "content.xml") {
                    newURL.addPath("content.xml");
                }
            }

            // this file exists and we are not just clicking "Save As" to change filter options
            // => ask for confirmation
            if (KIO::NetAccess::exists(newURL,  KIO::NetAccess::DestinationSide, this) && !justChangingFilterOptions) {
                bOk = KMessageBox::questionYesNo(this,
                                                 i18n("A document with this name already exists.\n"\
                                                      "Do you want to overwrite it?"),
                                                 i18n("Warning")) == KMessageBox::Yes;
            }
        } while (!bOk);

        delete dialog;

        if (bOk) {
            bool wantToSave = true;

            // don't change this line unless you know what you're doing :)
            if (!justChangingFilterOptions || d->rootDocument->confirmNonNativeSave(isExporting())) {
                if (!d->rootDocument->isNativeFormat(outputFormat, KoDocument::ForExport))
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
                    ret = d->rootPart->saveAs(newURL);

                    if (ret) {
                        kDebug(30003) << "Successful Save As!";
                        addRecentURL(newURL);
                        setReadWrite(true);
                    } else {
                        kDebug(30003) << "Failed Save As!";
                        d->rootDocument->setUrl(oldURL);
                        d->rootPart->setLocalFilePath(oldFile);
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
        bool needConfirm = d->rootDocument->confirmNonNativeSave(false) &&
                           !d->rootDocument->isNativeFormat(oldOutputFormat, KoDocument::ForExport);
        if (!needConfirm ||
                (needConfirm && exportConfirmation(oldOutputFormat /* not so old :) */))
           ) {
            // be sure d->rootDocument has the correct outputMimeType!
            ret = d->rootPart->save();

            if (!ret) {
                kDebug(30003) << "Failed Save!";
                d->rootPart->setUrl(oldURL);
                d->rootPart->setLocalFilePath(oldFile);
            }
        } else
            ret = false;
    }

// Now that there's a File/Export option, this is no longer necessary.
// If you continue to use File/Save to export to a foreign format,
// this signals your intention to continue working in a foreign format.
// You have already been warned by the DoNotAskAgain exportConfirmation
// about losing formatting when you first saved so don't set modified
// here or else it will be reported as a bug by some MSOffice user.
// You have been warned!  Do not click DoNotAskAgain!!!
#if 0
    if (ret && !isExporting()) {
        // When exporting to a non-native format, we don't reset modified.
        // This way the user will be reminded to save it again in the native format,
        // if he/she doesn't want to lose formatting.
        if (wasModified && d->rootDocument->outputMimeType() != _native_format)
            d->rootDocument->setModified(true);
    }
#endif

    if (!ret && reset_url)
        d->rootDocument->resetURL(); //clean the suggested filename as the save dialog was rejected

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
    KParts::MainWindow::resizeEvent(e);
}

bool KoMainWindow::queryClose()
{
    if (rootDocument() == 0)
        return true;
    //kDebug(30003) <<"KoMainWindow::queryClose() viewcount=" << rootDocument()->viewCount()
    //               << " shellcount=" << rootDocument()->shellCount() << endl;
    if (!d->forQuit && d->rootPart->shellCount() > 1)
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
        KoMainWindow *s = new KoMainWindow(newpart->componentData());
        s->show();
        newpart->addShell(s);
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

    newpart->addShell(this);
    newpart->showStartUpWidget(this, true /*Always show widget*/);
}

void KoMainWindow::slotFileNew()
{
    chooseNewDocument(InitDocFileNew);
}

void KoMainWindow::slotFileOpen()
{
#ifdef Q_WS_WIN
    // "kfiledialog:///OpenDialog" forces KDE style open dialog in Windows
	// TODO provide support for "last visited" directory
    KFileDialog *dialog = new KFileDialog(KUrl(""), QString(), this);
#else
    KFileDialog *dialog = new KFileDialog(KUrl("kfiledialog:///OpenDialog"), QString(), this);
#endif
    dialog->setObjectName("file dialog");
    dialog->setMode(KFile::File);
    if (!isImporting())
        dialog->setCaption(i18n("Open Document"));
    else
        dialog->setCaption(i18n("Import Document"));

    const QStringList mimeFilter = KoFilterManager::mimeFilter(KoServiceProvider::readNativeFormatMimeType(),
                                   KoFilterManager::Import,
                                   KoServiceProvider::readExtraNativeMimeTypes());
    dialog->setMimeFilter(mimeFilter);
    if (dialog->exec() != QDialog::Accepted) {
        delete dialog;
        return;
    }
    KUrl url(dialog->selectedUrl());
    delete dialog;

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

void KoMainWindow::slotDocumentInfo()
{
    if (!rootDocument())
        return;

    KoDocumentInfo *docInfo = rootDocument()->documentInfo();

    if (!docInfo)
        return;

    KoDocumentInfoDlg *dlg = new KoDocumentInfoDlg(this, docInfo, rootDocument()->documentRdf());
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
        setRootDocument(0);   // don't delete this shell when deleting the document
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

class ExportPdfDialog : public KPageDialog
{
public:
    ExportPdfDialog(const KUrl &startUrl, const KoPageLayout &pageLayout) : KPageDialog() {
        setFaceType(KPageDialog::List);
        setCaption(i18n("Export to PDF"));

        m_fileWidget = new KFileWidget(startUrl, this);
        m_fileWidget->setOperationMode(KFileWidget::Saving);
        m_fileWidget->setMode(KFile::File);
        m_fileWidget->setMimeFilter(QStringList() << "application/pdf");
        connect(m_fileWidget, SIGNAL(accepted()), this, SLOT(accept()));

        KPageWidgetItem *fileItem = new KPageWidgetItem(m_fileWidget, i18n( "File" ));
        fileItem->setIcon(koIcon("document-open"));
        addPage(fileItem);

        m_pageLayoutWidget = new KoPageLayoutWidget(this, pageLayout);
        m_pageLayoutWidget->showUnitchooser(false);
        KPageWidgetItem *optionsItem = new KPageWidgetItem(m_pageLayoutWidget, i18n("Configure"));
        optionsItem->setIcon(koIcon("configure"));
        addPage(optionsItem);

        resize(QSize(800, 600).expandedTo(minimumSizeHint()));
    }
    KUrl selectedUrl() const {
        // selectedUrl()( does not return the expected result. So, build up the KUrl the more complicated way
        //return m_fileWidget->selectedUrl();

        KUrl url = m_fileWidget->dirOperator()->url();
        url.adjustPath(KUrl::AddTrailingSlash);
        url.setFileName(m_fileWidget->locationEdit()->currentText());
        return url;
    }
    KoPageLayout pageLayout() const {
        return m_pageLayoutWidget->pageLayout();
    }
protected:
    virtual void slotButtonClicked(int button) {
        if (button == KDialog::Ok) {
            m_fileWidget->slotOk();
        } else {
            KPageDialog::slotButtonClicked(button);
        }
    }
private:
    KFileWidget *m_fileWidget;
    KoPageLayoutWidget *m_pageLayoutWidget;
};

KoPrintJob* KoMainWindow::exportToPdf(QString pdfFileName)
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
        KUrl startUrl = KUrl("kfiledialog:///SavePdfDialog");
        KoDocument* pDoc = rootDocument();
        /** if document has a file name, take file name and replace extension with .pdf */
        if (pDoc && pDoc->url().isValid()) {
            startUrl = pDoc->url();
            QString fileName = startUrl.fileName();
            fileName = fileName.replace( QRegExp( "\\.\\w{2,5}$", Qt::CaseInsensitive ), ".pdf" );
            startUrl.setFileName( fileName );
        }

        QPointer<ExportPdfDialog> dialog(new ExportPdfDialog(startUrl, pageLayout));
        if (dialog->exec() != QDialog::Accepted || !dialog) {
            delete dialog;
            return 0;
        }

        KUrl url = dialog->selectedUrl();
        pageLayout = dialog->pageLayout();
        delete dialog;

        if (KIO::NetAccess::exists(url,  KIO::NetAccess::DestinationSide, this)) {
            bool overwrite = KMessageBox::questionYesNo(this,
                                            i18n("A document with this name already exists.\n"\
                                                "Do you want to overwrite it?"),
                                            i18n("Warning")) == KMessageBox::Yes;
            if (!overwrite) {
                return 0;
            }
        }
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
    kDebug(30003) << "KoMainWindow::slotProgress" << value;
    if (value <= -1) {
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
    d->progress->setValue(value);
    qApp->processEvents();
}


void KoMainWindow::slotActivePartChanged(KParts::Part *newPart)
{

    // This looks very much like KParts::MainWindow::createGUI, but we have
    // to reimplement it because it works with an active part, whereas we work
    // with an active view _and_ an active part, depending for what.
    // Both are KXMLGUIClients, but e.g. the plugin query needs a QObject.
    //kDebug(30003) <<"KoMainWindow::slotActivePartChanged( Part * newPart) newPart =" << newPart;
    //kDebug(30003) <<"current active part is" << d->activePart;

    if (d->activePart && d->activePart == newPart) {
        //kDebug(30003) <<"no need to change the GUI";
        return;
    }


    KXMLGUIFactory *factory = guiFactory();

// ###  setUpdatesEnabled( false );

    if (d->activeView) {
        KParts::GUIActivateEvent ev(false);
        QApplication::sendEvent(d->activePart, &ev);
        QApplication::sendEvent(d->activeView, &ev);


        factory->removeClient(d->activeView);

        unplugActionList("toolbarlist");
        qDeleteAll(d->toolbarList);
        d->toolbarList.clear();
    }

    if (!d->mainWindowGuiIsBuilt) {
        // Load mainwindow plugins
        KParts::Plugin::loadPlugins(this, this, componentData(), true);
        createShellGUI();
    }

    if (newPart && d->manager->activeWidget() && d->manager->activeWidget()->inherits("KoView")) {
        d->activeView = (KoView *)d->manager->activeWidget();
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
        //QPtrListIterator<KToolBar> it = toolBarIterator();
        foreach(QWidget* it, factory->containers("ToolBar")) {
            KToolBar * tb = ::qobject_cast<KToolBar *>(it);
            if (tb) {
                KToggleAction * act = new KToggleAction(i18n("Show %1 Toolbar", tb->windowTitle()), this);
                actionCollection()->addAction(tb->objectName().toUtf8(), act);
                act->setCheckedState(KGuiItem(i18n("Hide %1 Toolbar", tb->windowTitle())));
                connect(act, SIGNAL(toggled(bool)), this, SLOT(slotToolbarToggled(bool)));
                act->setChecked(!tb->isHidden());
                d->toolbarList.append(act);
            } else
                kWarning(30003) << "Toolbar list contains a " << it->metaObject()->className() << " which is not a toolbar!";
        }
        plugActionList("toolbarlist", d->toolbarList);

        // Send the GUIActivateEvent only now, since it might show/hide toolbars too
        // (and this has priority over applyMainWindowSettings)
        KParts::GUIActivateEvent ev(true);
        QApplication::sendEvent(d->activePart, &ev);
        QApplication::sendEvent(d->activeView, &ev);
    } else {
        d->activeView = 0;
        d->activePart = 0;
    }
// ###  setUpdatesEnabled( true );
}

QLabel * KoMainWindow::statusBarLabel()
{
    if (!d->statusBarLabel) {
        d->statusBarLabel = new QLabel(statusBar());
        statusBar()->addPermanentWidget(d->statusBarLabel, 1);
    }
    return d->statusBarLabel;
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
        setRootDocument(0);   // don't delete this shell when deleting the document
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

void KoMainWindow::setDocToOpen(KoPart *part)
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
        if (rootDocument()) {
            KConfigGroup group = KGlobal::config()->group(d->rootPart->componentData().componentName()).group("DockWidget " + factory->id());
            collapsed = group.readEntry("Collapsed", collapsed);
        }
        if (titleBar && collapsed)
            titleBar->setCollapsed(true);
        d->dockWidgetsMap.insert(factory->id(), dockWidget);
    } else {
        dockWidget = d->dockWidgetsMap[ factory->id()];
    }

    KConfigGroup group(KGlobal::config(), "GUI");
    QFont dockWidgetFont  = KGlobalSettings::generalFont();
    qreal pointSize = group.readEntry("palettefontsize", dockWidgetFont.pointSize() * 0.75);
    pointSize = qMax(pointSize, KGlobalSettings::smallestReadableFont().pointSizeF());
    dockWidgetFont.setPointSizeF(pointSize);
#ifdef Q_WS_MAC
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

#include <KoMainWindow.moc>
