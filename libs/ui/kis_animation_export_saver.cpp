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
    m_d->exporter.setSaveFrameCallback(std::bind(&KisAnimationExportSaver::saveFrameCallback, this, _1, _2));
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

    m_d->exportConfiguration = cfg;
    return m_d->exporter.exportAnimation();
}

KisImportExportFilter::ConversionStatus KisAnimationExportSaver::saveFrameCallback(int time, KisPaintDeviceSP frame)
{
    KisImportExportFilter::ConversionStatus status = KisImportExportFilter::OK;

    QString frameNumber = QString("%1").arg(time - m_d->firstFrame + m_d->sequenceNumberingOffset, 4, 10, QChar('0'));
    QString filename = m_d->filenamePrefix + frameNumber + m_d->filenameSuffix;

    QRect rc = m_d->image->bounds();
    KisPainter::copyAreaOptimized(rc.topLeft(), frame, m_d->tmpDevice, rc);

    if (!m_d->tmpDoc->exportDocumentSync(QUrl::fromLocalFile(filename), m_d->outputMimeType, m_d->exportConfiguration)) {
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
