/* This file is part of the KDE project
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 * Copyright (C) 2000-2005 David Faure <faure@kde.org>
 * Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2010 Boudewijn Rempt <boud@kogmbh.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoDocument.h"
#include "KoDocument_p.h"

#include "KoDocumentAdaptor.h"
#include "KoGlobal.h"
#include "KoView.h"
#include "KoMainWindow.h"
#include "KoFilterManager.h"
#include "KoDocumentInfo.h"
#include "KoCanvasController.h"
#include "KoCanvasControllerWidget.h"
#include "rdf/KoDocumentRdfBase.h"
#ifdef SHOULD_BUILD_RDF
#include "rdf/KoDocumentRdf.h"
#endif
#include "KoOdfStylesReader.h"
#include "KoOdfReadStore.h"
#include "KoOdfWriteStore.h"
#include "KoEmbeddedDocumentSaver.h"
#include "KoXmlNS.h"
#include "KoOpenPane.h"
#include "KoApplication.h"
#include <KoProgressProxy.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>

#include <KoDpi.h>
#include <KoXmlWriter.h>

#include <kdialog.h>
#include <KUndoStack>
#include <kfileitem.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kparts/partmanager.h>
#include <ksavefile.h>
#include <kxmlguifactory.h>
#include <KIconLoader>
#include <kdebug.h>
#include <kdeprintdialog.h>
#include <knotification.h>

#include <QtCore/QBuffer>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtGui/QPainter>
#include <QtCore/QTimer>
#include <QtDBus/QDBusConnection>
#include <QtGui/QLayout>
#include <QtGui/QApplication>
#include <QtGui/QPrinter>
#include <QtGui/QPrintDialog>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>

// Define the protocol used here for embedded documents' URL
// This used to "store" but KUrl didn't like it,
// so let's simply make it "tar" !
#define STORE_PROTOCOL "tar"
// The internal path is a hack to make KUrl happy and for document children
#define INTERNAL_PROTOCOL "intern"
#define INTERNAL_PREFIX "intern:/"
// Warning, keep it sync in koStore.cc

QList<KoDocument*> *KoDocument::s_documentList = 0;

using namespace std;
class KoViewWrapperWidget;

/**********************************************************
 *
 * KoDocument
 *
 **********************************************************/

//static
QString KoDocument::newObjectName()
{
    static int s_docIFNumber = 0;
    QString name; name.setNum(s_docIFNumber++); name.prepend("document_");
    return name;
}

class KoDocument::Private
{
public:
    Private() :
            progressUpdater(0),
            profileStream(0),
            filterManager(0),
            specialOutputFlag(0),   // default is native format
            isImporting(false),
            isExporting(false),
            password(QString()),
            modifiedAfterAutosave(false),
            autosaving(false),
            shouldCheckAutoSaveFile(true),
            autoErrorHandlingEnabled(true),
            backupFile(true),
            backupPath(QString()),
            doNotSaveExtDoc(false),
            current(false),
            storeInternal(false),
            bLoading(false),
            startUpWidget(0),
            undoStack(0),
            canvasItem(0)

    {
        confirmNonNativeSave[0] = true;
        confirmNonNativeSave[1] = true;
        if (KGlobal::locale()->measureSystem() == KLocale::Imperial) {
            unit = KoUnit::Inch;
        } else {
            unit = KoUnit::Centimeter;
        }
    }

    /*
    Boud: I think it's a design mistake that the model owns the view(s); actually,
    I think the model shouldn't even know about the view, but just emit signals and
    expose slots. The application (as controller) should own the list of views/mainwindows
    and the list of docs and connect them together. That way, we can have KoDocument-based
    documents that do not depend on anything gui-related.
    */

    QList<KoView*> views;
    QList<KoMainWindow*> shells;

    KoViewWrapperWidget *wrapperWidget;
    KoDocumentInfo *docInfo;
#ifdef SHOULD_BUILD_RDF
    KoDocumentRdf *docRdf;
#else
    KoDocumentRdfBase *docRdf;
#endif
    KoProgressUpdater *progressUpdater;
    QTextStream *profileStream;
    QTime profileReferenceTime;

    KoUnit unit;

    KoFilterManager *filterManager; // The filter-manager to use when loading/saving [for the options]

    QByteArray mimeType; // The actual mimetype of the document
    QByteArray outputMimeType; // The mimetype to use when saving
    bool confirmNonNativeSave [2]; // used to pop up a dialog when saving for the
    // first time if the file is in a foreign format
    // (Save/Save As, Export)
    int specialOutputFlag; // See KoFileDialog in koMainWindow.cc
    bool isImporting;
    bool isExporting; // File --> Import/Export vs File --> Open/Save
    QString password; // The password used to encrypt an encrypted document

    QTimer autoSaveTimer;
    QString lastErrorMessage; // see openFile()
    int autoSaveDelay; // in seconds, 0 to disable.
    bool modifiedAfterAutosave;
    bool bSingleViewMode;
    bool autosaving;
    bool shouldCheckAutoSaveFile; // usually true
    bool autoErrorHandlingEnabled; // usually true
    bool backupFile;
    QString backupPath;
    bool doNotSaveExtDoc; // makes it possible to save only internally stored child documents
    bool current;
    bool storeInternal; // Store this doc internally even if url is external
    bool bLoading; // True while loading (openUrl is async)

    QPointer<KoOpenPane> startUpWidget;
    QString templateType;
    QList<KoVersionInfo> versionInfo;

    KUndoStack *undoStack;

    KoGridData gridData;
    KoGuidesData guidesData;

    KService::Ptr nativeService;

    bool bEmpty;

    KoPageLayout pageLayout;

    QGraphicsItem *canvasItem;
};

// Used in singleViewMode
class KoViewWrapperWidget : public QWidget
{
public:
    KoViewWrapperWidget(QWidget *parent)
            : QWidget(parent) {
        KGlobal::locale()->insertCatalog("koffice");
        // Tell the iconloader about share/apps/koffice/icons
        KIconLoader::global()->addAppDir("koffice");
        m_view = 0;
        // Avoid warning from KParts - we'll have the KoView as focus proxy anyway
        setFocusPolicy(Qt::ClickFocus);
    }

    virtual ~KoViewWrapperWidget() {
        setFocusProxy(0);   // to prevent a crash due to clearFocus (#53466)
    }

    virtual void resizeEvent(QResizeEvent *) {
        QWidget *wid = findChild<QWidget *>();
        if (wid)
            wid->setGeometry(0, 0, width(), height());
    }

    virtual void childEvent(QChildEvent *ev) {
        if (ev->type() == QEvent::ChildAdded)
            resizeEvent(0);
    }

    // Called by openFile()
    void setKoView(KoView *view) {
        m_view = view;
        setFocusProxy(m_view);
    }
    KoView *koView() const {
        return m_view;
    }
private:
    KoView *m_view;
};

KoBrowserExtension::KoBrowserExtension(KoDocument *doc)
        : KParts::BrowserExtension(doc)
{
    emit enableAction("print", true);
}

void KoBrowserExtension::print()
{
    kDebug(30003) << "implement; KoBrowserExtension::print";
    /*
        KoDocument *doc = static_cast<KoDocument *>( parent() );
        KoViewWrapperWidget *wrapper = static_cast<KoViewWrapperWidget *>( doc->widget() );
        KoView *view = wrapper->koView();
        // TODO remove code duplication (KoMainWindow), by moving this to KoView
        QPrinter printer;
        QPrintDialog *printDialog = KdePrint::createPrintDialog(&printer, view->printDialogPages(), view);
        // ### TODO: apply global koffice settings here
        view->setupPrinter( printer, *printDialog );
        if ( printDialog->exec() )
            view->print( printer, *printDialog );
    */
}

namespace {
    KoMainWindow *currentShell(KoDocument *doc)
    {
        QWidget *widget = qApp->activeWindow();
        KoMainWindow *shell = qobject_cast<KoMainWindow*>(widget);
        while (!shell && widget) {
            widget = widget->parentWidget();
            shell = qobject_cast<KoMainWindow*>(widget);
        }

        if (!shell)
            shell = doc->shells().first();
        return shell;
    }
    class DocumentProgressProxy : public KoProgressProxy {
    public:
        KoDocument *const m_doc;

        DocumentProgressProxy(KoDocument *doc)
            : m_doc(doc)
        {
        }

        ~DocumentProgressProxy() {
            // signal that the job is done
            setValue(-1);
        }

        int maximum() const {
            return 100;
        }

        void setValue(int value) {
            currentShell(m_doc)->slotProgress(value);
        }

        void setRange(int /*minimum*/, int /*maximum*/) {

        }

        void setFormat(const QString &/*format*/) {

        }
    };
}

KoDocument::KoDocument(QWidget *parentWidget, QObject *parent, bool singleViewMode)
        : KParts::ReadWritePart(parent)
        , d(new Private)
{
    if (s_documentList == 0)
        s_documentList = new QList<KoDocument*>;
    s_documentList->append(this);

    d->bEmpty = true;
    connect(&d->autoSaveTimer, SIGNAL(timeout()), this, SLOT(slotAutoSave()));
    setAutoSave(defaultAutoSave());
    d->bSingleViewMode = singleViewMode;

    setObjectName(newObjectName());
    new KoDocumentAdaptor(this);
    QDBusConnection::sessionBus().registerObject('/' + objectName(), this);


    // the parent setting *always* overrides! (Simon)
    if (parent) {
        if (::qobject_cast<KoDocument *>(parent))
            d->bSingleViewMode = ((KoDocument *)parent)->isSingleViewMode();
        else if (::qobject_cast<KParts::Part*>(parent))
            d->bSingleViewMode = true;
    }

    if (singleViewMode) {
        d->wrapperWidget = new KoViewWrapperWidget(parentWidget);
        setWidget(d->wrapperWidget);
        kDebug(30003) << "creating KoBrowserExtension";
        (void) new KoBrowserExtension(this);   // ## only if embedded into a browser?
    }

    d->docInfo = new KoDocumentInfo(this);
    d->docRdf = 0;
#ifdef SHOULD_BUILD_RDF
    d->docRdf  = new KoDocumentRdf(this);
#endif

    d->pageLayout.width = 0;
    d->pageLayout.height = 0;
    d->pageLayout.topMargin = 0;
    d->pageLayout.bottomMargin = 0;
    d->pageLayout.leftMargin = 0;
    d->pageLayout.rightMargin = 0;

    d->undoStack = new KUndoStack(this);
    d->undoStack->createUndoAction(actionCollection());
    d->undoStack->createRedoAction(actionCollection());

    KConfigGroup cfgGrp(componentData().config(), "Undo");
    d->undoStack->setUndoLimit(cfgGrp.readEntry("UndoLimit", 1000));

    connect(d->undoStack, SIGNAL(cleanChanged(bool)), this, SLOT(setDocumentClean(bool)));

    // A way to 'fix' the job's window, since we have no widget known to KParts
    if (!singleViewMode)
        connect(this, SIGNAL(started(KIO::Job*)), SLOT(slotStarted(KIO::Job*)));
}

