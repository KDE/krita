/* This file is part of the Krita project
 *
 * Copyright (C) 2014 Boudewijn Rempt <boud@valdyas.org>
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
#include <KoDialog.h>

#include <KisUsageLogger.h>
#include <klocalizedstring.h>
#include <kis_debug.h>
#include <kis_generator_layer.h>
#include <kis_generator_registry.h>
#include <kdesktopfile.h>
#include <kconfiggroup.h>
#include <kbackup.h>

#include <QTextBrowser>
#include <QApplication>
#include <QBuffer>
#include <QStandardPaths>
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
#include <QFuture>
#include <QFutureWatcher>

// Krita Image
#include <kis_image_animation_interface.h>
#include <kis_config.h>
#include <flake/kis_shape_layer.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_name_server.h>
#include <kis_paint_layer.h>
#include <kis_painter.h>
#include <kis_selection.h>
#include <kis_fill_painter.h>
#include <kis_document_undo_store.h>
#include <kis_idle_watcher.h>
#include <kis_signal_auto_connection.h>
#include <kis_canvas_widget_base.h>
#include "kis_layer_utils.h"

// Local
#include "KisViewManager.h"
#include "kis_clipboard.h"
#include "widgets/kis_custom_image_widget.h"
#include "canvas/kis_canvas2.h"
#include "flake/kis_shape_controller.h"
#include "kis_statusbar.h"
#include "widgets/kis_progress_widget.h"
#include "kis_canvas_resource_provider.h"
#include "KisResourceServerProvider.h"
#include "kis_node_manager.h"
#include "KisPart.h"
#include "KisApplication.h"
#include "KisDocument.h"
#include "KisImportExportManager.h"
#include "KisView.h"
#include "kis_grid_config.h"
#include "kis_guides_config.h"
#include "kis_image_barrier_lock_adapter.h"
#include "KisReferenceImagesLayer.h"

#include <mutex>
#include "kis_config_notifier.h"
#include "kis_async_action_feedback.h"
#include "KisCloneDocumentStroke.h"

#include <KisMirrorAxisConfig.h>


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

namespace {
constexpr int errorMessageTimeout = 5000;
constexpr int successMessageTimeout = 1000;
}


/**********************************************************
 *
 * KisDocument
 *
 **********************************************************/

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
        : KUndo2Stack(doc),
          m_doc(doc)
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

        if (image->tryUndoUnfinishedLod0Stroke() == UNDO_OK) {
            return;
        }

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
    Private(KisDocument *q)
        : docInfo(new KoDocumentInfo(q)) // deleted by QObject
        , importExportManager(new KisImportExportManager(q)) // deleted manually
        , autoSaveTimer(new QTimer(q))
        , undoStack(new UndoStack(q)) // deleted by QObject
        , m_bAutoDetectedMime(false)
        , modified(false)
        , readwrite(true)
        , firstMod(QDateTime::currentDateTime())
        , lastMod(firstMod)
        , nserver(new KisNameServer(1))
        , imageIdleWatcher(2000 /*ms*/)
        , globalAssistantsColor(KisConfig(true).defaultAssistantsColor())
        , savingLock(&savingMutex)
        , batchMode(false)
    {
        if (QLocale().measurementSystem() == QLocale::ImperialSystem) {
            unit = KoUnit::Inch;
        } else {
            unit = KoUnit::Centimeter;
        }
    }

    Private(const Private &rhs, KisDocument *q)
        : docInfo(new KoDocumentInfo(*rhs.docInfo, q))
        , unit(rhs.unit)
        , importExportManager(new KisImportExportManager(q))
        , mimeType(rhs.mimeType)
        , outputMimeType(rhs.outputMimeType)
        , autoSaveTimer(new QTimer(q))
        , undoStack(new UndoStack(q))
        , guidesConfig(rhs.guidesConfig)
        , mirrorAxisConfig(rhs.mirrorAxisConfig)
        , m_bAutoDetectedMime(rhs.m_bAutoDetectedMime)
        , m_url(rhs.m_url)
        , m_file(rhs.m_file)
        , modified(rhs.modified)
        , readwrite(rhs.readwrite)
        , firstMod(rhs.firstMod)
        , lastMod(rhs.lastMod)
        , nserver(new KisNameServer(*rhs.nserver))
        , preActivatedNode(0) // the node is from another hierarchy!
        , imageIdleWatcher(2000 /*ms*/)
        , assistants(rhs.assistants) // WARNING: assistants should not store pointers to the document!
        , globalAssistantsColor(rhs.globalAssistantsColor)
        , paletteList(rhs.paletteList)
        , gridConfig(rhs.gridConfig)
        , savingLock(&savingMutex)
        , batchMode(rhs.batchMode)
    {
        // TODO: clone assistants
    }

    ~Private() {
        // Don't delete m_d->shapeController because it's in a QObject hierarchy.
        delete nserver;
    }

    KoDocumentInfo *docInfo = 0;

    KoUnit unit;

    KisImportExportManager *importExportManager = 0; // The filter-manager to use when loading/saving [for the options]

    QByteArray mimeType; // The actual mimetype of the document
    QByteArray outputMimeType; // The mimetype to use when saving

    QTimer *autoSaveTimer;
    QString lastErrorMessage; // see openFile()
    QString lastWarningMessage;
    int autoSaveDelay = 300; // in seconds, 0 to disable.
    bool modifiedAfterAutosave = false;
    bool isAutosaving = false;
    bool disregardAutosaveFailure = false;
    int autoSaveFailureCount = 0;

    KUndo2Stack *undoStack = 0;

    KisGuidesConfig guidesConfig;
    KisMirrorAxisConfig mirrorAxisConfig;

    bool m_bAutoDetectedMime = false; // whether the mimetype in the arguments was detected by the part itself
    QUrl m_url; // local url - the one displayed to the user.
    QString m_file; // Local file - the only one the part implementation should deal with.

    QMutex savingMutex;

    bool modified = false;
    bool readwrite = false;

    QDateTime firstMod;
    QDateTime lastMod;

    KisNameServer *nserver;

    KisImageSP image;
    KisImageSP savingImage;

    KisNodeWSP preActivatedNode;
    KisShapeController* shapeController = 0;
    KoShapeController* koShapeController = 0;
    KisIdleWatcher imageIdleWatcher;
    QScopedPointer<KisSignalAutoConnection> imageIdleConnection;

    QList<KisPaintingAssistantSP> assistants;

    QColor globalAssistantsColor;

    KisSharedPtr<KisReferenceImagesLayer> referenceImagesLayer;

    QList<KoColorSet*> paletteList;

    KisGridConfig gridConfig;

    StdLockableWrapper<QMutex> savingLock;

    bool modifiedWhileSaving = false;
    QScopedPointer<KisDocument> backgroundSaveDocument;
    QPointer<KoUpdater> savingUpdater;
    QFuture<KisImportExportFilter::ConversionStatus> childSavingFuture;
    KritaUtils::ExportFileJob backgroundSaveJob;

    bool isRecovered = false;

    bool batchMode { false };

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

    class StrippedSafeSavingLocker;
};

