/* This file is part of the Krita project
 *
 * Copyright (C) 2014 Boudewijn Rempt <boud@kogmbh.com>
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

#include "KisMainWindow.h" // XXX: remove
#include <QMessageBox> // XXX: remove

#include <KisMimeDatabase.h>

#include <KoCanvasBase.h>
#include <KoColor.h>
#include <KoColorProfile.h>
#include <KoColorSpaceEngine.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoDocumentInfoDlg.h>
#include <KoDocumentInfo.h>
#include <KoDpi.h>
#include <KoUnit.h>
#include <KoFileDialog.h>
#include <KoID.h>
#include <KoOdfReadStore.h>
#include <KoProgressProxy.h>
#include <KoProgressUpdater.h>
#include <KoSelection.h>
#include <KoShape.h>
#include <KoShapeController.h>
#include <KoStore.h>
#include <KoUpdater.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <KoStoreDevice.h>

#include <klocalizedstring.h>
#include <kis_debug.h>
#include <kdesktopfile.h>
#include <kconfiggroup.h>
#include <QTemporaryFile>
#include <kbackup.h>

#include <QApplication>
#include <QBuffer>
#include <QDesktopServices>
#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <QFileInfo>
#include <QImage>
#include <QList>
#include <QPainter>
#include <QRect>
#include <QScopedPointer>
#include <QSize>
#include <QStringList>
#include <QtGlobal>
#include <QTimer>
#include <QWidget>

// Krita Image
#include <kis_config.h>
#include <flake/kis_shape_layer.h>
#include <kis_debug.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_name_server.h>
#include <kis_paint_layer.h>
#include <kis_painter.h>
#include <kis_selection.h>
#include <kis_fill_painter.h>
#include <kis_document_undo_store.h>
#include <kis_painting_assistants_decoration.h>
#include <kis_idle_watcher.h>
#include <kis_signal_auto_connection.h>
#include <kis_debug.h>

// Local
#include "KisViewManager.h"
#include "kis_clipboard.h"
#include "widgets/kis_custom_image_widget.h"
#include "canvas/kis_canvas2.h"
#include "flake/kis_shape_controller.h"
#include "kis_statusbar.h"
#include "widgets/kis_progress_widget.h"
#include "kis_canvas_resource_provider.h"
#include "kis_resource_server_provider.h"
#include "kis_node_manager.h"
#include "KisPart.h"
#include "KisApplication.h"
#include "KisDocument.h"
#include "KisImportExportManager.h"
#include "KisPart.h"
#include "KisView.h"
#include "kis_grid_config.h"
#include "kis_guides_config.h"
#include "kis_image_barrier_lock_adapter.h"
#include <mutex>


// Define the protocol used here for embedded documents' URL
// This used to "store" but QUrl didn't like it,
// so let's simply make it "tar" !
#define STORE_PROTOCOL "tar"
// The internal path is a hack to make QUrl happy and for document children
#define INTERNAL_PROTOCOL "intern"
#define INTERNAL_PREFIX "intern:/"
// Warning, keep it sync in koStore.cc

#include <unistd.h>

using namespace std;

/**********************************************************
 *
 * KisDocument
 *
 **********************************************************/

namespace {

class DocumentProgressProxy : public KoProgressProxy {
public:
    KisMainWindow *m_mainWindow;
    DocumentProgressProxy(KisMainWindow *mainWindow)
        : m_mainWindow(mainWindow)
    {
    }

    ~DocumentProgressProxy() override {
        // signal that the job is done
        setValue(-1);
    }

    int maximum() const override {
        return 100;
    }

    void setValue(int value) override {
        if (m_mainWindow) {
            m_mainWindow->slotProgress(value);
        }
    }

    void setRange(int /*minimum*/, int /*maximum*/) override {

    }

    void setFormat(const QString &/*format*/) override {

    }
};
}

//static
QString KisDocument::newObjectName()
{
    static int s_docIFNumber = 0;
    QString name; name.setNum(s_docIFNumber++); name.prepend("document_");
    return name;
}


class UndoStack : public KUndo2Stack
{
public:
    UndoStack(KisDocument *doc)
        : m_doc(doc)
    {
    }

    void setIndex(int idx) override {
        KisImageWSP image = this->image();
        image->requestStrokeCancellation();
        if(image->tryBarrierLock()) {
            KUndo2Stack::setIndex(idx);
            image->unlock();
        }
    }

    void notifySetIndexChangedOneCommand() override {
        KisImageWSP image = this->image();
        image->unlock();

        /**
         * Some very weird commands may emit blocking signals to
         * the GUI (e.g. KisGuiContextCommand). Here is the best thing
         * we can do to avoid the deadlock
         */
        while(!image->tryBarrierLock()) {
            QApplication::processEvents();
        }
    }

    void undo() override {
        KisImageWSP image = this->image();
        image->requestUndoDuringStroke();
        if(image->tryBarrierLock()) {
            KUndo2Stack::undo();
            image->unlock();
        }
    }

    void redo() override {
        KisImageWSP image = this->image();
        if(image->tryBarrierLock()) {
            KUndo2Stack::redo();
            image->unlock();
        }
    }

private:
    KisImageWSP image() {
        KisImageWSP currentImage = m_doc->image();
        Q_ASSERT(currentImage);
        return currentImage;
    }

private:
    KisDocument *m_doc;
};

class Q_DECL_HIDDEN KisDocument::Private
{
public:
    Private() :
        // XXX: the part should _not_ be modified from the document
        docInfo(0),
        progressUpdater(0),
        progressProxy(0),
        importExportManager(0),
        isImporting(false),
        isExporting(false),
        password(QString()),
        modifiedAfterAutosave(false),
        isAutosaving(false),
        backupFile(true),
        backupPath(QString()),
        doNotSaveExtDoc(false),
        storeInternal(false),
        isLoading(false),
        undoStack(0),
        m_saveOk(false),
        m_waitForSave(false),
        m_duringSaveAs(false),
        m_bAutoDetectedMime(false),
        modified(false),
        readwrite(true),
        disregardAutosaveFailure(false),
        nserver(0),
        macroNestDepth(0),
        imageIdleWatcher(2000 /*ms*/),
        suppressProgress(false),
        fileProgressProxy(0)
    {
        if (QLocale().measurementSystem() == QLocale::ImperialSystem) {
            unit = KoUnit::Inch;
        } else {
            unit = KoUnit::Centimeter;
        }
    }

