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
    struct JobWrapper {
        enum Status {
            New,
            Running,
            Completed
        };

        KisDabRenderingJob job;
        Status status = New;

        qreal opacity = OPACITY_OPAQUE_F;
        qreal flow = OPACITY_OPAQUE_F;
    };

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
            KisDabCacheUtils::ResourcesFactory _resourcesFactory,
            KisRunnableStrokeJobsInterface *_runnableJobsInterface)
        : cacheInterface(new DumbCacheInterface),
          colorSpace(_colorSpace),
          runnableJobsInterface(_runnableJobsInterface),
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

    QList<JobWrapper> jobs;
    int startSeqNo = 0;
    int lastPaintedJob = -1;
    QScopedPointer<CacheInterface> cacheInterface;
    const KoColorSpace *colorSpace;
    KisRunnableStrokeJobsInterface *runnableJobsInterface;
    qreal averageOpacity = 0.0;

    KisDabCacheUtils::ResourcesFactory resourcesFactory;

    QList<KisDabCacheUtils::DabRenderingResources*> cachedResources;
    QSet<KisFixedPaintDeviceSP> cachedPaintDevices;

    QMutex mutex;

    KisRollingMeanAccumulatorWrapper avgExecutionTime;
    KisRollingMeanAccumulatorWrapper avgDabSize;

    int findLastDabJobIndex(int startSearchIndex = -1);
    KisDabRenderingJob* createPostprocessingJob(const KisDabRenderingJob &postprocessingJob, int sourceDabJob);
    void cleanPaintedDabs();
    bool dabsHaveSeparateOriginal();

    KisDabCacheUtils::DabRenderingResources* fetchResourcesFromCache();
    void putResourcesToCache(KisDabCacheUtils::DabRenderingResources *resources);
};


KisDabRenderingQueue::KisDabRenderingQueue(const KoColorSpace *cs,
                                           KisDabCacheUtils::ResourcesFactory resourcesFactory,
                                           KisRunnableStrokeJobsInterface *runnableJobsInterface)
    : m_d(new Private(cs, resourcesFactory, runnableJobsInterface))
{
}

KisDabRenderingQueue::~KisDabRenderingQueue()
{
}

int KisDabRenderingQueue::Private::findLastDabJobIndex(int startSearchIndex)
{
    if (startSearchIndex < 0) {
        startSearchIndex = jobs.size() - 1;
    }

    for (int i = startSearchIndex; i >= 0; i--) {
        if (jobs[i].job.type == KisDabRenderingJob::Dab) {
            return i;
        }
    }

    return -1;
}

KisDabRenderingJob *KisDabRenderingQueue::Private::createPostprocessingJob(const KisDabRenderingJob &postprocessingJob, int sourceDabJob)
{
    KisDabRenderingJob *job = new KisDabRenderingJob(postprocessingJob);
    job->originalDevice = jobs[sourceDabJob].job.originalDevice;
    return job;
}

KisDabRenderingJob *KisDabRenderingQueue::addDab(const KisDabCacheUtils::DabRequestInfo &request,
                                                 qreal opacity, qreal flow)
{
    QMutexLocker l(&m_d->mutex);

    const int seqNo =
        !m_d->jobs.isEmpty() ?
            m_d->jobs.last().job.seqNo + 1:
            m_d->startSeqNo;

    KisDabCacheUtils::DabRenderingResources *resources = m_d->fetchResourcesFromCache();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(resources, 0);

    // We should sync the cached brush into the current seqNo
    resources->syncResourcesToSeqNo(seqNo, request.info);

    const int lastDabJobIndex = m_d->findLastDabJobIndex();

    Private::JobWrapper wrapper;

    bool shouldUseCache = false;
    m_d->cacheInterface->getDabType(lastDabJobIndex >= 0,
                                    resources,
                                    request,
                                    &wrapper.job.generationInfo,
                                    &shouldUseCache);

    m_d->putResourcesToCache(resources);
    resources = 0;

    // TODO: initialize via c-tor
    wrapper.job.seqNo = seqNo;
    wrapper.job.type =
        !shouldUseCache ? KisDabRenderingJob::Dab :
        wrapper.job.generationInfo.needsPostprocessing ? KisDabRenderingJob::Postprocess :
        KisDabRenderingJob::Copy;
    wrapper.job.parentQueue = this;
    wrapper.job.runnableJobsInterface = m_d->runnableJobsInterface;
    wrapper.opacity = opacity;
    wrapper.flow = flow;


    KisDabRenderingJob *jobToRun = 0;

    if (wrapper.job.type == KisDabRenderingJob::Dab) {
        jobToRun = new KisDabRenderingJob(wrapper.job);
        wrapper.status = Private::JobWrapper::Running;
    } else if (wrapper.job.type == KisDabRenderingJob::Postprocess ||
               wrapper.job.type == KisDabRenderingJob::Copy) {

        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(lastDabJobIndex >= 0, 0);

        if (m_d->jobs[lastDabJobIndex].status == Private::JobWrapper::Completed) {
            if (wrapper.job.type == KisDabRenderingJob::Postprocess) {
                jobToRun = m_d->createPostprocessingJob(wrapper.job, lastDabJobIndex);
                wrapper.status = Private::JobWrapper::Running;
                wrapper.job.originalDevice = m_d->jobs[lastDabJobIndex].job.originalDevice;
            } else if (wrapper.job.type == KisDabRenderingJob::Copy) {
                wrapper.status = Private::JobWrapper::Completed;
                wrapper.job.originalDevice = m_d->jobs[lastDabJobIndex].job.originalDevice;
                wrapper.job.postprocessedDevice = m_d->jobs[lastDabJobIndex].job.postprocessedDevice;
            }
        }
    }

    m_d->jobs.append(wrapper);

    KIS_SAFE_ASSERT_RECOVER_NOOP(m_d->startSeqNo ==
                                 m_d->jobs.first().job.seqNo);

    if (wrapper.job.type == KisDabRenderingJob::Dab) {
        m_d->cleanPaintedDabs();
    }

    // collect some statistics about the dab
    m_d->avgDabSize(KisAlgebra2D::maxDimension(wrapper.job.generationInfo.dstDabRect));

    return jobToRun;
}