KoDocument::~KoDocument()
{
    d->autoSaveTimer.stop();

    // Tell our views that the document is already destroyed and
    // that they shouldn't try to access it.
    foreach(KoView *view, d->views) {
        view->setDocumentDeleted();
    }
    delete d->startUpWidget;
    d->startUpWidget = 0;

    while(!d->shells.isEmpty()) {
        delete d->shells.takeFirst();
    }

    delete d->filterManager;

    delete d;
    s_documentList->removeOne(this);
    // last one?
    if (s_documentList->isEmpty()) {
        delete s_documentList;
        s_documentList = 0;
    }
}

bool KoDocument::isSingleViewMode() const
{
    return d->bSingleViewMode;
}

bool KoDocument::isEmbedded() const
{
    return dynamic_cast<KoDocument *>(parent()) != 0;
}

KoView *KoDocument::createView(QWidget *parent)
{
    KoView *view = createViewInstance(parent);
    addView(view);
    return view;
}

bool KoDocument::exportDocument(const KUrl & _url)
{
    bool ret;

    d->isExporting = true;

    //
    // Preserve a lot of state here because we need to restore it in order to
    // be able to fake a File --> Export.  Can't do this in saveFile() because,
    // for a start, KParts has already set url and m_file and because we need
    // to restore the modified flag etc. and don't want to put a load on anyone
    // reimplementing saveFile() (Note: importDocument() and exportDocument()
    // will remain non-virtual).
    //
    KUrl oldURL = url();
    QString oldFile = localFilePath();

    bool wasModified = isModified();
    QByteArray oldMimeType = mimeType();


    // save...
    ret = saveAs(_url);


    //
    // This is sooooo hacky :(
    // Hopefully we will restore enough state.
    //
    kDebug(30003) << "Restoring KoDocument state to before export";

    // always restore url & m_file because KParts has changed them
    // (regardless of failure or success)
    setUrl(oldURL);
    setLocalFilePath(oldFile);

    // on successful export we need to restore modified etc. too
    // on failed export, mimetype/modified hasn't changed anyway
    if (ret) {
        setModified(wasModified);
        d->mimeType = oldMimeType;
    }


    d->isExporting = false;

    return ret;
}

bool KoDocument::saveFile()
{
    kDebug(30003) << "doc=" << url().url();

    // Save it to be able to restore it after a failed save
    const bool wasModified = isModified();

    // The output format is set by koMainWindow, and by openFile
    QByteArray outputMimeType = d->outputMimeType;
    //Q_ASSERT( !outputMimeType.isEmpty() ); // happens when using the DCOP method saveAs
    if (outputMimeType.isEmpty())
        outputMimeType = d->outputMimeType = nativeFormatMimeType();

    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (backupFile()) {
        if (url().isLocalFile())
            KSaveFile::backupFile(url().toLocalFile(), d->backupPath);
        else {
            KIO::UDSEntry entry;
            if (KIO::NetAccess::stat(url(), entry, currentShell())) {     // this file exists => backup
                emit statusBarMessage(i18n("Making backup..."));
                KUrl backup;
                if (d->backupPath.isEmpty())
                    backup = url();
                else
                    backup = d->backupPath + '/' + url().fileName();
                backup.setPath(backup.path() + QString::fromLatin1("~"));
                KFileItem item(entry, url());
                Q_ASSERT(item.name() == url().fileName());
                KIO::FileCopyJob *job = KIO::file_copy(url(), backup, item.permissions(), KIO::Overwrite | KIO::HideProgressInfo);
                job->exec();
            }
        }
    }

    emit statusBarMessage(i18n("Saving..."));
    bool ret = false;
    bool suppressErrorDialog = false;
    if (!isNativeFormat(outputMimeType, ForExport)) {
        kDebug(30003) << "Saving to format" << outputMimeType << "in" << localFilePath();
        // Not native format : save using export filter
        if (!d->filterManager)
            d->filterManager = new KoFilterManager(this);

        KoFilter::ConversionStatus status = d->filterManager->exportDocument(localFilePath(), outputMimeType);
        ret = status == KoFilter::OK;
        suppressErrorDialog = (status == KoFilter::UserCancelled || status == KoFilter::BadConversionGraph);
    } else {
        // Native format => normal save
        Q_ASSERT(!localFilePath().isEmpty());
        ret = saveNativeFormat(localFilePath());
    }

    if (ret) {
        removeAutoSaveFiles();
        // Restart the autosave timer
        // (we don't want to autosave again 2 seconds after a real save)
        setAutoSave(d->autoSaveDelay);
    }

    QApplication::restoreOverrideCursor();
    if (!ret) {
        if (!suppressErrorDialog) {
            showSavingErrorDialog();
        }

        // couldn't save file so this new URL is invalid
        // FIXME: we should restore the current document's true URL instead of
        // setting it to nothing otherwise anything that depends on the URL
        // being correct will not work (i.e. the document will be called
        // "Untitled" which may not be true)
        //
        // Update: now the URL is restored in KoMainWindow but really, this
        // should still be fixed in KoDocument/KParts (ditto for file).
        // We still resetURL() here since we may or may not have been called
        // by KoMainWindow - Clarence
        resetURL();

        // As we did not save, restore the "was modified" status
        setModified(wasModified);
    }

    if (ret) {
        d->mimeType = outputMimeType;
        setConfirmNonNativeSave(isExporting(), false);
    }
    emit clearStatusBarMessage();

    if (ret) {
        KNotification *notify = new KNotification("DocumentSaved");
        notify->setText(i18n("Document <i>%1</i> saved", url().url()));
        notify->addContext("url", url().url());
        QTimer::singleShot(0, notify, SLOT(sendEvent()));
    }

    return ret;
}

QByteArray KoDocument::mimeType() const
{
    return d->mimeType;
}

void KoDocument::setMimeType(const QByteArray & mimeType)
{
    d->mimeType = mimeType;
}

void KoDocument::setOutputMimeType(const QByteArray & mimeType, int specialOutputFlag)
{
    d->outputMimeType = mimeType;
    d->specialOutputFlag = specialOutputFlag;
}

QByteArray KoDocument::outputMimeType() const
{
    return d->outputMimeType;
}

int KoDocument::specialOutputFlag() const
{
    return d->specialOutputFlag;
}

bool KoDocument::confirmNonNativeSave(const bool exporting) const
{
    // "exporting ? 1 : 0" is different from "exporting" because a bool is
    // usually implemented like an "int", not "unsigned : 1"
    return d->confirmNonNativeSave [ exporting ? 1 : 0 ];
}

void KoDocument::setConfirmNonNativeSave(const bool exporting, const bool on)
{
    d->confirmNonNativeSave [ exporting ? 1 : 0] = on;
}

bool KoDocument::wantExportConfirmation() const
{
    return true;
}

bool KoDocument::isImporting() const
{
    return d->isImporting;
}

bool KoDocument::isExporting() const
{
    return d->isExporting;
}

void KoDocument::setCheckAutoSaveFile(bool b)
{
    d->shouldCheckAutoSaveFile = b;
}

void KoDocument::setAutoErrorHandlingEnabled(bool b)
{
    d->autoErrorHandlingEnabled = b;
}

bool KoDocument::isAutoErrorHandlingEnabled() const
{
    return d->autoErrorHandlingEnabled;
}

void KoDocument::slotAutoSave()
{
    if (isModified() && d->modifiedAfterAutosave && !d->bLoading) {
        // Give a warning when trying to autosave an encrypted file when no password is known (should not happen)
        if (d->specialOutputFlag == SaveEncrypted && d->password.isNull()) {
            // That advice should also fix this error from occurring again
            emit statusBarMessage(i18n("The password of this encrypted document is not known. Autosave aborted! Please save your work manually."));
        } else {
            connect(this, SIGNAL(sigProgress(int)), currentShell(), SLOT(slotProgress(int)));
            emit statusBarMessage(i18n("Autosaving..."));
            d->autosaving = true;
            bool ret = saveNativeFormat(autoSaveFile(localFilePath()));
            setModified(true);
            if (ret) {
                d->modifiedAfterAutosave = false;
                d->autoSaveTimer.stop(); // until the next change
            }
            d->autosaving = false;
            emit clearStatusBarMessage();
            disconnect(this, SIGNAL(sigProgress(int)), currentShell(), SLOT(slotProgress(int)));
            if (!ret)
                emit statusBarMessage(i18n("Error during autosave! Partition full?"));
        }
    }
}

QAction *KoDocument::action(const QDomElement &element) const
{
    // First look in the document itself
    QAction *act = KParts::ReadWritePart::action(element);
    if (act)
        return act;

    Q_ASSERT(d->bSingleViewMode);
    // Then look in the first view (this is for the single view mode)
    if (!d->views.isEmpty())
        return d->views.first()->action(element);
    else
        return 0;
}