class KisDocument::Private::StrippedSafeSavingLocker {
public:
    StrippedSafeSavingLocker(QMutex *savingMutex, KisImageSP image)
        : m_locked(false)
        , m_image(image)
        , m_savingLock(savingMutex)
        , m_imageLock(image, true)

    {
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
            m_image->requestStrokeEnd();
            QApplication::processEvents();

            // one more try...
            m_locked = std::try_lock(m_imageLock, m_savingLock) < 0;
        }
    }

    ~StrippedSafeSavingLocker() {
        if (m_locked) {
            m_imageLock.unlock();
            m_savingLock.unlock();
        }
    }

    bool successfullyLocked() const {
        return m_locked;
    }

private:
    bool m_locked;
    KisImageSP m_image;
    StdLockableWrapper<QMutex> m_savingLock;
    KisImageBarrierLockAdapter m_imageLock;
};

KisDocument::KisDocument()
    : d(new Private(this))
{
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotConfigChanged()));
    connect(d->undoStack, SIGNAL(cleanChanged(bool)), this, SLOT(slotUndoStackCleanChanged(bool)));
    connect(d->autoSaveTimer, SIGNAL(timeout()), this, SLOT(slotAutoSave()));
    setObjectName(newObjectName());

    // preload the krita resources
    KisResourceServerProvider::instance();

    d->shapeController = new KisShapeController(this, d->nserver),
            d->koShapeController = new KoShapeController(0, d->shapeController),

            slotConfigChanged();
}

KisDocument::KisDocument(const KisDocument &rhs)
    : QObject(),
      d(new Private(*rhs.d, this))
{
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotConfigChanged()));
    connect(d->undoStack, SIGNAL(cleanChanged(bool)), this, SLOT(slotUndoStackCleanChanged(bool)));
    connect(d->autoSaveTimer, SIGNAL(timeout()), this, SLOT(slotAutoSave()));
    setObjectName(rhs.objectName());

    d->shapeController = new KisShapeController(this, d->nserver),
            d->koShapeController = new KoShapeController(0, d->shapeController),

            slotConfigChanged();

    // clone the image with keeping the GUIDs of the layers intact
    // NOTE: we expect the image to be locked!
    setCurrentImage(rhs.image()->clone(true), false);

    if (rhs.d->preActivatedNode) {
        // since we clone uuid's, we can use them for lacating new
        // nodes. Otherwise we would need to use findSymmetricClone()
        d->preActivatedNode =
                KisLayerUtils::findNodeByUuid(d->image->root(), rhs.d->preActivatedNode->uuid());
    }
}

KisDocument::~KisDocument()
{
    // wait until all the pending operations are in progress
    waitForSavingToComplete();

    /**
     * Push a timebomb, which will try to release the memory after
     * the document has been deleted
     */
    KisPaintDevice::createMemoryReleaseObject()->deleteLater();

    d->autoSaveTimer->disconnect(this);
    d->autoSaveTimer->stop();

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

KisDocument *KisDocument::clone()
{
    return new KisDocument(*this);
}

bool KisDocument::exportDocumentImpl(const KritaUtils::ExportFileJob &job, KisPropertiesConfigurationSP exportConfiguration)
{
    QFileInfo filePathInfo(job.filePath);

    if (filePathInfo.exists() && !filePathInfo.isWritable()) {
        slotCompleteSavingDocument(job,
                                   KisImportExportFilter::CreationError,
                                   i18n("%1 cannot be written to. Please save under a different name.", job.filePath));
        return false;
    }

    KisConfig cfg(true);
    if (cfg.backupFile() && filePathInfo.exists()) {

        QString backupDir;

        switch(cfg.readEntry<int>("backupfilelocation", 0)) {
        case 1:
            backupDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
            break;
        case 2:
            backupDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
            break;
        default:
            // Do nothing: the empty string is user file location
            break;
        }

        int numOfBackupsKept = cfg.readEntry<int>("numberofbackupfiles", 1);
        QString suffix = cfg.readEntry<QString>("backupfilesuffix", "~");

        if (numOfBackupsKept == 1) {
            KBackup::simpleBackupFile(job.filePath, backupDir, suffix);
        }
        else if (numOfBackupsKept > 2) {
            KBackup::numberedBackupFile(job.filePath, backupDir, suffix, numOfBackupsKept);
        }
    }

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!job.mimeType.isEmpty(), false);

    const QString actionName =
            job.flags & KritaUtils::SaveIsExporting ?
                i18n("Exporting Document...") :
                i18n("Saving Document...");

    bool started =
            initiateSavingInBackground(actionName,
                                       this, SLOT(slotCompleteSavingDocument(KritaUtils::ExportFileJob,KisImportExportFilter::ConversionStatus,QString)),
                                       job, exportConfiguration);

    if (!started) {
        emit canceled(QString());
    }

    return started;
}

