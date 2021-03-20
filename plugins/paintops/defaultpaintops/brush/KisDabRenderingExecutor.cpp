/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisDabRenderingExecutor.h"

#include "KisDabRenderingQueue.h"
#include "KisDabRenderingQueueCache.h"
#include "KisDabRenderingJob.h"
#include "KisRenderedDab.h"
#include "KisRunnableStrokeJobsInterface.h"
#include "KisRunnableStrokeJobData.h"
#include <tool/strokes/FreehandStrokeRunnableJobDataWithUpdate.h>

struct KisDabRenderingExecutor::Private
{
    QScopedPointer<KisDabRenderingQueue> renderingQueue;
    KisRunnableStrokeJobsInterface *runnableJobsInterface;
};

KisDabRenderingExecutor::KisDabRenderingExecutor(const KoColorSpace *cs,
                                                 KisDabCacheUtils::ResourcesFactory resourcesFactory,
                                                 KisRunnableStrokeJobsInterface *runnableJobsInterface,
                                                 KisPressureMirrorOption *mirrorOption,
                                                 KisPrecisionOption *precisionOption)
    : m_d(new Private)
{
    m_d->runnableJobsInterface = runnableJobsInterface;

    m_d->renderingQueue.reset(
        new KisDabRenderingQueue(cs, resourcesFactory));

    KisDabRenderingQueueCache *cache = new KisDabRenderingQueueCache();
    cache->setMirrorPostprocessing(mirrorOption);
    cache->setPrecisionOption(precisionOption);

    m_d->renderingQueue->setCacheInterface(cache);
}

KisDabRenderingExecutor::~KisDabRenderingExecutor()
{
}

void KisDabRenderingExecutor::addDab(const KisDabCacheUtils::DabRequestInfo &request,
                                     qreal opacity, qreal flow)
{
    KisDabRenderingJobSP job = m_d->renderingQueue->addDab(request, opacity, flow);
    if (job) {
        m_d->runnableJobsInterface->addRunnableJob(
            new FreehandStrokeRunnableJobDataWithUpdate(
                        new KisDabRenderingJobRunner(job, m_d->renderingQueue.data(), m_d->runnableJobsInterface),
                        KisStrokeJobData::CONCURRENT));
    }
}

QList<KisRenderedDab> KisDabRenderingExecutor::takeReadyDabs(bool returnMutableDabs,
                                                             int oneTimeLimit,
                                                             bool *someDabsLeft)
{
    return m_d->renderingQueue->takeReadyDabs(returnMutableDabs, oneTimeLimit, someDabsLeft);
}

bool KisDabRenderingExecutor::hasPreparedDabs() const
{
    return m_d->renderingQueue->hasPreparedDabs();
}

qreal KisDabRenderingExecutor::averageDabRenderingTime() const
{
    return m_d->renderingQueue->averageExecutionTime();
}

int KisDabRenderingExecutor::averageDabSize() const
{
    return m_d->renderingQueue->averageDabSize();
}
