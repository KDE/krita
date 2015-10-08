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

#include <QProgressDialog>
#include <QDesktopServices>

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
    QProgressDialog progressDialog;
    KisAnimationExporter *exporter;

    Private(QWidget *parent)
        : parentWidget(parent),
          progressDialog(i18n("Exporting frames..."), i18n("Cancel"), 0, 0),
          exporter(0)
    {}
};

KisAnimationExporterUI::KisAnimationExporterUI(QWidget *parent)
    : m_d(new Private(parent))
{
    connect(&m_d->progressDialog, SIGNAL(canceled()), this, SLOT(cancel()));
}

KisAnimationExporterUI::~KisAnimationExporterUI()
{
    if (m_d->exporter) {
        delete m_d->exporter;
    }
}

void KisAnimationExporterUI::exportSequence(KisDocument *document)
{
    KoFileDialog dialog(m_d->parentWidget, KoFileDialog::SaveFile, "krita/exportsequence");
    dialog.setCaption(i18n("Export sequence"));
    dialog.setDefaultDir(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation));
    dialog.setMimeTypeFilters(KisImportExportManager::mimeFilter("application/x-krita", KisImportExportManager::Export));
    QString filename = dialog.filename();

    if (filename.isEmpty()) return;

    const KisTimeRange currentRange = document->image()->animationInterface()->currentRange();
    int firstFrame = currentRange.start();
    int lastFrame = currentRange.end();

    m_d->progressDialog.setWindowModality(Qt::ApplicationModal);
    m_d->progressDialog.setMinimum(firstFrame);
    m_d->progressDialog.setMaximum(lastFrame);
    m_d->progressDialog.setValue(firstFrame);

    m_d->exporter = new KisAnimationExporter(document, filename, firstFrame, lastFrame);
    connect(m_d->exporter, SIGNAL(sigExportProgress(int)), this, SLOT(progress(int)));

    m_d->exporter->startExport();
    m_d->progressDialog.exec();
}

void KisAnimationExporterUI::progress(int currentFrame)
{
    m_d->progressDialog.setValue(currentFrame);
}

void KisAnimationExporterUI::cancel()
{
    if (m_d->exporter) {
        m_d->exporter->stopExport();
    }
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
    QMutex mutex;
    QWaitCondition exportFinished;

    Private(KisDocument *document, int fromTime, int toTime)
        : document(document),
          image(document->image()),
          firstFrame(fromTime),
          lastFrame(toTime),
          tmpDoc(KisPart::instance()->createDocument()),
          exporting(false)
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

    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(baseFilename);
    QString mimefilter = mime.name();
    m_d->tmpDoc->setOutputMimeType(mimefilter.toLatin1());
    m_d->tmpDoc->setSaveInBatchMode(true);

    connect(this, SIGNAL(sigFrameReadyToSave()), this, SLOT(frameReadyToSave()), Qt::QueuedConnection);
}

KisAnimationExporter::~KisAnimationExporter()
{
}

void KisAnimationExporter::startExport()
{
    m_d->exporting = true;
    m_d->currentFrame = m_d->firstFrame;
    connect(m_d->image->animationInterface(), SIGNAL(sigFrameReady(int)), this, SLOT(frameReadyToCopy(int)), Qt::DirectConnection);

    m_d->image->animationInterface()->requestFrameRegeneration(m_d->currentFrame, m_d->image->bounds());
}

void KisAnimationExporter::stopExport()
{
    if (!m_d->exporting) return;

    m_d->exporting = false;

    disconnect(m_d->image->animationInterface(), 0, this, 0);
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
    m_d->tmpDoc->exportDocument(QUrl::fromLocalFile(filename));

    emit sigExportProgress(m_d->currentFrame);

    if (m_d->exporting && m_d->currentFrame < m_d->lastFrame) {
        m_d->currentFrame++;
        m_d->image->animationInterface()->requestFrameRegeneration(m_d->currentFrame, m_d->image->bounds());
    } else {
        stopExport();
    }
}

#include "kis_animation_exporter.moc"