bool KisDocument::exportDocument(const QUrl &url, const QByteArray &mimeType, bool showWarnings, KisPropertiesConfigurationSP exportConfiguration)
{
    using namespace KritaUtils;

    SaveFlags flags = SaveIsExporting;
    if (showWarnings) {
        flags |= SaveShowWarnings;
    }

    KisUsageLogger::log(QString("Exporting Document: %1 as %2. %3 * %4 pixels, %5 layers, %6 frames, %7 framerate. Export configuration: %8")
                        .arg(url.toLocalFile())
                        .arg(QString::fromLatin1(mimeType))
                        .arg(d->image->width())
                        .arg(d->image->height())
                        .arg(d->image->nlayers())
                        .arg(d->image->animationInterface()->totalLength())
                        .arg(d->image->animationInterface()->framerate())
                        .arg(exportConfiguration ? exportConfiguration->toXML() : "No configuration"));

    return exportDocumentImpl(KritaUtils::ExportFileJob(url.toLocalFile(),
                                                        mimeType,
                                                        flags),
                              exportConfiguration);

}

bool KisDocument::saveAs(const QUrl &_url, const QByteArray &mimeType, bool showWarnings, KisPropertiesConfigurationSP exportConfiguration)
{
    using namespace KritaUtils;

    KisUsageLogger::log(QString("Saving Document %9 as %1 (mime: %2). %3 * %4 pixels, %5 layers.  %6 frames, %7 framerate. Export configuration: %8")
                        .arg(_url.toLocalFile())
                        .arg(QString::fromLatin1(mimeType))
                        .arg(d->image->width())
                        .arg(d->image->height())
                        .arg(d->image->nlayers())
                        .arg(d->image->animationInterface()->totalLength())
                        .arg(d->image->animationInterface()->framerate())
                        .arg(exportConfiguration ? exportConfiguration->toXML() : "No configuration")
                        .arg(url().toLocalFile()));


    return exportDocumentImpl(ExportFileJob(_url.toLocalFile(),
                                            mimeType,
                                            showWarnings ? SaveShowWarnings : SaveNone),
                              exportConfiguration);
}

bool KisDocument::save(bool showWarnings, KisPropertiesConfigurationSP exportConfiguration)
{
    return saveAs(url(), mimeType(), showWarnings, exportConfiguration);
}

QByteArray KisDocument::serializeToNativeByteArray()
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);

    QScopedPointer<KisImportExportFilter> filter(KisImportExportManager::filterForMimeType(nativeFormatMimeType(), KisImportExportManager::Export));
    filter->setBatchMode(true);
    filter->setMimeType(nativeFormatMimeType());

    Private::StrippedSafeSavingLocker locker(&d->savingMutex, d->image);
    if (!locker.successfullyLocked()) {
        return byteArray;
    }

    d->savingImage = d->image;

    if (filter->convert(this, &buffer) != KisImportExportFilter::OK) {
        qWarning() << "serializeToByteArray():: Could not export to our native format";
    }

    return byteArray;
}

void KisDocument::slotCompleteSavingDocument(const KritaUtils::ExportFileJob &job, KisImportExportFilter::ConversionStatus status, const QString &errorMessage)
{
    if (status == KisImportExportFilter::UserCancelled)
        return;

    const QString fileName = QFileInfo(job.filePath).fileName();

    if (status != KisImportExportFilter::OK) {
        emit statusBarMessage(i18nc("%1 --- failing file name, %2 --- error message",
                                    "Error during saving %1: %2",
                                    fileName,
                                    exportErrorToUserMessage(status, errorMessage)), errorMessageTimeout);

        if (!fileBatchMode()) {
            const QString filePath = job.filePath;
            QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("Could not save %1\nReason: %2", filePath, exportErrorToUserMessage(status, errorMessage)));
        }
    } else {
        if (!(job.flags & KritaUtils::SaveIsExporting)) {
            const QString existingAutoSaveBaseName = localFilePath();
            const bool wasRecovered = isRecovered();

            setUrl(QUrl::fromLocalFile(job.filePath));
            setLocalFilePath(job.filePath);
            setMimeType(job.mimeType);
            updateEditingTime(true);

            if (!d->modifiedWhileSaving) {
                /**
                 * If undo stack is already clean/empty, it doesn't emit any
                 * signals, so we might forget update document modified state
                 * (which was set, e.g. while recovering an autosave file)
                 */

                if (d->undoStack->isClean()) {
                    setModified(false);
                } else {
                    d->undoStack->setClean();
                }
            }
            setRecovered(false);
            removeAutoSaveFiles(existingAutoSaveBaseName, wasRecovered);
        }

        emit completed();
        emit sigSavingFinished();

        emit statusBarMessage(i18n("Finished saving %1", fileName), successMessageTimeout);
    }
}

QByteArray KisDocument::mimeType() const
{
    return d->mimeType;
}

void KisDocument::setMimeType(const QByteArray & mimeType)
{
    d->mimeType = mimeType;
}

bool KisDocument::fileBatchMode() const
{
    return d->batchMode;
}

void KisDocument::setFileBatchMode(const bool batchMode)
{
    d->batchMode = batchMode;
}

KisDocument* KisDocument::lockAndCloneForSaving()
{
    // force update of all the asynchronous nodes before cloning
    QApplication::processEvents();
    KisLayerUtils::forceAllDelayedNodesUpdate(d->image->root());

    KisMainWindow *window = KisPart::instance()->currentMainwindow();
    if (window) {
        if (window->viewManager()) {
            if (!window->viewManager()->blockUntilOperationsFinished(d->image)) {
                return 0;
            }
        }
    }

    Private::StrippedSafeSavingLocker locker(&d->savingMutex, d->image);
    if (!locker.successfullyLocked()) {
        return 0;
    }

    return new KisDocument(*this);
}

