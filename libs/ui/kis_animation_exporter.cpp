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

KisImportExportFilter::ConversionStatus KisAnimationExporterUI::exportSequence(KisDocument *document)
{
    KoFileDialog dialog(m_d->parentWidget, KoFileDialog::SaveFile, "exportsequence");
    dialog.setCaption(i18n("Export sequence"));
    dialog.setDefaultDir(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation));
    dialog.setMimeTypeFilters(KisImportExportManager::mimeFilter(KisImportExportManager::Export));
    QString filename = dialog.filename();

    // if the user presses cancel, it returns empty
    if (filename.isEmpty()) return KisImportExportFilter::UserCancelled;

    const KisTimeRange fullClipRange = document->image()->animationInterface()->fullClipRange();
    int firstFrame = fullClipRange.start();
    int lastFrame = fullClipRange.end();

    m_d->exporter = new KisAnimationExportSaver(document, filename, firstFrame, lastFrame);
    return m_d->exporter->exportAnimation();
}


struct KisAnimationExporter::Private
{
    Private(KisDocument *document, int fromTime, int toTime)
        : document(document)
        , image(document->image())
        , firstFrame(fromTime)
        , lastFrame(toTime)
        , currentFrame(-1)
        , batchMode(document->fileBatchMode())
        , isCancelled(false)
        , status(KisImportExportFilter::OK)
        , tmpDevice(new KisPaintDevice(image->colorSpace()))
    {
    }

    KisDocument *document;
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

};

KisAnimationExporter::KisAnimationExporter(KisDocument *document, int fromTime, int toTime)
    : m_d(new Private(document, fromTime, toTime))
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
    QScopedPointer<QProgressDialog> progress;

    if (!m_d->batchMode) {
        QString message = i18n("Export frames...");
        progress.reset(new QProgressDialog(message, "", 0, 0, KisPart::instance()->currentMainwindow()));
        progress->setWindowModality(Qt::ApplicationModal);
        progress->setCancelButton(0);
        progress->setMinimumDuration(0);
        progress->setValue(0);

        emit m_d->document->statusBarMessage(message);
        emit m_d->document->sigProgress(0);
        connect(m_d->document, SIGNAL(sigProgressCanceled()), this, SLOT(cancel()));
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
        disconnect(m_d->document, SIGNAL(sigProgressCanceled()), this, SLOT(cancel()));
        emit m_d->document->sigProgress(100);
        emit m_d->document->clearStatusBarMessage();
        progress.reset();
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

    if (m_d->isCancelled) {
        m_d->status = KisImportExportFilter::UserCancelled;
        emit sigFinished();
        return;
    }

    KisImportExportFilter::ConversionStatus result =
        KisImportExportFilter::OK;
    int time = m_d->currentFrame;

    result = m_d->saveFrameCallback(time, m_d->tmpDevice, m_d->exportConfiguration);

    if (!m_d->batchMode) {
        int length = m_d->lastFrame - m_d->firstFrame + 1;
        emit m_d->document->sigProgress((time - m_d->firstFrame) * 100 / length);
    }

    qDebug() << result << time << m_d->lastFrame;

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
    Private(KisDocument *document, int fromTime, int toTime, int _sequenceNumberingOffset)
        : document(document)
        , image(document->image())
        , firstFrame(fromTime)
        , lastFrame(toTime)
        , sequenceNumberingOffset(_sequenceNumberingOffset)
        , tmpDoc(KisPart::instance()->createDocument())
        , exporter(document, fromTime, toTime)
    {
        tmpDoc->setAutoSave(0);

        tmpImage = new KisImage(tmpDoc->createUndoStore(),
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

    QScopedPointer<KisDocument> tmpDoc;
    KisImageSP tmpImage;
    KisPaintDeviceSP tmpDevice;

    KisAnimationExporter exporter;

    QString filenamePrefix;
    QString filenameSuffix;
};

KisAnimationExportSaver::KisAnimationExportSaver(KisDocument *document, const QString &baseFilename, int fromTime, int toTime, int sequenceNumberingOffset)
    : m_d(new Private(document, fromTime, toTime, sequenceNumberingOffset))
{
    int baseLength = baseFilename.lastIndexOf(".");
    if (baseLength > -1) {
        m_d->filenamePrefix = baseFilename.left(baseLength);
        m_d->filenameSuffix = baseFilename.right(baseFilename.length() - baseLength);
    } else {
        m_d->filenamePrefix = baseFilename;
    }

    QString mimefilter = KisMimeDatabase::mimeTypeForFile(baseFilename);
    m_d->tmpDoc->setOutputMimeType(mimefilter.toLatin1());
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
        if (m_d->document->fileBatchMode()) {
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
                                          "directory. The are going to be "
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
    if (!m_d->tmpDoc->exportDocument(QUrl::fromLocalFile(filename), exportConfiguration)) {
        status = KisImportExportFilter::CreationError;
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
