/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2004 Clarence Dang <dang@kde.org>
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_brushop.h"

#include <QRect>

#include <kis_image.h>
#include <kis_vec.h>
#include <kis_debug.h>

#include <KoColorTransformation.h>
#include <KoColor.h>

#include <kis_brush.h>
#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_brush_based_paintop_settings.h>
#include <kis_lod_transform.h>
#include <kis_paintop_plugin_utils.h>
#include "krita_utils.h"
#include <QtConcurrent>
#include "kis_algebra_2d.h"
#include <KisDabRenderingExecutor.h>
#include <KisDabCacheUtils.h>
#include <KisRenderedDab.h>
#include "KisBrushOpResources.h"

#include <KisRunnableStrokeJobData.h>
#include <KisRunnableStrokeJobsInterface.h>

#include <QSharedPointer>
#include <QThread>
#include "kis_image_config.h"
#include "kis_wrapped_rect.h"


KisBrushOp::KisBrushOp(const KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP node, KisImageSP image)
    : KisBrushBasedPaintOp(settings, painter, SupportsGradientMode | SupportsLightnessMode)
    , m_opacityOption(node)
    , m_avgSpacing(50)
    , m_avgNumDabs(50)
    , m_avgUpdateTimePerDab(50)
    , m_idealNumRects(KisImageConfig(true).maxNumberOfThreads())
    , m_minUpdatePeriod(10)
    , m_maxUpdatePeriod(100)
{
    Q_UNUSED(image);
    Q_ASSERT(settings);

    /**
     * We do our own threading here, so we need to forbid the brushes
     * to do threading internally
     */
    m_brush->setThreadingAllowed(false);

    m_airbrushOption.readOptionSetting(settings);

    m_opacityOption.readOptionSetting(settings);
    m_flowOption.readOptionSetting(settings);
    m_sizeOption.readOptionSetting(settings);
    m_ratioOption.readOptionSetting(settings);
    m_spacingOption.readOptionSetting(settings);
    m_rateOption.readOptionSetting(settings);
    m_softnessOption.readOptionSetting(settings);
    m_rotationOption.readOptionSetting(settings);
    m_scatterOption.readOptionSetting(settings);
    m_sharpnessOption.readOptionSetting(settings);
    m_lightnessStrengthOption.readOptionSetting(settings);

    m_opacityOption.resetAllSensors();
    m_flowOption.resetAllSensors();
    m_sizeOption.resetAllSensors();
    m_ratioOption.resetAllSensors();
    m_rateOption.resetAllSensors();
    m_softnessOption.resetAllSensors();
    m_sharpnessOption.resetAllSensors();
    m_rotationOption.resetAllSensors();
    m_scatterOption.resetAllSensors();
    m_sharpnessOption.resetAllSensors();
    m_lightnessStrengthOption.resetAllSensors();

    m_rotationOption.applyFanCornersInfo(this);

    m_precisionOption.setHasImprecisePositionOptions(
        m_precisionOption.hasImprecisePositionOptions() |
        m_scatterOption.isChecked() |
        m_rotationOption.isChecked() |
        m_airbrushOption.enabled);

    m_brush->notifyBrushIsGoingToBeClonedForStroke();

    KisBrushSP baseBrush = m_brush;
    auto resourcesFactory =
        [baseBrush, settings, painter] () {
            KisDabCacheUtils::DabRenderingResources *resources =
                new KisBrushOpResources(settings, painter);
            resources->brush = baseBrush->clone().dynamicCast<KisBrush>();

            return resources;
        };


    m_dabExecutor.reset(
        new KisDabRenderingExecutor(
                    painter->device()->compositionSourceColorSpace(),
                    resourcesFactory,
                    painter->runnableStrokeJobsInterface(),
                    &m_mirrorOption,
                    &m_precisionOption));
}

KisBrushOp::~KisBrushOp()
{
}

