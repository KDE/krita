/* This file is part of the KDE project
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 * Copyright (C) 2000-2005 David Faure <faure@kde.org>
 * Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2010-2012 Boudewijn Rempt <boud@kogmbh.com>
 * Copyright (C) 2011 Inge Wallin <ingwa@kogmbh.com>
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

#include "KoMainWindow.h" // XXX: remove
#include <KMessageBox> // XXX: remove
#include <KNotification> // XXX: remove

#include "KoDocument.h"
#include "KoPart.h"
#include "KoServiceProvider.h"
#include "KoPartAdaptor.h"
#include "KoGlobal.h"
#include "KoEmbeddedDocumentSaver.h"
#include "KoFilterManager.h"
#include "KoDocumentInfo.h"
#ifdef SHOULD_BUILD_RDF
#include "rdf/KoDocumentRdf.h"
#endif
#include "KoOdfStylesReader.h"
#include "KoOdfReadStore.h"
#include "KoOdfWriteStore.h"
#include "KoXmlNS.h"

#include "KoApplication.h"
#include <KoProgressProxy.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>
#include <KoDocumentRdfBase.h>
#include <KoDpi.h>
#include <KoXmlWriter.h>

#include <kmimetype.h>
#include <kfileitem.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kparts/partmanager.h>
#include <ksavefile.h>
#include <kxmlguifactory.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kdesktopfile.h>
#include <kconfiggroup.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <kfileitem.h>
#include <kio/netaccess.h>

#include <QBuffer>
#include <QDir>
#include <QFileInfo>
#include <QPainter>
#include <QTimer>
#include <QtDBus/QDBusConnection>
#include <QApplication>

// Define the protocol used here for embedded documents' URL
// This used to "store" but KUrl didn't like it,
// so let's simply make it "tar" !
#define STORE_PROTOCOL "tar"
// The internal path is a hack to make KUrl happy and for document children
#define INTERNAL_PROTOCOL "intern"
#define INTERNAL_PREFIX "intern:/"
// Warning, keep it sync in koStore.cc
#include <kactioncollection.h>
#include "KoUndoStackAction.h"

using namespace std;

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
            progressProxy(0),
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
            storeInternal(false),
            isLoading(false),
            undoStack(0),
            parentPart(0)

    {
        confirmNonNativeSave[0] = true;
        confirmNonNativeSave[1] = true;
        if (KGlobal::locale()->measureSystem() == KLocale::Imperial) {
            unit = KoUnit::Inch;
        } else {
            unit = KoUnit::Centimeter;
        }
    }


    KoDocumentInfo *docInfo;
#ifdef SHOULD_BUILD_RDF
    KoDocumentRdf *docRdf;
#else
    KoDocumentRdfBase *docRdf;
#endif
    KoProgressUpdater *progressUpdater;
    KoProgressProxy *progressProxy;
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
    bool autosaving;
    bool shouldCheckAutoSaveFile; // usually true
    bool autoErrorHandlingEnabled; // usually true
    bool backupFile;
    QString backupPath;
    bool doNotSaveExtDoc; // makes it possible to save only internally stored child documents
    bool storeInternal; // Store this doc internally even if url is external
    bool isLoading; // True while loading (openUrl is async)

    QList<KoVersionInfo> versionInfo;

    KUndo2Stack *undoStack;

    KoGridData gridData;
    KoGuidesData guidesData;

    KService::Ptr nativeService;

    bool isEmpty;

    KoPageLayout pageLayout;

    KoPart *parentPart;

};

KoDocument::KoDocument(KoPart *parent, KUndo2Stack *undoStack)
        : d(new Private)
{
    d->parentPart = parent;

    d->isEmpty = true;
    connect(&d->autoSaveTimer, SIGNAL(timeout()), this, SLOT(slotAutoSave()));
    setAutoSave(defaultAutoSave());

    setObjectName(newObjectName());
    d->docInfo = new KoDocumentInfo(this);
    d->docRdf = 0;
#ifdef SHOULD_BUILD_RDF
    {
        KConfigGroup cfgGrp(d->parentPart->componentData().config(), "RDF");
        bool rdfEnabled = cfgGrp.readEntry("rdf_enabled", false);
        if (rdfEnabled) {
            setDocumentRdf(new KoDocumentRdf(this));
        }
    }
#endif

    d->pageLayout.width = 0;
    d->pageLayout.height = 0;
    d->pageLayout.topMargin = 0;
    d->pageLayout.bottomMargin = 0;
    d->pageLayout.leftMargin = 0;
    d->pageLayout.rightMargin = 0;

    d->undoStack = undoStack;
    d->undoStack->setParent(this);

    KConfigGroup cfgGrp(d->parentPart->componentData().config(), "Undo");
    d->undoStack->setUndoLimit(cfgGrp.readEntry("UndoLimit", 1000));

    connect(d->undoStack, SIGNAL(cleanChanged(bool)), this, SLOT(setDocumentClean(bool)));

}

KoDocument::~KoDocument()
{
    d->autoSaveTimer.stop();
    delete d->filterManager;
    delete d;
}


KoPart *KoDocument::documentPart()
{
    return d->parentPart;
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
    KUrl oldURL = d->parentPart->url();
    QString oldFile = d->parentPart->localFilePath();

    bool wasModified = isModified();
    QByteArray oldMimeType = mimeType();

    // save...
    ret = d->parentPart->saveAs(_url);


    //
    // This is sooooo hacky :(
    // Hopefully we will restore enough state.
    //
    kDebug(30003) << "Restoring KoDocument state to before export";

    // always restore url & m_file because KParts has changed them
    // (regardless of failure or success)
    d->parentPart->setUrl(oldURL);
    d->parentPart->setLocalFilePath(oldFile);

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
    kDebug(30003) << "doc=" << d->parentPart->url().url();

    // Save it to be able to restore it after a failed save
    const bool wasModified = isModified();

    // The output format is set by koMainWindow, and by openFile
    QByteArray outputMimeType = d->outputMimeType;
    if (outputMimeType.isEmpty())
        outputMimeType = d->outputMimeType = nativeFormatMimeType();

    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (backupFile()) {
        if (d->parentPart->url().isLocalFile())
            KSaveFile::backupFile(d->parentPart->url().toLocalFile(), d->backupPath);
        else {
            KIO::UDSEntry entry;
            if (KIO::NetAccess::stat(d->parentPart->url(),
                                     entry,
                                     d->parentPart->currentShell())) {     // this file exists => backup
                emit statusBarMessage(i18n("Making backup..."));
                KUrl backup;
                if (d->backupPath.isEmpty())
                    backup = d->parentPart->url();
                else
                    backup = d->backupPath + '/' + d->parentPart->url().fileName();
                backup.setPath(backup.path() + QString::fromLatin1("~"));
                KFileItem item(entry, d->parentPart->url());
                Q_ASSERT(item.name() == d->parentPart->url().fileName());
                KIO::FileCopyJob *job = KIO::file_copy(d->parentPart->url(), backup, item.permissions(), KIO::Overwrite | KIO::HideProgressInfo);
                job->exec();
            }
        }
    }

    emit statusBarMessage(i18n("Saving..."));
    bool ret = false;
    bool suppressErrorDialog = false;
    if (!isNativeFormat(outputMimeType, ForExport)) {
        kDebug(30003) << "Saving to format" << outputMimeType << "in" << d->parentPart->localFilePath();
        // Not native format : save using export filter
        if (!d->filterManager)
            d->filterManager = new KoFilterManager(this);

        KoFilter::ConversionStatus status = d->filterManager->exportDocument(d->parentPart->localFilePath(), outputMimeType);
        ret = status == KoFilter::OK;
        suppressErrorDialog = (status == KoFilter::UserCancelled || status == KoFilter::BadConversionGraph);
    } else {
        // Native format => normal save
        Q_ASSERT(!d->parentPart->localFilePath().isEmpty());
        ret = saveNativeFormat(d->parentPart->localFilePath());
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
            d->parentPart->showSavingErrorDialog();
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
        notify->setText(i18n("Document <i>%1</i> saved", d->parentPart->url().url()));
        notify->addContext("url", d->parentPart->url().url());
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

bool KoDocument::saveInBatchMode() const
{
    if (d->filterManager) {
        return d->filterManager->getBatchMode();
    }
    return true;
}

void KoDocument::setSaveInBatchMode(const bool batchMode)
{
    if (!d->filterManager) {
        d->filterManager = new KoFilterManager(this, d->progressUpdater);
    }
    d->filterManager->setBatchMode(batchMode);
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
    if (isModified() && d->modifiedAfterAutosave && !d->isLoading) {
        // Give a warning when trying to autosave an encrypted file when no password is known (should not happen)
        if (d->specialOutputFlag == SaveEncrypted && d->password.isNull()) {
            // That advice should also fix this error from occurring again
            emit statusBarMessage(i18n("The password of this encrypted document is not known. Autosave aborted! Please save your work manually."));
        } else {
            connect(this, SIGNAL(sigProgress(int)), d->parentPart->currentShell(), SLOT(slotProgress(int)));
            emit statusBarMessage(i18n("Autosaving..."));
            d->autosaving = true;
            bool ret = saveNativeFormat(autoSaveFile(d->parentPart->localFilePath()));
            setModified(true);
            if (ret) {
                d->modifiedAfterAutosave = false;
                d->autoSaveTimer.stop(); // until the next change
            }
            d->autosaving = false;
            emit clearStatusBarMessage();
            disconnect(this, SIGNAL(sigProgress(int)), d->parentPart->currentShell(), SLOT(slotProgress(int)));
            if (!ret) {
                emit statusBarMessage(i18n("Error during autosave! Partition full?"));
            }
        }
    }
}

void KoDocument::setReadWrite(bool readwrite)
{
    d->parentPart->setReadWrite(readwrite);
    setAutoSave(d->autoSaveDelay);
}

void KoDocument::setAutoSave(int delay)
{
    d->autoSaveDelay = delay;
    if (d->parentPart->isReadWrite() && d->autoSaveDelay > 0)
        d->autoSaveTimer.start(d->autoSaveDelay * 1000);
    else
        d->autoSaveTimer.stop();
}

KoDocumentInfo *KoDocument::documentInfo() const
{
    return d->docInfo;
}

KoDocumentRdfBase *KoDocument::documentRdf() const
{
#ifdef SHOULD_BUILD_RDF
    if (d->docRdf && d->docRdf->model()) {
        return d->docRdf;
    }
#endif
    return 0;
}

void KoDocument::setDocumentRdf(KoDocumentRdf *rdfDocument)
{
    delete d->docRdf;
    d->docRdf = 0;
#ifdef SHOULD_BUILD_RDF
    if (rdfDocument->model()) {
        d->docRdf = rdfDocument;
    }
#endif
}

KoDocumentRdfBase *KoDocument::documentRdfBase() const
{
    return d->docRdf;
}


bool KoDocument::isModified() const
{
    if (d->parentPart->isModified()) {
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
    kDebug(30003) << "KoDocument::savingTo mimeType=" << mimeType;
    QByteArray nativeOasisMime = nativeOasisMimeType();
    bool oasis = !mimeType.isEmpty() && (mimeType == nativeOasisMime || mimeType == nativeOasisMime + "-template" || mimeType.startsWith("application/vnd.oasis.opendocument"));

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
        return saveNativeFormatCalligra(store);
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
        odfStore.closeManifestWriter(false);
        delete store;
        return false;
    }

    // Save embedded objects
    if (!embeddedSaver.saveEmbeddedDocuments(documentContext)) {
        kDebug(30003) << "save embedded documents failed";
        odfStore.closeManifestWriter(false);
        delete store;
        return false;
    }

    if (store->open("meta.xml")) {
        if (!d->docInfo->saveOasis(store) || !store->close()) {
            odfStore.closeManifestWriter(false);
            delete store;
            return false;
        }
        manifestWriter->addManifestEntry("meta.xml", "text/xml");
    } else {
        d->lastErrorMessage = i18n("Not able to write '%1'. Partition full?", QString("meta.xml"));
        odfStore.closeManifestWriter(false);
        delete store;
        return false;
    }

    if (d->docRdf && !d->docRdf->saveOasis(store, manifestWriter)) {
        d->lastErrorMessage = i18n("Not able to write RDF metadata. Partition full?");
        odfStore.closeManifestWriter(false);
        delete store;
        return false;
    }

    if (store->open("Thumbnails/thumbnail.png")) {
        if (!saveOasisPreview(store, manifestWriter) || !store->close()) {
            d->lastErrorMessage = i18n("Error while trying to write '%1'. Partition full?", QString("Thumbnails/thumbnail.png"));
            odfStore.closeManifestWriter(false);
            delete store;
            return false;
        }
        // No manifest entry!
    } else {
        d->lastErrorMessage = i18n("Not able to write '%1'. Partition full?", QString("Thumbnails/thumbnail.png"));
        odfStore.closeManifestWriter(false);
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
            odfStore.closeManifestWriter(false);
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

bool KoDocument::saveNativeFormatCalligra(KoStore *store)
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
    kDebug(30003) << "Saving done of url:" << d->parentPart->url().url();
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

QString KoDocument::checkImageMimeTypes(const QString &mimeType, const KUrl &url) const
{
    if (!url.isLocalFile()) return mimeType;

    QStringList imageMimeTypes;
    imageMimeTypes << "image/jpeg"
                   << "image/x-psd" << "image/photoshop" << "image/x-photoshop" << "image/x-vnd.adobe.photoshop" << "image/vnd.adobe.photoshop"
                   << "image/x-portable-pixmap" << "image/x-portable-graymap" << "image/x-portable-bitmap"
                   << "application/pdf"
                   << "image/x-exr"
                   << "image/x-xcf"
                   << "image/x-eps"
                   << "image/png"
                   << "image/bmp" << "image/x-xpixmap" << "image/gif" << "image/x-xbitmap"
                   << "image/tiff"
                   << "image/openraster"
                   << "image/jp2";

    if (!imageMimeTypes.contains(mimeType)) return mimeType;

    int accuracy = 0;

    QFile f(url.toLocalFile());
    f.open(QIODevice::ReadOnly);
    QByteArray ba = f.read(qMin(f.size(), (qint64)512)); // should be enough for images
    KMimeType::Ptr mime = KMimeType::findByContent(ba, &accuracy);
    f.close();

    return mime->name();
}

// Called for embedded documents
bool KoDocument::saveToStore(KoStore *_store, const QString & _path)
{
    kDebug(30003) << "Saving document to store" << _path;

    // Use the path as the internal url
    if (_path.startsWith(STORE_PROTOCOL))
        d->parentPart->setUrl(KUrl(_path));
    else // ugly hack to pass a relative URI
        d->parentPart->setUrl(KUrl(INTERNAL_PREFIX +  _path));

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
    const QImage preview(pix.toImage().convertToFormat(QImage::Format_ARGB32, Qt::ColorOnly));
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
    paintContent(p, rc);
    p.end();

    return pix.scaled(QSize(previewWidth, previewHeight), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

QString KoDocument::autoSaveFile(const QString & path) const
{
    QString retval;

    // Using the extension allows to avoid relying on the mime magic when opening
    KMimeType::Ptr mime = KMimeType::mimeType(nativeFormatMimeType());
    if (! mime) {
        qFatal("It seems your installation is broken/incomplete cause we failed to load the native mimetype \"%s\".", nativeFormatMimeType().constData());
    }
    QString extension = mime->property("X-KDE-NativeExtension").toString();
    if (extension.isEmpty()) extension = mime->mainExtension();

    if (path.isEmpty()) {
        // Never saved? Use a temp file in $HOME then. Mark it with the pid so two instances don't overwrite each other's autosave file
        retval = QString("%1/.%2-%3-%4-autosave%5").arg(QDir::homePath()).arg(d->parentPart->componentData().componentName()).arg(kapp->applicationPid()).arg(objectName()).arg(extension);
    } else {
        KUrl url = KUrl::fromPath(path);
        Q_ASSERT(url.isLocalFile());
        QString dir = url.directory(KUrl::AppendTrailingSlash);
        QString filename = url.fileName();
        retval = QString("%1.%2-autosave%3").arg(dir).arg(filename).arg(extension);
    }
    return retval;
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

// XXX going to go to KoPart...
bool KoDocument::openUrl(const KUrl & _url)
{
    kDebug(30003) << "url=" << _url.url();
    d->lastErrorMessage.clear();

    // Reimplemented, to add a check for autosave files and to improve error reporting
    if (!_url.isValid()) {
        d->lastErrorMessage = i18n("Malformed URL\n%1", _url.url());  // ## used anywhere ?
        return false;
    }
    // XXX
    //d->parentPart->abortLoad();

    KUrl url(_url);
    bool autosaveOpened = false;
    d->isLoading = true;
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
                d->isLoading = false;
                return false;
            }
        }
    }

    bool ret = d->parentPart->openUrl(url);

    if (autosaveOpened) {
        resetURL(); // Force save to act like 'Save As'
        setReadWrite(true); // enable save button
        QFile::remove(url.toLocalFile()); // and remove the autosave file
    }
    else {
        if (d->parentPart) {
            d->parentPart->addRecentURLToAllShells(_url);
        }
        if (ret) {
            // Detect readonly local-files; remote files are assumed to be writable, unless we add a KIO::stat here (async).
            KFileItem file(url, mimeType(), KFileItem::Unknown);
            setReadWrite(file.isWritable());
        }
    }
    return ret;
}

// It seems that people have started to save .docx files as .doc and
// similar for xls and ppt.  So let's make a small replacement table
// here and see if we can open the files anyway.
static struct MimetypeReplacement {
    const char *typeFromName;         // If the mime type from the name is this...
    const char *typeFromContents;     // ...and findByFileContents() reports this type...
    const char *useThisType;          // ...then use this type for real.
} replacementMimetypes[] = {
    // doc / docx
    {
        "application/msword",
        "application/vnd.openxmlformats-officedocument.wordprocessingml.document",
        "application/vnd.openxmlformats-officedocument.wordprocessingml.document"
    },
    {
        "application/msword",
        "application/zip",
        "application/vnd.openxmlformats-officedocument.wordprocessingml.document"
    },
    {
        "application/vnd.openxmlformats-officedocument.wordprocessingml.document",
        "application/msword",
        "application/msword"
    },
    {
        "application/vnd.openxmlformats-officedocument.wordprocessingml.document",
        "application/x-ole-storage",
        "application/msword"
    },

    // xls / xlsx
    {
        "application/vnd.ms-excel",
        "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",
        "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"
    },
    {
        "application/vnd.ms-excel",
        "application/zip",
        "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"
    },
    {
        "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",
        "application/vnd.ms-excel",
        "application/vnd.ms-excel"
    },
    {
        "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",
        "application/x-ole-storage",
        "application/vnd.ms-excel"
    },

    // ppt / pptx
    {
        "application/vnd.ms-powerpoint",
        "application/vnd.openxmlformats-officedocument.presentationml.presentation",
        "application/vnd.openxmlformats-officedocument.presentationml.presentation"
    },
    {
        "application/vnd.ms-powerpoint",
        "application/zip",
        "application/vnd.openxmlformats-officedocument.presentationml.presentation"
    },
    {
        "application/vnd.openxmlformats-officedocument.presentationml.presentation",
        "application/vnd.ms-powerpoint",
        "application/vnd.ms-powerpoint"
    },
    {
        "application/vnd.openxmlformats-officedocument.presentationml.presentation",
        "application/x-ole-storage",
        "application/vnd.ms-powerpoint"
    }
};

bool KoDocument::openFile()
{
    //kDebug(30003) <<"for" << localFilePath();
    if (!QFile::exists(d->parentPart->localFilePath())) {
        QApplication::restoreOverrideCursor();
        if (d->autoErrorHandlingEnabled)
            // Maybe offer to create a new document with that name ?
            KMessageBox::error(0, i18n("The file %1 does not exist.", d->parentPart->localFilePath()));
        d->isLoading = false;
        return false;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    d->specialOutputFlag = 0;
    QByteArray _native_format = nativeFormatMimeType();

    KUrl u(d->parentPart->localFilePath());
    QString typeName = d->parentPart->arguments().mimeType();

    if (typeName.isEmpty()) {
        typeName = KMimeType::findByUrl(u, 0, true)->name();
    }

    // for images, always check content.
    typeName = checkImageMimeTypes(typeName, u);

    // Sometimes it seems that arguments().mimeType() contains a much
    // too generic mime type.  In that case, let's try some educated
    // guesses based on what we know about file extension.
    //
    // FIXME: Should we just ignore this and always call
    //        KMimeType::findByUrl()? David Faure says that it's
    //        impossible for findByUrl() to fail to initiate the
    //        mimetype for "*.doc" to application/msword.  This hints
    //        that we should do that.  But why does it happen like
    //        this at all?
    if (typeName == "application/zip") {
        QString filename = u.fileName();

        // None of doc, xls or ppt are really zip files.  But docx,
        // xlsx and pptx are.  This miscategorization seems to only
        // crop up when there is a, say, docx file saved as doc.  The
        // conversion to the docx mimetype will happen below.
        if (filename.endsWith(".doc"))
            typeName = "application/msword";
        else if (filename.endsWith(".xls"))
            typeName = "application/vnd.ms-excel";
        else if (filename.endsWith(".ppt"))
            typeName = "application/vnd.ms-powerpoint";

        // Potentially more guesses here...
    } else if (typeName == "application/x-ole-storage") {
        QString filename = u.fileName();

        // None of docx, xlsx or pptx are really OLE files.  But doc,
        // xls and ppt are.  This miscategorization seems to only crop
        // up when there is a, say, doc file saved as docx.  The
        // conversion to the doc mimetype will happen below.
        if (filename.endsWith(".docx"))
            typeName = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
        else if (filename.endsWith(".xlsx"))
            typeName = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
        else if (filename.endsWith(".pptx"))
            typeName = "application/vnd.openxmlformats-officedocument.presentationml.presentation";

        // Potentially more guesses here...
    }
    //kDebug(30003) << "mimetypes 3:" << typeName;

    // In some cases docx files are saved as doc and similar.  We have
    // a small hardcoded table for those cases.  Check if this is
    // applicable here.
    for (uint i = 0; i < sizeof(replacementMimetypes) / sizeof(struct MimetypeReplacement); ++i) {
        struct MimetypeReplacement *replacement = &replacementMimetypes[i];

        if (typeName == replacement->typeFromName) {
            //kDebug(30003) << "found potential replacement target:" << typeName;
            int accuracy;
            QString typeFromContents = KMimeType::findByFileContent(u.path(), &accuracy)->name();
            //kDebug(30003) << "found potential replacement:" << typeFromContents;
            if (typeFromContents == replacement->typeFromContents) {
                typeName = replacement->useThisType;
                //kDebug(30003) << "So really use this:" << typeName;
                break;
            }
        }
    }
    //kDebug(30003) << "mimetypes 4:" << typeName;

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
        kDebug(30003) << "loading" << u.fileName() << ", using directory store for" << d->parentPart->localFilePath() << "; typeName=" << typeName;
    }
    kDebug(30003) << d->parentPart->localFilePath() << "type:" << typeName;

    QString importedFile = d->parentPart->localFilePath();

    // create the main progress monitoring object for loading, this can
    // contain subtasks for filtering and loading
    KoProgressProxy *progressProxy = 0;
    if (d->progressProxy) {
        progressProxy = d->progressProxy;
    }

    d->progressUpdater = new KoProgressUpdater(progressProxy,
            KoProgressUpdater::Unthreaded,
            d->profileStream);

    d->progressUpdater->setReferenceTime(d->profileReferenceTime);
    d->progressUpdater->start();

    setupOpenFileSubProgress();

    if (!isNativeFormat(typeName.toLatin1(), ForImport)) {
        if (!d->filterManager)
            d->filterManager = new KoFilterManager(this, d->progressUpdater);
        KoFilter::ConversionStatus status;
        importedFile = d->filterManager->importDocument(d->parentPart->localFilePath(), typeName, status);
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

            case KoFilter::InvalidFormat:
                msg = i18n("Invalid file format"); break;

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

            d->isLoading = false;
            delete d->progressUpdater;
            d->progressUpdater = 0;
            return false;
        }
        d->isEmpty = false;
        kDebug(30003) << "importedFile" << importedFile << "status:" << static_cast<int>(status);
    }

    QApplication::restoreOverrideCursor();

    bool ok = true;

    if (!importedFile.isEmpty()) { // Something to load (tmp or native file) ?
        // The filter, if any, has been applied. It's all native format now.
        if (!loadNativeFormat(importedFile)) {
            ok = false;
            if (d->autoErrorHandlingEnabled) {
                d->parentPart->showLoadingErrorDialog();
            }
        }
    }

    if (importedFile != d->parentPart->localFilePath()) {
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
            if (!getenv("CALLIGRA_DEBUG_FILTERS"))
#endif
            QFile::remove(importedFile);
        }
    }

    if (ok) {
        setMimeTypeAfterLoading(typeName);

        KNotification *notify = new KNotification("DocumentLoaded");
        notify->setText(i18n("Document <i>%1</i> loaded", d->parentPart->url().url()));
        notify->addContext("url", d->parentPart->url().url());
        QTimer::singleShot(0, notify, SLOT(sendEvent()));
        d->parentPart->deleteOpenPane();
    }

    if (progressUpdater()) {
        QPointer<KoUpdater> updater
                = progressUpdater()->startSubtask(1, "clear undo stack");
        updater->setProgress(0);
        undoStack()->clear();
        updater->setProgress(100);
    }
    delete d->progressUpdater;
    d->progressUpdater = 0;

    d->isLoading = false;

    return ok;
}

KoProgressUpdater *KoDocument::progressUpdater() const
{
    return d->progressUpdater;
}

void KoDocument::setProgressProxy(KoProgressProxy *progressProxy)
{
    d->progressProxy = progressProxy;
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
        KoXmlDocument doc = KoXmlDocument(true);
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
        d->isEmpty = false;
        return res;
    } else { // It's a calligra store (tar.gz, zip, directory, etc.)
        in.close();

        return loadNativeFormatFromStore(file);
    }
}

bool KoDocument::loadNativeFormatFromStore(const QString& file)
{
    KoStore::Backend backend = (d->specialOutputFlag == SaveAsDirectoryStore) ? KoStore::Directory : KoStore::Auto;
    KoStore *store = KoStore::createStore(file, KoStore::Read, "", backend);

    if (store->bad()) {
        d->lastErrorMessage = i18n("Not a valid Calligra file: %1", file);
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

    if (store->bad()) {
        delete store;
        return false;
    }

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

        KoXmlDocument doc = KoXmlDocument(true);
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
        KoXmlDocument doc = KoXmlDocument(true);
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
    d->isEmpty = false;
    return res;
}

// For embedded documents
bool KoDocument::loadFromStore(KoStore *_store, const QString& url)
{
    if (_store->open(url)) {
        KoXmlDocument doc = KoXmlDocument(true);
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
        d->parentPart->setUrl(url);
    } else {
        d->parentPart->setUrl(KUrl(INTERNAL_PREFIX + url));
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

    d->parentPart->save(); //finally save the document + the new version
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

    d->parentPart->setModified(mod);

    if (mod) {
        d->isEmpty = false;
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
        name = d->parentPart->url().fileName();

    if (name.isEmpty())
        name = i18n("Untitled");

    int res = KMessageBox::warningYesNoCancel(0,
              i18n("<p>The document <b>'%1'</b> has been modified.</p><p>Do you want to save it?</p>", name));

    switch (res) {
    case KMessageBox::Yes :
        d->parentPart->save(); // NOTE: External files always in native format. ###TODO: Handle non-native format
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

QString KoDocument::prettyPathOrUrl() const
{
    QString url( d->parentPart->url().pathOrUrl() );
#ifdef Q_WS_WIN
    if (d->parentPart->url().isLocalFile()) {
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
    const QString url(d->parentPart->url().fileName());
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
    emit titleModified(caption(), isModified());
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
    return createDomDocument(d->parentPart->componentData().componentName(), tagName, version);
}

//static
QDomDocument KoDocument::createDomDocument(const QString& appName, const QString& tagName, const QString& version)
{
    QDomImplementation impl;
    QString url = QString("http://www.calligra.org/DTD/%1-%2.dtd").arg(appName).arg(version);
    QDomDocumentType dtype = impl.createDocumentType(tagName,
                             QString("-//KDE//DTD %1 %2//EN").arg(appName).arg(version),
                             url);
    // The namespace URN doesn't need to include the version number.
    QString namespaceURN = QString("http://www.calligra.org/DTD/%1").arg(appName);
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
        d->nativeService = KoServiceProvider::readNativeService(d->parentPart->componentData());

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
        if (!service->serviceTypes().contains("CalligraPart"))
            kWarning(30003) << "Wrong desktop file, CalligraPart isn't mentioned";
        else if (!KServiceType::serviceType("CalligraPart"))
            kWarning(30003) << "The CalligraPart service type isn't installed!";
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
    // This implementation is temporary while we treat both calligra-1.3 and OASIS formats as native.
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

void KoDocument::setErrorMessage(const QString& errMsg)
{
    d->lastErrorMessage = errMsg;
}

QString KoDocument::errorMessage() const
{
    return d->lastErrorMessage;
}

bool KoDocument::isAutosaving() const
{
    return d->autosaving;
}

bool KoDocument::isLoading() const
{
    return d->isLoading;
}

void KoDocument::removeAutoSaveFiles()
{
    // Eliminate any auto-save file
    QString asf = autoSaveFile(d->parentPart->localFilePath());   // the one in the current dir
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
    return    !d->parentPart->url().protocol().isEmpty()
            && d->parentPart->url().protocol() != STORE_PROTOCOL
            && d->parentPart->url().protocol() != INTERNAL_PROTOCOL;
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
    settingsWriter->addConfigItem("unit", unit().symbol());
}


void KoDocument::initEmpty()
{
    setEmpty();
    setModified(false);
}

QList<KoVersionInfo> & KoDocument::versionList()
{
    return d->versionInfo;
}

KUndo2Stack *KoDocument::undoStack()
{
    return d->undoStack;
}

void KoDocument::addCommand(KUndo2Command *command)
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

bool KoDocument::isEmpty() const
{
    return d->isEmpty;
}

void KoDocument::setEmpty()
{
    d->isEmpty = true;
}


// static
int KoDocument::defaultAutoSave()
{
    return 300;
}

void KoDocument::resetURL() {
    d->parentPart->setUrl(KUrl());
    d->parentPart->setLocalFilePath(QString());
}

int KoDocument::pageCount() const {
    return 1;
}

void KoDocument::setupOpenFileSubProgress() {}

void KoDocument::setModified() {
    d->parentPart->setModified();
}


QString KoDocument::localFilePath() const
{
    return d->parentPart->localFilePath();
}

KUrl KoDocument::url() const {
    return d->parentPart->url();
}

void KoDocument::setUrl(const KUrl& url) {
    d->parentPart->setUrl(url);
}

#include <KoDocument.moc>