    ~Private() {
        // Don't delete m_d->shapeController because it's in a QObject hierarchy.
        delete nserver;
    }

    KoDocumentInfo *docInfo;

    KoProgressUpdater *progressUpdater;
    KoProgressProxy *progressProxy;

    KoUnit unit;

    KisImportExportManager *importExportManager; // The filter-manager to use when loading/saving [for the options]

    QByteArray mimeType; // The actual mimetype of the document
    QByteArray outputMimeType; // The mimetype to use when saving

    bool isImporting;
    bool isExporting; // File --> Import/Export vs File --> Open/Save
    QString password; // The password used to encrypt an encrypted document

    QTimer autoSaveTimer;
    QString lastErrorMessage; // see openFile()
    int autoSaveDelay; // in seconds, 0 to disable.
    bool modifiedAfterAutosave;
    bool isAutosaving;
    bool backupFile;
    QString backupPath;
    bool doNotSaveExtDoc; // makes it possible to save only internally stored child documents
    bool storeInternal; // Store this doc internally even if url is external
    bool isLoading; // True while loading (openUrl is async)

    KUndo2Stack *undoStack;

    KisGuidesConfig guidesConfig;

    bool isEmpty;

    KoPageLayout pageLayout;

    QUrl m_originalURL; // for saveAs
    QString m_originalFilePath; // for saveAs
    bool m_saveOk;
    bool m_waitForSave;
    bool m_duringSaveAs;
    bool m_bAutoDetectedMime; // whether the mimetype in the arguments was detected by the part itself
    QUrl m_url; // local url - the one displayed to the user.
    QString m_file; // Local file - the only one the part implementation should deal with.
    QEventLoop m_eventLoop;
    QMutex savingMutex;

    bool modified;
    bool readwrite;

    QDateTime firstMod;
    QDateTime lastMod;

    bool disregardAutosaveFailure;

    KisNameServer *nserver;
    qint32 macroNestDepth;

    KisImageSP image;
    KisNodeSP preActivatedNode;
    KisShapeController* shapeController;
    KoShapeController* koShapeController;
    KisIdleWatcher imageIdleWatcher;
    QScopedPointer<KisSignalAutoConnection> imageIdleConnection;

    bool suppressProgress;
    KoProgressProxy* fileProgressProxy;

    QList<KisPaintingAssistantSP> assistants;
    KisGridConfig gridConfig;

    void setImageAndInitIdleWatcher(KisImageSP _image) {
        image = _image;

        imageIdleWatcher.setTrackedImage(image);

        if (image) {
            imageIdleConnection.reset(
                        new KisSignalAutoConnection(
                            &imageIdleWatcher, SIGNAL(startedIdleMode()),
                            image.data(), SLOT(explicitRegenerateLevelOfDetail())));
        }
    }

    class SafeSavingLocker;
};

class KisDocument::Private::SafeSavingLocker {
public:
    SafeSavingLocker(KisDocument::Private *_d, KisDocument *document)
        : d(_d)
        , m_locked(false)
        , m_imageLock(d->image, true)
        , m_savingLock(&d->savingMutex)
        , m_document(document)
    {
        const int realAutoSaveInterval = KisConfig().autoSaveInterval();
        const int emergencyAutoSaveInterval = 10; // sec

        /**
         * Initial try to lock both objects. Locking the image guards
         * us from any image composition threads running in the
         * background, while savingMutex guards us from entering the
         * saving code twice by autosave and main threads.
         *
         * Since we are trying to lock multiple objects, so we should
         * do it in a safe manner.
         */
        m_locked = std::try_lock(m_imageLock, m_savingLock) < 0;

        if (!m_locked) {
            if (d->isAutosaving) {
                d->disregardAutosaveFailure = true;
                if (realAutoSaveInterval) {
                    document->setAutoSave(emergencyAutoSaveInterval);
                }
            } else {
                d->image->requestStrokeEnd();
                QApplication::processEvents();

                // one more try...
                m_locked = std::try_lock(m_imageLock, m_savingLock) < 0;
            }
        }

        if (m_locked) {
            d->disregardAutosaveFailure = false;
        }
    }

    ~SafeSavingLocker() {
         if (m_locked) {
             m_imageLock.unlock();
             m_savingLock.unlock();

             const int realAutoSaveInterval = KisConfig().autoSaveInterval();
             m_document->setAutoSave(realAutoSaveInterval);
         }
     }

    bool successfullyLocked() const {
        return m_locked;
    }

private:
    KisDocument::Private *d;
    bool m_locked;

    KisImageBarrierLockAdapter m_imageLock;
    StdLockableWrapper<QMutex> m_savingLock;
    KisDocument *m_document;
};

KisDocument::KisDocument()
    : d(new Private())
{
    d->undoStack = new UndoStack(this);
    d->undoStack->setParent(this);

    d->isEmpty = true;
    d->importExportManager = new KisImportExportManager(this);
    d->importExportManager->setProgresUpdater(d->progressUpdater);

    connect(&d->autoSaveTimer, SIGNAL(timeout()), this, SLOT(slotAutoSave()));
    setAutoSave(defaultAutoSave());

    setObjectName(newObjectName());

    d->docInfo = new KoDocumentInfo(this);

    d->pageLayout.width = 0;
    d->pageLayout.height = 0;
    d->pageLayout.topMargin = 0;
    d->pageLayout.bottomMargin = 0;
    d->pageLayout.leftMargin = 0;
    d->pageLayout.rightMargin = 0;

    d->firstMod = QDateTime::currentDateTime();
    d->lastMod = QDateTime::currentDateTime();


    // preload the krita resources
    KisResourceServerProvider::instance();

    d->nserver = new KisNameServer(1);

    d->shapeController = new KisShapeController(this, d->nserver);
    d->koShapeController = new KoShapeController(0, d->shapeController);

    undoStack()->setUndoLimit(KisConfig().undoStackLimit());
    connect(d->undoStack, SIGNAL(indexChanged(int)), this, SLOT(slotUndoStackIndexChanged(int)));
    setBackupFile(KisConfig().backupFile());
}