bool KisDocument::exportDocumentSync(const QUrl &url, const QByteArray &mimeType, KisPropertiesConfigurationSP exportConfiguration)
{
    Private::StrippedSafeSavingLocker locker(&d->savingMutex, d->image);
    if (!locker.successfullyLocked()) {
        return false;
    }

    d->savingImage = d->image;

    const QString fileName = url.toLocalFile();

    KisImportExportFilter::ConversionStatus status =
            d->importExportManager->
            exportDocument(fileName, fileName, mimeType, false, exportConfiguration);

    d->savingImage = 0;

    return status == KisImportExportFilter::OK;
}

bool KisDocument::initiateSavingInBackground(const QString actionName,
                                             const QObject *receiverObject, const char *receiverMethod,
                                             const KritaUtils::ExportFileJob &job,
                                             KisPropertiesConfigurationSP exportConfiguration)
{
    return initiateSavingInBackground(actionName, receiverObject, receiverMethod,
                                      job, exportConfiguration, std::unique_ptr<KisDocument>());
}

bool KisDocument::initiateSavingInBackground(const QString actionName,
                                             const QObject *receiverObject, const char *receiverMethod,
                                             const KritaUtils::ExportFileJob &job,
                                             KisPropertiesConfigurationSP exportConfiguration,
                                             std::unique_ptr<KisDocument> &&optionalClonedDocument)
{
    KIS_ASSERT_RECOVER_RETURN_VALUE(job.isValid(), false);

    QScopedPointer<KisDocument> clonedDocument;

    if (!optionalClonedDocument) {
        clonedDocument.reset(lockAndCloneForSaving());
    } else {
        clonedDocument.reset(optionalClonedDocument.release());
    }

    // we block saving until the current saving is finished!
    if (!clonedDocument || !d->savingMutex.tryLock()) {
        return false;
    }

    KIS_ASSERT_RECOVER_RETURN_VALUE(!d->backgroundSaveDocument, false);
    KIS_ASSERT_RECOVER_RETURN_VALUE(!d->backgroundSaveJob.isValid(), false);
    d->backgroundSaveDocument.reset(clonedDocument.take());
    d->backgroundSaveJob = job;
    d->modifiedWhileSaving = false;

    if (d->backgroundSaveJob.flags & KritaUtils::SaveInAutosaveMode) {
        d->backgroundSaveDocument->d->isAutosaving = true;
    }

    connect(d->backgroundSaveDocument.data(),
            SIGNAL(sigBackgroundSavingFinished(KisImportExportFilter::ConversionStatus,QString)),
            this,
            SLOT(slotChildCompletedSavingInBackground(KisImportExportFilter::ConversionStatus,QString)));


    connect(this, SIGNAL(sigCompleteBackgroundSaving(KritaUtils::ExportFileJob,KisImportExportFilter::ConversionStatus,QString)),
            receiverObject, receiverMethod, Qt::UniqueConnection);

    bool started =
            d->backgroundSaveDocument->startExportInBackground(actionName,
                                                               job.filePath,
                                                               job.filePath,
                                                               job.mimeType,
                                                               job.flags & KritaUtils::SaveShowWarnings,
                                                               exportConfiguration);

    if (!started) {
        // the state should have been deinitialized in slotChildCompletedSavingInBackground()

        KIS_SAFE_ASSERT_RECOVER (!d->backgroundSaveDocument && !d->backgroundSaveJob.isValid()) {
            d->backgroundSaveDocument.take()->deleteLater();
            d->savingMutex.unlock();
            d->backgroundSaveJob = KritaUtils::ExportFileJob();
        }
    }

    return started;
}


void KisDocument::slotChildCompletedSavingInBackground(KisImportExportFilter::ConversionStatus status, const QString &errorMessage)
{
    KIS_SAFE_ASSERT_RECOVER(!d->savingMutex.tryLock()) {
        d->savingMutex.unlock();
        return;
    }

    KIS_SAFE_ASSERT_RECOVER_RETURN(d->backgroundSaveDocument);

    if (d->backgroundSaveJob.flags & KritaUtils::SaveInAutosaveMode) {
        d->backgroundSaveDocument->d->isAutosaving = false;
    }

    d->backgroundSaveDocument.take()->deleteLater();
    d->savingMutex.unlock();

    KIS_SAFE_ASSERT_RECOVER_RETURN(d->backgroundSaveJob.isValid());
    const KritaUtils::ExportFileJob job = d->backgroundSaveJob;
    d->backgroundSaveJob = KritaUtils::ExportFileJob();

    KisUsageLogger::log(QString("Completed saving %1 (mime: %2). Result: %3")
                        .arg(job.filePath)
                        .arg(QString::fromLatin1(job.mimeType))
                        .arg(status != KisImportExportFilter::OK ? exportErrorToUserMessage(status, errorMessage) : "OK"));

    emit sigCompleteBackgroundSaving(job, status, errorMessage);
}

void KisDocument::slotAutoSaveImpl(std::unique_ptr<KisDocument> &&optionalClonedDocument)
{
    if (!d->modified || !d->modifiedAfterAutosave) return;
    const QString autoSaveFileName = generateAutoSaveFileName(localFilePath());

    emit statusBarMessage(i18n("Autosaving... %1", autoSaveFileName), successMessageTimeout);

    const bool hadClonedDocument = bool(optionalClonedDocument);
    bool started = false;

    if (d->image->isIdle() || hadClonedDocument) {
        started = initiateSavingInBackground(i18n("Autosaving..."),
                                             this, SLOT(slotCompleteAutoSaving(KritaUtils::ExportFileJob,KisImportExportFilter::ConversionStatus,QString)),
                                             KritaUtils::ExportFileJob(autoSaveFileName, nativeFormatMimeType(), KritaUtils::SaveIsExporting | KritaUtils::SaveInAutosaveMode),
                                             0,
                                             std::move(optionalClonedDocument));
    } else {
        emit statusBarMessage(i18n("Autosaving postponed: document is busy..."), errorMessageTimeout);
    }

    if (!started && !hadClonedDocument && d->autoSaveFailureCount >= 3) {
        KisCloneDocumentStroke *stroke = new KisCloneDocumentStroke(this);
        connect(stroke, SIGNAL(sigDocumentCloned(KisDocument*)),
                this, SLOT(slotInitiateAsyncAutosaving(KisDocument*)),
                Qt::BlockingQueuedConnection);

        KisStrokeId strokeId = d->image->startStroke(stroke);
        d->image->endStroke(strokeId);

        setInfiniteAutoSaveInterval();

    } else if (!started) {
        setEmergencyAutoSaveInterval();
    } else {
        d->modifiedAfterAutosave = false;
    }
}

