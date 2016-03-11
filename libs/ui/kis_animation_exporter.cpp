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
#include <QWaitCondition>
#include <QMimeDatabase>
#include <QEventLoop>

#include "KoFileDialog.h"
#include "KisDocument.h"
#include "kis_image.h"
#include "KisImportExportManager.h"
#include "kis_image_animation_interface.h"
#include "KisPart.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_time_range.h"
#include "kis_painter.h"

struct KisAnimationExporterUI::Private
{
    QWidget *parentWidget;
    KisAnimationExporter *exporter;

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
    KoFileDialog dialog(m_d->parentWidget, KoFileDialog::SaveFile, "krita/exportsequence");
    dialog.setCaption(i18n("Export sequence"));
    dialog.setDefaultDir(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation));
    dialog.setMimeTypeFilters(KisImportExportManager::mimeFilter("application/x-krita", KisImportExportManager::Export));
    QString filename = dialog.filename();

    if (filename.isEmpty()) return KisImportExportFilter::FileNotFound;

    const KisTimeRange fullClipRange = document->image()->animationInterface()->fullClipRange();
    int firstFrame = fullClipRange.start();
    int lastFrame = fullClipRange.end();

    m_d->exporter = new KisAnimationExporter(document, filename, firstFrame, lastFrame);
    return m_d->exporter->exportAnimation();
}


struct KisAnimationExporter::Private
{
    KisDocument *document;
    KisImageWSP image;

    QString filenamePrefix;
    QString filenameSuffix;

    int firstFrame;
    int currentFrame;
    int lastFrame;

    QScopedPointer<KisDocument> tmpDoc;
    KisImageWSP tmpImage;
    KisPaintDeviceSP tmpDevice;

    bool exporting;
    bool batchMode;
    KisImportExportFilter::ConversionStatus status;

    QMutex mutex;
    QWaitCondition exportFinished;

    Private(KisDocument *document, int fromTime, int toTime)
        : document(document),
          image(document->image()),
          firstFrame(fromTime),
          lastFrame(toTime),
          tmpDoc(KisPart::instance()->createDocument()),
          exporting(false),
          batchMode(false)
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
};

KisAnimationExporter::KisAnimationExporter(KisDocument *document, const QString &baseFilename, int fromTime, int toTime)
    : m_d(new Private(document, fromTime, toTime))
{
    int baseLength = baseFilename.lastIndexOf(".");
    if (baseLength > -1) {
        m_d->filenamePrefix = baseFilename.left(baseLength);
        m_d->filenameSuffix = baseFilename.right(baseFilename.length() - baseLength);
    } else {
        m_d->filenamePrefix = baseFilename;
    }
    m_d->batchMode = document->fileBatchMode();

    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(baseFilename);
    QString mimefilter = mime.name();
    m_d->tmpDoc->setOutputMimeType(mimefilter.toLatin1());
    m_d->tmpDoc->setFileBatchMode(true);

    connect(this, SIGNAL(sigFrameReadyToSave()), this, SLOT(frameReadyToSave()), Qt::QueuedConnection);
}

KisAnimationExporter::~KisAnimationExporter()
{
}

KisImportExportFilter::ConversionStatus KisAnimationExporter::exportAnimation()
{

    if (!m_d->batchMode) {
        emit m_d->document->statusBarMessage(i18n("Export frames"));
        emit m_d->document->sigProgress(0);
        connect(m_d->document, SIGNAL(sigProgressCanceled()), this, SLOT(cancel()));
    }
    m_d->status = KisImportExportFilter::OK;
    m_d->exporting = true;
    m_d->currentFrame = m_d->firstFrame;
    connect(m_d->image->animationInterface(), SIGNAL(sigFrameReady(int)), this, SLOT(frameReadyToCopy(int)), Qt::DirectConnection);
    m_d->image->animationInterface()->requestFrameRegeneration(m_d->currentFrame, m_d->image->bounds());

    QEventLoop loop;
    loop.connect(this, SIGNAL(sigFinished()), SLOT(quit()));
    loop.exec();

    return m_d->status;
}

void KisAnimationExporter::stopExport()
{
    if (!m_d->exporting) return;

    m_d->exporting = false;

    disconnect(m_d->image->animationInterface(), 0, this, 0);

    if (!m_d->batchMode) {
        disconnect(m_d->document, SIGNAL(sigProgressCanceled()), this, SLOT(cancel()));
        emit m_d->document->sigProgress(100);
        emit m_d->document->clearStatusBarMessage();
    }
    emit sigFinished();
}

void KisAnimationExporter::cancel()
{
    m_d->status = KisImportExportFilter::ProgressCancelled;
    stopExport();
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
    QString frameNumber = QString("%1").arg(m_d->currentFrame, 4, 10, QChar('0'));
    QString filename = m_d->filenamePrefix + frameNumber + m_d->filenameSuffix;

    if (m_d->tmpDoc->exportDocument(QUrl::fromLocalFile(filename))) {

        if (m_d->exporting && m_d->currentFrame < m_d->lastFrame) {
            if (!m_d->batchMode) {
                emit m_d->document->sigProgress((m_d->currentFrame - m_d->firstFrame) * 100 /
                                                (m_d->lastFrame - m_d->firstFrame));
            }
            m_d->currentFrame++;
            m_d->image->animationInterface()->requestFrameRegeneration(m_d->currentFrame, m_d->image->bounds());
            return; //continue
        }
    } else {
        //error
        m_d->status = KisImportExportFilter::InternalError;
    }
    stopExport(); //finish
}

#include "kis_animation_exporter.moc"
