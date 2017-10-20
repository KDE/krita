/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisDabRenderingQueue.h"

#include "KisDabRenderingJob.h"
#include "KisRenderedDab.h"
#include "kis_painter.h"

#include <QSet>
#include <QMutex>
#include <QMutexLocker>
#include <KisRollingMeanAccumulatorWrapper.h>

#include "kis_algebra_2d.h"

struct KisDabRenderingQueue::Private
{
    struct DumbCacheInterface : public CacheInterface {
        void getDabType(bool hasDabInCache,
                                KisDabCacheUtils::DabRenderingResources *resources,
                                const KisDabCacheUtils::DabRequestInfo &request,
                                /* out */
                                KisDabCacheUtils::DabGenerationInfo *di,
                                bool *shouldUseCache) override
        {
            Q_UNUSED(hasDabInCache);
            Q_UNUSED(resources);
            Q_UNUSED(request);

            di->needsPostprocessing = false;
            *shouldUseCache = false;
        }

        bool hasSeparateOriginal(KisDabCacheUtils::DabRenderingResources *resources) const override
        {
            Q_UNUSED(resources);
            return false;
        }

    };

    Private(const KoColorSpace *_colorSpace,
            KisDabCacheUtils::ResourcesFactory _resourcesFactory)
        : cacheInterface(new DumbCacheInterface),
          colorSpace(_colorSpace),
          resourcesFactory(_resourcesFactory),
          avgExecutionTime(50),
          avgDabSize(50)
    {
        KIS_SAFE_ASSERT_RECOVER_NOOP(resourcesFactory);
    }

    ~Private() {
        qDeleteAll(cachedResources);
        cachedResources.clear();
    }

    QList<KisDabRenderingJobSP> jobs;
    int nextSeqNoToUse = 0;
    int lastPaintedJob = -1;
    int lastDabJobInQueue = -1;
    QScopedPointer<CacheInterface> cacheInterface;
    const KoColorSpace *colorSpace;
    qreal averageOpacity = 0.0;

    KisDabCacheUtils::ResourcesFactory resourcesFactory;

    QList<KisDabCacheUtils::DabRenderingResources*> cachedResources;
    QSet<KisFixedPaintDeviceSP> cachedPaintDevices;

    QMutex mutex;

    KisRollingMeanAccumulatorWrapper avgExecutionTime;
    KisRollingMeanAccumulatorWrapper avgDabSize;

    int calculateLastDabJobIndex(int startSearchIndex);
    void cleanPaintedDabs();
    bool dabsHaveSeparateOriginal();

    KisDabCacheUtils::DabRenderingResources* fetchResourcesFromCache();
    void putResourcesToCache(KisDabCacheUtils::DabRenderingResources *resources);
};


KisDabRenderingQueue::KisDabRenderingQueue(const KoColorSpace *cs,
                                           KisDabCacheUtils::ResourcesFactory resourcesFactory)
    : m_d(new Private(cs, resourcesFactory))
{
}

KisDabRenderingQueue::~KisDabRenderingQueue()
{
}

int KisDabRenderingQueue::Private::calculateLastDabJobIndex(int startSearchIndex)
{
    if (startSearchIndex < 0) {
        startSearchIndex = jobs.size() - 1;
    }

    // try to use cached value
    if (startSearchIndex >= lastDabJobInQueue) {
        return lastDabJobInQueue;
    }

    // if we are below the cached value, just iterate through the dabs
    // (which is extremely(!) slow)
    for (int i = startSearchIndex; i >= 0; i--) {
        if (jobs[i]->type == KisDabRenderingJob::Dab) {
            return i;
        }
    }

    return -1;
}