void KisDocument::slotAutoSave()
{
    slotAutoSaveImpl(std::unique_ptr<KisDocument>());
}

void KisDocument::slotInitiateAsyncAutosaving(KisDocument *clonedDocument)
{
    slotAutoSaveImpl(std::unique_ptr<KisDocument>(clonedDocument));
}

void KisDocument::slotCompleteAutoSaving(const KritaUtils::ExportFileJob &job, KisImportExportFilter::ConversionStatus status, const QString &errorMessage)
{
    Q_UNUSED(job);

    const QString fileName = QFileInfo(job.filePath).fileName();

    if (status != KisImportExportFilter::OK) {
        setEmergencyAutoSaveInterval();
        emit statusBarMessage(i18nc("%1 --- failing file name, %2 --- error message",
                                    "Error during autosaving %1: %2",
                                    fileName,
                                    exportErrorToUserMessage(status, errorMessage)), errorMessageTimeout);
    } else {
        KisConfig cfg(true);
        d->autoSaveDelay = cfg.autoSaveInterval();

        if (!d->modifiedWhileSaving) {
            d->autoSaveTimer->stop(); // until the next change
            d->autoSaveFailureCount = 0;
        } else {
            setNormalAutoSaveInterval();
        }

        emit statusBarMessage(i18n("Finished autosaving %1", fileName), successMessageTimeout);
    }
}

bool KisDocument::startExportInBackground(const QString &actionName,
                                          const QString &location,
                                          const QString &realLocation,
                                          const QByteArray &mimeType,
                                          bool showWarnings,
                                          KisPropertiesConfigurationSP exportConfiguration)
{
    d->savingImage = d->image;

    KisMainWindow *window = KisPart::instance()->currentMainwindow();
    if (window) {
        if (window->viewManager()) {
            d->savingUpdater = window->viewManager()->createThreadedUpdater(actionName);
            d->importExportManager->setUpdater(d->savingUpdater);
        }
    }

    KisImportExportFilter::ConversionStatus initializationStatus;
    d->childSavingFuture =
            d->importExportManager->exportDocumentAsyc(location,
                                                       realLocation,
                                                       mimeType,
                                                       initializationStatus,
                                                       showWarnings,
                                                       exportConfiguration);

    if (initializationStatus != KisImportExportFilter::ConversionStatus::OK) {
        if (d->savingUpdater) {
            d->savingUpdater->cancel();
        }
        d->savingImage.clear();
        emit sigBackgroundSavingFinished(initializationStatus, this->errorMessage());
        return false;
    }

    typedef QFutureWatcher<KisImportExportFilter::ConversionStatus> StatusWatcher;
    StatusWatcher *watcher = new StatusWatcher();
    watcher->setFuture(d->childSavingFuture);

    connect(watcher, SIGNAL(finished()), SLOT(finishExportInBackground()));
    connect(watcher, SIGNAL(finished()), watcher, SLOT(deleteLater()));

    return true;
}

void KisDocument::finishExportInBackground()
{
    KIS_SAFE_ASSERT_RECOVER(d->childSavingFuture.isFinished()) {
        emit sigBackgroundSavingFinished(KisImportExportFilter::InternalError, "");
        return;
    }

    KisImportExportFilter::ConversionStatus status =
            d->childSavingFuture.result();
    const QString errorMessage = this->errorMessage();

    d->savingImage.clear();
    d->childSavingFuture = QFuture<KisImportExportFilter::ConversionStatus>();
    d->lastErrorMessage.clear();

    if (d->savingUpdater) {
        d->savingUpdater->setProgress(100);
    }

    emit sigBackgroundSavingFinished(status, errorMessage);
}

void KisDocument::setReadWrite(bool readwrite)
{
    d->readwrite = readwrite;
    setNormalAutoSaveInterval();

    Q_FOREACH (KisMainWindow *mainWindow, KisPart::instance()->mainWindows()) {
        mainWindow->setReadWrite(readwrite);
    }
}

void KisDocument::setAutoSaveDelay(int delay)
{
    if (isReadWrite() && delay > 0) {
        d->autoSaveTimer->start(delay * 1000);
    } else {
        d->autoSaveTimer->stop();
    }
}

void KisDocument::setNormalAutoSaveInterval()
{
    setAutoSaveDelay(d->autoSaveDelay);
    d->autoSaveFailureCount = 0;
}

void KisDocument::setEmergencyAutoSaveInterval()
{
    const int emergencyAutoSaveInterval = 10; /* sec */
    setAutoSaveDelay(emergencyAutoSaveInterval);
    d->autoSaveFailureCount++;
}

void KisDocument::setInfiniteAutoSaveInterval()
{
    setAutoSaveDelay(-1);
}

KoDocumentInfo *KisDocument::documentInfo() const
{
    return d->docInfo;
}

bool KisDocument::isModified() const
{
    return d->modified;
}

QPixmap KisDocument::generatePreview(const QSize& size)
{
    KisImageSP image = d->image;
    if (d->savingImage) image = d->savingImage;

    if (image) {
        QRect bounds = image->bounds();
        QSize newSize = bounds.size();
        newSize.scale(size, Qt::KeepAspectRatio);
        QPixmap px = QPixmap::fromImage(image->convertToQImage(newSize, 0));
        if (px.size() == QSize(0,0)) {
            px = QPixmap(newSize);
            QPainter gc(&px);
            QBrush checkBrush = QBrush(KisCanvasWidgetBase::createCheckersImage(newSize.width() / 5));
            gc.fillRect(px.rect(), checkBrush);
            gc.end();
        }
        return px;
    }
    return QPixmap(size);
}