KisDocument::~KisDocument()
{
    /**
     * Push a timebomb, which will try to release the memory after
     * the document has been deleted
     */
    KisPaintDevice::createMemoryReleaseObject()->deleteLater();

    d->autoSaveTimer.disconnect(this);
    d->autoSaveTimer.stop();

    delete d->importExportManager;

    // Despite being QObject they needs to be deleted before the image
    delete d->shapeController;

    delete d->koShapeController;

    if (d->image) {
        d->image->notifyAboutToBeDeleted();

        /**
         * WARNING: We should wait for all the internal image jobs to
         * finish before entering KisImage's destructor. The problem is,
         * while execution of KisImage::~KisImage, all the weak shared
         * pointers pointing to the image enter an inconsistent
         * state(!). The shared counter is already zero and destruction
         * has started, but the weak reference doesn't know about it,
         * because KisShared::~KisShared hasn't been executed yet. So all
         * the threads running in background and having weak pointers will
         * enter the KisImage's destructor as well.
         */

        d->image->requestStrokeCancellation();
        d->image->waitForDone();

        // clear undo commands that can still point to the image
        d->undoStack->clear();
        d->image->waitForDone();

        KisImageWSP sanityCheckPointer = d->image;
        Q_UNUSED(sanityCheckPointer);
        // The following line trigger the deletion of the image
        d->image.clear();

        // check if the image has actually been deleted
        KIS_SAFE_ASSERT_RECOVER_NOOP(!sanityCheckPointer.isValid());
    }


    delete d;
}

bool KisDocument::reload()
{
    // XXX: reimplement!
    return false;
}

bool KisDocument::exportDocument(const QUrl &_url, KisPropertiesConfigurationSP exportConfiguration)
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
    QUrl oldURL = url();
    QString oldFile = localFilePath();

    bool wasModified = isModified();
    QByteArray oldMimeType = mimeType();

    // save...
    ret = saveAs(_url, exportConfiguration);

    //
    // This is sooooo hacky :(
    // Hopefully we will restore enough state.
    //
    dbgUI << "Restoring KisDocument state to before export";

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

bool KisDocument::saveAs(const QUrl &url, KisPropertiesConfigurationSP exportConfiguration)
{
    if (!url.isValid() || !url.isLocalFile()) {
        errKrita << "saveAs: Malformed URL " << url.url() << endl;
        return false;
    }
    d->m_duringSaveAs = true;
    d->m_originalURL = d->m_url;
    d->m_originalFilePath = d->m_file;
    d->m_url = url; // Store where to upload in saveToURL
    d->m_file = d->m_url.toLocalFile();

    bool result = save(exportConfiguration); // Save local file and upload local file

    if (!result) {
        d->m_url = d->m_originalURL;
        d->m_file = d->m_originalFilePath;
        d->m_duringSaveAs = false;
        d->m_originalURL = QUrl();
        d->m_originalFilePath.clear();
    }

    return result;
}

bool KisDocument::save(KisPropertiesConfigurationSP exportConfiguration)
{
    d->m_saveOk = false;
    if (d->m_file.isEmpty()) { // document was created empty
        d->m_file = d->m_url.toLocalFile();
    }

    updateEditingTime(true);

    setFileProgressProxy();
    setUrl(url());

    bool ok = saveFile(exportConfiguration);

    clearFileProgressProxy();

    if (ok) {
        return saveToUrl();
    }
    else {
        emit canceled(QString());
    }
    return false;
}


bool KisDocument::waitSaveComplete()
{
    return d->m_saveOk;
}



