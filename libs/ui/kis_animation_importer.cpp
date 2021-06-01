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
#include "kis_assign_profile_processing_visitor.h"
#include "commands/kis_image_layer_add_command.h"
#include <QRegExp>

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

KisImportExportErrorCode KisAnimationImporter::import(QStringList files, int firstFrame, int step, bool autoAddHoldframes, bool startfrom0, int isAscending, bool assignDocumentProfile)
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

    KisPaintLayerSP paintLayer = 0;
    KisRasterKeyframeChannel *contentChannel = 0;

    const QRegExp rx(QLatin1String("(\\d+)"));    //regex for extracting numbers
    QStringList fileNumberRxList;

    int pos = 0;

    while ((pos = rx.indexIn(files.at(0), pos)) != -1) {
        fileNumberRxList << rx.cap(1);
        pos += rx.matchedLength();
    }

    int firstFrameNumber = 0;
    bool ok;

    if (!fileNumberRxList.isEmpty()) {
        fileNumberRxList.last().toInt(&ok);    // selects the last number of file name of the first frame (useful for descending order)
    }

    if (firstFrameNumber == 0){
        startfrom0 = false;     // if enabled, the zeroth frame will be places in -1 slot, leading to an error
    }

    fileNumberRxList.clear();

    int offset = (startfrom0 ? 1 : 0);    //offset added to consider file numbering starts from 1 instead of 0

    int autoframe = 0;

    Q_FOREACH(QString file, files) {
        bool successfullyLoaded = importDoc->openPath(file, KisDocument::DontAddToRecent);
        if (!successfullyLoaded) {
            status = ImportExportCodes::InternalError;
            break;
        }



        if (frame == firstFrame) {
            const KoColorSpace *cs = importDoc->image()->colorSpace();
            paintLayer = new KisPaintLayer(m_d->image, m_d->image->nextLayerName(), OPACITY_OPAQUE_U8, cs);
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

        if (!autoAddHoldframes) {
            contentChannel->importFrame(frame, importDoc->image()->projection(), NULL);    // as first frame added will go to second slot i.e #1 instead of #0
        } else {
            pos = 0;

            while ((pos = rx.indexIn(file, pos)) != -1) {
                fileNumberRxList << rx.cap(1);
                pos += rx.matchedLength();
            }

            int filenum = fileNumberRxList.last().toInt(&ok);

            if (isAscending == 0) {
                autoframe = firstFrame + filenum - offset;
            } else {
                autoframe = firstFrame + (firstFrameNumber - filenum); //places the first frame #0 (or #1) slot, and later frames are added as per the difference
            }

            if (ok) {
                contentChannel->importFrame(autoframe , importDoc->image()->projection(), NULL);
            } else {
                // if it fails to extract a number, the next frame will simply be added to next slot
                contentChannel->importFrame(autoframe + 1, importDoc->image()->projection(), NULL);
            }
            fileNumberRxList.clear();
        }

        frame += step;
        filesProcessed++;
    }

    if (paintLayer && assignDocumentProfile) {

        if (paintLayer->colorSpace()->colorModelId() == m_d->image->colorSpace()->colorModelId()) {

            const KoColorSpace *srcColorSpace = paintLayer->colorSpace();
            const KoColorSpace *dstColorSpace = KoColorSpaceRegistry::instance()->colorSpace(
                        srcColorSpace->colorModelId().id()
                        , srcColorSpace->colorDepthId().id()
                        , m_d->image->colorSpace()->profile());

            KisAssignProfileProcessingVisitor *visitor = new KisAssignProfileProcessingVisitor(srcColorSpace, dstColorSpace);
            visitor->visit(paintLayer.data(), undo);
        }
    }

    undo->endMacro();

    return status;
}

void KisAnimationImporter::cancel()
{
    m_d->stop = true;
}
