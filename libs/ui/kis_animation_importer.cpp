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
#include <KoUpdater.h>
#include <QApplication>
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
    KoUpdaterPtr updater;
};

KisAnimationImporter::KisAnimationImporter(KisImageSP image, KoUpdaterPtr updater)
    : m_d(new Private())
{
    m_d->document = 0;
    m_d->image = image;
    m_d->stop = false;
    m_d->updater = updater;
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

    m_d->image->lock();
    KisUndoAdapter *undo = m_d->image->undoAdapter();
    undo->beginMacro(kundo2_i18n("Import animation"));

    QScopedPointer<KisDocument> importDoc(KisPart::instance()->createDocument());
    importDoc->setFileBatchMode(true);

    KisImportExportFilter::ConversionStatus status = KisImportExportFilter::OK;
    int frame = firstFrame;
    int filesProcessed = 0;

    if (m_d->updater) {
        m_d->updater->setRange(0, files.size());
    }

    KisRasterKeyframeChannel *contentChannel = 0;
    Q_FOREACH(QString file, files) {
        bool successfullyLoaded = importDoc->openUrl(QUrl::fromLocalFile(file), KisDocument::DontAddToRecent);
        if (!successfullyLoaded) {
            status = KisImportExportFilter::InternalError;
            break;
        }

        if (frame == firstFrame) {
            const KoColorSpace *cs = importDoc->image()->colorSpace();
            KisPaintLayerSP paintLayer = new KisPaintLayer(m_d->image, m_d->image->nextLayerName(), OPACITY_OPAQUE_U8, cs);
            undo->addCommand(new KisImageLayerAddCommand(m_d->image, paintLayer, m_d->image->rootLayer(), m_d->image->rootLayer()->childCount()));

            paintLayer->enableAnimation();
            contentChannel = qobject_cast<KisRasterKeyframeChannel*>(paintLayer->getKeyframeChannel(KisKeyframeChannel::Content.id(), true));
        }

        if (m_d->updater) {
            if (m_d->updater->interrupted()) {
                m_d->stop = true;
            } else {
                m_d->updater->setValue(filesProcessed + 1);

                // the updater doesn't call that automatically,
                // it is "threaded" by default
                qApp->processEvents();
            }
        }

        if (m_d->stop) {
            status = KisImportExportFilter::ProgressCancelled;
            break;
        }

        contentChannel->importFrame(frame, importDoc->image()->projection(), NULL);
        frame += step;
        filesProcessed++;
    }

    undo->endMacro();
    m_d->image->unlock();

    return status;
}

void KisAnimationImporter::cancel()
{
    m_d->stop = true;
}