bool KisDocument::saveFile(KisPropertiesConfigurationSP exportConfiguration)
{
    // Unset the error message
    setErrorMessage("");

    // Save it to be able to restore it after a failed save
    const bool wasModified = isModified();

    // The output format is set by koMainWindow, and by openFile
    QByteArray outputMimeType = d->outputMimeType;
    if (outputMimeType.isEmpty())
        outputMimeType = d->outputMimeType = nativeFormatMimeType();

    if (backupFile()) {
        Q_ASSERT(url().isLocalFile());
        KBackup::backupFile(url().toLocalFile(), d->backupPath);
    }

    qApp->processEvents();

    bool ret = false;
    bool suppressErrorDialog = false;
    KisImportExportFilter::ConversionStatus status = KisImportExportFilter::OK;

    setFileProgressUpdater(i18n("Saving Document"));

    QFileInfo fi(localFilePath());
    QString tempororaryFileName;
    {
        QTemporaryFile tf(QDir::tempPath() + "/XXXXXX" + fi.baseName() + "." + fi.completeSuffix());
        tf.open();
        tempororaryFileName = tf.fileName();
    }
    Q_ASSERT(!tempororaryFileName.isEmpty());

    Private::SafeSavingLocker locker(d, this);
    if (locker.successfullyLocked()) {
        status = d->importExportManager->exportDocument(tempororaryFileName, outputMimeType, !d->isExporting , exportConfiguration);
    } else {
        status = KisImportExportFilter::UsageError;
    }

    ret = status == KisImportExportFilter::OK;
    suppressErrorDialog = (status == KisImportExportFilter::UserCancelled || status == KisImportExportFilter::BadConversionGraph);
    dbgFile << "Export status was" << status;

    if (ret) {
        if (!d->isAutosaving && !d->suppressProgress) {
            QPointer<KoUpdater> updater = d->progressUpdater->startSubtask(1, "clear undo stack");
            updater->setProgress(0);
            d->undoStack->setClean();
            updater->setProgress(100);
        } else {
            d->undoStack->setClean();
        }

        QFile tempFile(tempororaryFileName);
        QString s = localFilePath();
        QFile dstFile(s);
        while (QFileInfo(s).exists()) {
            s.append("_");
        }
        bool r;
        if (s != localFilePath()) {
            r = dstFile.rename(s);
            if (!r) {
               setErrorMessage(i18n("Could not rename original file to %1: %2", dstFile.fileName(), dstFile. errorString()));
            }
         }

        if (tempFile.exists()) {
            r = tempFile.copy(localFilePath());
            if (!r) {
                setErrorMessage(i18n("Copying the temporary file failed: %1 to %2: %3", tempFile.fileName(), dstFile.fileName(), tempFile.errorString()));
            }
            else {
                r = tempFile.remove();
                if (!r) {
                    setErrorMessage(i18n("Could not remove temporary file %1: %2", tempFile.fileName(), tempFile.errorString()));
                }
                else if (s != localFilePath()) {
                    r = dstFile.remove();
                    if (!r) {
                        setErrorMessage(i18n("Could not remove saved original file: %1", dstFile.errorString()));
                    }
                }
            }
        }
        else {
            setErrorMessage(i18n("The temporary file %1 is gone before we could copy it!", tempFile.fileName()));
        }

        if (errorMessage().isEmpty()) {
            if (!isAutosaving()) {
                removeAutoSaveFiles();
            }
        }
        else {
            qWarning() << "Error while saving:" << errorMessage();
        }
        // Restart the autosave timer
        // (we don't want to autosave again 2 seconds after a real save)
        if (!isAutosaving()) {
            setAutoSave(d->autoSaveDelay);
        }

        d->mimeType = outputMimeType;
    }
    else {
        if (!suppressErrorDialog) {

            if (errorMessage().isEmpty()) {
                setErrorMessage(KisImportExportFilter::conversionStatusString(status));
            }

            if (errorMessage().isEmpty()) {
                QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("Could not save\n%1", localFilePath()));
            } else {
                QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("Could not save %1\nReason: %2", localFilePath(), errorMessage()));
            }

        }

        // couldn't save file so this new URL is invalid
        // FIXME: we should restore the current document's true URL instead of
        // setting it to nothing otherwise anything that depends on the URL
        // being correct will not work (i.e. the document will be called
        // "Untitled" which may not be true)
        //
        // Update: now the URL is restored in KisMainWindow but really, this
        // should still be fixed in KisDocument/KParts (ditto for file).
        // We still resetURL() here since we may or may not have been called
        // by KisMainWindow - Clarence
        resetURL();

        // As we did not save, restore the "was modified" status
        setModified(wasModified);
    }

    emit sigSavingFinished();
    clearFileProgressUpdater();

    return ret;
}


QByteArray KisDocument::mimeType() const
{
    return d->mimeType;
}

void KisDocument::setMimeType(const QByteArray & mimeType)
{
    d->mimeType = mimeType;
}

void KisDocument::setOutputMimeType(const QByteArray & mimeType)
{
    d->outputMimeType = mimeType;
}

QByteArray KisDocument::outputMimeType() const
{
    return d->outputMimeType;
}

bool KisDocument::fileBatchMode() const
{
    return d->importExportManager->batchMode();
}

void KisDocument::setFileBatchMode(const bool batchMode)
{
    d->importExportManager->setBatchMode(batchMode);
}

bool KisDocument::isImporting() const
{
    return d->isImporting;
}

bool KisDocument::isExporting() const
{
    return d->isExporting;
}

void KisDocument::slotAutoSave()
{
    if (d->modified && d->modifiedAfterAutosave && !d->isLoading) {
        connect(this, SIGNAL(sigProgress(int)), KisPart::instance()->currentMainwindow(), SLOT(slotProgress(int)));
        emit statusBarMessage(i18n("Autosaving..."));
        d->isAutosaving = true;
        QString autoSaveFileName = autoSaveFile(localFilePath());
        bool ret = saveNativeFormat(autoSaveFileName);
        setModified(true);
        if (ret) {
            d->modifiedAfterAutosave = false;
            d->autoSaveTimer.stop(); // until the next change
        }
        d->isAutosaving = false;
        emit clearStatusBarMessage();
        disconnect(this, SIGNAL(sigProgress(int)), KisPart::instance()->currentMainwindow(), SLOT(slotProgress(int)));
        if (!ret && !d->disregardAutosaveFailure) {
            emit statusBarMessage(i18n("Error during autosave! Partition full?"));
        }
    }
}

void KisDocument::setReadWrite(bool readwrite)
{
    d->readwrite = readwrite;
    setAutoSave(d->autoSaveDelay);

    Q_FOREACH (KisMainWindow *mainWindow, KisPart::instance()->mainWindows()) {
        mainWindow->setReadWrite(readwrite);
    }

}

void KisDocument::setAutoSave(int delay)
{
    d->autoSaveDelay = delay;
    if (isReadWrite() && d->autoSaveDelay > 0)
        d->autoSaveTimer.start(d->autoSaveDelay * 1000);
    else
        d->autoSaveTimer.stop();
}

KoDocumentInfo *KisDocument::documentInfo() const
{
    return d->docInfo;
}

bool KisDocument::isModified() const
{
    return d->modified;
}

bool KisDocument::saveNativeFormat(const QString & file)
{
    Q_ASSERT(file.endsWith(".kra") || file.endsWith(".kra~"));
    return exportDocument(QUrl::fromLocalFile(file));
}

QPixmap KisDocument::generatePreview(const QSize& size)
{
    if (d->image) {
        QRect bounds = d->image->bounds();
        QSize newSize = bounds.size();
        newSize.scale(size, Qt::KeepAspectRatio);
        return QPixmap::fromImage(d->image->convertToQImage(newSize, 0));
    }
    return QPixmap(size);
}