KisDabRenderingJobSP KisDabRenderingQueue::addDab(const KisDabCacheUtils::DabRequestInfo &request,
                                                 qreal opacity, qreal flow)
{
    QMutexLocker l(&m_d->mutex);

    const int seqNo = m_d->nextSeqNoToUse++;

    KisDabCacheUtils::DabRenderingResources *resources = m_d->fetchResourcesFromCache();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(resources, KisDabRenderingJobSP());

    // We should sync the cached brush into the current seqNo
    resources->syncResourcesToSeqNo(seqNo, request.info);

    const int lastDabJobIndex = m_d->lastDabJobInQueue;

    KisDabRenderingJobSP job(new KisDabRenderingJob());

    bool shouldUseCache = false;
    m_d->cacheInterface->getDabType(lastDabJobIndex >= 0,
                                    resources,
                                    request,
                                    &job->generationInfo,
                                    &shouldUseCache);

    m_d->putResourcesToCache(resources);
    resources = 0;

    // TODO: initialize via c-tor
    job->seqNo = seqNo;
    job->type =
        !shouldUseCache ? KisDabRenderingJob::Dab :
        job->generationInfo.needsPostprocessing ? KisDabRenderingJob::Postprocess :
        KisDabRenderingJob::Copy;
    job->opacity = opacity;
    job->flow = flow;


    if (job->type == KisDabRenderingJob::Dab) {
        job->status = KisDabRenderingJob::Running;
    } else if (job->type == KisDabRenderingJob::Postprocess ||
               job->type == KisDabRenderingJob::Copy) {

        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(lastDabJobIndex >= 0, KisDabRenderingJobSP());

        if (m_d->jobs[lastDabJobIndex]->status == KisDabRenderingJob::Completed) {
            if (job->type == KisDabRenderingJob::Postprocess) {
                job->status = KisDabRenderingJob::Running;
                job->originalDevice = m_d->jobs[lastDabJobIndex]->originalDevice;
            } else if (job->type == KisDabRenderingJob::Copy) {
                job->status = KisDabRenderingJob::Completed;
                job->originalDevice = m_d->jobs[lastDabJobIndex]->originalDevice;
                job->postprocessedDevice = m_d->jobs[lastDabJobIndex]->postprocessedDevice;
            }
        }
    }

    m_d->jobs.append(job);

    KisDabRenderingJobSP jobToRun;
    if (job->status == KisDabRenderingJob::Running) {
        jobToRun = job;
    }

    if (job->type == KisDabRenderingJob::Dab) {
        m_d->lastDabJobInQueue = m_d->jobs.size() - 1;
        m_d->cleanPaintedDabs();
    }

    // collect some statistics about the dab
    m_d->avgDabSize(KisAlgebra2D::maxDimension(job->generationInfo.dstDabRect));

    return jobToRun;
}

QList<KisDabRenderingJobSP> KisDabRenderingQueue::notifyJobFinished(int seqNo, int usecsTime)
{
    QMutexLocker l(&m_d->mutex);

    QList<KisDabRenderingJobSP> dependentJobs;

    /**
     * Here we use binary search for locating the necessary original dab
     */
    auto finishedJobIt =
        std::lower_bound(m_d->jobs.begin(), m_d->jobs.end(), seqNo,
                         [] (KisDabRenderingJobSP job, int seqNo) {
                             return job->seqNo < seqNo;
                         });

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(finishedJobIt != m_d->jobs.end(), dependentJobs);
    KisDabRenderingJobSP finishedJob = *finishedJobIt;

    KIS_SAFE_ASSERT_RECOVER_NOOP(finishedJob->status == KisDabRenderingJob::Running);
    KIS_SAFE_ASSERT_RECOVER_NOOP(finishedJob->seqNo == seqNo);
    KIS_SAFE_ASSERT_RECOVER_NOOP(finishedJob->originalDevice);
    KIS_SAFE_ASSERT_RECOVER_NOOP(finishedJob->postprocessedDevice);

    finishedJob->status = KisDabRenderingJob::Completed;

    if (finishedJob->type == KisDabRenderingJob::Dab) {
        for (auto it = finishedJobIt + 1; it != m_d->jobs.end(); ++it) {
            KisDabRenderingJobSP j = *it;

            // next dab job closes the chain
            if (j->type == KisDabRenderingJob::Dab) break;

            // the non 'dab'-type job couldn't have
            // been started before the source ob was completed
            KIS_SAFE_ASSERT_RECOVER_BREAK(j->status == KisDabRenderingJob::New);

            if (j->type == KisDabRenderingJob::Copy) {

                j->originalDevice = finishedJob->originalDevice;
                j->postprocessedDevice = finishedJob->postprocessedDevice;
                j->status = KisDabRenderingJob::Completed;

            } else if (j->type == KisDabRenderingJob::Postprocess) {

                j->originalDevice = finishedJob->originalDevice;
                j->status = KisDabRenderingJob::Running;
                dependentJobs << j;
            }
        }
    }

    if (usecsTime >= 0) {
        m_d->avgExecutionTime(usecsTime);
    }

    return dependentJobs;
}

void KisDabRenderingQueue::Private::cleanPaintedDabs()
{
    const int nextToBePainted = lastPaintedJob + 1;
    const int lastSourceJob = calculateLastDabJobIndex(qMin(nextToBePainted, jobs.size() - 1));

    if (lastPaintedJob >= 0) {
        int numRemovedJobs = 0;
        int numRemovedJobsBeforeLastSource = 0;

        auto it = jobs.begin();
        for (int i = 0; i <= lastPaintedJob; i++) {
            KisDabRenderingJobSP job = *it;

            if (i < lastSourceJob || job->type != KisDabRenderingJob::Dab){

                // cache unique 'original' devices
                if (job->type == KisDabRenderingJob::Dab &&
                    job->postprocessedDevice != job->originalDevice) {
                    cachedPaintDevices << job->originalDevice;
                    job->originalDevice = 0;
                }

                it = jobs.erase(it);
                numRemovedJobs++;
                if (i < lastSourceJob) {
                    numRemovedJobsBeforeLastSource++;
                }

            } else  {
                ++it;
            }
        }

        KIS_SAFE_ASSERT_RECOVER_RETURN(jobs.size() > 0);

        lastPaintedJob -= numRemovedJobs;
        lastDabJobInQueue -= numRemovedJobsBeforeLastSource;
    }
}

