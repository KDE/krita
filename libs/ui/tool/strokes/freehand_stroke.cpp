/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#include "freehand_stroke.h"

#include <QElapsedTimer>

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

struct FreehandStrokeStrategy::Private
{
    Private(KisResourcesSnapshotSP _resources)
        : resources(_resources),
          needsAsynchronousUpdates(_resources->presetNeedsAsynchronousUpdates())
    {
    }

    Private(const Private &rhs)
        : randomSource(rhs.randomSource),
          resources(rhs.resources),
          needsAsynchronousUpdates(rhs.needsAsynchronousUpdates)
    {
    }

    KisStrokeRandomSource randomSource;
    KisResourcesSnapshotSP resources;

    KisStrokeEfficiencyMeasurer efficiencyMeasurer;

    QElapsedTimer timeSinceLastUpdate;
    int currentUpdatePeriod = 40;

    const bool needsAsynchronousUpdates = false;
    std::mutex updateEntryMutex;
};

FreehandStrokeStrategy::FreehandStrokeStrategy(bool needsIndirectPainting,
                                               const QString &indirectPaintingCompositeOp,
                                               KisResourcesSnapshotSP resources,
                                               PainterInfo *painterInfo,
                                               const KUndo2MagicString &name)
    : KisPainterBasedStrokeStrategy("FREEHAND_STROKE", name,
                                    resources, painterInfo),
      m_d(new Private(resources))
{
    init(needsIndirectPainting, indirectPaintingCompositeOp);
}

FreehandStrokeStrategy::FreehandStrokeStrategy(bool needsIndirectPainting,
                                               const QString &indirectPaintingCompositeOp,
                                               KisResourcesSnapshotSP resources,
                                               QVector<PainterInfo*> painterInfos,
                                               const KUndo2MagicString &name)
    : KisPainterBasedStrokeStrategy("FREEHAND_STROKE", name,
                                    resources, painterInfos),
      m_d(new Private(resources))
{
    init(needsIndirectPainting, indirectPaintingCompositeOp);
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

void FreehandStrokeStrategy::init(bool needsIndirectPainting,
                                  const QString &indirectPaintingCompositeOp)
{
    setNeedsIndirectPainting(needsIndirectPainting);
    setIndirectPaintingCompositeOp(indirectPaintingCompositeOp);
    setSupportsWrapAroundMode(true);
    enableJob(KisSimpleStrokeStrategy::JOB_DOSTROKE);

    if (m_d->needsAsynchronousUpdates) {
        /**
         * In case the paintop uses asynchronous updates, we should set priority to it,
         * because FPS is controlled separately, not by the queue's merging algorithm.
         */
        setBalancingRatioOverride(0.01); // set priority to updates
    }

    if (m_d->needsAsynchronousUpdates) {
        m_d->timeSinceLastUpdate.start();
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
    PainterInfo *info = 0;

    if (UpdateData *d = dynamic_cast<UpdateData*>(data)) {
        // this job is lod-clonable in contrast to FreehandStrokeRunnableJobDataWithUpdate!
        tryDoUpdate(d->forceUpdate);

    } else if (Data *d = dynamic_cast<Data*>(data)) {
        info = painterInfos()[d->painterInfoId];

        KisUpdateTimeMonitor::instance()->reportPaintOpPreset(info->painter->preset());
        KisRandomSourceSP rnd = m_d->randomSource.source();

        switch(d->type) {
        case Data::POINT:
            d->pi1.setRandomSource(rnd);
            info->painter->paintAt(d->pi1, info->dragDistance);
            m_d->efficiencyMeasurer.addSample(d->pi1.pos());
            break;
        case Data::LINE:
            d->pi1.setRandomSource(rnd);
            d->pi2.setRandomSource(rnd);
            info->painter->paintLine(d->pi1, d->pi2, info->dragDistance);
            m_d->efficiencyMeasurer.addSample(d->pi2.pos());
            break;
        case Data::CURVE:
            d->pi1.setRandomSource(rnd);
            d->pi2.setRandomSource(rnd);
            info->painter->paintBezierCurve(d->pi1,
                                            d->control1,
                                            d->control2,
                                            d->pi2,
                                            info->dragDistance);
            m_d->efficiencyMeasurer.addSample(d->pi2.pos());
            break;
        case Data::POLYLINE:
            info->painter->paintPolyline(d->points, 0, d->points.size());
            m_d->efficiencyMeasurer.addSamples(d->points);
            break;
        case Data::POLYGON:
            info->painter->paintPolygon(d->points);
            m_d->efficiencyMeasurer.addSamples(d->points);
            break;
        case Data::RECT:
            info->painter->paintRect(d->rect);
            m_d->efficiencyMeasurer.addSample(d->rect.topLeft());
            m_d->efficiencyMeasurer.addSample(d->rect.topRight());
            m_d->efficiencyMeasurer.addSample(d->rect.bottomRight());
            m_d->efficiencyMeasurer.addSample(d->rect.bottomLeft());
            break;
        case Data::ELLIPSE:
            info->painter->paintEllipse(d->rect);
            // TODO: add speed measures
            break;
        case Data::PAINTER_PATH:
            info->painter->paintPainterPath(d->path);
            // TODO: add speed measures
            break;
        case Data::QPAINTER_PATH:
            info->painter->drawPainterPath(d->path, d->pen);
            break;
        case Data::QPAINTER_PATH_FILL: {
            info->painter->setBackgroundColor(d->customColor);
            info->painter->fillPainterPath(d->path);}
            info->painter->drawPainterPath(d->path, d->pen);
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

            Q_FOREACH (PainterInfo *info, painterInfos()) {
                KisPaintOp *paintop = info->painter->paintOp();
                KIS_SAFE_ASSERT_RECOVER_RETURN(paintop);

                // TODO: well, we should count all N simultaneous painters for FPS rate!
                QVector<KisRunnableStrokeJobData*> jobs;
                m_d->currentUpdatePeriod = paintop->doAsyncronousUpdate(jobs);

                if (!jobs.isEmpty()) {
                    jobs.append(new KisRunnableStrokeJobData(
                                    [this] () {
                                        this->issueSetDirtySignals();
                                    },
                                    KisStrokeJobData::SEQUENTIAL));

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

    Q_FOREACH (PainterInfo *info, painterInfos()) {
        dirtyRects.append(info->painter->takeDirtyRegion());
    }

    //KisUpdateTimeMonitor::instance()->reportJobFinished(data, dirtyRects);
    targetNode()->setDirty(dirtyRects);
}

KisStrokeStrategy* FreehandStrokeStrategy::createLodClone(int levelOfDetail)
{
    if (!m_d->resources->presetAllowsLod()) return 0;

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
