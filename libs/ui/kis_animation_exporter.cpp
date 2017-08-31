/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
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

#include "kis_animation_exporter.h"

#include <QDesktopServices>
#include <QProgressDialog>
#include <KisMimeDatabase.h>
#include <QEventLoop>
#include <QMessageBox>

#include "KoFileDialog.h"
#include "KisDocument.h"
#include <KoUpdater.h>
#include "kis_image.h"
#include "KisImportExportManager.h"
#include "kis_image_animation_interface.h"
#include "KisPart.h"
#include "KisMainWindow.h"

#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_time_range.h"
#include "kis_painter.h"

#include "kis_image_lock_hijacker.h"


struct KisAnimationExporterUI::Private
{
    QWidget *parentWidget;
    KisAnimationExportSaver *exporter;

    Private(QWidget *parent)
        : parentWidget(parent),
          exporter(0)
    {}
};

KisAnimationExporterUI::KisAnimationExporterUI(QWidget *parent)
    : m_d(new Private(parent))
{
}

KisAnimationExporterUI::~KisAnimationExporterUI()
{
    if (m_d->exporter) {
        delete m_d->exporter;
    }
}

struct KisAnimationExporter::Private
{
    Private(KisImageSP _image, int fromTime, int toTime, KoUpdaterPtr _updater)
        : updater(_updater)
        , image(_image)
        , firstFrame(fromTime)
        , lastFrame(toTime)
        , currentFrame(-1)
        , batchMode(!updater)
        , isCancelled(false)
        , status(KisImportExportFilter::OK)
        , tmpDevice(new KisPaintDevice(image->colorSpace()))
    {
        KIS_SAFE_ASSERT_RECOVER_NOOP(bool(updater) == !batchMode);
    }

    KoUpdaterPtr updater;
    KisImageWSP image;

    int firstFrame;
    int lastFrame;
    int currentFrame;

    bool batchMode;
    bool isCancelled;

    KisImportExportFilter::ConversionStatus status;

    SaveFrameCallback saveFrameCallback;

    KisPaintDeviceSP tmpDevice;

    KisPropertiesConfigurationSP exportConfiguration;
    QProgressDialog progress;

};

KisAnimationExporter::KisAnimationExporter(KisImageWSP image, int fromTime, int toTime, KoUpdaterPtr updater)
    : m_d(new Private(image, fromTime, toTime, updater))
{
    connect(m_d->image->animationInterface(), SIGNAL(sigFrameReady(int)),
            this, SLOT(frameReadyToCopy(int)), Qt::DirectConnection);

    connect(this, SIGNAL(sigFrameReadyToSave()),
            this, SLOT(frameReadyToSave()), Qt::QueuedConnection);
}

KisAnimationExporter::~KisAnimationExporter()
{
}

void KisAnimationExporter::setExportConfiguration(KisPropertiesConfigurationSP exportConfiguration)
{
    m_d->exportConfiguration = exportConfiguration;
}

void KisAnimationExporter::setSaveFrameCallback(SaveFrameCallback func)
{
    m_d->saveFrameCallback = func;
}

KisImportExportFilter::ConversionStatus KisAnimationExporter::exportAnimation()
{

    if (!m_d->batchMode) {
        QString message = i18n("Preparing to export frames...");

        m_d->progress.reset();
        m_d->progress.setLabelText(message);
        m_d->progress.setWindowModality(Qt::ApplicationModal);
        m_d->progress.setCancelButton(0);
        m_d->progress.setMinimumDuration(0);
        m_d->progress.setValue(0);
        m_d->progress.setMinimum(0);
        m_d->progress.setMaximum(100);
    }

    if (m_d->updater) {
        m_d->updater->setProgress(0);
    }

    /**
     * HACK ALERT: Here we remove the image lock! We do it in a GUI
     *             thread under the barrier lock held, so it is
     *             guaranteed no other stroke will accidentally be
     *             started by this. And showing an app-modal dialog to
     *             the user will prevent him from doing anything
     *             nasty.
     */
    KisImageLockHijacker badGuy(m_d->image);
    Q_UNUSED(badGuy);

    KIS_ASSERT_RECOVER(!m_d->image->locked()) { return KisImportExportFilter::InternalError; }

    m_d->status = KisImportExportFilter::OK;
    m_d->currentFrame = m_d->firstFrame;
    m_d->image->animationInterface()->requestFrameRegeneration(m_d->currentFrame, m_d->image->bounds());

    QEventLoop loop;
    loop.connect(this, SIGNAL(sigFinished()), SLOT(quit()));
    loop.exec();

    if (!m_d->batchMode) {
        m_d->progress.reset();
    }

    if (m_d->updater) {
        m_d->updater->setProgress(100);
    }

    return m_d->status;
}

