/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisDabRenderingJob.h"

#include <QElapsedTimer>

#include <KisRunnableStrokeJobsInterface.h>
#include <KisRunnableStrokeJobData.h>

#include "KisDabCacheUtils.h"
#include "KisDabRenderingQueue.h"

#include <tool/strokes/FreehandStrokeRunnableJobDataWithUpdate.h>


KisDabRenderingJob::KisDabRenderingJob()
{
}

KisDabRenderingJob::KisDabRenderingJob(int _seqNo, KisDabCacheUtils::DabGenerationInfo _generationInfo, KisDabRenderingJob::JobType _type)
    : seqNo(_seqNo),
      generationInfo(_generationInfo),
      type(_type)
{
}

KisDabRenderingJob::KisDabRenderingJob(const KisDabRenderingJob &rhs)
    : seqNo(rhs.seqNo),
      generationInfo(rhs.generationInfo),
      type(rhs.type),
      originalDevice(rhs.originalDevice),
      postprocessedDevice(rhs.postprocessedDevice),
      status(rhs.status),
      opacity(rhs.opacity),
      flow(rhs.flow)
{
}

KisDabRenderingJob &KisDabRenderingJob::operator=(const KisDabRenderingJob &rhs)
{
    seqNo = rhs.seqNo;
    generationInfo = rhs.generationInfo;
    type = rhs.type;
    originalDevice = rhs.originalDevice;
    postprocessedDevice = rhs.postprocessedDevice;
    status = rhs.status;
    opacity = rhs.opacity;
    flow = rhs.flow;

    return *this;
}

QPoint KisDabRenderingJob::dstDabOffset() const
{
    /// Recenter generated low-res dab around the center
    /// of the idel theoretical dab rect
    const QPoint p1 = generationInfo.dstDabRect.topLeft();
    const QPoint s1 = QPoint(generationInfo.dstDabRect.width(),
                             generationInfo.dstDabRect.height());
    const QPoint s2 = QPoint(postprocessedDevice->bounds().width(),
                             postprocessedDevice->bounds().height());
    return p1 + (s1 - s2) / 2;
}



KisDabRenderingJobRunner::KisDabRenderingJobRunner(KisDabRenderingJobSP job,
                                                   KisDabRenderingQueue *parentQueue,
                                                   KisRunnableStrokeJobsInterface *runnableJobsInterface)
    : m_job(job),
      m_parentQueue(parentQueue),
      m_runnableJobsInterface(runnableJobsInterface)
{
}

KisDabRenderingJobRunner::~KisDabRenderingJobRunner()
{
}

int KisDabRenderingJobRunner::executeOneJob(KisDabRenderingJob *job,
                                            KisDabCacheUtils::DabRenderingResources *resources,
                                            KisDabRenderingQueue *parentQueue)
{
    using namespace KisDabCacheUtils;

    KIS_SAFE_ASSERT_RECOVER_NOOP(job->type == KisDabRenderingJob::Dab ||
                                 job->type == KisDabRenderingJob::Postprocess);

    QElapsedTimer executionTime;
    executionTime.start();

    resources->syncResourcesToSeqNo(job->seqNo, job->generationInfo.info);

    if (job->type == KisDabRenderingJob::Dab) {
        // TODO: thing about better interface for the reverse queue link
        job->originalDevice = parentQueue->fetchCachedPaintDevce();

        generateDab(job->generationInfo, resources, &job->originalDevice);
    }

    // by now the original device should be already prepared
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(job->originalDevice, 0);

    if (job->type == KisDabRenderingJob::Dab ||
        job->type == KisDabRenderingJob::Postprocess) {

        if (job->generationInfo.needsPostprocessing) {
            // TODO: cache postprocessed device

            if (!job->postprocessedDevice ||
                *job->originalDevice->colorSpace() != *job->postprocessedDevice->colorSpace()) {

                job->postprocessedDevice = parentQueue->fetchCachedPaintDevce();
                *job->postprocessedDevice = *job->originalDevice;
            } else {
                *job->postprocessedDevice = *job->originalDevice;
            }

            postProcessDab(job->postprocessedDevice,
                           job->generationInfo.dstDabRect.topLeft(),
                           job->generationInfo.info,
                           resources);
        } else {
            job->postprocessedDevice = job->originalDevice;
        }
    }

    return executionTime.nsecsElapsed() / 1000;
}

void KisDabRenderingJobRunner::run()
{
    int executionTime = 0;

    KisDabCacheUtils::DabRenderingResources *resources = m_parentQueue->fetchResourcesFromCache();

    executionTime = executeOneJob(m_job.data(), resources, m_parentQueue);
    QList<KisDabRenderingJobSP> jobs = m_parentQueue->notifyJobFinished(m_job->seqNo, executionTime);

    while (!jobs.isEmpty()) {
        QVector<KisRunnableStrokeJobData*> dataList;

        // start all-but-the-first jobs asynchronously
        for (int i = 1; i < jobs.size(); i++) {
            dataList.append(new FreehandStrokeRunnableJobDataWithUpdate(
                                new KisDabRenderingJobRunner(jobs[i], m_parentQueue, m_runnableJobsInterface),
                                KisStrokeJobData::CONCURRENT));
        }

        m_runnableJobsInterface->addRunnableJobs(dataList);


        // execute the first job in the current thread
        KisDabRenderingJobSP job = jobs.first();
        executionTime = executeOneJob(job.data(), resources, m_parentQueue);
        jobs = m_parentQueue->notifyJobFinished(job->seqNo, executionTime);
    }

    m_parentQueue->putResourcesToCache(resources);
}