KisSpacingInformation KisBrushOp::paintAt(const KisPaintInformation& info)
{
    if (!painter()->device()) return KisSpacingInformation(1.0);

    KisBrushSP brush = m_brush;
    Q_ASSERT(brush);
    if (!brush)
        return KisSpacingInformation(1.0);

    if (!brush->canPaintFor(info))
        return KisSpacingInformation(1.0);

    qreal scale = m_sizeOption.apply(info);
    scale *= KisLodTransform::lodToScale(painter()->device());
    if (checkSizeTooSmall(scale)) return KisSpacingInformation();

    qreal rotation = m_rotationOption.apply(info);
    qreal ratio = m_ratioOption.apply(info);

    KisDabShape shape(scale, ratio, rotation);
    QPointF cursorPos =
        m_scatterOption.apply(info,
                              brush->maskWidth(shape, 0, 0, info),
                              brush->maskHeight(shape, 0, 0, info));

    m_opacityOption.setFlow(m_flowOption.apply(info));

    quint8 dabOpacity = OPACITY_OPAQUE_U8;
    quint8 dabFlow = OPACITY_OPAQUE_U8;

    m_opacityOption.apply(info, &dabOpacity, &dabFlow);

    KisDabCacheUtils::DabRequestInfo request(painter()->paintColor(),
                                             cursorPos,
                                             shape,
                                             info,
                                             m_softnessOption.apply(info),
                                             m_lightnessStrengthOption.apply(info));

    m_dabExecutor->addDab(request, qreal(dabOpacity) / 255.0, qreal(dabFlow) / 255.0);


    KisSpacingInformation spacingInfo =
        effectiveSpacing(scale, rotation, &m_airbrushOption, &m_spacingOption, info);

    // gather statistics about dabs
    m_avgSpacing(spacingInfo.scalarApprox());

    return spacingInfo;
}

struct KisBrushOp::UpdateSharedState
{
    // rendering data
    KisPainter *painter = 0;
    QList<KisRenderedDab> dabsQueue;

    // speed metrics
    QVector<QPointF> dabPoints;
    QElapsedTimer dabRenderingTimer;

    // final report
    QVector<QRect> allDirtyRects;
};

void KisBrushOp::addMirroringJobs(Qt::Orientation direction,
                                  QVector<QRect> &rects,
                                  UpdateSharedStateSP state,
                                  QVector<KisRunnableStrokeJobData*> &jobs)
{
    jobs.append(new KisRunnableStrokeJobData(0, KisStrokeJobData::SEQUENTIAL));


    /**
     * Some KisRenderedDab may share their devices, so we should mirror them
     * carefully, avoiding doing that twice. KisDabRenderingQueue is implemented in
     * a way that duplicated dabs can go only sequentially, one after another, so
     * we don't have to use complex deduplication algorithms here.
     */
    KisFixedPaintDeviceSP prevDabDevice = 0;
    for (KisRenderedDab &dab : state->dabsQueue) {
        const bool skipMirrorPixels = prevDabDevice && prevDabDevice == dab.device;

        jobs.append(
            new KisRunnableStrokeJobData(
                [state, &dab, direction, skipMirrorPixels] () {
                    state->painter->mirrorDab(direction, &dab, skipMirrorPixels);
                },
                KisStrokeJobData::CONCURRENT));

        prevDabDevice = dab.device;
    }

    jobs.append(new KisRunnableStrokeJobData(0, KisStrokeJobData::SEQUENTIAL));

    for (QRect &rc : rects) {
        state->painter->mirrorRect(direction, &rc);

        jobs.append(
            new KisRunnableStrokeJobData(
                [rc, state] () {
                    state->painter->bltFixed(rc, state->dabsQueue);
                },
                KisStrokeJobData::CONCURRENT));
    }

    state->allDirtyRects.append(rects);
}