QString KisDocument::autoSaveFile(const QString & path) const
{
    QString retval;

    // Using the extension allows to avoid relying on the mime magic when opening
    const QString extension (".kra");

    if (path.isEmpty()) {
        // Never saved?
#ifdef Q_OS_WIN
        // On Windows, use the temp location (https://bugs.kde.org/show_bug.cgi?id=314921)
        retval = QString("%1%2.%3-%4-%5-autosave%6").arg(QDir::tempPath()).arg(QDir::separator()).arg("krita").arg(qApp->applicationPid()).arg(objectName()).arg(extension);
#else
        // On Linux, use a temp file in $HOME then. Mark it with the pid so two instances don't overwrite each other's autosave file
        retval = QString("%1%2.%3-%4-%5-autosave%6").arg(QDir::homePath()).arg(QDir::separator()).arg("krita").arg(qApp->applicationPid()).arg(objectName()).arg(extension);
#endif
    } else {
        QFileInfo fi(path);
        QString dir = fi.absolutePath();
        QString filename = fi.fileName();
        retval = QString("%1%2.%3-autosave%4").arg(dir).arg(QDir::separator()).arg(filename).arg(extension);
    }
    return retval;
}

bool KisDocument::importDocument(const QUrl &_url)
{
    bool ret;

    dbgUI << "url=" << _url.url();
    d->isImporting = true;

    // open...
    ret = openUrl(_url);

    // reset url & m_file (kindly? set by KisParts::openUrl()) to simulate a
    // File --> Import
    if (ret) {
        dbgUI << "success, resetting url";
        resetURL();
        setTitleModified();
    }

    d->isImporting = false;

    return ret;
}


bool KisDocument::openUrl(const QUrl &_url, KisDocument::OpenUrlFlags flags)
{
    if (!_url.isLocalFile()) {
        return false;
    }
    dbgUI << "url=" << _url.url();
    d->lastErrorMessage.clear();

    // Reimplemented, to add a check for autosave files and to improve error reporting
    if (!_url.isValid()) {
        d->lastErrorMessage = i18n("Malformed URL\n%1", _url.url());  // ## used anywhere ?
        return false;
    }

    QUrl url(_url);
    bool autosaveOpened = false;
    d->isLoading = true;
    if (url.isLocalFile() && !fileBatchMode()) {
        QString file = url.toLocalFile();
        QString asf = autoSaveFile(file);
        if (QFile::exists(asf)) {
            KisApplication *kisApp = static_cast<KisApplication*>(qApp);
            kisApp->hideSplashScreen();
            //dbgUI <<"asf=" << asf;
            // ## TODO compare timestamps ?
            int res = QMessageBox::warning(0,
                                           i18nc("@title:window", "Krita"),
                                           i18n("An autosaved file exists for this document.\nDo you want to open it instead?"),
                                           QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);
            switch (res) {
            case QMessageBox::Yes :
                url.setPath(asf);
                autosaveOpened = true;
                break;
            case QMessageBox::No :
                QFile::remove(asf);
                break;
            default: // Cancel
                d->isLoading = false;
                return false;
            }
        }
    }

    bool ret = openUrlInternal(url);

    if (autosaveOpened) {
        resetURL(); // Force save to act like 'Save As'
        setReadWrite(true); // enable save button
        setModified(true);
    }
    else {
        if( !(flags & OPEN_URL_FLAG_DO_NOT_ADD_TO_RECENT_FILES) ) {
            KisPart::instance()->addRecentURLToAllMainWindows(_url);
        }

        if (ret) {
            // Detect readonly local-files; remote files are assumed to be writable
            QFileInfo fi(url.toLocalFile());
            setReadWrite(fi.isWritable());
        }
    }
    return ret;
}

bool KisDocument::openFile()
{
    //dbgUI <<"for" << localFilePath();
    if (!QFile::exists(localFilePath())) {
        QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("File %1 does not exist.", localFilePath()));
        d->isLoading = false;
        return false;
    }

    QString filename = localFilePath();
    QString typeName = mimeType();

    if (typeName.isEmpty()) {
        typeName = KisMimeDatabase::mimeTypeForFile(filename);
    }

    //qDebug() << "mimetypes 4:" << typeName;

    // Allow to open backup files, don't keep the mimetype application/x-trash.
    if (typeName == "application/x-trash") {
        QString path = filename;
        while (path.length() > 0) {
            path.chop(1);
            typeName = KisMimeDatabase::mimeTypeForFile(path);
            //qDebug() << "\t" << path << typeName;
            if (!typeName.isEmpty()) {
                break;
            }
        }
        //qDebug() << "chopped" << filename  << "to" << path << "Was trash, is" << typeName;
    }
    dbgUI << localFilePath() << "type:" << typeName;

    setFileProgressUpdater(i18n("Opening Document"));

    KisImportExportFilter::ConversionStatus status;

    status = d->importExportManager->importDocument(localFilePath(), typeName);
    if (status != KisImportExportFilter::OK) {
        QString msg = KisImportExportFilter::conversionStatusString(status);
        if (!msg.isEmpty()) {
            QString errorMsg(i18n("Could not open %2.\nReason: %1.\n%3", msg, prettyPathOrUrl(), errorMessage()));
            QMessageBox::critical(0, i18nc("@title:window", "Krita"), errorMsg);
        }
        d->isLoading = false;
        clearFileProgressUpdater();
        return false;
    }
    d->isEmpty = false;

    setMimeTypeAfterLoading(typeName);
    emit sigLoadingFinished();

    if (!d->suppressProgress && d->progressUpdater) {
        QPointer<KoUpdater> updater = d->progressUpdater->startSubtask(1, "clear undo stack");
        updater->setProgress(0);
        undoStack()->clear();
        updater->setProgress(100);

        clearFileProgressUpdater();
    } else {
        undoStack()->clear();
    }
    d->isLoading = false;

    return true;
}

KoProgressUpdater *KisDocument::progressUpdater() const
{
    return d->progressUpdater;
}

void KisDocument::setProgressProxy(KoProgressProxy *progressProxy)
{
    d->progressProxy = progressProxy;
}

