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

#include "kis_animation_importer.h"

#include <QStatusBar>

#include "KoColorSpace.h"
#include "KisPart.h"
#include "KisDocument.h"
#include "kis_image.h"
#include "kis_undo_adapter.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_raster_keyframe_channel.h"
#include "commands/kis_image_layer_add_command.h"

struct KisAnimationImporter::Private
{
    KisImageSP image;
    KisDocument *document;
    bool stop;
};

KisAnimationImporter::KisAnimationImporter(KisImageSP image)
    : m_d(new Private())
{
    m_d->document= 0;
    m_d->image = image;
    m_d->stop = false;
}

KisAnimationImporter::KisAnimationImporter(KisDocument* document)
    : m_d(new Private())
{
    m_d->document= document;
    m_d->image = document->image();
    m_d->stop = false;
}

KisAnimationImporter::~KisAnimationImporter()
{}

KisImportExportFilter::ConversionStatus KisAnimationImporter::import(QStringList files, int firstFrame, int step)
{
    Q_ASSERT(step > 0);

    bool batchMode;

    if (m_d->document) {
        batchMode= m_d->document->fileBatchMode();

        if (!batchMode) {
            emit m_d->document->statusBarMessage(i18n("Import frames"));
            emit m_d->document->sigProgress(0);
            connect(m_d->document, SIGNAL(sigProgressCanceled()), this, SLOT(cancel()));
        }
    } else batchMode = false;

    m_d->image->lock();
    KisUndoAdapter *undo = m_d->image->undoAdapter();
    undo->beginMacro(kundo2_i18n("Import animation"));

    QScopedPointer<KisDocument> importDoc(KisPart::instance()->createDocument());
    importDoc->setFileBatchMode(true);

    KisImportExportFilter::ConversionStatus status = KisImportExportFilter::OK;
    int frame = firstFrame;

    KisRasterKeyframeChannel *contentChannel = 0;

    Q_FOREACH(QString file, files) {
        bool successfullyLoaded = importDoc->openUrl(QUrl::fromLocalFile(file), KisDocument::DontAddToRecent);
        if (!successfullyLoaded) {
            status = KisImportExportFilter::InternalError;
            break;
        }

        if (m_d->stop) {
            status = KisImportExportFilter::ProgressCancelled;
            break;
        }

        if (frame == firstFrame) {
            const KoColorSpace *cs = importDoc->image()->colorSpace();
            KisPaintLayerSP paintLayer = new KisPaintLayer(m_d->image, m_d->image->nextLayerName(), OPACITY_OPAQUE_U8, cs);
            undo->addCommand(new KisImageLayerAddCommand(m_d->image, paintLayer, m_d->image->rootLayer(), m_d->image->rootLayer()->childCount()));

            paintLayer->enableAnimation();
            contentChannel = qobject_cast<KisRasterKeyframeChannel*>(paintLayer->getKeyframeChannel(KisKeyframeChannel::Content.id(), true));
        }

        if (!batchMode) {
            emit m_d->document->sigProgress((frame - firstFrame) * 100 / (step * files.size()));
        }
        contentChannel->importFrame(frame, importDoc->image()->projection(), NULL);
        frame += step;
    }

    if (!batchMode) {
        disconnect(m_d->document, SIGNAL(sigProgressCanceled()), this, SLOT(cancel()));
        emit m_d->document->sigProgress(100);
        emit m_d->document->clearStatusBarMessage();
    }
    undo->endMacro();
    m_d->image->unlock();

    return status;
}

void KisAnimationImporter::cancel()
{
    m_d->stop = true;
}