std::pair<int, bool> KisBrushOp::doAsyncronousUpdate(QVector<KisRunnableStrokeJobData*> &jobs)
{
    bool someDabsAreStillInQueue = false;
    const bool hasPreparedDabsAtStart = m_dabExecutor->hasPreparedDabs();

    if (!m_updateSharedState && hasPreparedDabsAtStart) {

        m_updateSharedState = toQShared(new UpdateSharedState());
        UpdateSharedStateSP state = m_updateSharedState;

        state->painter = painter();

        {
            const qreal dabRenderingTime = m_dabExecutor->averageDabRenderingTime();
            const qreal totalRenderingTimePerDab = dabRenderingTime + m_avgUpdateTimePerDab.rollingMeanSafe();

            // we limit the number of fetched dabs to fit the maximum update period and not
            // make visual hiccups
            const int dabsLimit =
                totalRenderingTimePerDab > 0 ?
                    qMax(10, int(m_maxUpdatePeriod  / totalRenderingTimePerDab * m_idealNumRects)) :
                    -1;

            state->dabsQueue = m_dabExecutor->takeReadyDabs(painter()->hasMirroring(), dabsLimit, &someDabsAreStillInQueue);
        }

        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!state->dabsQueue.isEmpty(),
                                             std::make_pair(m_currentUpdatePeriod, false));

        const int diameter = m_dabExecutor->averageDabSize();
        const qreal spacing = m_avgSpacing.rollingMean();

        const int idealNumRects = m_idealNumRects;

        QVector<QRect> rects;

        // wrap the dabs if needed
        if (painter()->device()->defaultBounds()->wrapAroundMode()) {
            /**
             * In WA mode we do two things:
             *
             * 1) We ensure that the parallel threads do not access the same are on
             *    the image. For normal updates that is ensured by the code in KisImage
             *    and the scheduler. Here we should do that manually by adjusting 'rects'
             *    so that they would not intersect in the wrapped space.
             *
             * 2) We duplicate dabs, to ensure that all the pieces of dabs are painted
             *    inside the wrapped rect. No pieces are dabs are painted twice, because
             *    we paint only the parts intersecting the wrap rect.
             */

            const QRect wrapRect = painter()->device()->defaultBounds()->imageBorderRect();

            QList<KisRenderedDab> wrappedDabs;

            Q_FOREACH (const KisRenderedDab &dab, state->dabsQueue) {
                const QVector<QPoint> normalizationOrigins =
                    KisWrappedRect::normalizationOriginsForRect(dab.realBounds(), wrapRect);

                Q_FOREACH(const QPoint &pt, normalizationOrigins) {
                    KisRenderedDab newDab = dab;

                    newDab.offset = pt;

                    rects.append(newDab.realBounds() & wrapRect);
                    wrappedDabs.append(newDab);
                }
            }

            state->dabsQueue = wrappedDabs;

        } else {
            // just get all rects
            Q_FOREACH (const KisRenderedDab &dab, state->dabsQueue) {
                rects.append(dab.realBounds());
            }
        }

        // split/merge rects into non-overlapping areas
        rects = KisPaintOpUtils::splitDabsIntoRects(rects,
                                                    idealNumRects, diameter, spacing);

        state->allDirtyRects = rects;

        Q_FOREACH (const KisRenderedDab &dab, state->dabsQueue) {
            state->dabPoints.append(dab.realBounds().center());
        }

        state->dabRenderingTimer.start();

        Q_FOREACH (const QRect &rc, rects) {
            jobs.append(
                new KisRunnableStrokeJobData(
                    [rc, state] () {
                        state->painter->bltFixed(rc, state->dabsQueue);
                    },
                    KisStrokeJobData::CONCURRENT));
        }

        /**
         * After the dab has been rendered once, we should mirror it either one
         * (h __or__ v) or three (h __and__ v) times. This sequence of 'if's achieves
         * the goal without any extra copying. Please note that it has __no__ 'else'
         * branches, which is done intentionally!
         */
        if (state->painter->hasHorizontalMirroring()) {
            addMirroringJobs(Qt::Horizontal, rects, state, jobs);
        }

        if (state->painter->hasVerticalMirroring()) {
            addMirroringJobs(Qt::Vertical, rects, state, jobs);
        }

        if (state->painter->hasHorizontalMirroring() && state->painter->hasVerticalMirroring()) {
            addMirroringJobs(Qt::Horizontal, rects, state, jobs);
        }

        jobs.append(
            new KisRunnableStrokeJobData(
                [state, this, someDabsAreStillInQueue] () {
                    Q_FOREACH(const QRect &rc, state->allDirtyRects) {
                        state->painter->addDirtyRect(rc);
                    }

                    state->painter->setAverageOpacity(state->dabsQueue.last().averageOpacity);

                    const int updateRenderingTime = state->dabRenderingTimer.elapsed();
                    const qreal dabRenderingTime = m_dabExecutor->averageDabRenderingTime();

                    m_avgNumDabs(state->dabsQueue.size());

                    const qreal currentUpdateTimePerDab = qreal(updateRenderingTime) / state->dabsQueue.size();
                    m_avgUpdateTimePerDab(currentUpdateTimePerDab);

                    /**
                     * NOTE: using currentUpdateTimePerDab in the calculation for the next update time instead
                     *       of the average one makes rendering speed about 40% faster. It happens because the
                     *       adaptation period is shorter than if it used
                     */
                    const qreal totalRenderingTimePerDab = dabRenderingTime + currentUpdateTimePerDab;

                    const int approxDabRenderingTime =
                        qreal(totalRenderingTimePerDab) * m_avgNumDabs.rollingMean() / m_idealNumRects;

                    m_currentUpdatePeriod =
                        someDabsAreStillInQueue ? m_minUpdatePeriod :
                        qBound(m_minUpdatePeriod, int(1.5 * approxDabRenderingTime), m_maxUpdatePeriod);


                    { // debug chunk
//                        ENTER_FUNCTION() << ppVar(state->allDirtyRects.size()) << ppVar(state->dabsQueue.size()) << ppVar(dabRenderingTime) << ppVar(updateRenderingTime);
//                        ENTER_FUNCTION() << ppVar(m_currentUpdatePeriod) << ppVar(someDabsAreStillInQueue);
                    }

                    // release all the dab devices
                    state->dabsQueue.clear();

                    m_updateSharedState.clear();
                },
                KisStrokeJobData::SEQUENTIAL));
    } else if (m_updateSharedState && hasPreparedDabsAtStart) {
        someDabsAreStillInQueue = true;
    }

    return std::make_pair(m_currentUpdatePeriod, someDabsAreStillInQueue);
}

