/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "freehand_stroke.h"

#include <QElapsedTimer>
#include <QThread>
#include <QApplication>

#include "kis_canvas_resource_provider.h"
#include <brushengine/kis_paintop_preset.h>
#include <brushengine/kis_paintop_settings.h>
#include "kis_painter.h"
#include "kis_paintop.h"

#include "kis_update_time_monitor.h"

#include <brushengine/kis_stroke_random_source.h>
#include <KisRunnableStrokeJobsInterface.h>
#include "FreehandStrokeRunnableJobDataWithUpdate.h"
#include <mutex>

#include "KisStrokeEfficiencyMeasurer.h"
#include <KisStrokeSpeedMonitor.h>
#include <strokes/KisFreehandStrokeInfo.h>
#include <strokes/KisMaskedFreehandStrokePainter.h>

#include "brushengine/kis_paintop_utils.h"
#include "KisAsyncronousStrokeUpdateHelper.h"

struct FreehandStrokeStrategy::Private
{
    Private(KisResourcesSnapshotSP _resources)
        : resources(_resources),
          needsAsynchronousUpdates(_resources->presetNeedsAsynchronousUpdates())
    {
        if (needsAsynchronousUpdates) {
            timeSinceLastUpdate.start();
        }
    }

    Private(const Private &rhs)
        : randomSource(rhs.randomSource),
          resources(rhs.resources),
          needsAsynchronousUpdates(rhs.needsAsynchronousUpdates)
    {
        if (needsAsynchronousUpdates) {
            timeSinceLastUpdate.start();
        }
    }

    KisStrokeRandomSource randomSource;
    KisResourcesSnapshotSP resources;

    KisStrokeEfficiencyMeasurer efficiencyMeasurer;

    QElapsedTimer timeSinceLastUpdate;
    int currentUpdatePeriod = 40;

    const bool needsAsynchronousUpdates = false;
    std::mutex updateEntryMutex;
};

FreehandStrokeStrategy::FreehandStrokeStrategy(KisResourcesSnapshotSP resources,
                                               KisFreehandStrokeInfo *strokeInfo,
                                               const KUndo2MagicString &name)
    : KisPainterBasedStrokeStrategy(QLatin1String("FREEHAND_STROKE"), name,
                                    resources, strokeInfo),
      m_d(new Private(resources))
{
    init();
}

FreehandStrokeStrategy::FreehandStrokeStrategy(KisResourcesSnapshotSP resources,
                                               QVector<KisFreehandStrokeInfo*> strokeInfos,
                                               const KUndo2MagicString &name)
    : KisPainterBasedStrokeStrategy(QLatin1String("FREEHAND_STROKE"), name,
                                    resources, strokeInfos),
      m_d(new Private(resources))
{
    init();
}

FreehandStrokeStrategy::FreehandStrokeStrategy(const FreehandStrokeStrategy &rhs, int levelOfDetail)
    : KisPainterBasedStrokeStrategy(rhs, levelOfDetail),
      m_d(new Private(*rhs.m_d))
{
    m_d->randomSource.setLevelOfDetail(levelOfDetail);
}

FreehandStrokeStrategy::~FreehandStrokeStrategy()
{
    KisStrokeSpeedMonitor::instance()->notifyStrokeFinished(m_d->efficiencyMeasurer.averageCursorSpeed(),
                                                            m_d->efficiencyMeasurer.averageRenderingSpeed(),
                                                            m_d->efficiencyMeasurer.averageFps(),
                                                            m_d->resources->currentPaintOpPreset());

    KisUpdateTimeMonitor::instance()->endStrokeMeasure();
}

void FreehandStrokeStrategy::init()
{
    setSupportsWrapAroundMode(true);
    setSupportsMaskingBrush(true);
    setSupportsIndirectPainting(true);
    enableJob(KisSimpleStrokeStrategy::JOB_DOSTROKE);

    if (m_d->needsAsynchronousUpdates) {
        /**
         * In case the paintop uses asynchronous updates, we should set priority to it,
         * because FPS is controlled separately, not by the queue's merging algorithm.
         */
        setBalancingRatioOverride(0.01); // set priority to updates
    }

    KisUpdateTimeMonitor::instance()->startStrokeMeasure();
    m_d->efficiencyMeasurer.setEnabled(KisStrokeSpeedMonitor::instance()->haveStrokeSpeedMeasurement());
}