QList<KisRenderedDab> KisDabRenderingQueue::takeReadyDabs(bool returnMutableDabs)
{
    QMutexLocker l(&m_d->mutex);

    QList<KisRenderedDab> renderedDabs;
    if (m_d->jobs.isEmpty()) return renderedDabs;

    KIS_SAFE_ASSERT_RECOVER_NOOP(
        m_d->jobs.isEmpty() ||
        m_d->jobs.first()->type == KisDabRenderingJob::Dab);

    const int copyJobAfterInclusive =
        returnMutableDabs && !m_d->dabsHaveSeparateOriginal() ?
            m_d->lastDabJobInQueue :
            std::numeric_limits<int>::max();

    for (int i = 0; i < m_d->jobs.size(); i++) {
        KisDabRenderingJobSP j = m_d->jobs[i];

        if (j->status != KisDabRenderingJob::Completed) break;

        if (i <= m_d->lastPaintedJob) continue;

        KisRenderedDab dab;
        KisFixedPaintDeviceSP resultDevice = j->postprocessedDevice;

        if (i >= copyJobAfterInclusive) {
            resultDevice = new KisFixedPaintDevice(*resultDevice);
        }

        dab.device = resultDevice;
        dab.offset = j->dstDabOffset();
        dab.opacity = j->opacity;
        dab.flow = j->flow;

        m_d->averageOpacity = KisPainter::blendAverageOpacity(j->opacity, m_d->averageOpacity);
        dab.averageOpacity = m_d->averageOpacity;


        renderedDabs.append(dab);

        m_d->lastPaintedJob = i;
    }

    m_d->cleanPaintedDabs();
    return renderedDabs;
}

bool KisDabRenderingQueue::hasPreparedDabs() const
{
    QMutexLocker l(&m_d->mutex);

    const int nextToBePainted = m_d->lastPaintedJob + 1;

    return
        nextToBePainted >= 0 &&
        nextToBePainted < m_d->jobs.size() &&
            m_d->jobs[nextToBePainted]->status == KisDabRenderingJob::Completed;
}

void KisDabRenderingQueue::setCacheInterface(KisDabRenderingQueue::CacheInterface *interface)
{
    m_d->cacheInterface.reset(interface);
}

KisFixedPaintDeviceSP KisDabRenderingQueue::fetchCachedPaintDevce()
{
    QMutexLocker l(&m_d->mutex);

    KisFixedPaintDeviceSP result;

    if (m_d->cachedPaintDevices.isEmpty()) {
        result = new KisFixedPaintDevice(m_d->colorSpace);
    } else {
        // there is no difference from which side to take elements from QSet
        auto it = m_d->cachedPaintDevices.begin();
        result = *it;
        m_d->cachedPaintDevices.erase(it);
    }

    return result;
}

void KisDabRenderingQueue::recyclePaintDevicesForCache(const QVector<KisFixedPaintDeviceSP> devices)
{
    QMutexLocker l(&m_d->mutex);

    Q_FOREACH (KisFixedPaintDeviceSP device, devices) {
        // the set automatically checks if the device is unique in the set
        m_d->cachedPaintDevices << device;
    }
}

int KisDabRenderingQueue::averageExecutionTime() const
{
    QMutexLocker l(&m_d->mutex);
    return qRound(m_d->avgExecutionTime.rollingMean());
}

int KisDabRenderingQueue::averageDabSize() const
{
    QMutexLocker l(&m_d->mutex);
    return qRound(m_d->avgDabSize.rollingMean());
}

bool KisDabRenderingQueue::Private::dabsHaveSeparateOriginal()
{
    KisDabCacheUtils::DabRenderingResources *resources = fetchResourcesFromCache();

    const bool result = cacheInterface->hasSeparateOriginal(resources);

    putResourcesToCache(resources);

    return result;
}

KisDabCacheUtils::DabRenderingResources *KisDabRenderingQueue::Private::fetchResourcesFromCache()
{
    KisDabCacheUtils::DabRenderingResources *resources = 0;

    // fetch/create a temporary resources object
    if (!cachedResources.isEmpty()) {
        resources = cachedResources.takeLast();
    } else {
        resources = resourcesFactory();
    }

    return resources;
}

void KisDabRenderingQueue::Private::putResourcesToCache(KisDabCacheUtils::DabRenderingResources *resources)
{
    cachedResources << resources;
}

KisDabCacheUtils::DabRenderingResources *KisDabRenderingQueue::fetchResourcesFromCache()
{
    // TODO: make a separate lock for that
    QMutexLocker l(&m_d->mutex);
    return m_d->fetchResourcesFromCache();
}

void KisDabRenderingQueue::putResourcesToCache(KisDabCacheUtils::DabRenderingResources *resources)
{
    QMutexLocker l(&m_d->mutex);
    m_d->putResourcesToCache(resources);
}

int KisDabRenderingQueue::testingGetQueueSize() const
{
    QMutexLocker l(&m_d->mutex);

    return m_d->jobs.size();
}