QString KisDocument::generateAutoSaveFileName(const QString & path) const
{
    QString retval;

    // Using the extension allows to avoid relying on the mime magic when opening
    const QString extension (".kra");
    QString prefix = KisConfig(true).readEntry<bool>("autosavefileshidden") ? QString(".") : QString();
    QRegularExpression autosavePattern1("^\\..+-autosave.kra$");
    QRegularExpression autosavePattern2("^.+-autosave.kra$");

    QFileInfo fi(path);
    QString dir = fi.absolutePath();
    QString filename = fi.fileName();

    if (path.isEmpty() || autosavePattern1.match(filename).hasMatch() || autosavePattern2.match(filename).hasMatch()) {
        // Never saved?
#ifdef Q_OS_WIN
        // On Windows, use the temp location (https://bugs.kde.org/show_bug.cgi?id=314921)
        retval = QString("%1%2%7%3-%4-%5-autosave%6").arg(QDir::tempPath()).arg(QDir::separator()).arg("krita").arg(qApp->applicationPid()).arg(objectName()).arg(extension).arg(prefix);
#else
        // On Linux, use a temp file in $HOME then. Mark it with the pid so two instances don't overwrite each other's autosave file
        retval = QString("%1%2%7%3-%4-%5-autosave%6").arg(QDir::homePath()).arg(QDir::separator()).arg("krita").arg(qApp->applicationPid()).arg(objectName()).arg(extension).arg(prefix);
#endif
    } else {
        retval = QString("%1%2%5%3-autosave%4").arg(dir).arg(QDir::separator()).arg(filename).arg(extension).arg(prefix);
    }

    //qDebug() << "generateAutoSaveFileName() for path" << path << ":" << retval;
    return retval;
}

bool KisDocument::importDocument(const QUrl &_url)
{
    bool ret;

    dbgUI << "url=" << _url.url();

    // open...
    ret = openUrl(_url);

    // reset url & m_file (kindly? set by KisParts::openUrl()) to simulate a
    // File --> Import
    if (ret) {
        dbgUI << "success, resetting url";
        resetURL();
        setTitleModified();
    }

    return ret;
}


bool KisDocument::openUrl(const QUrl &_url, OpenFlags flags)
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
    if (url.isLocalFile() && !fileBatchMode()) {
        QString file = url.toLocalFile();
        QString asf = generateAutoSaveFileName(file);
        if (QFile::exists(asf)) {
            KisApplication *kisApp = static_cast<KisApplication*>(qApp);
            kisApp->hideSplashScreen();
            //dbgUI <<"asf=" << asf;
            // ## TODO compare timestamps ?
            int res = QMessageBox::warning(0,
                                           i18nc("@title:window", "Krita"),
                                           i18n("An autosaved file exists for this document.\nDo you want to open the autosaved file instead?"),
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
                return false;
            }
        }
    }

    bool ret = openUrlInternal(url);

    if (autosaveOpened || flags & RecoveryFile) {
        setReadWrite(true); // enable save button
        setModified(true);
        setRecovered(true);
    }
    else {
        if (ret) {

            if (!(flags & DontAddToRecent)) {
                KisPart::instance()->addRecentURLToAllMainWindows(_url);
            }

            // Detect readonly local-files; remote files are assumed to be writable
            QFileInfo fi(url.toLocalFile());
            setReadWrite(fi.isWritable());
        }

        setRecovered(false);
    }

    return ret;
}

class DlgLoadMessages : public KoDialog {
public:
    DlgLoadMessages(const QString &title, const QString &message, const QStringList &warnings) {
        setWindowTitle(title);
        setWindowIcon(KisIconUtils::loadIcon("warning"));
        QWidget *page = new QWidget(this);
        QVBoxLayout *layout = new QVBoxLayout(page);
        QHBoxLayout *hlayout = new QHBoxLayout();
        QLabel *labelWarning= new QLabel();
        labelWarning->setPixmap(KisIconUtils::loadIcon("warning").pixmap(32, 32));
        hlayout->addWidget(labelWarning);
        hlayout->addWidget(new QLabel(message));
        layout->addLayout(hlayout);
        QTextBrowser *browser = new QTextBrowser();
        QString warning = "<html><body><p><b>";
        if (warnings.size() == 1) {
            warning += "</b> Reason:</p>";
        }
        else {
            warning += "</b> Reasons:</p>";
        }
        warning += "<p/><ul>";

        Q_FOREACH(const QString &w, warnings) {
            warning += "\n<li>" + w + "</li>";
        }
        warning += "</ul>";
        browser->setHtml(warning);
        browser->setMinimumHeight(200);
        browser->setMinimumWidth(400);
        layout->addWidget(browser);
        setMainWidget(page);
        setButtons(KoDialog::Ok);
        resize(minimumSize());
    }
};