KisSpacingInformation KisBrushOp::updateSpacingImpl(const KisPaintInformation &info) const
{
    const qreal scale = m_sizeOption.apply(info) * KisLodTransform::lodToScale(painter()->device());
    qreal rotation = m_rotationOption.apply(info);
    return effectiveSpacing(scale, rotation, &m_airbrushOption, &m_spacingOption, info);
}

KisTimingInformation KisBrushOp::updateTimingImpl(const KisPaintInformation &info) const
{
    return KisPaintOpPluginUtils::effectiveTiming(&m_airbrushOption, &m_rateOption, info);
}

void KisBrushOp::paintLine(const KisPaintInformation& pi1, const KisPaintInformation& pi2, KisDistanceInformation *currentDistance)
{
    if (m_sharpnessOption.isChecked() && m_brush && (m_brush->width() == 1) && (m_brush->height() == 1)) {

        if (!m_lineCacheDevice) {
            m_lineCacheDevice = source()->createCompositionSourceDevice();
        }
        else {
            m_lineCacheDevice->clear();
        }

        KisPainter p(m_lineCacheDevice);
        p.setPaintColor(painter()->paintColor());
        p.drawDDALine(pi1.pos(), pi2.pos());

        QRect rc = m_lineCacheDevice->extent();
        painter()->bitBlt(rc.x(), rc.y(), m_lineCacheDevice, rc.x(), rc.y(), rc.width(), rc.height());
    //fixes Bug 338011
    painter()->renderMirrorMask(rc, m_lineCacheDevice);
    }
    else {
        KisPaintOp::paintLine(pi1, pi2, currentDistance);
    }
}