KoProgressProxy* KisDocument::progressProxy() const
{
    if (!d->progressProxy) {
        KisMainWindow *mainWindow = 0;
        if (KisPart::instance()->mainwindowCount() > 0) {
            mainWindow = KisPart::instance()->mainWindows()[0];
        }
        d->progressProxy = new DocumentProgressProxy(mainWindow);
    }
    return d->progressProxy;
}

// shared between openFile and koMainWindow's "create new empty document" code
void KisDocument::setMimeTypeAfterLoading(const QString& mimeType)
{
    d->mimeType = mimeType.toLatin1();
    d->outputMimeType = d->mimeType;
}


bool KisDocument::loadNativeFormat(const QString & file_)
{
    return openUrl(QUrl::fromLocalFile(file_));
}

bool KisDocument::isStoredExtern() const
{
    return !storeInternal() && hasExternURL();
}


void KisDocument::setModified()
{
    d->modified = true;
}

void KisDocument::setModified(bool mod)
{
    if (mod) {
        updateEditingTime(false);
    }

    if (d->isAutosaving)   // ignore setModified calls due to autosaving
        return;

    if ( !d->readwrite && d->modified ) {
        errKrita << "Can't set a read-only document to 'modified' !" << endl;
        return;
    }

    //dbgUI<<" url:" << url.path();
    //dbgUI<<" mod="<<mod<<" MParts mod="<<KisParts::ReadWritePart::isModified()<<" isModified="<<isModified();

    if (mod && !d->modifiedAfterAutosave) {
        // First change since last autosave -> start the autosave timer
        setAutoSave(d->autoSaveDelay);
    }
    d->modifiedAfterAutosave = mod;

    if (mod == isModified())
        return;

    d->modified = mod;

    if (mod) {
        d->isEmpty = false;
        documentInfo()->updateParameters();
    }

    // This influences the title
    setTitleModified();
    emit modified(mod);
}

void KisDocument::updateEditingTime(bool forceStoreElapsed)
{
    QDateTime now = QDateTime::currentDateTime();
    int firstModDelta = d->firstMod.secsTo(now);
    int lastModDelta = d->lastMod.secsTo(now);

    if (lastModDelta > 30) {
        d->docInfo->setAboutInfo("editing-time", QString::number(d->docInfo->aboutInfo("editing-time").toInt() + d->firstMod.secsTo(d->lastMod)));
        d->firstMod = now;
    } else if (firstModDelta > 60 || forceStoreElapsed) {
        d->docInfo->setAboutInfo("editing-time", QString::number(d->docInfo->aboutInfo("editing-time").toInt() + firstModDelta));
        d->firstMod = now;
    }

    d->lastMod = now;
}

QString KisDocument::prettyPathOrUrl() const
{
    QString _url(url().toDisplayString());
#ifdef Q_OS_WIN
    if (url().isLocalFile()) {
        _url = QDir::toNativeSeparators(_url);
    }
#endif
    return _url;
}

// Get caption from document info (title(), in about page)
QString KisDocument::caption() const
{
    QString c;
    if (documentInfo()) {
        c = documentInfo()->aboutInfo("title");
    }
    const QString _url(url().fileName());
    if (!c.isEmpty() && !_url.isEmpty()) {
        c = QString("%1 - %2").arg(c).arg(_url);
    }
    else if (c.isEmpty()) {
        c = _url; // Fall back to document URL
    }
    return c;
}

void KisDocument::setTitleModified()
{
    emit titleModified(caption(), isModified());
}

QDomDocument KisDocument::createDomDocument(const QString& tagName, const QString& version) const
{
    return createDomDocument("krita", tagName, version);
}

//static
QDomDocument KisDocument::createDomDocument(const QString& appName, const QString& tagName, const QString& version)
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

bool KisDocument::isNativeFormat(const QByteArray& mimetype) const
{
    if (mimetype == nativeFormatMimeType())
        return true;
    return extraNativeMimeTypes().contains(mimetype);
}

void KisDocument::setErrorMessage(const QString& errMsg)
{
    d->lastErrorMessage = errMsg;
}

QString KisDocument::errorMessage() const
{
    return d->lastErrorMessage;
}

void KisDocument::showLoadingErrorDialog()
{
    if (errorMessage().isEmpty()) {
        QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("Could not open\n%1", localFilePath()));
    }
    else {
        QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("Could not open %1\nReason: %2", localFilePath(), errorMessage()));
    }
}

bool KisDocument::isLoading() const
{
    return d->isLoading;
}

void KisDocument::removeAutoSaveFiles()
{
    // Eliminate any auto-save file
    QString asf = autoSaveFile(localFilePath());   // the one in the current dir
    if (QFile::exists(asf))
        QFile::remove(asf);
    asf = autoSaveFile(QString());   // and the one in $HOME
    if (QFile::exists(asf))
        QFile::remove(asf);
}

void KisDocument::setBackupFile(bool _b)
{
    d->backupFile = _b;
}

bool KisDocument::backupFile()const
{
    return d->backupFile;
}


void KisDocument::setBackupPath(const QString & _path)
{
    d->backupPath = _path;
}

QString KisDocument::backupPath()const
{
    return d->backupPath;
}


bool KisDocument::storeInternal() const
{
    return d->storeInternal;
}

void KisDocument::setStoreInternal(bool i)
{
    d->storeInternal = i;
    //dbgUI<<"="<<d->storeInternal<<" doc:"<<url().url();
}