void FreehandStrokeStrategy::initStrokeCallback()
{
    KisPainterBasedStrokeStrategy::initStrokeCallback();
    m_d->efficiencyMeasurer.notifyRenderingStarted();
}

void FreehandStrokeStrategy::finishStrokeCallback()
{
    m_d->efficiencyMeasurer.notifyRenderingFinished();
    KisPainterBasedStrokeStrategy::finishStrokeCallback();
}


void FreehandStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    if (KisAsyncronousStrokeUpdateHelper::UpdateData *d =
            dynamic_cast<KisAsyncronousStrokeUpdateHelper::UpdateData*>(data)) {

        // this job is lod-clonable in contrast to FreehandStrokeRunnableJobDataWithUpdate!
        tryDoUpdate(d->forceUpdate);

    } else if (Data *d = dynamic_cast<Data*>(data)) {
        KisMaskedFreehandStrokePainter *maskedPainter = this->maskedPainter(d->strokeInfoId);

        KisUpdateTimeMonitor::instance()->reportPaintOpPreset(maskedPainter->preset());
        KisRandomSourceSP rnd = m_d->randomSource.source();
        KisPerStrokeRandomSourceSP strokeRnd = m_d->randomSource.perStrokeSource();

        switch(d->type) {
        case Data::POINT:
            d->pi1.setRandomSource(rnd);
            d->pi1.setPerStrokeRandomSource(strokeRnd);
            maskedPainter->paintAt(d->pi1);
            m_d->efficiencyMeasurer.addSample(d->pi1.pos());
            break;
        case Data::LINE:
            d->pi1.setRandomSource(rnd);
            d->pi2.setRandomSource(rnd);
            d->pi1.setPerStrokeRandomSource(strokeRnd);
            d->pi2.setPerStrokeRandomSource(strokeRnd);
            maskedPainter->paintLine(d->pi1, d->pi2);
            m_d->efficiencyMeasurer.addSample(d->pi2.pos());
            break;
        case Data::CURVE:
            d->pi1.setRandomSource(rnd);
            d->pi2.setRandomSource(rnd);
            d->pi1.setPerStrokeRandomSource(strokeRnd);
            d->pi2.setPerStrokeRandomSource(strokeRnd);
            maskedPainter->paintBezierCurve(d->pi1,
                                         d->control1,
                                         d->control2,
                                         d->pi2);
            m_d->efficiencyMeasurer.addSample(d->pi2.pos());
            break;
        case Data::POLYLINE:
            maskedPainter->paintPolyline(d->points, 0, d->points.size());
            m_d->efficiencyMeasurer.addSamples(d->points);
            break;
        case Data::POLYGON:
            maskedPainter->paintPolygon(d->points);
            m_d->efficiencyMeasurer.addSamples(d->points);
            break;
        case Data::RECT:
            maskedPainter->paintRect(d->rect);
            m_d->efficiencyMeasurer.addSample(d->rect.topLeft());
            m_d->efficiencyMeasurer.addSample(d->rect.topRight());
            m_d->efficiencyMeasurer.addSample(d->rect.bottomRight());
            m_d->efficiencyMeasurer.addSample(d->rect.bottomLeft());
            break;
        case Data::ELLIPSE:
            maskedPainter->paintEllipse(d->rect);
            // TODO: add speed measures
            break;
        case Data::PAINTER_PATH:
            maskedPainter->paintPainterPath(d->path);
            // TODO: add speed measures
            break;
        case Data::QPAINTER_PATH:
            maskedPainter->drawPainterPath(d->path, d->pen);
            break;
        case Data::QPAINTER_PATH_FILL:
            maskedPainter->drawAndFillPainterPath(d->path, d->pen, d->customColor);
            break;
        };

        tryDoUpdate();
    } else {
        KisPainterBasedStrokeStrategy::doStrokeCallback(data);

        FreehandStrokeRunnableJobDataWithUpdate *dataWithUpdate =
            dynamic_cast<FreehandStrokeRunnableJobDataWithUpdate*>(data);

        if (dataWithUpdate) {
            tryDoUpdate();
        }
    }
}