bool KisDocument::openFile()
{
    //dbgUI <<"for" << localFilePath();
    if (!QFile::exists(localFilePath())) {
        QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("File %1 does not exist.", localFilePath()));
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

    KisMainWindow *window = KisPart::instance()->currentMainwindow();
    KoUpdaterPtr updater;
    if (window && window->viewManager()) {
        updater = window->viewManager()->createUnthreadedUpdater(i18n("Opening document"));
        d->importExportManager->setUpdater(updater);
    }

    KisImportExportFilter::ConversionStatus status;

    status = d->importExportManager->importDocument(localFilePath(), typeName);

    if (status != KisImportExportFilter::OK) {
        if (window && window->viewManager()) {
            updater->cancel();
        }
        QString msg = KisImportExportFilter::conversionStatusString(status);
        if (!msg.isEmpty()) {
            DlgLoadMessages dlg(i18nc("@title:window", "Krita"),
                                i18n("Could not open %2.\nReason: %1.", msg, prettyPathOrUrl()),
                                errorMessage().split("\n") + warningMessage().split("\n"));
            dlg.exec();
        }
        return false;
    }
    else if (!warningMessage().isEmpty()) {
        DlgLoadMessages dlg(i18nc("@title:window", "Krita"),
                            i18n("There were problems opening %1.", prettyPathOrUrl()),
                            warningMessage().split("\n"));
        dlg.exec();
        setUrl(QUrl());
    }

    setMimeTypeAfterLoading(typeName);
    emit sigLoadingFinished();

    undoStack()->clear();

    return true;
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

    if (mod && !d->autoSaveTimer->isActive()) {
        // First change since last autosave -> start the autosave timer
        setNormalAutoSaveInterval();
    }
    d->modifiedAfterAutosave = mod;
    d->modifiedWhileSaving = mod;

    if (mod == isModified())
        return;

    d->modified = mod;

    if (mod) {
        documentInfo()->updateParameters();
    }

    // This influences the title
    setTitleModified();
    emit modified(mod);
}

void KisDocument::setRecovered(bool value)
{
    d->isRecovered = value;
}