bool KisDocument::hasExternURL() const
{
    return    !url().scheme().isEmpty()
            && url().scheme() != STORE_PROTOCOL
            && url().scheme() != INTERNAL_PROTOCOL;
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

QString KisDocument::tagNameToDocumentType(const QString& localName)
{
    for (unsigned int i = 0 ; i < numTN2DT ; ++i)
        if (localName == TN2DTArray[i].localName)
            return i18n(TN2DTArray[i].documentType);
    return localName;
}

KoPageLayout KisDocument::pageLayout(int /*pageNumber*/) const
{
    return d->pageLayout;
}

void KisDocument::setPageLayout(const KoPageLayout &pageLayout)
{
    d->pageLayout = pageLayout;
}

KoUnit KisDocument::unit() const
{
    return d->unit;
}

void KisDocument::setUnit(const KoUnit &unit)
{
    if (d->unit != unit) {
        d->unit = unit;
        emit unitChanged(unit);
    }
}

KUndo2Stack *KisDocument::undoStack()
{
    return d->undoStack;
}

KisImportExportManager *KisDocument::importExportManager() const
{
    return d->importExportManager;
}

void KisDocument::addCommand(KUndo2Command *command)
{
    if (command)
        d->undoStack->push(command);
}

void KisDocument::beginMacro(const KUndo2MagicString & text)
{
    d->undoStack->beginMacro(text);
}

void KisDocument::endMacro()
{
    d->undoStack->endMacro();
}

void KisDocument::slotUndoStackIndexChanged(int idx)
{
    // even if the document was already modified, call setModified to re-start autosave timer
    setModified(idx != d->undoStack->cleanIndex());
}

void KisDocument::clearUndoHistory()
{
    d->undoStack->clear();
}

KisGridConfig KisDocument::gridConfig() const
{
    return d->gridConfig;
}

void KisDocument::setGridConfig(const KisGridConfig &config)
{
    d->gridConfig = config;
}

const KisGuidesConfig& KisDocument::guidesConfig() const
{
    return d->guidesConfig;
}

void KisDocument::setGuidesConfig(const KisGuidesConfig &data)
{
    if (d->guidesConfig == data) return;

    d->guidesConfig = data;
    emit sigGuidesConfigChanged(d->guidesConfig);
}

bool KisDocument::isEmpty() const
{
    return d->isEmpty;
}

void KisDocument::setEmpty(bool empty)
{
    d->isEmpty = empty;
}


// static
int KisDocument::defaultAutoSave()
{
    return 300;
}

void KisDocument::resetURL() {
    setUrl(QUrl());
    setLocalFilePath(QString());
}

int KisDocument::pageCount() const {
    return 1;
}

KoDocumentInfoDlg *KisDocument::createDocumentInfoDialog(QWidget *parent, KoDocumentInfo *docInfo) const
{
    return new KoDocumentInfoDlg(parent, docInfo);
}

bool KisDocument::isReadWrite() const
{
    return d->readwrite;
}

QUrl KisDocument::url() const
{
    return d->m_url;
}

bool KisDocument::closeUrl(bool promptToSave)
{
    if (promptToSave) {
        if ( isReadWrite() && isModified()) {
            Q_FOREACH (KisView *view, KisPart::instance()->views()) {
                if (view && view->document() == this) {
                    if (!view->queryClose()) {
                        return false;
                    }
                }
            }
        }
    }
    // Not modified => ok and delete temp file.
    d->mimeType = QByteArray();

    // It always succeeds for a read-only part,
    // but the return value exists for reimplementations
    // (e.g. pressing cancel for a modified read-write part)
    return true;
}



void KisDocument::setUrl(const QUrl &url)
{
    d->m_url = url;
}

QString KisDocument::localFilePath() const
{
    return d->m_file;
}


void KisDocument::setLocalFilePath( const QString &localFilePath )
{
    d->m_file = localFilePath;
}

bool KisDocument::saveToUrl()
{
    if ( d->m_url.isLocalFile() ) {
        setModified( false );
        emit completed();
        // if m_url is a local file there won't be a temp file -> nothing to remove
        d->m_saveOk = true;
        d->m_duringSaveAs = false;
        d->m_originalURL = QUrl();
        d->m_originalFilePath.clear();
        return true; // Nothing to do
    }
    return false;
}


bool KisDocument::openUrlInternal(const QUrl &url)
{
    if ( !url.isValid() )
        return false;

    if (d->m_bAutoDetectedMime) {
        d->mimeType = QByteArray();
        d->m_bAutoDetectedMime = false;
    }

    QByteArray mimetype = d->mimeType;

    if ( !closeUrl() )
        return false;

    d->mimeType = mimetype;
    setUrl(url);

    d->m_file.clear();

    if (d->m_url.isLocalFile()) {
        d->m_file = d->m_url.toLocalFile();
        bool ret;
        // set the mimetype only if it was not already set (for example, by the host application)
        if (d->mimeType.isEmpty()) {
            // get the mimetype of the file
            // using findByUrl() to avoid another string -> url conversion
            QString mime = KisMimeDatabase::mimeTypeForFile(d->m_url.toLocalFile());
            d->mimeType = mime.toLocal8Bit();
            d->m_bAutoDetectedMime = true;
        }
        setFileProgressProxy();
        setUrl(d->m_url);
        ret = openFile();
        clearFileProgressProxy();

        if (ret) {
            emit completed();
        } else {
            emit canceled(QString());
        }
        return ret;
    }
    return false;
}

KisImageWSP KisDocument::newImage(const QString& name, qint32 width, qint32 height, const KoColorSpace* colorspace)
{
    KoColor backgroundColor(Qt::white, colorspace);

    /**
     * FIXME: check whether this is a good value
     */
    double defaultResolution=1.;

    newImage(name, width, height, colorspace, backgroundColor, "",
             defaultResolution);
    return image();
}

bool KisDocument::newImage(const QString& name, qint32 width, qint32 height, const KoColorSpace * cs, const KoColor &bgColor, const QString &imageDescription, const double imageResolution) {
    return newImage(name, width, height, cs, bgColor, false, 1, imageDescription, imageResolution);
}

bool KisDocument::newImage(const QString& name,
                           qint32 width, qint32 height,
                           const KoColorSpace* cs,
                           const KoColor &bgColor, bool backgroundAsLayer,
                           int numberOfLayers,
                           const QString &description, const double imageResolution)
{
    Q_ASSERT(cs);

    KisConfig cfg;

    KisImageSP image;
    KisPaintLayerSP layer;

    if (!cs) return false;

    QApplication::setOverrideCursor(Qt::BusyCursor);

    image = new KisImage(createUndoStore(), width, height, cs, name);

    Q_CHECK_PTR(image);

    connect(image.data(), SIGNAL(sigImageModified()), this, SLOT(setImageModified()));
    image->setResolution(imageResolution, imageResolution);

    image->assignImageProfile(cs->profile());
    documentInfo()->setAboutInfo("title", name);
    if (name != i18n("Unnamed") && !name.isEmpty()) {
        setUrl(QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation) + '/' + name + ".kra"));
    }
    documentInfo()->setAboutInfo("abstract", description);

    layer = new KisPaintLayer(image.data(), image->nextLayerName(), OPACITY_OPAQUE_U8, cs);
    Q_CHECK_PTR(layer);

    if (backgroundAsLayer) {
        image->setDefaultProjectionColor(KoColor(cs));

        if (bgColor.opacityU8() == OPACITY_OPAQUE_U8) {
            layer->paintDevice()->setDefaultPixel(bgColor);
        } else {
            // Hack: with a semi-transparent background color, the projection isn't composited right if we just set the default pixel
            KisFillPainter painter;
            painter.begin(layer->paintDevice());
            painter.fillRect(0, 0, width, height, bgColor, bgColor.opacityU8());
        }
    } else {
        image->setDefaultProjectionColor(bgColor);
    }
    layer->setDirty(QRect(0, 0, width, height));

    image->addNode(layer.data(), image->rootLayer().data());
    setCurrentImage(image);

    for(int i = 1; i < numberOfLayers; ++i) {
        KisPaintLayerSP layer = new KisPaintLayer(image, image->nextLayerName(), OPACITY_OPAQUE_U8, cs);
        image->addNode(layer, image->root(), i);
        layer->setDirty(QRect(0, 0, width, height));
    }

    cfg.defImageWidth(width);
    cfg.defImageHeight(height);
    cfg.defImageResolution(imageResolution);
    cfg.defColorModel(image->colorSpace()->colorModelId().id());
    cfg.setDefaultColorDepth(image->colorSpace()->colorDepthId().id());
    cfg.defColorProfile(image->colorSpace()->profile()->name());

    QApplication::restoreOverrideCursor();

    return true;
}