QDomDocument KoDocument::domDocument() const
{
    // When embedded into e.g. konqueror, we want the view's GUI (hopefully a reduced one)
    // to be used.
    Q_ASSERT(d->bSingleViewMode);
    if (d->views.isEmpty())
        return QDomDocument();
    else
        return d->views.first()->domDocument();
}

void KoDocument::setManager(KParts::PartManager *manager)
{
    KParts::ReadWritePart::setManager(manager);
    if (d->bSingleViewMode && d->views.count() == 1)
        d->views.first()->setPartManager(manager);
}

void KoDocument::setReadWrite(bool readwrite)
{
    KParts::ReadWritePart::setReadWrite(readwrite);

    foreach(KoView *view, d->views) {
        view->updateReadWrite(readwrite);
    }
    foreach(KoMainWindow *mainWindow, d->shells) {
        mainWindow->setReadWrite(readwrite);
    }
    setAutoSave(d->autoSaveDelay);
}

void KoDocument::setAutoSave(int delay)
{
    d->autoSaveDelay = delay;
    if (isReadWrite() && !isEmbedded() && d->autoSaveDelay > 0)
        d->autoSaveTimer.start(d->autoSaveDelay * 1000);
    else
        d->autoSaveTimer.stop();
}

void KoDocument::addView(KoView *view)
{
    if (!view)
        return;

    d->views.append(view);
    view->updateReadWrite(isReadWrite());

    if (d->views.size() == 1) {
        KoApplication *app = qobject_cast<KoApplication*>(KApplication::kApplication());
        if (0 != app) {
            emit app->documentOpened('/'+objectName());
        }
    }
}

void KoDocument::removeView(KoView *view)
{
    d->views.removeAll(view);

    if (d->views.isEmpty()) {
        KoApplication *app = qobject_cast<KoApplication*>(KApplication::kApplication());
        if (0 != app) {
            emit app->documentClosed('/'+objectName());
        }
    }
}

QList<KoView*> KoDocument::views() const
{
    return d->views;
}

int KoDocument::viewCount() const
{
    return d->views.count();
}

KParts::Part *KoDocument::hitTest(QWidget *widget, const QPoint &globalPos)
{
    foreach(KoView *view, d->views) {
        if (static_cast<QWidget *>(view) == widget) {
            QPoint canvasPos(view->canvas()->mapFromGlobal(globalPos));
            canvasPos.rx() += view->canvasXOffset();
            canvasPos.ry() += view->canvasYOffset();

            KParts::Part *part = view->hitTest(canvasPos);
            if (part)
                return part;
        }
    }

    return 0;
}

KoDocumentInfo *KoDocument::documentInfo() const
{
    return d->docInfo;
}

KoDocumentRdf *KoDocument::documentRdf() const
{
#ifdef SHOULD_BUILD_RDF
    return d->docRdf;
#endif
    return 0;
}

KoDocumentRdfBase *KoDocument::documentRdfBase() const
{
    return d->docRdf;
}


void KoDocument::paintEverything(QPainter &painter, const QRect &rect, KoView *view)
{
    Q_UNUSED(view);
    paintContent(painter, rect);
}

bool KoDocument::isModified() const
{
    if (KParts::ReadWritePart::isModified()) {
        //kDebug(30003)<<" Modified doc='"<<url().url()<<"' extern="<<isStoredExtern();
        return true;
    }
    return false;
}

bool KoDocument::saveNativeFormat(const QString & file)
{
    d->lastErrorMessage.clear();
    //kDebug(30003) <<"Saving to store";

    KoStore::Backend backend = KoStore::Auto;
#if 0
    if (d->specialOutputFlag == SaveAsKOffice1dot1) {
        kDebug(30003) << "Saving as KOffice-1.1 format, using a tar.gz";
        backend = KoStore::Tar; // KOffice-1.0/1.1 used tar.gz for the native mimetype
        //// TODO more backwards compat stuff (embedded docs etc.)
    } else
#endif
        if (d->specialOutputFlag == SaveAsDirectoryStore) {
            backend = KoStore::Directory;
            kDebug(30003) << "Saving as uncompressed XML, using directory store.";
        }
#ifdef QCA2
        else if (d->specialOutputFlag == SaveEncrypted) {
            backend = KoStore::Encrypted;
            kDebug(30003) << "Saving using encrypted backend.";
        }
#endif
        else if (d->specialOutputFlag == SaveAsFlatXML) {
            kDebug(30003) << "Saving as a flat XML file.";
            QFile f(file);
            if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
                bool success = saveToStream(&f);
                f.close();
                return success;
            } else
                return false;
        }

    kDebug(30003) << "KoDocument::saveNativeFormat nativeFormatMimeType=" << nativeFormatMimeType();
    // OLD: bool oasis = d->specialOutputFlag == SaveAsOASIS;
    // OLD: QCString mimeType = oasis ? nativeOasisMimeType() : nativeFormatMimeType();
    QByteArray mimeType = d->outputMimeType;
    QByteArray nativeOasisMime = nativeOasisMimeType();
    bool oasis = !mimeType.isEmpty() && (mimeType == nativeOasisMime || mimeType == nativeOasisMime + "-template");

    // TODO: use std::auto_ptr or create store on stack [needs API fixing],
    // to remove all the 'delete store' in all the branches
    KoStore *store = KoStore::createStore(file, KoStore::Write, mimeType, backend);
    if (d->specialOutputFlag == SaveEncrypted && !d->password.isNull())
        store->setPassword(d->password);
    if (store->bad()) {
        d->lastErrorMessage = i18n("Could not create the file for saving");   // more details needed?
        delete store;
        return false;
    }

    if (oasis) {
        return saveNativeFormatODF(store, mimeType);
    } else {
        return saveNativeFormatKOffice(store);
    }
}

bool KoDocument::saveNativeFormatODF(KoStore *store, const QByteArray &mimeType)
{
    kDebug(30003) << "Saving to OASIS format";
    // Tell KoStore not to touch the file names
    store->disallowNameExpansion();
    KoOdfWriteStore odfStore(store);
    KoXmlWriter *manifestWriter = odfStore.manifestWriter(mimeType);
    KoEmbeddedDocumentSaver embeddedSaver;
    SavingContext documentContext(odfStore, embeddedSaver);

    if (!saveOdf(documentContext)) {
        kDebug(30003) << "saveOdf failed";
        delete store;
        return false;
    }

    // Save embedded objects
    if (!embeddedSaver.saveEmbeddedDocuments(documentContext)) {
        kDebug(30003) << "save embedded documents failed";
        delete store;
        return false;
    }

    if (store->open("meta.xml")) {
        if (!d->docInfo->saveOasis(store) || !store->close()) {
            delete store;
            return false;
        }
        manifestWriter->addManifestEntry("meta.xml", "text/xml");
    } else {
        d->lastErrorMessage = i18n("Not able to write '%1'. Partition full?", QString("meta.xml"));
        delete store;
        return false;
    }

    if (d->docRdf && !d->docRdf->saveOasis(store, manifestWriter)) {
        d->lastErrorMessage = i18n("Not able to write RDF metadata. Partition full?");
        delete store;
        return false;
    }

    if (store->open("Thumbnails/thumbnail.png")) {
        if (!saveOasisPreview(store, manifestWriter) || !store->close()) {
            d->lastErrorMessage = i18n("Error while trying to write '%1'. Partition full?", QString("Thumbnails/thumbnail.png"));
            delete store;
            return false;
        }
        // No manifest entry!
    } else {
        d->lastErrorMessage = i18n("Not able to write '%1'. Partition full?", QString("Thumbnails/thumbnail.png"));
        delete store;
        return false;
    }

    if (!d->versionInfo.isEmpty()) {
        if (store->open("VersionList.xml")) {
            KoStoreDevice dev(store);
            KoXmlWriter *xmlWriter = KoOdfWriteStore::createOasisXmlWriter(&dev,
                                     "VL:version-list");
            for (int i = 0; i < d->versionInfo.size(); ++i) {
                KoVersionInfo *version = &d->versionInfo[i];
                xmlWriter->startElement("VL:version-entry");
                xmlWriter->addAttribute("VL:title", version->title);
                xmlWriter->addAttribute("VL:comment", version->comment);
                xmlWriter->addAttribute("VL:creator", version->saved_by);
                xmlWriter->addAttribute("dc:date-time", version->date.toString(Qt::ISODate));
                xmlWriter->endElement();
            }
            xmlWriter->endElement(); // root element
            xmlWriter->endDocument();
            delete xmlWriter;
            store->close();
            manifestWriter->addManifestEntry("VersionList.xml", "text/xml");

            for (int i = 0; i < d->versionInfo.size(); ++i) {
                KoVersionInfo *version = &d->versionInfo[i];
                store->addDataToFile(version->data, "Versions/" + version->title);
            }
        } else {
            d->lastErrorMessage = i18n("Not able to write '%1'. Partition full?", QString("VersionList.xml"));
            delete store;
            return false;
        }
    }

    // Write out manifest file
    if (!odfStore.closeManifestWriter()) {
        d->lastErrorMessage = i18n("Error while trying to write '%1'. Partition full?", QString("META-INF/manifest.xml"));
        delete store;
        return false;
    }

    // Remember the given password, if necessary
    if (store->isEncrypted() && !d->isExporting)
        d->password = store->password();

    delete store;

    return true;
}

bool KoDocument::saveNativeFormatKOffice(KoStore *store)
{
    kDebug(30003) << "Saving root";
    if (store->open("root")) {
        KoStoreDevice dev(store);
        if (!saveToStream(&dev) || !store->close()) {
            kDebug(30003) << "saveToStream failed";
            delete store;
            return false;
        }
    } else {
        d->lastErrorMessage = i18n("Not able to write '%1'. Partition full?", QString("maindoc.xml"));
        delete store;
        return false;
    }
    if (store->open("documentinfo.xml")) {
        QDomDocument doc = d->docInfo->save();
        KoStoreDevice dev(store);

        QByteArray s = doc.toByteArray(); // this is already Utf8!
        (void)dev.write(s.data(), s.size());
        (void)store->close();
    }

    if (store->open("preview.png")) {
        // ### TODO: missing error checking (The partition could be full!)
        savePreview(store);
        (void)store->close();
    }

    if (!completeSaving(store)) {
        delete store;
        return false;
    }
    kDebug(30003) << "Saving done of url:" << url().url();
    if (!store->finalize()) {
        delete store;
        return false;
    }
    // Success
    delete store;
    return true;
}

