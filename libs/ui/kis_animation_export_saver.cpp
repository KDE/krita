/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_animation_export_saver.h"

#include <QMessageBox>
#include <QEventLoop>
#include <QProgressDialog>
#include <QFileInfo>
#include <QDir>

#include "kis_animation_exporter.h"
#include <KoUpdater.h>

#include <KisMimeDatabase.h>
#include "KisPart.h"
#include "KisDocument.h"

#include "kis_image.h"
#include "kis_assert.h"

#include "kis_paint_layer.h"
#include "kis_painter.h"


/**********************************************************************************/
/*****  KisAnimationExportSaver::Private  *****************************************/
/**********************************************************************************/

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
        , exporter(_document->image())
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
        tmpImage->addNode(paintLayer, tmpImage->root(), KisLayerSP(0));

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
    KisPropertiesConfigurationSP exportConfiguration;

    QList<KisTimeRange> dirtyRangesList;
    QList<KisTimeRange> rangesInProgress;
    int numDirtyRanges = 0;

    QString filenamePrefix;
    QString filenameSuffix;

    QProgressDialog progress;
    QEventLoop waitLoop;
    KisImportExportFilter::ConversionStatus status;

    static QList<KisTimeRange> calculateDirtyRanges(KisNodeSP rootNode, const KisTimeRange &exportRange);
    KisTimeRange takeRangeInProgress(int time);

    bool isBatchMode() const {
        return !updater;
    }

    int numDirtyRangesLeft() const {
        return dirtyRangesList.size() + rangesInProgress.size();
    }
};

QList<KisTimeRange> KisAnimationExportSaver::Private::calculateDirtyRanges(KisNodeSP rootNode, const KisTimeRange &exportRange)
{
    Q_UNUSED(rootNode);
    // TODO: implement a real dirty frames calculation
    QList<KisTimeRange> result;
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!exportRange.isInfinite(), result);

    for (int i = exportRange.start(); i <= exportRange.end(); i++) {
        result << KisTimeRange(i, 1);
    }
    return result;
}

KisTimeRange KisAnimationExportSaver::Private::takeRangeInProgress(int time)
{
    KisTimeRange renderedRange;
    KIS_SAFE_ASSERT_RECOVER_NOOP(!renderedRange.isValid());

    for (auto it = rangesInProgress.begin(); it != rangesInProgress.end();) {
        if (it->contains(time)) {
            renderedRange = *it;
            it = rangesInProgress.erase(it);
            break;
        }
        ++it;
    }

    KIS_SAFE_ASSERT_RECOVER_NOOP(renderedRange.isValid());

    return renderedRange;
}

/**********************************************************************************/
/*****  KisAnimationExportSaver  **************************************************/
/**********************************************************************************/

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
    m_d->exporter.setSaveFrameCallback(std::bind(&KisAnimationExportSaver::saveFrameCallback, this, _1, _2));

    connect(&m_d->exporter, SIGNAL(sigFramePrepared(int)), SLOT(slotFrameRenderingCompleted(int)));
    connect(&m_d->exporter,
            SIGNAL(sigFrameFailed(int,KisImportExportFilter::ConversionStatus)),
            SLOT(slotFrameRenderingFailed(int,KisImportExportFilter::ConversionStatus)));
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
        if (m_d->isBatchMode()) {
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

    m_d->exportConfiguration = cfg;

    m_d->dirtyRangesList =
        Private::calculateDirtyRanges(
            m_d->image->root(),
            KisTimeRange::fromTime(m_d->firstFrame, m_d->lastFrame));
    m_d->numDirtyRanges = m_d->dirtyRangesList.size();

    if (!m_d->numDirtyRanges) {
        return KisImportExportFilter::OK;
    }

    return exportAnimationWithProgress();
}

KisImportExportFilter::ConversionStatus KisAnimationExportSaver::exportAnimationWithProgress()
{
    if (!m_d->isBatchMode()) {
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

    KIS_ASSERT_RECOVER_RETURN_VALUE(!m_d->image->locked(), KisImportExportFilter::InternalError);

    m_d->status = KisImportExportFilter::OK;

    tryInitiateFrameRendering();
    updateProgressStatus();

    m_d->waitLoop.exec();

    if (m_d->updater) {
        m_d->updater->setProgress(100);
    }

    if (!m_d->isBatchMode()) {
        m_d->progress.reset();
    }

    // TODO: add feedback!
    m_d->image->waitForDone();

    return m_d->status;
}

void KisAnimationExportSaver::tryInitiateFrameRendering()
{
    if (!m_d->dirtyRangesList.isEmpty()) {
        const KisTimeRange range = m_d->dirtyRangesList.takeFirst();
        m_d->exporter.startFrameRegeneration(range.start());
        m_d->rangesInProgress << range;
    }
}

KisImportExportFilter::ConversionStatus KisAnimationExportSaver::saveFrameCallback(int time, KisPaintDeviceSP frame)
{
    KisTimeRange range = m_d->takeRangeInProgress(time);

    KIS_SAFE_ASSERT_RECOVER(range.isValid()) {
        range = KisTimeRange(time, 1);
    }

    KisImportExportFilter::ConversionStatus status = KisImportExportFilter::OK;

    for (int i = range.start(); i <= range.end(); i++) {
        QString frameNumber = QString("%1").arg(i - m_d->firstFrame + m_d->sequenceNumberingOffset, 4, 10, QChar('0'));
        QString filename = m_d->filenamePrefix + frameNumber + m_d->filenameSuffix;

        QRect rc = m_d->image->bounds();
        KisPainter::copyAreaOptimized(rc.topLeft(), frame, m_d->tmpDevice, rc);

        if (!m_d->tmpDoc->exportDocumentSync(QUrl::fromLocalFile(filename), m_d->outputMimeType, m_d->exportConfiguration)) {
            status = KisImportExportFilter::InternalError;
            break;
        }
    }

    return status;
}

void KisAnimationExportSaver::slotFrameRenderingCompleted(int time)
{
    Q_UNUSED(time);

    if (!m_d->numDirtyRangesLeft()) {
        m_d->waitLoop.quit();
    } else {
        tryInitiateFrameRendering();
    }

    updateProgressStatus();
}

void KisAnimationExportSaver::updateProgressStatus()
{
    const int processedRanges = m_d->numDirtyRanges - m_d->numDirtyRangesLeft();
    const int progressPercent = qRound(qreal(processedRanges) / m_d->numDirtyRanges * 100.0);

    if (!m_d->isBatchMode()) {
        const QString dialogText =
                i18n("Exporting Frame %1 of %2",
                     processedRanges,
                     m_d->numDirtyRanges);

        m_d->progress.setLabelText(dialogText);
        m_d->progress.setValue(progressPercent);
    }

    if (m_d->updater) {
        m_d->updater->setProgress(progressPercent);
    }
}

void KisAnimationExportSaver::slotFrameRenderingFailed(int time, KisImportExportFilter::ConversionStatus status)
{
    m_d->waitLoop.quit();
    m_d->status = KisImportExportFilter::CreationError;
}

QString KisAnimationExportSaver::savedFilesMask() const
{
    return m_d->filenamePrefix + "%04d" + m_d->filenameSuffix;
}

QString KisAnimationExportSaver::savedFilesMaskWildcard() const
{
    return m_d->filenamePrefix + "????" + m_d->filenameSuffix;
}
