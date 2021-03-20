/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

KisImportExportErrorCode KisAnimationImporter::import(QStringList files, int firstFrame, int step)
{
    Q_ASSERT(step > 0);

    KisUndoAdapter *undo = m_d->image->undoAdapter();
    undo->beginMacro(kundo2_i18n("Import animation"));

    QScopedPointer<KisDocument> importDoc(KisPart::instance()->createDocument());
    importDoc->setFileBatchMode(true);

    KisImportExportErrorCode status = ImportExportCodes::OK;
    int frame = firstFrame;
    int filesProcessed = 0;

    if (m_d->updater) {
        m_d->updater->setRange(0, files.size());
    }

    KisRasterKeyframeChannel *contentChannel = 0;
    Q_FOREACH(QString file, files) {
        bool successfullyLoaded = importDoc->openPath(file, KisDocument::DontAddToRecent);
        if (!successfullyLoaded) {
            status = ImportExportCodes::InternalError;
            break;
        }

        if (frame == firstFrame) {
            const KoColorSpace *cs = importDoc->image()->colorSpace();
            KisPaintLayerSP paintLayer = new KisPaintLayer(m_d->image, m_d->image->nextLayerName(), OPACITY_OPAQUE_U8, cs);
            undo->addCommand(new KisImageLayerAddCommand(m_d->image, paintLayer, m_d->image->rootLayer(), m_d->image->rootLayer()->childCount()));

            paintLayer->enableAnimation();
            contentChannel = qobject_cast<KisRasterKeyframeChannel*>(paintLayer->getKeyframeChannel(KisKeyframeChannel::Raster.id(), true));
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
            status = ImportExportCodes::Cancelled;
            break;
        }

        contentChannel->importFrame(frame, importDoc->image()->projection(), NULL);
        frame += step;
        filesProcessed++;
    }

    undo->endMacro();

    return status;
}

void KisAnimationImporter::cancel()
{
    m_d->stop = true;
}
