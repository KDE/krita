/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisAsyncAnimationFramesSavingRenderer.h"

#include "kis_image.h"
#include "kis_paint_device.h"
#include "KisImportExportFilter.h"
#include "KisPart.h"
#include "KisDocument.h"
#include "kis_time_span.h"
#include "kis_paint_layer.h"


struct KisAsyncAnimationFramesSavingRenderer::Private
{
    Private(KisImageSP image, const KisTimeSpan &_range, int _sequenceNumberingOffset, bool _onlyNeedsUniqueFrames, KisPropertiesConfigurationSP _exportConfiguration)
        : savingDoc(KisPart::instance()->createDocument()),
          range(_range),
          sequenceNumberingOffset(_sequenceNumberingOffset),
          onlyNeedsUniqueFrames(_onlyNeedsUniqueFrames),
          exportConfiguration(_exportConfiguration)
    {

        savingDoc->setInfiniteAutoSaveInterval();
        savingDoc->setFileBatchMode(true);

        KisImageSP savingImage = new KisImage(savingDoc->createUndoStore(),
                                              image->bounds().width(),
                                              image->bounds().height(),
                                              image->colorSpace(),
                                              QString());

        savingImage->setResolution(image->xRes(), image->yRes());
        savingDoc->setCurrentImage(savingImage);

        KisPaintLayer* paintLayer = new KisPaintLayer(savingImage, "paint device", 255);
        savingImage->addNode(paintLayer, savingImage->root(), KisLayerSP(0));

        savingDevice = paintLayer->paintDevice();
    }

    QScopedPointer<KisDocument> savingDoc;
    KisPaintDeviceSP savingDevice;

    KisTimeSpan range;
    int sequenceNumberingOffset = 0;

    bool onlyNeedsUniqueFrames;

    QString filenamePrefix;
    QString filenameSuffix;

    QByteArray outputMimeType;
    KisPropertiesConfigurationSP exportConfiguration;
};

KisAsyncAnimationFramesSavingRenderer::KisAsyncAnimationFramesSavingRenderer(KisImageSP image,
                                                                             const QString &fileNamePrefix,
                                                                             const QString &fileNameSuffix,
                                                                             const QByteArray &outputMimeType,
                                                                             const KisTimeSpan &range,
                                                                             const int sequenceNumberingOffset,
                                                                             const bool onlyNeedsUniqueFrames,
                                                                             KisPropertiesConfigurationSP exportConfiguration)
    : m_d(new Private(image, range, sequenceNumberingOffset, onlyNeedsUniqueFrames, exportConfiguration))
{
    m_d->filenamePrefix = fileNamePrefix;
    m_d->filenameSuffix = fileNameSuffix;
    m_d->outputMimeType = outputMimeType;

    connect(this, SIGNAL(sigCompleteRegenerationInternal(int)), SLOT(notifyFrameCompleted(int)));
    connect(this, SIGNAL(sigCancelRegenerationInternal(int)), SLOT(notifyFrameCancelled(int)));
}




KisAsyncAnimationFramesSavingRenderer::~KisAsyncAnimationFramesSavingRenderer()
{
}

void KisAsyncAnimationFramesSavingRenderer::frameCompletedCallback(int frame, const KisRegion &requestedRegion)
{
    KisImageSP image = requestedImage();
    if (!image) return;

    KIS_SAFE_ASSERT_RECOVER (requestedRegion == image->bounds()) {
        emit sigCancelRegenerationInternal(frame);
        return;
    }

    m_d->savingDevice->makeCloneFromRough(image->projection(), image->bounds());

    KisImportExportErrorCode status = ImportExportCodes::OK;

    QString frameNumber = QString("%1").arg(frame + m_d->sequenceNumberingOffset, 4, 10, QChar('0'));
    QString filename = m_d->filenamePrefix + frameNumber + m_d->filenameSuffix;

    if (!m_d->savingDoc->exportDocumentSync(QUrl::fromLocalFile(filename), m_d->outputMimeType, m_d->exportConfiguration)) {
        status = ImportExportCodes::InternalError;
    }

    //Get all identical frames to this one and either copy or symlink based on settings.
    KisTimeSpan identicals = KisTimeSpan::calculateIdenticalFramesRecursive(image->root(), frame);
    identicals &= m_d->range;
    if( !m_d->onlyNeedsUniqueFrames && identicals.start() < identicals.end() ) {
        for (int identicalFrame = (identicals.start() + 1); identicalFrame <= identicals.end(); identicalFrame++) {
            QString identicalFrameNumber = QString("%1").arg(identicalFrame + m_d->sequenceNumberingOffset, 4, 10, QChar('0'));
            QString identicalFrameName = m_d->filenamePrefix + identicalFrameNumber + m_d->filenameSuffix;

            QFile::copy(filename, identicalFrameName);

            /*  This would be nice to do but sym-linking on windows isn't possible without
             *  way more other work to be done. This works on linux though!
             *
             *  if (m_d->linkRedundantFrames) {
             *      QFile::link(filename, identicalFrameName);
             *  } else {
             *      QFile::copy(filename, identicalFrameName);
             *  }
             */
        }
    }

    if (status.isOk()) {
        emit sigCompleteRegenerationInternal(frame);
    } else {
        emit sigCancelRegenerationInternal(frame);
    }
}

void KisAsyncAnimationFramesSavingRenderer::frameCancelledCallback(int frame)
{
    notifyFrameCancelled(frame);
}