QList<KisDabRenderingJob *> KisDabRenderingQueue::notifyJobFinished(KisDabRenderingJob *job, int usecsTime)
{
    QMutexLocker l(&m_d->mutex);

    QList<KisDabRenderingJob *> dependentJobs;

    const int jobIndex = job->seqNo - m_d->startSeqNo;
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(
        jobIndex >= 0 && jobIndex < m_d->jobs.size(), dependentJobs);


    Private::JobWrapper &wrapper = m_d->jobs[jobIndex];
    KisDabRenderingJob &finishedJob = wrapper.job;

    KIS_SAFE_ASSERT_RECOVER_NOOP(m_d->jobs[jobIndex].status == Private::JobWrapper::Running);
    KIS_SAFE_ASSERT_RECOVER_NOOP(finishedJob.seqNo == job->seqNo);
    KIS_SAFE_ASSERT_RECOVER_NOOP(finishedJob.type == job->type);
    KIS_SAFE_ASSERT_RECOVER_NOOP(job->originalDevice);
    KIS_SAFE_ASSERT_RECOVER_NOOP(job->postprocessedDevice);

    wrapper.status = Private::JobWrapper::Completed;
    finishedJob.originalDevice = job->originalDevice;
    finishedJob.postprocessedDevice = job->postprocessedDevice;

    if (finishedJob.type == KisDabRenderingJob::Dab) {
        for (int i = jobIndex + 1; i < m_d->jobs.size(); i++) {
            Private::JobWrapper &w = m_d->jobs[i];
            KisDabRenderingJob &j = w.job;

            // next dab job closes the chain
            if (j.type == KisDabRenderingJob::Dab) break;

            // the non 'dab'-type job couldn't have
            // been started before the source ob was completed
            KIS_SAFE_ASSERT_RECOVER_BREAK(w.status == Private::JobWrapper::New);

            if (j.type == KisDabRenderingJob::Copy) {
                j.originalDevice = job->originalDevice;
                j.postprocessedDevice = job->postprocessedDevice;
                w.status = Private::JobWrapper::Completed;
            } else if (j.type == KisDabRenderingJob::Postprocess) {
                dependentJobs << m_d->createPostprocessingJob(j, jobIndex);
                w.status = Private::JobWrapper::Running;
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
    const int sourceJob = findLastDabJobIndex(qMin(nextToBePainted, jobs.size() - 1));

    if (sourceJob >= 1) {
        // recycle and remove first 'sourceJob' jobs

        // cache unique 'original' devices
        for (auto it = jobs.begin(); it != jobs.begin() + sourceJob; ++it) {
            if (it->job.type == KisDabRenderingJob::Dab &&
                it->job.postprocessedDevice != it->job.originalDevice) {
                cachedPaintDevices << it->job.originalDevice;
                it->job.originalDevice = 0;
            }
        }

        jobs.erase(jobs.begin(), jobs.begin() + sourceJob);

        KIS_SAFE_ASSERT_RECOVER_RETURN(jobs.size() > 0);

        lastPaintedJob -= sourceJob;
        startSeqNo = jobs.first().job.seqNo;
    }
}

QList<KisRenderedDab> KisDabRenderingQueue::takeReadyDabs(bool returnMutableDabs)
{
    QMutexLocker l(&m_d->mutex);

    QList<KisRenderedDab> renderedDabs;
    if (m_d->startSeqNo < 0) return renderedDabs;

    KIS_SAFE_ASSERT_RECOVER_NOOP(
        m_d->jobs.isEmpty() ||
        m_d->jobs.first().job.type == KisDabRenderingJob::Dab);

    const int copyJobAfter =
        returnMutableDabs && !m_d->dabsHaveSeparateOriginal() ?
            m_d->findLastDabJobIndex(m_d->jobs.size() - 1) :
            std::numeric_limits<int>::max();

    for (int i = 0; i < m_d->jobs.size(); i++) {
        Private::JobWrapper &w = m_d->jobs[i];
        KisDabRenderingJob &j = w.job;

        if (w.status != Private::JobWrapper::Completed) break;

        if (i <= m_d->lastPaintedJob) continue;

        KisRenderedDab dab;
        KisFixedPaintDeviceSP resultDevice = j.postprocessedDevice;

        if (i >= copyJobAfter) {
            resultDevice = new KisFixedPaintDevice(*resultDevice);
        }

        dab.device = resultDevice;
        dab.offset = j.dstDabOffset();
        dab.opacity = w.opacity;
        dab.flow = w.flow;

        m_d->averageOpacity = KisPainter::blendAverageOpacity(w.opacity, m_d->averageOpacity);
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
            m_d->jobs[nextToBePainted].status == Private::JobWrapper::Completed;
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