KoShapeBasedDocumentBase *KisDocument::shapeController() const
{
    return d->shapeController;
}

KoShapeLayer* KisDocument::shapeForNode(KisNodeSP layer) const
{
    return d->shapeController->shapeForNode(layer);
}

vKisNodeSP KisDocument::activeNodes() const
{
    vKisNodeSP nodes;
    Q_FOREACH (KisView *v, KisPart::instance()->views()) {
        if (v->document() == this && v->viewManager()) {
            KisNodeSP activeNode = v->viewManager()->activeNode();
            if (activeNode && !nodes.contains(activeNode)) {
                if (activeNode->inherits("KisMask")) {
                    activeNode = activeNode->parent();
                }
                nodes.append(activeNode);
            }
        }
    }
    return nodes;
}

QList<KisPaintingAssistantSP> KisDocument::assistants() const
{
    return d->assistants;
}

void KisDocument::setAssistants(const QList<KisPaintingAssistantSP> value)
{
    d->assistants = value;
}

void KisDocument::setPreActivatedNode(KisNodeSP activatedNode)
{
    d->preActivatedNode = activatedNode;
}

KisNodeSP KisDocument::preActivatedNode() const
{
    return d->preActivatedNode;
}

void KisDocument::setFileProgressUpdater(const QString &text)
{
    d->suppressProgress = d->importExportManager->batchMode();

    if (!d->suppressProgress) {
        d->progressUpdater = new KoProgressUpdater(d->progressProxy, KoProgressUpdater::Unthreaded);
        d->progressUpdater->start(100, text);
        d->importExportManager->setProgresUpdater(d->progressUpdater);

        connect(this, SIGNAL(sigProgress(int)), KisPart::instance()->currentMainwindow(), SLOT(slotProgress(int)));
        connect(KisPart::instance()->currentMainwindow(), SIGNAL(sigProgressCanceled()), this, SIGNAL(sigProgressCanceled()));
    }
}

void KisDocument::clearFileProgressUpdater()
{
    if (!d->suppressProgress && d->progressUpdater) {
        disconnect(KisPart::instance()->currentMainwindow(), SIGNAL(sigProgressCanceled()), this, SIGNAL(sigProgressCanceled()));
        disconnect(this, SIGNAL(sigProgress(int)), KisPart::instance()->currentMainwindow(), SLOT(slotProgress(int)));
        delete d->progressUpdater;
        d->importExportManager->setProgresUpdater(0);
        d->progressUpdater = 0;
    }
}

void KisDocument::setFileProgressProxy()
{
    if (!d->progressProxy && !d->importExportManager->batchMode()) {
        d->fileProgressProxy = progressProxy();
    } else {
        d->fileProgressProxy = 0;
    }
}

void KisDocument::clearFileProgressProxy()
{
    if (d->fileProgressProxy) {
        setProgressProxy(0);
        delete d->fileProgressProxy;
        d->fileProgressProxy = 0;
    }
}

KisImageWSP KisDocument::image() const
{
    return d->image;
}


void KisDocument::setCurrentImage(KisImageSP image)
{
    if (d->image) {
        // Disconnect existing sig/slot connections
        d->image->disconnect(this);
        d->shapeController->setImage(0);
        d->image = 0;
    }

    if (!image) return;

    d->setImageAndInitIdleWatcher(image);
    d->shapeController->setImage(image);
    setModified(false);
    connect(d->image, SIGNAL(sigImageModified()), this, SLOT(setImageModified()));
    d->image->initialRefreshGraph();
    setAutoSave(KisConfig().autoSaveInterval());
}

void KisDocument::initEmpty()
{
    KisConfig cfg;
    const KoColorSpace * rgb = KoColorSpaceRegistry::instance()->rgb8();
    newImage("", cfg.defImageWidth(), cfg.defImageHeight(), rgb);
}

void KisDocument::setImageModified()
{
    setModified(true);
}


KisUndoStore* KisDocument::createUndoStore()
{
    return new KisDocumentUndoStore(this);
}

bool KisDocument::isAutosaving() const
{
    return d->isAutosaving;
}