void KisAnimationExporter::cancel()
{
    m_d->isCancelled = true;
}

void KisAnimationExporter::frameReadyToCopy(int time)
{
    if (time != m_d->currentFrame) return;

    QRect rc = m_d->image->bounds();
    KisPainter::copyAreaOptimized(rc.topLeft(), m_d->image->projection(), m_d->tmpDevice, rc);

    emit sigFrameReadyToSave();
}

void KisAnimationExporter::frameReadyToSave()
{
    KIS_ASSERT_RECOVER(m_d->saveFrameCallback) {
        m_d->status = KisImportExportFilter::InternalError;
        emit sigFinished();
        return;
    }

    // TODO: refactor to a signal!
    if (m_d->isCancelled || (m_d->updater && m_d->updater->interrupted())) {
        m_d->status = KisImportExportFilter::UserCancelled;
        emit sigFinished();
        return;
    }

    KisImportExportFilter::ConversionStatus result =
        KisImportExportFilter::OK;
    int time = m_d->currentFrame;

    result = m_d->saveFrameCallback(time, m_d->tmpDevice, m_d->exportConfiguration);

    if (m_d->updater) {
        int length = m_d->lastFrame - m_d->firstFrame;
        m_d->updater->setProgress((time - m_d->firstFrame) * 100 / length);
    }

    // TODO: make translatable!!
    QString dialogText = QString("Exporting Frame ").append(QString::number(time)).append(" of ").append(QString::number(m_d->lastFrame));
    int percentageProcessed = (float(time) / float(m_d->lastFrame) * 100);

    m_d->progress.setLabelText(dialogText);
    m_d->progress.setValue(int(percentageProcessed));

    if (result == KisImportExportFilter::OK && time < m_d->lastFrame) {
        m_d->currentFrame = time + 1;
        m_d->image->animationInterface()->requestFrameRegeneration(m_d->currentFrame, m_d->image->bounds());
    } else {
        m_d->status = result;
        emit sigFinished();
    }
}

struct KisAnimationExportSaver::Private
{
    Private(KisDocument *_document, int fromTime, int toTime, int _sequenceNumberingOffset, KoUpdaterPtr _updater)
        : document(_document)
        , image(_document->image())
        , firstFrame(fromTime)
        , lastFrame(toTime)
        , sequenceNumberingOffset(_sequenceNumberingOffset)
        , updater(_updater)
        , tmpDoc(KisPart::instance()->createDocument())
        , exporter(_document->image(), fromTime, toTime, _updater)
    {
        tmpDoc->setAutoSaveDelay(0);

        KisImageSP tmpImage = new KisImage(tmpDoc->createUndoStore(),
                                image->bounds().width(),
                                image->bounds().height(),
                                image->colorSpace(),
                                QString());

        tmpImage->setResolution(image->xRes(), image->yRes());
        tmpDoc->setCurrentImage(tmpImage);

        KisPaintLayer* paintLayer = new KisPaintLayer(tmpImage, "paint device", 255);
        tmpImage->addNode(paintLayer, tmpImage->rootLayer(), KisLayerSP(0));

        tmpDevice = paintLayer->paintDevice();
    }

    KisDocument *document;
    KisImageWSP image;
    int firstFrame;
    int lastFrame;
    int sequenceNumberingOffset;
    KoUpdaterPtr updater;

    QScopedPointer<KisDocument> tmpDoc;
    KisPaintDeviceSP tmpDevice;
    QByteArray outputMimeType;

    KisAnimationExporter exporter;

    QString filenamePrefix;
    QString filenameSuffix;
};

