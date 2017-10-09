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

KisDabRenderingJob::KisDabRenderingJob(int _seqNo, KisDabCacheUtils::DabGenerationInfo _generationInfo, KisDabCacheUtils::DabRenderingResources *_resources, KisDabRenderingJob::JobType _type)
    : seqNo(_seqNo),
      generationInfo(_generationInfo),
      resources(_resources),
      type(_type)
{
}

KisDabRenderingJob::KisDabRenderingJob(const KisDabRenderingJob &rhs)
    : KisSharedRunnable(),
      seqNo(rhs.seqNo),
      generationInfo(rhs.generationInfo),
      resources(rhs.resources),
      type(rhs.type),
      originalDevice(rhs.originalDevice),
      postprocessedDevice(rhs.postprocessedDevice),
      parentQueue(rhs.parentQueue),
      runnableJobsInterface(rhs.runnableJobsInterface)
{
}

KisDabRenderingJob &KisDabRenderingJob::operator=(const KisDabRenderingJob &rhs)
{
    seqNo = rhs.seqNo;
    generationInfo = rhs.generationInfo;
    resources = rhs.resources;
    type = rhs.type;
    originalDevice = rhs.originalDevice;
    postprocessedDevice = rhs.postprocessedDevice;
    parentQueue = rhs.parentQueue;
    runnableJobsInterface = rhs.runnableJobsInterface;

    return *this;
}

int KisDabRenderingJob::executeOneJob(KisDabRenderingJob *job)
{
    using namespace KisDabCacheUtils;

    KIS_SAFE_ASSERT_RECOVER_NOOP(job->type == KisDabRenderingJob::Dab ||
                                 job->type == KisDabRenderingJob::Postprocess);

    QElapsedTimer executionTime;
    executionTime.start();

    if (job->type == KisDabRenderingJob::Dab) {
        // TODO: thing about better interface for the reverse queue link
        job->originalDevice = job->parentQueue->fetchCachedPaintDevce();

        generateDab(job->generationInfo, job->resources, &job->originalDevice);
    }

    // by now the original device should be already prepared
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(job->originalDevice, 0);

    if (job->type == KisDabRenderingJob::Dab ||
        job->type == KisDabRenderingJob::Postprocess) {

        if (job->generationInfo.needsPostprocessing) {
            // TODO: cache postprocessed device

            if (!job->postprocessedDevice ||
                *job->originalDevice->colorSpace() != *job->postprocessedDevice->colorSpace()) {

                job->postprocessedDevice = new KisFixedPaintDevice(*job->originalDevice);
            } else {
                *job->postprocessedDevice = *job->originalDevice;
            }

            postProcessDab(job->postprocessedDevice,
                           job->generationInfo.dstDabRect.topLeft(),
                           job->generationInfo.info,
                           job->resources);
        } else {
            job->postprocessedDevice = job->originalDevice;
        }
    }

    return executionTime.nsecsElapsed() / 1000;
}

void KisDabRenderingJob::runShared()
{
    int executionTime = 0;

    executionTime = executeOneJob(this);
    QList<KisDabRenderingJob *> jobs = parentQueue->notifyJobFinished(this, executionTime);

    while (!jobs.isEmpty()) {
        QVector<KisRunnableStrokeJobData*> dataList;

        // start all-but-the-first jobs asynchronously
        for (int i = 1; i < jobs.size(); i++) {
            dataList.append(new FreehandStrokeRunnableJobDataWithUpdate(jobs[i], KisStrokeJobData::CONCURRENT));
        }

        runnableJobsInterface->addRunnableJobs(dataList);


        // execute the first job in the current thread
        KisDabRenderingJob *job = jobs.first();
        executionTime = executeOneJob(job);
        jobs = parentQueue->notifyJobFinished(job, executionTime);

        // mimic the behavior of the thread pool
        if (job->autoDelete()) {
            delete job;
        }
    }
}

QPoint KisDabRenderingJob::dstDabOffset() const
{
    return generationInfo.dstDabRect.topLeft();
}