bool KoDocument::saveToStream(QIODevice *dev)
{
    QDomDocument doc = saveXML();
    // Save to buffer
    QByteArray s = doc.toByteArray(); // utf8 already
    dev->open(QIODevice::WriteOnly);
    int nwritten = dev->write(s.data(), s.size());
    if (nwritten != (int)s.size())
        kWarning(30003) << "wrote " << nwritten << "- expected" <<  s.size();
    return nwritten == (int)s.size();
}

// Called for embedded documents
bool KoDocument::saveToStore(KoStore *_store, const QString & _path)
{
    kDebug(30003) << "Saving document to store" << _path;

    // Use the path as the internal url
    if (_path.startsWith(STORE_PROTOCOL))
        setUrl(KUrl(_path));
    else // ugly hack to pass a relative URI
        setUrl(KUrl(INTERNAL_PREFIX +  _path));

    // In the current directory we're the king :-)
    if (_store->open("root")) {
        KoStoreDevice dev(_store);
        if (!saveToStream(&dev)) {
            _store->close();
            return false;
        }
        if (!_store->close())
            return false;
    }

    if (!completeSaving(_store))
        return false;

    // Now that we're done leave the directory again
    _store->popDirectory();

    kDebug(30003) << "Saved document to store";

    return true;
}

bool KoDocument::saveOasisPreview(KoStore *store, KoXmlWriter *manifestWriter)
{
    const QPixmap pix = generatePreview(QSize(128, 128));
    QImage preview(pix.toImage().convertToFormat(QImage::Format_ARGB32, Qt::ColorOnly));

    // ### TODO: freedesktop.org Thumbnail specification (date...)
    KoStoreDevice io(store);
    if (!io.open(QIODevice::WriteOnly))
        return false;
    if (! preview.save(&io, "PNG", 0))
        return false;
    io.close();
    manifestWriter->addManifestEntry("Thumbnails/", "");
    manifestWriter->addManifestEntry("Thumbnails/thumbnail.png", "");
    return true;
}

bool KoDocument::savePreview(KoStore *store)
{
    QPixmap pix = generatePreview(QSize(256, 256));
    // Reducing to 8bpp reduces file sizes quite a lot.
    const QImage preview(pix.toImage().convertToFormat(QImage::Format_Indexed8, Qt::AvoidDither | Qt::DiffuseDither));
    KoStoreDevice io(store);
    if (!io.open(QIODevice::WriteOnly))
        return false;
    if (! preview.save(&io, "PNG"))     // ### TODO What is -9 in quality terms?
        return false;
    io.close();
    return true;
}