KisAnimationExportSaver::KisAnimationExportSaver(KisDocument *document, const QString &baseFilename, int fromTime, int toTime, int sequenceNumberingOffset, KoUpdaterPtr updater)
    : m_d(new Private(document, fromTime, toTime, sequenceNumberingOffset, updater))
{
    int baseLength = baseFilename.lastIndexOf(".");
    if (baseLength > -1) {
        m_d->filenamePrefix = baseFilename.left(baseLength);
        m_d->filenameSuffix = baseFilename.right(baseFilename.length() - baseLength);
    } else {
        m_d->filenamePrefix = baseFilename;
    }

    m_d->outputMimeType = KisMimeDatabase::mimeTypeForFile(baseFilename).toLatin1();
    m_d->tmpDoc->setFileBatchMode(true);

    using namespace std::placeholders; // For _1 placeholder
    m_d->exporter.setSaveFrameCallback(std::bind(&KisAnimationExportSaver::saveFrameCallback, this, _1, _2, _3));
}

KisAnimationExportSaver::~KisAnimationExportSaver()
{
}

KisImportExportFilter::ConversionStatus KisAnimationExportSaver::exportAnimation(KisPropertiesConfigurationSP cfg)
{
    QFileInfo info(savedFilesMaskWildcard());

    QDir dir(info.absolutePath());

    if (!dir.exists()) {
        dir.mkpath(info.absolutePath());
    }
    KIS_SAFE_ASSERT_RECOVER_NOOP(dir.exists());

    QStringList filesList = dir.entryList({ info.fileName() });

    if (!filesList.isEmpty()) {
        // we are in batch mode!
        if (!m_d->updater) {
            return KisImportExportFilter::CreationError;
        }

        QStringList truncatedList = filesList;

        while (truncatedList.size() > 3) {
            truncatedList.takeLast();
        }

        QString exampleFiles = truncatedList.join(", ");
        if (truncatedList.size() != filesList.size()) {
            exampleFiles += QString(", ...");
        }

        QMessageBox::StandardButton result =
                QMessageBox::warning(0,
                                     i18n("Delete old frames?"),
                                     i18n("Frames with the same naming "
                                          "scheme exist in the destination "
                                          "directory. They are going to be "
                                          "deleted, continue?\n\n"
                                          "Directory: %1\n"
                                          "Files: %2",
                                          info.absolutePath(), exampleFiles),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);

        if (result == QMessageBox::Yes) {
            Q_FOREACH (const QString &file, filesList) {
                if (!dir.remove(file)) {
                    QMessageBox::critical(0,
                                          i18n("Failed to delete"),
                                          i18n("Failed to delete an old frame file:\n\n"
                                               "%1\n\n"
                                               "Rendering cancelled.", dir.absoluteFilePath(file)));
                    return KisImportExportFilter::CreationError;
                }
            }
        } else {
            return KisImportExportFilter::UserCancelled;
        }
    }

    m_d->exporter.setExportConfiguration(cfg);
    return m_d->exporter.exportAnimation();
}

KisImportExportFilter::ConversionStatus KisAnimationExportSaver::saveFrameCallback(int time, KisPaintDeviceSP frame, KisPropertiesConfigurationSP exportConfiguration)
{
    KisImportExportFilter::ConversionStatus status = KisImportExportFilter::OK;

    QString frameNumber = QString("%1").arg(time - m_d->firstFrame + m_d->sequenceNumberingOffset, 4, 10, QChar('0'));
    QString filename = m_d->filenamePrefix + frameNumber + m_d->filenameSuffix;

    QRect rc = m_d->image->bounds();
    KisPainter::copyAreaOptimized(rc.topLeft(), frame, m_d->tmpDevice, rc);

    if (!m_d->tmpDoc->exportDocumentSync(QUrl::fromLocalFile(filename), m_d->outputMimeType, exportConfiguration)) {
        status = KisImportExportFilter::InternalError;
    }

    return status;
}

QString KisAnimationExportSaver::savedFilesMask() const
{
    return m_d->filenamePrefix + "%04d" + m_d->filenameSuffix;
}

QString KisAnimationExportSaver::savedFilesMaskWildcard() const
{
    return m_d->filenamePrefix + "????" + m_d->filenameSuffix;
}
