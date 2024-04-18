/*
 *  SPDX-FileCopyrightText: 2016 Eugene Ingerman geneing at gmail dot com
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisImageThumbnailStrokeStrategy.h"

#include <kis_paint_device.h>
#include <kis_painter.h>
#include "krita_utils.h"
#include "kis_transform_worker.h"
#include "kis_filter_strategy.h"
#include <KoColorSpaceRegistry.h>
#include <KoUpdater.h>
#include "KisRunnableStrokeJobUtils.h"
#include "KisRunnableStrokeJobsInterface.h"

const qreal oversample = 2.;
const int thumbnailTileDim = 128;


KisImageThumbnailStrokeStrategyBase::
KisImageThumbnailStrokeStrategyBase(KisPaintDeviceSP device,
                                    const QRect& rect,
                                    const QSize& thumbnailSize,
                                    bool isPixelArt,
                                    const KoColorProfile *profile,
                                    KoColorConversionTransformation::Intent renderingIntent,
                                    KoColorConversionTransformation::ConversionFlags conversionFlags)
    : KisIdleTaskStrokeStrategy(QLatin1String("OverviewThumbnail"), kundo2_i18n("Update overview thumbnail")),
      m_device(device),
      m_rect(rect),
      m_thumbnailSize(thumbnailSize),
      m_isPixelArt(isPixelArt),
      m_profile(profile),
      m_renderingIntent(renderingIntent),
      m_conversionFlags(conversionFlags)
{
}

KisImageThumbnailStrokeStrategyBase::~KisImageThumbnailStrokeStrategyBase()
{
}

void KisImageThumbnailStrokeStrategyBase::initStrokeCallback()
{
    using KritaUtils::addJobConcurrent;
    using KritaUtils::addJobSequential;
    KisIdleTaskStrokeStrategy::initStrokeCallback();

    const QRect imageRect = m_device->defaultBounds()->bounds();

    m_thumbnailOversampledSize = oversample * m_thumbnailSize;

    if ((m_thumbnailOversampledSize.width() > imageRect.width()) || (m_thumbnailOversampledSize.height() > imageRect.height())) {
        m_thumbnailOversampledSize.scale(imageRect.size(), Qt::KeepAspectRatio);
    }

    m_thumbnailDevice = new KisPaintDevice(m_device->colorSpace());

    QVector<KisRunnableStrokeJobData*> jobs;

    QVector<QRect> tileRects = KritaUtils::splitRectIntoPatches(QRect(QPoint(0, 0), m_thumbnailOversampledSize), QSize(thumbnailTileDim, thumbnailTileDim));
    Q_FOREACH (const QRect &rc, tileRects) {
        addJobConcurrent(jobs, [this, tileRect = rc] () {
            //we aren't going to use oversample capability of createThumbnailDevice because it recomputes exact bounds for each small patch, which is
            //slow. We'll handle scaling separately.
            KisPaintDeviceSP thumbnailTile = m_device->createThumbnailDeviceOversampled(m_thumbnailOversampledSize.width(), m_thumbnailOversampledSize.height(), 1, m_device->defaultBounds()->bounds(), tileRect);
            KisPainter::copyAreaOptimized(tileRect.topLeft(), thumbnailTile, m_thumbnailDevice, tileRect);
        });
    }

    addJobSequential(jobs, [this] () {
        KoDummyUpdaterHolder updaterHolder;
        qreal xscale = m_thumbnailSize.width() / (qreal)m_thumbnailOversampledSize.width();
        qreal yscale = m_thumbnailSize.height() / (qreal)m_thumbnailOversampledSize.height();
        QString algorithm = m_isPixelArt ? "Box" : "Bilinear";
        KisTransformWorker worker(m_thumbnailDevice, xscale, yscale, 0.0, 0.0, 0.0, 0.0, 0.0,
                                  updaterHolder.updater(), KisFilterStrategyRegistry::instance()->value(algorithm));
        worker.run();

        reportThumbnailGenerationCompleted(m_thumbnailDevice, QRect(QPoint(0,0), m_thumbnailSize));
    });

    runnableJobsInterface()->addRunnableJobs(jobs);
}

void KisImageThumbnailStrokeStrategy::reportThumbnailGenerationCompleted(KisPaintDeviceSP device, const QRect &rect)
{
    QImage overviewImage;
    overviewImage = device->convertToQImage(m_profile,
                                            rect,
                                            m_renderingIntent,
                                            m_conversionFlags);
    emit thumbnailUpdated(overviewImage);
}