QPixmap KoDocument::generatePreview(const QSize& size)
{
    qreal docWidth, docHeight;
    int pixmapSize = qMax(size.width(), size.height());

    if (d->pageLayout.width > 1.0) {
        docWidth = d->pageLayout.width / 72 * KoDpi::dpiX();
        docHeight = d->pageLayout.height / 72 * KoDpi::dpiY();
    } else {
        // If we don't have a page layout, just draw the top left hand corner
        docWidth = 500.0;
        docHeight = 500.0;
    }

    qreal ratio = docWidth / docHeight;

    int previewWidth, previewHeight;
    if (ratio > 1.0) {
        previewWidth = (int) pixmapSize;
        previewHeight = (int)(pixmapSize / ratio);
    } else {
        previewWidth = (int)(pixmapSize * ratio);
        previewHeight = (int) pixmapSize;
    }

    QPixmap pix((int)docWidth, (int)docHeight);

    pix.fill(QColor(245, 245, 245));

    QRect rc(0, 0, pix.width(), pix.height());

    QPainter p;
    p.begin(&pix);
    paintEverything(p, rc);
    p.end();

    return pix.scaled(QSize(previewWidth, previewHeight), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

QString KoDocument::autoSaveFile(const QString & path) const
{
    // Using the extension allows to avoid relying on the mime magic when opening
    KMimeType::Ptr mime = KMimeType::mimeType(nativeFormatMimeType());
    if (! mime) {
        qFatal("It seems your installation is broken/incomplete cause we failed to load the native mimetype \"%s\".", nativeFormatMimeType().constData());
    }
    QString extension = mime->property("X-KDE-NativeExtension").toString();
    if (path.isEmpty()) {
        // Never saved? Use a temp file in $HOME then
        // Yes, two open unnamed docs will overwrite each other's autosave file,
        // but hmm, we can only do something if that's in the same process anyway...
        QString ret = QDir::homePath() + "/." + componentData().componentName() + ".autosave" + extension;
        return ret;
    } else {
        KUrl url = KUrl::fromPath(path);
        Q_ASSERT(url.isLocalFile());
        QString dir = url.directory(KUrl::AppendTrailingSlash);
        QString filename = url.fileName();
        return dir + '.' + filename + ".autosave" + extension;
    }
}

bool KoDocument::checkAutoSaveFile()
{
    QString asf = autoSaveFile(QString());   // the one in $HOME
    //kDebug(30003) <<"asf=" << asf;
    if (QFile::exists(asf)) {
        QDateTime date = QFileInfo(asf).lastModified();
        QString dateStr = date.toString(Qt::LocalDate);
        int res = KMessageBox::warningYesNoCancel(
                      0, i18n("An autosaved file for an unnamed document exists in %1.\nThis file is dated %2\nDo you want to open it?",
                              asf, dateStr));
        switch (res) {
        case KMessageBox::Yes : {
            KUrl url;
            url.setPath(asf);
            bool ret = openUrl(url);
            if (ret)
                resetURL();
            return ret;
        }
        case KMessageBox::No :
            QFile::remove(asf);
            return false;
        default: // Cancel
            return false;
        }
    }
    return false;
}

bool KoDocument::importDocument(const KUrl & _url)
{
    bool ret;

    kDebug(30003) << "url=" << _url.url();
    d->isImporting = true;

    // open...
    ret = openUrl(_url);

    // reset url & m_file (kindly? set by KParts::openUrl()) to simulate a
    // File --> Import
    if (ret) {
        kDebug(30003) << "success, resetting url";
        resetURL();
        setTitleModified();
    }

    d->isImporting = false;

    return ret;
}

bool KoDocument::openUrl(const KUrl & _url)
{
    kDebug(30003) << "url=" << _url.url();
    d->lastErrorMessage.clear();

    // Reimplemented, to add a check for autosave files and to improve error reporting
    if (!_url.isValid()) {
        d->lastErrorMessage = i18n("Malformed URL\n%1", _url.url());  // ## used anywhere ?
        return false;
    }
    abortLoad();

    KUrl url(_url);
    bool autosaveOpened = false;
    d->bLoading = true;
    if (url.isLocalFile() && d->shouldCheckAutoSaveFile) {
        QString file = url.toLocalFile();
        QString asf = autoSaveFile(file);
        if (QFile::exists(asf)) {
            //kDebug(30003) <<"asf=" << asf;
            // ## TODO compare timestamps ?
            int res = KMessageBox::warningYesNoCancel(0,
                      i18n("An autosaved file exists for this document.\nDo you want to open it instead?"));
            switch (res) {
            case KMessageBox::Yes :
                url.setPath(asf);
                autosaveOpened = true;
                break;
            case KMessageBox::No :
                QFile::remove(asf);
                break;
            default: // Cancel
                d->bLoading = false;
                return false;
            }
        }
    }

    bool ret = KParts::ReadWritePart::openUrl(url);

    if (autosaveOpened)
        resetURL(); // Force save to act like 'Save As'
    else {
        // We have no koffice shell when we are being embedded as a readonly part.
        //if ( d->shells.isEmpty() )
        //    kWarning(30003) << "no shell yet !";
        // Add to recent actions list in our shells
        foreach(KoMainWindow *mainWindow, d->shells) {
            mainWindow->addRecentURL(_url);
        }
    }
    return ret;
}

bool KoDocument::openFile()
{
    //kDebug(30003) <<"for" << localFilePath();
    if (!QFile::exists(localFilePath())) {
        QApplication::restoreOverrideCursor();
        if (d->autoErrorHandlingEnabled)
            // Maybe offer to create a new document with that name ?
            KMessageBox::error(0, i18n("The file %1 does not exist.", localFilePath()));
        d->bLoading = false;
        return false;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    d->specialOutputFlag = 0;
    QByteArray _native_format = nativeFormatMimeType();

    KUrl u;
    u.setPath(localFilePath());
    QString typeName = arguments().mimeType();
    if (typeName.isEmpty())
        typeName = KMimeType::findByUrl(u, 0, true)->name();

    // Allow to open backup files, don't keep the mimetype application/x-trash.
    if (typeName == "application/x-trash") {
        QString path = u.path();
        KMimeType::Ptr mime = KMimeType::mimeType(typeName);
        const QStringList patterns = mime ? mime->patterns() : QStringList();
        // Find the extension that makes it a backup file, and remove it
        for (QStringList::ConstIterator it = patterns.begin(); it != patterns.end(); ++it) {
            QString ext = *it;
            if (!ext.isEmpty() && ext[0] == '*') {
                ext.remove(0, 1);
                if (path.endsWith(ext)) {
                    path.truncate(path.length() - ext.length());
                    break;
                }
            }
        }
        typeName = KMimeType::findByPath(path, 0, true)->name();
    }

    // Special case for flat XML files (e.g. using directory store)
    if (u.fileName() == "maindoc.xml" || u.fileName() == "content.xml" || typeName == "inode/directory") {
        typeName = _native_format; // Hmm, what if it's from another app? ### Check mimetype
        d->specialOutputFlag = SaveAsDirectoryStore;
        kDebug(30003) << "loading" << u.fileName() << ", using directory store for" << localFilePath() << "; typeName=" << typeName;
    }
    kDebug(30003) << localFilePath() << "type:" << typeName;

    QString importedFile = localFilePath();

    // create the main progress monitoring object for loading, this can
    // contain subtasks for filtering and loading
    DocumentProgressProxy proxyProgress(this);
    d->progressUpdater = new KoProgressUpdater(&proxyProgress,
                                               KoProgressUpdater::Threaded,
                                               d->profileStream);
    d->progressUpdater->setReferenceTime(d->profileReferenceTime);

    if (!isNativeFormat(typeName.toLatin1(), ForImport)) {
        if (!d->filterManager)
            d->filterManager = new KoFilterManager(this, d->progressUpdater);
        KoFilter::ConversionStatus status;
        importedFile = d->filterManager->importDocument(localFilePath(), status);
        if (status != KoFilter::OK) {
            QApplication::restoreOverrideCursor();

            QString msg;
            switch (status) {
            case KoFilter::OK: break;

            case KoFilter::CreationError:
                msg = i18n("Creation error"); break;

            case KoFilter::FileNotFound:
                msg = i18n("File not found"); break;

            case KoFilter::StorageCreationError:
                msg = i18n("Cannot create storage"); break;

            case KoFilter::BadMimeType:
                msg = i18n("Bad MIME type"); break;

            case KoFilter::EmbeddedDocError:
                msg = i18n("Error in embedded document"); break;

            case KoFilter::WrongFormat:
                msg = i18n("Format not recognized"); break;

            case KoFilter::NotImplemented:
                msg = i18n("Not implemented"); break;

            case KoFilter::ParsingError:
                msg = i18n("Parsing error"); break;

            case KoFilter::PasswordProtected:
                msg = i18n("Document is password protected"); break;

            case KoFilter::InternalError:
            case KoFilter::UnexpectedEOF:
            case KoFilter::UnexpectedOpcode:
            case KoFilter::StupidError: // ?? what is this ??
            case KoFilter::UsageError:
                msg = i18n("Internal error"); break;

            case KoFilter::OutOfMemory:
                msg = i18n("Out of memory"); break;

            case KoFilter::UserCancelled:
            case KoFilter::BadConversionGraph:
                // intentionally we do not prompt the error message here
                break;

            default: msg = i18n("Unknown error"); break;
            }

            if (d->autoErrorHandlingEnabled && !msg.isEmpty()) {
                QString errorMsg(i18n("Could not open\n%2.\nReason: %1", msg, prettyPathOrUrl()));
                KMessageBox::error(0, errorMsg);
            }

            d->bLoading = false;
            delete d->progressUpdater;
            d->progressUpdater = 0;
            return false;
        }
        d->bEmpty = false;
        kDebug(30003) << "importedFile" << importedFile << "status:" << static_cast<int>(status);
    }

    QApplication::restoreOverrideCursor();

    bool ok = true;

    if (!importedFile.isEmpty()) { // Something to load (tmp or native file) ?
        // The filter, if any, has been applied. It's all native format now.
        if (!loadNativeFormat(importedFile)) {
            ok = false;
            if (d->autoErrorHandlingEnabled) {
                showLoadingErrorDialog();
            }
        }
    }

    if (importedFile != localFilePath()) {
        // We opened a temporary file (result of an import filter)
        // Set document URL to empty - we don't want to save in /tmp !
        // But only if in readwrite mode (no saving problem otherwise)
        // --
        // But this isn't true at all.  If this is the result of an
        // import, then importedFile=temporary_file.kwd and
        // file/m_url=foreignformat.ext so m_url is correct!
        // So don't resetURL() or else the caption won't be set when
        // foreign files are opened (an annoying bug).
        // - Clarence
        //
#if 0
        if (isReadWrite())
            resetURL();
#endif

        // remove temp file - uncomment this to debug import filters
        if (!importedFile.isEmpty()) {
#ifndef NDEBUG
            if (!getenv("KOFFICE_DEBUG_FILTERS"))
#endif
            QFile::remove(importedFile);
        }
    }

    if (ok && d->bSingleViewMode) {
        // See addClient below
        KXMLGUIFactory *guiFactory = factory();
        if (guiFactory)  // 0 when splitting views in konq, for some reason
            guiFactory->removeClient(this);

        if (!d->views.isEmpty()) {
            // We already had a view (this happens when doing reload in konqueror)
            KoView *v = d->views.first();
            if (guiFactory)
                guiFactory->removeClient(v);
            removeView(v);
            delete v;
            Q_ASSERT(d->views.isEmpty());
        }

        KoView *view = createView(d->wrapperWidget);
        d->wrapperWidget->setKoView(view);
        view->show();

        // Ok, now we have a view, so action() and domDocument() will work as expected
        // -> rebuild GUI
        if (guiFactory)
            guiFactory->addClient(this);
    }

    if (ok) {
        setMimeTypeAfterLoading(typeName);

        KNotification *notify = new KNotification("DocumentLoaded");
        notify->setText(i18n("Document <i>%1</i> loaded", url().url()));
        notify->addContext("url", url().url());
        QTimer::singleShot(0, notify, SLOT(sendEvent()));
        deleteOpenPane();
    }
    d->bLoading = false;

    QPointer<KoUpdater> updater
            = progressUpdater()->startSubtask(1, "clear undo stack");
    updater->setProgress(0);
    undoStack()->clear();
    updater->setProgress(100);

    delete d->progressUpdater;
    d->progressUpdater = 0;

    return ok;
}

KoProgressUpdater *KoDocument::progressUpdater() const
{
    return d->progressUpdater;
}

// shared between openFile and koMainWindow's "create new empty document" code
void KoDocument::setMimeTypeAfterLoading(const QString& mimeType)
{
    d->mimeType = mimeType.toLatin1();

    d->outputMimeType = d->mimeType;

    const bool needConfirm = !isNativeFormat(d->mimeType, ForImport);
    setConfirmNonNativeSave(false, needConfirm);
    setConfirmNonNativeSave(true, needConfirm);
}

// The caller must call store->close() if loadAndParse returns true.
bool KoDocument::oldLoadAndParse(KoStore *store, const QString& filename, KoXmlDocument& doc)
{
    //kDebug(30003) <<"Trying to open" << filename;

    if (!store->open(filename)) {
        kWarning(30003) << "Entry " << filename << " not found!";
        d->lastErrorMessage = i18n("Could not find %1", filename);
        return false;
    }
    // Error variables for QDomDocument::setContent
    QString errorMsg;
    int errorLine, errorColumn;
    bool ok = doc.setContent(store->device(), &errorMsg, &errorLine, &errorColumn);
    store->close();
    if (!ok) {
        kError(30003) << "Parsing error in " << filename << "! Aborting!" << endl
        << " In line: " << errorLine << ", column: " << errorColumn << endl
        << " Error message: " << errorMsg << endl;
        d->lastErrorMessage = i18n("Parsing error in %1 at line %2, column %3\nError message: %4"
                                   , filename  , errorLine, errorColumn ,
                                   QCoreApplication::translate("QXml", errorMsg.toUtf8(), 0,
                                                               QCoreApplication::UnicodeUTF8));
        return false;
    }
    kDebug(30003) << "File" << filename << " loaded and parsed";
    return true;
}

bool KoDocument::loadNativeFormat(const QString & file_)
{
    QString file = file_;
    QFileInfo fileInfo(file);
    if (!fileInfo.exists()) { // check duplicated from openUrl, but this is useful for templates
        d->lastErrorMessage = i18n("The file %1 does not exist.", file);
        return false;
    }
    if (!fileInfo.isFile()) {
        file = file += "/content.xml";
        QFileInfo fileInfo2(file);
        if (!fileInfo2.exists() || !fileInfo2.isFile()) {
            d->lastErrorMessage = i18n("%1 is not a file." , file_);
            return false;
        }
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    kDebug(30003) << file;

    QFile in;
    bool isRawXML = false;
    if (d->specialOutputFlag != SaveAsDirectoryStore) { // Don't try to open a directory ;)
        in.setFileName(file);
        if (!in.open(QIODevice::ReadOnly)) {
            QApplication::restoreOverrideCursor();
            d->lastErrorMessage = i18n("Could not open the file for reading (check read permissions).");
            return false;
        }

        char buf[6];
        buf[5] = 0;
        int pos = 0;
        do {
            if (in.read(buf + pos , 1) < 1) {
                QApplication::restoreOverrideCursor();
                in.close();
                d->lastErrorMessage = i18n("Could not read the beginning of the file.");
                return false;
            }

            if (QChar(buf[pos]).isSpace())
                continue;
            pos++;
        } while (pos < 5);
        isRawXML = (strncasecmp(buf, "<?xml", 5) == 0);
        if (! isRawXML)
            // also check for broken MathML files, which seem to be rather common
            isRawXML = (strncasecmp(buf, "<math", 5) == 0);   // file begins with <math ?
        //kDebug(30003) <<"PATTERN=" << buf;
    }
    // Is it plain XML?
    if (isRawXML) {
        in.seek(0);
        QString errorMsg;
        int errorLine;
        int errorColumn;
        KoXmlDocument doc;
        bool res;
        if (doc.setContent(&in, &errorMsg, &errorLine, &errorColumn)) {
            res = loadXML(doc, 0);
            if (res)
                res = completeLoading(0);
        } else {
            kError(30003) << "Parsing Error! Aborting! (in KoDocument::loadNativeFormat (QFile))" << endl
            << "  Line: " << errorLine << " Column: " << errorColumn << endl
            << "  Message: " << errorMsg << endl;
            d->lastErrorMessage = i18n("parsing error in the main document at line %1, column %2\nError message: %3", errorLine, errorColumn, i18n(errorMsg.toUtf8()));
            res = false;
        }

        QApplication::restoreOverrideCursor();
        in.close();
        d->bEmpty = false;
        return res;
    } else { // It's a koffice store (tar.gz, zip, directory, etc.)
        in.close();

        return loadNativeFormatFromStore(file);
    }
}

bool KoDocument::loadNativeFormatFromStore(const QString& file)
{
    KoStore::Backend backend = (d->specialOutputFlag == SaveAsDirectoryStore) ? KoStore::Directory : KoStore::Auto;
    KoStore *store = KoStore::createStore(file, KoStore::Read, "", backend);

    if (store->bad()) {
        d->lastErrorMessage = i18n("Not a valid KOffice file: %1", file);
        delete store;
        QApplication::restoreOverrideCursor();
        return false;
    }

    // Remember that the file was encrypted
    if (d->specialOutputFlag == 0 && store->isEncrypted() && !d->isImporting)
        d->specialOutputFlag = SaveEncrypted;

    const bool success = loadNativeFormatFromStoreInternal(store);

    // Retrieve the password after loading the file, only then is it guaranteed to exist
    if (success && store->isEncrypted() && !d->isImporting)
        d->password = store->password();

    delete store;

    return success;
}

bool KoDocument::loadNativeFormatFromStore(QByteArray &data)
{
    bool succes;
    KoStore::Backend backend = (d->specialOutputFlag == SaveAsDirectoryStore) ? KoStore::Directory : KoStore::Auto;
    QBuffer buffer(&data);
    KoStore *store = KoStore::createStore(&buffer, KoStore::Read, "", backend);

    if (store->bad())
        return false;

    // Remember that the file was encrypted
    if (d->specialOutputFlag == 0 && store->isEncrypted() && !d->isImporting)
        d->specialOutputFlag = SaveEncrypted;

    succes = loadNativeFormatFromStoreInternal(store);

    // Retrieve the password after loading the file, only then is it guaranteed to exist
    if (succes && store->isEncrypted() && !d->isImporting)
        d->password = store->password();

    delete store;

    return succes;
}

bool KoDocument::loadNativeFormatFromStoreInternal(KoStore *store)
{
    bool oasis = true;

    if (oasis && store->hasFile("manifest.rdf") && d->docRdf) {
        d->docRdf->loadOasis(store);
    }

    // OASIS/OOo file format?
    if (store->hasFile("content.xml")) {
        store->disallowNameExpansion();

        // We could check the 'mimetype' file, but let's skip that and be tolerant.

        if (!loadOasisFromStore(store)) {
            QApplication::restoreOverrideCursor();
            return false;
        }

    } else if (store->hasFile("root")) {   // Fallback to "old" file format (maindoc.xml)
        oasis = false;

        KoXmlDocument doc;
        bool ok = oldLoadAndParse(store, "root", doc);
        if (ok)
            ok = loadXML(doc, store);
        if (!ok) {
            QApplication::restoreOverrideCursor();
            return false;
        }

    } else {
        kError(30003) << "ERROR: No maindoc.xml" << endl;
        d->lastErrorMessage = i18n("Invalid document: no file 'maindoc.xml'.");
        QApplication::restoreOverrideCursor();
        return false;
    }

    if (oasis && store->hasFile("meta.xml")) {
        KoXmlDocument metaDoc;
        KoOdfReadStore oasisStore(store);
        if (oasisStore.loadAndParse("meta.xml", metaDoc, d->lastErrorMessage)) {
            d->docInfo->loadOasis(metaDoc);
        }
    } else if (!oasis && store->hasFile("documentinfo.xml")) {
        KoXmlDocument doc;
        if (oldLoadAndParse(store, "documentinfo.xml", doc)) {
            d->docInfo->load(doc);
        }
    } else {
        //kDebug( 30003 ) <<"cannot open document info";
        delete d->docInfo;
        d->docInfo = new KoDocumentInfo(this);
    }

    if (oasis && store->hasFile("VersionList.xml")) {
        KNotification *notify = new KNotification("DocumentHasVersions");
        notify->setText(i18n("Document <i>%1</i> contains several versions. Go to File->Versions to open an old version.", store->urlOfStore().url()));
        notify->addContext("url", store->urlOfStore().url());
        QTimer::singleShot(0, notify, SLOT(sendEvent()));

        KoXmlDocument versionInfo;
        KoOdfReadStore oasisStore(store);
        if (oasisStore.loadAndParse("VersionList.xml", versionInfo, d->lastErrorMessage)) {
            KoXmlNode list = KoXml::namedItemNS(versionInfo, KoXmlNS::VL, "version-list");
            KoXmlElement e;
            forEachElement(e, list) {
                if (e.localName() == "version-entry" && e.namespaceURI() == KoXmlNS::VL) {
                    KoVersionInfo version;
                    version.comment = e.attribute("comment");
                    version.title = e.attribute("title");
                    version.saved_by = e.attribute("creator");
                    version.date = QDateTime::fromString(e.attribute("date-time"), Qt::ISODate);
                    store->extractFile("Versions/" + version.title, version.data);
                    d->versionInfo.append(version);
                }
            }
        }
    }

    bool res = completeLoading(store);
    QApplication::restoreOverrideCursor();
    d->bEmpty = false;
    return res;
}

// For embedded documents
bool KoDocument::loadFromStore(KoStore *_store, const QString& url)
{
    if (_store->open(url)) {
        KoXmlDocument doc;
        doc.setContent(_store->device());
        if (!loadXML(doc, _store)) {
            _store->close();
            return false;
        }
        _store->close();
    } else {
        kWarning() << "couldn't open " << url;
    }

    _store->pushDirectory();
    // Store as document URL
    if (url.startsWith(STORE_PROTOCOL)) {
        setUrl(url);
    } else {
        setUrl(KUrl(INTERNAL_PREFIX + url));
        _store->enterDirectory(url);
    }

    bool result = completeLoading(_store);

    // Restore the "old" path
    _store->popDirectory();

    return result;
}

bool KoDocument::loadOasisFromStore(KoStore *store)
{
    KoOdfReadStore odfStore(store);
    if (! odfStore.loadAndParse(d->lastErrorMessage)) {
        return false;
    }
    return loadOdf(odfStore);
}

bool KoDocument::addVersion(const QString& comment)
{
    kDebug(30003) << "Saving the new version....";

    KoStore::Backend backend = KoStore::Auto;
    if (d->specialOutputFlag != 0)
        return false;

    QByteArray mimeType = d->outputMimeType;
    QByteArray nativeOasisMime = nativeOasisMimeType();
    bool oasis = !mimeType.isEmpty() && (mimeType == nativeOasisMime || mimeType == nativeOasisMime + "-template");

    if (!oasis)
        return false;

    // TODO: use std::auto_ptr or create store on stack [needs API fixing],
    // to remove all the 'delete store' in all the branches
    QByteArray data;
    QBuffer buffer(&data);
    KoStore *store = KoStore::createStore(&buffer/*file*/, KoStore::Write, mimeType, backend);
    if (store->bad()) {
        delete store;
        return false;
    }

    kDebug(30003) << "Saving to OASIS format";
    // Tell KoStore not to touch the file names
    store->disallowNameExpansion();
    KoOdfWriteStore odfStore(store);

    KoXmlWriter *manifestWriter = odfStore.manifestWriter(mimeType);
    Q_UNUSED(manifestWriter); // XXX why?

    KoEmbeddedDocumentSaver embeddedSaver;
    SavingContext documentContext(odfStore, embeddedSaver);

    if (!saveOdf(documentContext)) {
        kDebug(30003) << "saveOdf failed";
        delete store;
        return false;
    }

    // Save embedded objects
    if (!embeddedSaver.saveEmbeddedDocuments(documentContext)) {
        kDebug(30003) << "save embedded documents failed";
        delete store;
        return false;
    }

    // Write out manifest file
    if (!odfStore.closeManifestWriter()) {
        d->lastErrorMessage = i18n("Error while trying to write '%1'. Partition full?", QString("META-INF/manifest.xml"));
        delete store;
        return false;
    }

    if (!store->finalize()) {
        delete store;
        return false;
    }
    delete store;

    KoVersionInfo version;
    version.comment = comment;
    version.title = "Version" + QString::number(d->versionInfo.count() + 1);
    version.saved_by = documentInfo()->authorInfo("creator");
    version.date = QDateTime::currentDateTime();
    version.data = data;
    d->versionInfo.append(version);

    save(); //finally save the document + the new version
    return true;
}

bool KoDocument::isStoredExtern() const
{
    return !storeInternal() && hasExternURL();
}

void KoDocument::setModified(bool mod)
{
    if (isAutosaving())   // ignore setModified calls due to autosaving
        return;

    //kDebug(30003)<<" url:" << url.path();
    //kDebug(30003)<<" mod="<<mod<<" MParts mod="<<KParts::ReadWritePart::isModified()<<" isModified="<<isModified();

    if (mod && !d->modifiedAfterAutosave) {
        // First change since last autosave -> start the autosave timer
        setAutoSave(d->autoSaveDelay);
    }
    d->modifiedAfterAutosave = mod;

    if (mod == isModified())
        return;

    KParts::ReadWritePart::setModified(mod);

    if (mod) {
        d->bEmpty = false;
    }

    // This influences the title
    setTitleModified();
    emit modified(mod);
}

int KoDocument::queryCloseDia()
{
    //kDebug(30003);

    QString name;
    if (documentInfo()) {
        name = documentInfo()->aboutInfo("title");
    }
    if (name.isEmpty())
        name = url().fileName();

    if (name.isEmpty())
        name = i18n("Untitled");

    int res = KMessageBox::warningYesNoCancel(0,
              i18n("<p>The document <b>'%1'</b> has been modified.</p><p>Do you want to save it?</p>", name));

    switch (res) {
    case KMessageBox::Yes :
        save(); // NOTE: External files always in native format. ###TODO: Handle non-native format
        setModified(false);   // Now when queryClose() is called by closeEvent it won't do anything.
        break;
    case KMessageBox::No :
        removeAutoSaveFiles();
        setModified(false);   // Now when queryClose() is called by closeEvent it won't do anything.
        break;
    default : // case KMessageBox::Cancel :
        return res; // cancels the rest of the files
    }
    return res;
}

void KoDocument::setTitleModified(const QString &caption, bool mod)
{
    //kDebug(30003)<<" url:"<<url().url()<<" caption:"<<caption<<" mod:"<<mod;
    KoDocument *doc = dynamic_cast<KoDocument *>(parent());
    if (doc) {
        doc->setTitleModified(caption, mod);
        return;
    }
    // we must be root doc so update caption in all related windows
    foreach(KoMainWindow *mainWindow, d->shells) {
        mainWindow->updateCaption(caption, mod);
        mainWindow->updateReloadFileAction(this);
        mainWindow->updateVersionsFileAction(this);
    }
}

QString KoDocument::prettyPathOrUrl() const
{
    QString url( this->url().pathOrUrl() );
#ifdef Q_WS_WIN
    if (this->url().isLocalFile()) {
        url = QDir::convertSeparators(url);
    }
#endif
    return url;
}

// Get caption from document info (title(), in about page)
QString KoDocument::caption() const
{
    QString c;
    if (documentInfo()) {
        c = documentInfo()->aboutInfo("title");
    }
    const QString url(this->url().fileName());
    if (!c.isEmpty() && !url.isEmpty()) {
        c = QString("%1 - %2").arg(c).arg(url);
    }
    else if (c.isEmpty()) {
        c = url; // Fall back to document URL
    }
    return c;
}

void KoDocument::setTitleModified()
{
    //kDebug(30003)<<" url:"<<url().url()<<" extern:"<<isStoredExtern()<<" current:"<<d->current;
    KoDocument *doc = dynamic_cast<KoDocument *>(parent());
    if ((url().isEmpty() || isStoredExtern()) && d->current) {
        if (doc) {
            doc->setTitleModified(caption(), isModified());
            return;
        }
        // we must be root doc so update caption in all related windows
        setTitleModified(caption(), isModified());
        return;
    }
    if (doc) {
        // internal doc or not current doc, so pass on the buck
        doc->setTitleModified();
    }
}

bool KoDocument::completeLoading(KoStore*)
{
    return true;
}

bool KoDocument::completeSaving(KoStore*)
{
    return true;
}

QDomDocument KoDocument::createDomDocument(const QString& tagName, const QString& version) const
{
    return createDomDocument(componentData().componentName(), tagName, version);
}

//static
QDomDocument KoDocument::createDomDocument(const QString& appName, const QString& tagName, const QString& version)
{
    QDomImplementation impl;
    QString url = QString("http://www.koffice.org/DTD/%1-%2.dtd").arg(appName).arg(version);
    QDomDocumentType dtype = impl.createDocumentType(tagName,
                             QString("-//KDE//DTD %1 %2//EN").arg(appName).arg(version),
                             url);
    // The namespace URN doesn't need to include the version number.
    QString namespaceURN = QString("http://www.koffice.org/DTD/%1").arg(appName);
    QDomDocument doc = impl.createDocument(namespaceURN, tagName, dtype);
    doc.insertBefore(doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""), doc.documentElement());
    return doc;
}

QDomDocument KoDocument::saveXML()
{
    kError(30003) << "not implemented" << endl;
    d->lastErrorMessage = i18n("Internal error: saveXML not implemented");
    return QDomDocument();
}

KService::Ptr KoDocument::nativeService()
{
    if (!d->nativeService)
        d->nativeService = readNativeService(componentData());

    return d->nativeService;
}

QByteArray KoDocument::nativeFormatMimeType() const
{
    KService::Ptr service = const_cast<KoDocument *>(this)->nativeService();
    if (!service) {
        kWarning(30003) << "No native service defined to read NativeMimeType from desktop file!";
        return QByteArray();
    }
    QByteArray nativeMimeType = service->property("X-KDE-NativeMimeType").toString().toLatin1();
#ifndef NDEBUG
    if (nativeMimeType.isEmpty()) {
        // shouldn't happen, let's find out why it happened
        if (!service->serviceTypes().contains("KOfficePart"))
            kWarning(30003) << "Wrong desktop file, KOfficePart isn't mentioned";
        else if (!KServiceType::serviceType("KOfficePart"))
            kWarning(30003) << "The KOfficePart service type isn't installed!";
        else
            kWarning(30003) << "Failed to read NativeMimeType from desktop file!";
    }
#endif
    return nativeMimeType;
}

QByteArray KoDocument::nativeOasisMimeType() const
{
    KService::Ptr service = const_cast<KoDocument *>(this)->nativeService();
    if (!service) {
        return KoDocument::nativeFormatMimeType();
    }
    return service->property("X-KDE-NativeOasisMimeType").toString().toLatin1();
}


//static
KService::Ptr KoDocument::readNativeService(const KComponentData &componentData)
{
    QString instname = componentData.isValid() ? componentData.componentName() : KGlobal::mainComponent().componentName();

    // The new way is: we look for a foopart.desktop in the kde_services dir.
    QString servicepartname = instname + "part.desktop";
    KService::Ptr service = KService::serviceByDesktopPath(servicepartname);
    if (service)
        kDebug(30003) << servicepartname << " found.";
    if (!service) {
        // The old way is kept as fallback for compatibility, but in theory this is really never used anymore.

        // Try by path first, so that we find the global one (which has the native mimetype)
        // even if the user created a kword.desktop in ~/.kde/share/applnk or any subdir of it.
        // If he created it under ~/.kde/share/applnk/Office/ then no problem anyway.
        service = KService::serviceByDesktopPath(QString::fromLatin1("Office/%1.desktop").arg(instname));
    }
    if (!service)
        service = KService::serviceByDesktopName(instname);

    return service;
}

QByteArray KoDocument::readNativeFormatMimeType(const KComponentData &componentData)   //static
{
    KService::Ptr service = readNativeService(componentData);
    if (!service)
        return QByteArray();

    if (service->property("X-KDE-NativeMimeType").toString().isEmpty()) {
        // It may be that the servicetype "KOfficePart" is missing, which leads to this property not being known
        KServiceType::Ptr ptr = KServiceType::serviceType("KOfficePart");
        if (!ptr)
            kError(30003) << "The serviceType KOfficePart is missing. Check that you have a kofficepart.desktop file in the share/servicetypes directory." << endl;
        else {
            kWarning(30003) << service->entryPath() << ": no X-KDE-NativeMimeType entry!";
        }
    }

    return service->property("X-KDE-NativeMimeType").toString().toLatin1();
}

QStringList KoDocument::readExtraNativeMimeTypes(const KComponentData &componentData)   //static
{
    KService::Ptr service = readNativeService(componentData);
    if (!service)
        return QStringList();
    return service->property("X-KDE-ExtraNativeMimeTypes").toStringList();
}

bool KoDocument::isNativeFormat(const QByteArray& mimetype, ImportExportType importExportType) const
{
    if (mimetype == nativeFormatMimeType())
        return true;
    return extraNativeMimeTypes(importExportType).contains(mimetype);
}

QStringList KoDocument::extraNativeMimeTypes(KoDocument::ImportExportType importExportType) const
{
    Q_UNUSED(importExportType);
    QStringList lst;
    // This implementation is temporary while we treat both koffice-1.3 and OASIS formats as native.
    // But it's good to have this virtual method, in case some app want to
    // support more than one native format.
    KService::Ptr service = const_cast<KoDocument *>(this)->nativeService();
    if (!service)   // can't happen
        return lst;
    return service->property("X-KDE-ExtraNativeMimeTypes").toStringList();
}

int KoDocument::supportedSpecialFormats() const
{
    // Apps which support special output flags can add reimplement and add to this.
    // E.g. this is how did "saving in the 1.1 format".
    // SaveAsDirectoryStore is a given since it's implemented by KoDocument itself.
    // SaveEncrypted is implemented in KoDocument as well, if QCA2 was found.
#ifdef QCA2
    return SaveAsDirectoryStore | SaveEncrypted;
#else
    return SaveAsDirectoryStore;
#endif
}

void KoDocument::addShell(KoMainWindow *shell)
{
    if (d->shells.indexOf(shell) == -1) {
        //kDebug(30003) <<"shell" << (void*)shell <<"added to doc" << this;
        d->shells.append(shell);
        connect(shell, SIGNAL(documentSaved()), d->undoStack, SLOT(setClean()));
    }
}

void KoDocument::removeShell(KoMainWindow *shell)
{
    //kDebug(30003) <<"shell" << (void*)shell <<"removed from doc" << this;
    d->shells.removeAll(shell);
}

const QList<KoMainWindow*>& KoDocument::shells() const
{
    return d->shells;
}

int KoDocument::shellCount() const
{
    return d->shells.count();
}

// DCOPObject *KoDocument::dcopObject()
// {
//     if ( !d->dcopObject )
//         d->dcopObject = new KoDocumentIface( this );
//     return d->dcopObject;
// }

// QByteArray KoDocument::dcopObjectId() const
// {
//     return const_cast<KoDocument *>(this)->dcopObject()->objId();
// }

void KoDocument::setErrorMessage(const QString& errMsg)
{
    d->lastErrorMessage = errMsg;
}

QString KoDocument::errorMessage() const
{
    return d->lastErrorMessage;
}

void KoDocument::showSavingErrorDialog()
{
    if (d->lastErrorMessage.isEmpty()) {
        KMessageBox::error(0, i18n("Could not save\n%1", localFilePath()));
    } else if (d->lastErrorMessage != "USER_CANCELED") {
        KMessageBox::error(0, i18n("Could not save %1\nReason: %2", localFilePath(), d->lastErrorMessage));
    }
}

void KoDocument::showLoadingErrorDialog()
{
    if (d->lastErrorMessage.isEmpty()) {
        KMessageBox::error(0, i18n("Could not open\n%1", prettyPathOrUrl()));
    } else if (d->lastErrorMessage != "USER_CANCELED") {
        KMessageBox::error(0, i18n("Could not open %1\nReason: %2", prettyPathOrUrl(), d->lastErrorMessage));
    }
}

bool KoDocument::isAutosaving() const
{
    return d->autosaving;
}

bool KoDocument::isLoading() const
{
    return d->bLoading;
}

void KoDocument::removeAutoSaveFiles()
{
    // Eliminate any auto-save file
    QString asf = autoSaveFile(localFilePath());   // the one in the current dir
    if (QFile::exists(asf))
        QFile::remove(asf);
    asf = autoSaveFile(QString());   // and the one in $HOME
    if (QFile::exists(asf))
        QFile::remove(asf);
}

void KoDocument::setBackupFile(bool _b)
{
    d->backupFile = _b;
}

bool KoDocument::backupFile()const
{
    return d->backupFile;
}


void KoDocument::setBackupPath(const QString & _path)
{
    d->backupPath = _path;
}

QString KoDocument::backupPath()const
{
    return d->backupPath;
}

void KoDocument::setCurrent(bool on)
{
    //kDebug(30003)<<" url:"<<url().url()<<" set to:"<<on;
    KoDocument *doc = dynamic_cast<KoDocument *>(parent());
    if (doc) {
        if (!isStoredExtern()) {
            // internal doc so set next external to current (for safety)
            doc->setCurrent(true);
            return;
        }
        // only externally stored docs shall have file name in title
        d->current = on;
        if (!on) {
            doc->setCurrent(true);      // let my next external parent take over
            return;
        }
        doc->forceCurrent(false);   // everybody else should keep off
    } else
        d->current = on;

    setTitleModified();
}

void KoDocument::forceCurrent(bool on)
{
    //kDebug(30003)<<" url:"<<url().url()<<" force to:"<<on;
    d->current = on;
    KoDocument *doc = dynamic_cast<KoDocument *>(parent());
    if (doc) {
        doc->forceCurrent(false);
    }
}

bool KoDocument::isCurrent() const
{
    return d->current;
}

bool KoDocument::storeInternal() const
{
    return d->storeInternal;
}

void KoDocument::setStoreInternal(bool i)
{
    d->storeInternal = i;
    //kDebug(30003)<<"="<<d->storeInternal<<" doc:"<<url().url();
}

bool KoDocument::hasExternURL() const
{
    return !url().protocol().isEmpty() && url().protocol() != STORE_PROTOCOL && url().protocol() != INTERNAL_PROTOCOL;
}

void KoDocument::slotStarted(KIO::Job *job)
{
    if (job && job->ui()) {
        job->ui()->setWindow(currentShell());
    }
}

static const struct {
    const char *localName;
    const char *documentType;
} TN2DTArray[] = {
    { "text", I18N_NOOP("a word processing") },
    { "spreadsheet", I18N_NOOP("a spreadsheet") },
    { "presentation", I18N_NOOP("a presentation") },
    { "chart", I18N_NOOP("a chart") },
    { "drawing", I18N_NOOP("a drawing") }
};
static const unsigned int numTN2DT = sizeof(TN2DTArray) / sizeof(*TN2DTArray);

QString KoDocument::tagNameToDocumentType(const QString& localName)
{
    for (unsigned int i = 0 ; i < numTN2DT ; ++i)
        if (localName == TN2DTArray[i].localName)
            return i18n(TN2DTArray[i].documentType);
    return localName;
}

KoPageLayout KoDocument::pageLayout(int /*pageNumber*/) const
{
    return d->pageLayout;
}

void KoDocument::setPageLayout(const KoPageLayout &pageLayout)
{
    d->pageLayout = pageLayout;
}

KoUnit KoDocument::unit() const
{
    return d->unit;
}

void KoDocument::setUnit(const KoUnit &unit)
{
    if (d->unit != unit) {
        d->unit = unit;
        emit unitChanged(unit);
    }
}

void KoDocument::saveUnitOdf(KoXmlWriter *settingsWriter) const
{
    settingsWriter->addConfigItem("unit", unitName());
}

QString KoDocument::unitName() const
{
    return KoUnit::unitName(unit());
}

void KoDocument::showStartUpWidget(KoMainWindow *parent, bool alwaysShow)
{
#ifndef NDEBUG
    if (d->templateType.isEmpty())
        kDebug(30003) << "showStartUpWidget called, but setTemplateType() never called. This will not show a lot";
#endif

    if (!alwaysShow) {
        KConfigGroup cfgGrp(componentData().config(), "TemplateChooserDialog");
        QString fullTemplateName = cfgGrp.readPathEntry("AlwaysUseTemplate", QString());

        if (!fullTemplateName.isEmpty()) {
            openTemplate(fullTemplateName);
            shells().first()->setRootDocument(this);
            return;
        }
    }

    parent->factory()->container("mainToolBar", parent)->hide();

    if (d->startUpWidget) {
        d->startUpWidget->show();
    } else {
        d->startUpWidget = createOpenPane(parent->centralWidget(), componentData(), d->templateType);
    }

    parent->setDocToOpen(this);
}

void KoDocument::openExistingFile(const KUrl& url)
{
    openUrl(url);
    setModified(false);
}

void KoDocument::openTemplate(const KUrl& url)
{
    bool ok = loadNativeFormat(url.toLocalFile());
    setModified(false);
    undoStack()->clear();

    if (ok) {
        QString mimeType = KMimeType::findByUrl( url, 0, true )->name();
        // in case this is a open document template remove the -template from the end
        mimeType.remove( QRegExp( "-template$" ) );
        setMimeTypeAfterLoading(mimeType);
        deleteOpenPane();
        resetURL();
        setEmpty();
    } else {
        showLoadingErrorDialog();
        initEmpty();
    }
}

void KoDocument::initEmpty()
{
    setEmpty();
    setModified(false);
}

void KoDocument::startCustomDocument()
{
    deleteOpenPane();
}

KoOpenPane *KoDocument::createOpenPane(QWidget *parent, const KComponentData &componentData,
                                       const QString& templateType)
{
    const QStringList mimeFilter = KoFilterManager::mimeFilter(KoDocument::readNativeFormatMimeType(),
                                                               KoFilterManager::Import, KoDocument::readExtraNativeMimeTypes());

    KoOpenPane *openPane = new KoOpenPane(parent, componentData, mimeFilter, templateType);
    QList<CustomDocumentWidgetItem> widgetList = createCustomDocumentWidgets(openPane);
    foreach(const CustomDocumentWidgetItem & item, widgetList) {
        openPane->addCustomDocumentWidget(item.widget, item.title, item.icon);
        connect(item.widget, SIGNAL(documentSelected()), this, SLOT(startCustomDocument()));
    }
    openPane->show();

    connect(openPane, SIGNAL(openExistingFile(const KUrl&)),
            this, SLOT(openExistingFile(const KUrl&)));
    connect(openPane, SIGNAL(openTemplate(const KUrl&)),
            this, SLOT(openTemplate(const KUrl&)));

    return openPane;
}

void KoDocument::setTemplateType(const QString& _templateType)
{
    d->templateType = _templateType;
}

QString KoDocument::templateType() const
{
    return d->templateType;
}

void KoDocument::deleteOpenPane()
{
    if (d->startUpWidget) {
        d->startUpWidget->hide();
        d->startUpWidget->deleteLater();

        shells().first()->factory()->container("mainToolBar",
                                               shells().first())->show();
        shells().first()->setRootDocument(this);
    } else {
        emit closeEmbedInitDialog();
    }
}

QList<KoDocument::CustomDocumentWidgetItem> KoDocument::createCustomDocumentWidgets(QWidget * /*parent*/)
{
    return QList<CustomDocumentWidgetItem>();
}

bool KoDocument::showEmbedInitDialog(QWidget *parent)
{
    KDialog dlg(parent);
    dlg.setCaption(i18n("Embedding Object"));
    KoOpenPane *pane = createOpenPane(&dlg, componentData(), templateType());
    pane->layout()->setMargin(0);
    dlg.setMainWidget(pane);
    KConfigGroup cfg = KSharedConfig::openConfig("EmbedInitDialog")->group(QString());
    /*dlg.setInitialSize(*/dlg.restoreDialogSize(cfg /*)*/);
    connect(this, SIGNAL(closeEmbedInitDialog()), &dlg, SLOT(accept()));

    bool ok = dlg.exec() == QDialog::Accepted;

    dlg.saveDialogSize(cfg);

    return ok;
}

QList<KoVersionInfo> & KoDocument::versionList()
{
    return d->versionInfo;
}

KUndoStack *KoDocument::undoStack()
{
    return d->undoStack;
}

void KoDocument::addCommand(QUndoCommand *command)
{
    if (command)
        d->undoStack->push(command);
}

void KoDocument::beginMacro(const QString & text)
{
    d->undoStack->beginMacro(text);
}

void KoDocument::endMacro()
{
    d->undoStack->endMacro();
}


void KoDocument::setDocumentClean(bool clean)
{
    setModified(!clean);
}

void KoDocument::setProfileStream(QTextStream *profilestream)
{
    d->profileStream = profilestream;
}

void KoDocument::setProfileReferenceTime(const QTime& referenceTime)
{
    d->profileReferenceTime = referenceTime;
}

void KoDocument::clearUndoHistory()
{
    d->undoStack->clear();
}

KoGridData &KoDocument::gridData()
{
    return d->gridData;
}

KoGuidesData &KoDocument::guidesData()
{
    return d->guidesData;
}

KoMainWindow *KoDocument::currentShell()
{
    return ::currentShell(this);
}

bool KoDocument::isEmpty() const
{
    return d->bEmpty;
}

void KoDocument::setEmpty()
{
    d->bEmpty = true;
}

QGraphicsItem *KoDocument::canvasItem()
{
    if (!d->canvasItem) {
        d->canvasItem = createCanvasItem();
    }
    return d->canvasItem;
}

QGraphicsItem *KoDocument::createCanvasItem()
{
    KoView *view = createView();
    QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget();
    QWidget *canvasController = view->findChild<KoCanvasControllerWidget*>();
    proxy->setWidget(canvasController);
    return proxy;
}

// static
int KoDocument::defaultAutoSave()
{
    return 300;
}

#include <KoDocument_p.moc>
#include <KoDocument.moc>