void FreehandStrokeStrategy::tryDoUpdate(bool forceEnd)
{
    // we should enter this function only once!
    std::unique_lock<std::mutex> entryLock(m_d->updateEntryMutex, std::try_to_lock);
    if (!entryLock.owns_lock()) return;

    if (m_d->needsAsynchronousUpdates) {
        if (forceEnd || m_d->timeSinceLastUpdate.elapsed() > m_d->currentUpdatePeriod) {
            m_d->timeSinceLastUpdate.restart();

            for (int i = 0; i < numMaskedPainters(); i++) {
                KisMaskedFreehandStrokePainter *maskedPainter = this->maskedPainter(i);

                // TODO: well, we should count all N simultaneous painters for FPS rate!
                QVector<KisRunnableStrokeJobData*> jobs;

                bool needsMoreUpdates = false;

                std::tie(m_d->currentUpdatePeriod, needsMoreUpdates) =
                    maskedPainter->doAsyncronousUpdate(jobs);

                if (!jobs.isEmpty() ||
                    maskedPainter->hasDirtyRegion() ||
                    (forceEnd && needsMoreUpdates)) {

                    jobs.append(new KisRunnableStrokeJobData(
                                    [this] () {
                                        this->issueSetDirtySignals();
                                    },
                                    KisStrokeJobData::SEQUENTIAL));

                    if (forceEnd && needsMoreUpdates) {
                        jobs.append(new KisRunnableStrokeJobData(
                                        [this] () {
                                            this->tryDoUpdate(true);
                                        },
                                        KisStrokeJobData::SEQUENTIAL));
                    }


                    runnableJobsInterface()->addRunnableJobs(jobs);
                    m_d->efficiencyMeasurer.notifyFrameRenderingStarted();
                }

            }
        }
    } else {
        issueSetDirtySignals();
    }


}

void FreehandStrokeStrategy::issueSetDirtySignals()
{
    QVector<QRect> dirtyRects;

    for (int i = 0; i < numMaskedPainters(); i++) {
        KisMaskedFreehandStrokePainter *maskedPainter = this->maskedPainter(i);
        dirtyRects.append(maskedPainter->takeDirtyRegion());
    }

    if (needsMaskingUpdates()) {

        // optimize the rects so that they would never intersect with each other!
        // that is a mandatory step for the multithreaded execution of merging jobs

        // sanity check: updates from the brush should have already been normalized
        //               to the wrapping rect
        const KisDefaultBoundsBaseSP defaultBounds = targetNode()->projection()->defaultBounds();
        if (defaultBounds->wrapAroundMode()) {
            const QRect wrapRect = defaultBounds->imageBorderRect();
            for (auto it = dirtyRects.begin(); it != dirtyRects.end(); ++it) {
                KIS_SAFE_ASSERT_RECOVER(wrapRect.contains(*it)) {
                    ENTER_FUNCTION() << ppVar(*it) << ppVar(wrapRect);
                    *it = *it & wrapRect;
                }
            }
        }

        const int maxPatchSizeForMaskingUpdates = 64;
        const QRect totalRect =
            std::accumulate(dirtyRects.constBegin(), dirtyRects.constEnd(), QRect(), std::bit_or<QRect>());

        dirtyRects = KisPaintOpUtils::splitAndFilterDabRect(totalRect, dirtyRects, maxPatchSizeForMaskingUpdates);

        QVector<KisRunnableStrokeJobData*> jobs = doMaskingBrushUpdates(dirtyRects);

        jobs.append(new KisRunnableStrokeJobData(
            [this, dirtyRects] () {
                this->targetNode()->setDirty(dirtyRects);
            },
            KisStrokeJobData::SEQUENTIAL));

        runnableJobsInterface()->addRunnableJobs(jobs);

    } else {
        targetNode()->setDirty(dirtyRects);
    }

    //KisUpdateTimeMonitor::instance()->reportJobFinished(data, dirtyRects);
}

KisStrokeStrategy* FreehandStrokeStrategy::createLodClone(int levelOfDetail)
{
    if (!m_d->resources->presetAllowsLod()) return 0;
    if (!m_d->resources->currentNode()->supportsLodPainting()) return 0;

    FreehandStrokeStrategy *clone = new FreehandStrokeStrategy(*this, levelOfDetail);
    return clone;
}

void FreehandStrokeStrategy::notifyUserStartedStroke()
{
    m_d->efficiencyMeasurer.notifyCursorMoveStarted();
}

void FreehandStrokeStrategy::notifyUserEndedStroke()
{
    m_d->efficiencyMeasurer.notifyCursorMoveFinished();
}