bool KisDocument::isRecovered() const
{
    return d->isRecovered;
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
    const QString _url(url().fileName());

    // if URL is empty...it is probably an unsaved file
    if (_url.isEmpty()) {
        c = " [" + i18n("Not Saved") + "] ";
    } else {
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

void KisDocument::setWarningMessage(const QString& warningMsg)
{
    d->lastWarningMessage = warningMsg;
}

QString KisDocument::warningMessage() const
{
    return d->lastWarningMessage;
}


void KisDocument::removeAutoSaveFiles(const QString &autosaveBaseName, bool wasRecovered)
{
    //qDebug() << "removeAutoSaveFiles";
    // Eliminate any auto-save file
    QString asf = generateAutoSaveFileName(autosaveBaseName);   // the one in the current dir

    //qDebug() << "\tfilename:" << asf << "exists:" << QFile::exists(asf);
    if (QFile::exists(asf)) {
        //qDebug() << "\tremoving autosavefile" << asf;
        QFile::remove(asf);
    }
    asf = generateAutoSaveFileName(QString());   // and the one in $HOME

    //qDebug() << "Autsavefile in $home" << asf;
    if (QFile::exists(asf)) {
        //qDebug() << "\tremoving autsavefile 2" << asf;
        QFile::remove(asf);
    }

    QList<QRegularExpression> expressions;

    expressions << QRegularExpression("^\\..+-autosave.kra$")
                << QRegularExpression("^.+-autosave.kra$");

    Q_FOREACH(const QRegularExpression &rex, expressions) {
        if (wasRecovered &&
                !autosaveBaseName.isEmpty() &&
                rex.match(QFileInfo(autosaveBaseName).fileName()).hasMatch() &&
                QFile::exists(autosaveBaseName)) {

            QFile::remove(autosaveBaseName);
        }
    }
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

void KisDocument::slotUndoStackCleanChanged(bool value)
{
    setModified(!value);
}

void KisDocument::slotConfigChanged()
{
    KisConfig cfg(true);
    d->undoStack->setUndoLimit(cfg.undoStackLimit());
    d->autoSaveDelay = cfg.autoSaveInterval();
    setNormalAutoSaveInterval();
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

QList<KoColorSet *> &KisDocument::paletteList()
{
    return d->paletteList;
}

void KisDocument::setPaletteList(const QList<KoColorSet *> &paletteList)
{
    d->paletteList = paletteList;
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

const KisMirrorAxisConfig& KisDocument::mirrorAxisConfig() const
{
    return d->mirrorAxisConfig;
}

void KisDocument::setMirrorAxisConfig(const KisMirrorAxisConfig &config)
{
    if (d->mirrorAxisConfig == config) {
        return;
    }

    d->mirrorAxisConfig = config;
    setModified(true);

    emit sigMirrorAxisConfigChanged();
}

void KisDocument::resetURL() {
    setUrl(QUrl());
    setLocalFilePath(QString());
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

bool KisDocument::openUrlInternal(const QUrl &url)
{
    if ( !url.isValid() ) {
        return false;
    }

    if (d->m_bAutoDetectedMime) {
        d->mimeType = QByteArray();
        d->m_bAutoDetectedMime = false;
    }

    QByteArray mimetype = d->mimeType;

    if ( !closeUrl() ) {
        return false;
    }

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

        setUrl(d->m_url);
        ret = openFile();

        if (ret) {
            emit completed();
        } else {
            emit canceled(QString());
        }
        return ret;
    }
    return false;
}

bool KisDocument::newImage(const QString& name,
                           qint32 width, qint32 height,
                           const KoColorSpace* cs,
                           const KoColor &bgColor, KisConfig::BackgroundStyle bgStyle,
                           int numberOfLayers,
                           const QString &description, const double imageResolution)
{
    Q_ASSERT(cs);

    KisImageSP image;

    if (!cs) return false;

    QApplication::setOverrideCursor(Qt::BusyCursor);

    image = new KisImage(createUndoStore(), width, height, cs, name);

    Q_CHECK_PTR(image);

    connect(image, SIGNAL(sigImageModified()), this, SLOT(setImageModified()), Qt::UniqueConnection);
    image->setResolution(imageResolution, imageResolution);

    image->assignImageProfile(cs->profile());
    documentInfo()->setAboutInfo("title", name);
    documentInfo()->setAboutInfo("abstract", description);

    KisLayerSP layer;
    if (bgStyle == KisConfig::RASTER_LAYER || bgStyle == KisConfig::FILL_LAYER) {
        KoColor strippedAlpha = bgColor;
        strippedAlpha.setOpacity(OPACITY_OPAQUE_U8);

        if (bgStyle == KisConfig::RASTER_LAYER) {
            layer = new KisPaintLayer(image.data(), "Background", OPACITY_OPAQUE_U8, cs);;
            layer->paintDevice()->setDefaultPixel(strippedAlpha);
        } else if (bgStyle == KisConfig::FILL_LAYER) {
            KisFilterConfigurationSP filter_config = KisGeneratorRegistry::instance()->get("color")->defaultConfiguration();
            filter_config->setProperty("color", strippedAlpha.toQColor());
            layer = new KisGeneratorLayer(image.data(), "Background Fill", filter_config, image->globalSelection());
        }

        layer->setOpacity(bgColor.opacityU8());

        if (numberOfLayers > 1) {
            //Lock bg layer if others are present.
            layer->setUserLocked(true);
        }
    }
    else { // KisConfig::CANVAS_COLOR (needs an unlocked starting layer).
        image->setDefaultProjectionColor(bgColor);
        layer = new KisPaintLayer(image.data(), image->nextLayerName(), OPACITY_OPAQUE_U8, cs);
    }

    Q_CHECK_PTR(layer);
    image->addNode(layer.data(), image->rootLayer().data());
    layer->setDirty(QRect(0, 0, width, height));

    setCurrentImage(image);

    for(int i = 1; i < numberOfLayers; ++i) {
        KisPaintLayerSP layer = new KisPaintLayer(image, image->nextLayerName(), OPACITY_OPAQUE_U8, cs);
        image->addNode(layer, image->root(), i);
        layer->setDirty(QRect(0, 0, width, height));
    }

    KisConfig cfg(false);
    cfg.defImageWidth(width);
    cfg.defImageHeight(height);
    cfg.defImageResolution(imageResolution);
    cfg.defColorModel(image->colorSpace()->colorModelId().id());
    cfg.setDefaultColorDepth(image->colorSpace()->colorDepthId().id());
    cfg.defColorProfile(image->colorSpace()->profile()->name());

    KisUsageLogger::log(i18n("Created image \"%1\", %2 * %3 pixels, %4 dpi. Color model: %6 %5 (%7). Layers: %8"
                             , name
                             , width, height
                             , imageResolution * 72.0
                             , image->colorSpace()->colorModelId().name(), image->colorSpace()->colorDepthId().name()
                             , image->colorSpace()->profile()->name()
                             , numberOfLayers));

    QApplication::restoreOverrideCursor();

    return true;
}

bool KisDocument::isSaving() const
{
    const bool result = d->savingMutex.tryLock();
    if (result) {
        d->savingMutex.unlock();
    }
    return !result;
}

void KisDocument::waitForSavingToComplete()
{
    if (isSaving()) {
        KisAsyncActionFeedback f(i18nc("progress dialog message when the user closes the document that is being saved", "Waiting for saving to complete..."), 0);
        f.waitForMutex(&d->savingMutex);
    }
}

KoShapeControllerBase *KisDocument::shapeController() const
{
    return d->shapeController;
}

KoShapeLayer* KisDocument::shapeForNode(KisNodeSP layer) const
{
    return d->shapeController->shapeForNode(layer);
}

QList<KisPaintingAssistantSP> KisDocument::assistants() const
{
    return d->assistants;
}

void KisDocument::setAssistants(const QList<KisPaintingAssistantSP> &value)
{
    d->assistants = value;
}

KisSharedPtr<KisReferenceImagesLayer> KisDocument::referenceImagesLayer() const
{
    return d->referenceImagesLayer.data();
}

void KisDocument::setReferenceImagesLayer(KisSharedPtr<KisReferenceImagesLayer> layer, bool updateImage)
{
    if (d->referenceImagesLayer) {
        d->referenceImagesLayer->disconnect(this);
    }

    if (updateImage) {
        if (layer) {
            d->image->addNode(layer);
        } else {
            d->image->removeNode(d->referenceImagesLayer);
        }
    }

    d->referenceImagesLayer = layer;

    if (d->referenceImagesLayer) {
        connect(d->referenceImagesLayer, SIGNAL(sigUpdateCanvas(QRectF)),
                this, SIGNAL(sigReferenceImagesChanged()));
    }
}

void KisDocument::setPreActivatedNode(KisNodeSP activatedNode)
{
    d->preActivatedNode = activatedNode;
}

KisNodeSP KisDocument::preActivatedNode() const
{
    return d->preActivatedNode;
}

KisImageWSP KisDocument::image() const
{
    return d->image;
}

KisImageSP KisDocument::savingImage() const
{
    return d->savingImage;
}


void KisDocument::setCurrentImage(KisImageSP image, bool forceInitialUpdate)
{
    if (d->image) {
        // Disconnect existing sig/slot connections
        d->image->setUndoStore(new KisDumbUndoStore());
        d->image->disconnect(this);
        d->shapeController->setImage(0);
        d->image = 0;
    }

    if (!image) return;

    d->setImageAndInitIdleWatcher(image);
    d->image->setUndoStore(new KisDocumentUndoStore(this));
    d->shapeController->setImage(image);
    setModified(false);
    connect(d->image, SIGNAL(sigImageModified()), this, SLOT(setImageModified()), Qt::UniqueConnection);

    if (forceInitialUpdate) {
        d->image->initialRefreshGraph();
    }
}

void KisDocument::hackPreliminarySetImage(KisImageSP image)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(!d->image);

    d->setImageAndInitIdleWatcher(image);
    d->shapeController->setImage(image);
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

QString KisDocument::exportErrorToUserMessage(KisImportExportFilter::ConversionStatus status, const QString &errorMessage)
{
    return errorMessage.isEmpty() ? KisImportExportFilter::conversionStatusString(status) : errorMessage;
}

void KisDocument::setAssistantsGlobalColor(QColor color)
{
    d->globalAssistantsColor = color;
}

QColor KisDocument::assistantsGlobalColor()
{
    return d->globalAssistantsColor;
}

QRectF KisDocument::documentBounds() const
{
    QRectF bounds = d->image->bounds();

    if (d->referenceImagesLayer) {
        bounds |= d->referenceImagesLayer->boundingImageRect();
    }

    return bounds;
}
