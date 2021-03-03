/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisDabRenderingQueueTest.h"

#include <simpletest.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <../KisDabRenderingQueue.h>
#include <../KisRenderedDab.h>
#include <../KisDabRenderingJob.h>

struct SurrogateCacheInterface : public KisDabRenderingQueue::CacheInterface
{
    void getDabType(bool hasDabInCache,
                    KisDabCacheUtils::DabRenderingResources *resources,
                    const KisDabCacheUtils::DabRequestInfo &request,
                    /* out */
                    KisDabCacheUtils::DabGenerationInfo *di,
                    bool *shouldUseCache) override
    {
        Q_UNUSED(resources);
        Q_UNUSED(request);

        if (!hasDabInCache || typeOverride == KisDabRenderingJob::Dab) {
            di->needsPostprocessing = false;
            *shouldUseCache = false;
        } else if (typeOverride == KisDabRenderingJob::Copy) {
            di->needsPostprocessing = false;
            *shouldUseCache = true;
        } else if (typeOverride == KisDabRenderingJob::Postprocess) {
            di->needsPostprocessing = true;
            *shouldUseCache = true;
        }

        di->info = request.info;
    }

    bool hasSeparateOriginal(KisDabCacheUtils::DabRenderingResources *resources) const override {
        Q_UNUSED(resources);
        return typeOverride == KisDabRenderingJob::Postprocess;
    }

    KisDabRenderingJob::JobType typeOverride = KisDabRenderingJob::Dab;
};

#include <kis_mask_generator.h>
#include "kis_auto_brush.h"

KisDabCacheUtils::DabRenderingResources *testResourcesFactory()
{
    KisDabCacheUtils::DabRenderingResources *resources =
        new KisDabCacheUtils::DabRenderingResources();

    KisCircleMaskGenerator* circle = new KisCircleMaskGenerator(10, 1.0, 1.0, 1.0, 2, false);
    KisBrushSP brush(new KisAutoBrush(circle, 0.0, 0.0));
    resources->brush = brush;

    return resources;
}

void KisDabRenderingQueueTest::testCachedDabs()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();

    SurrogateCacheInterface *cacheInterface = new SurrogateCacheInterface();

    KisDabRenderingQueue queue(cs, testResourcesFactory);
    queue.setCacheInterface(cacheInterface);

    KoColor color;
    QPointF pos1(10,10);
    QPointF pos2(20,20);
    KisDabShape shape;
    KisPaintInformation pi1(pos1);
    KisPaintInformation pi2(pos2);

    KisDabCacheUtils::DabRequestInfo request1(color, pos1, shape, pi1, 1.0);
    KisDabCacheUtils::DabRequestInfo request2(color, pos2, shape, pi2, 1.0);

    cacheInterface->typeOverride = KisDabRenderingJob::Dab;
    KisDabRenderingJobSP job0 = queue.addDab(request1, OPACITY_OPAQUE_F, OPACITY_OPAQUE_F);

    QVERIFY(job0);
    QCOMPARE(job0->seqNo, 0);
    QCOMPARE(job0->generationInfo.info.pos(), request1.info.pos());
    QCOMPARE(job0->type, KisDabRenderingJob::Dab);
    QVERIFY(!job0->originalDevice);
    QVERIFY(!job0->postprocessedDevice);

    cacheInterface->typeOverride = KisDabRenderingJob::Dab;
    KisDabRenderingJobSP job1 = queue.addDab(request2, OPACITY_OPAQUE_F, OPACITY_OPAQUE_F);

    QVERIFY(job1);
    QCOMPARE(job1->seqNo, 1);
    QCOMPARE(job1->generationInfo.info.pos(), request2.info.pos());
    QCOMPARE(job1->type, KisDabRenderingJob::Dab);
    QVERIFY(!job1->originalDevice);
    QVERIFY(!job1->postprocessedDevice);

    cacheInterface->typeOverride = KisDabRenderingJob::Copy;
    KisDabRenderingJobSP job2 = queue.addDab(request2, OPACITY_OPAQUE_F, OPACITY_OPAQUE_F);
    QVERIFY(!job2);

    cacheInterface->typeOverride = KisDabRenderingJob::Copy;
    KisDabRenderingJobSP job3 = queue.addDab(request2, OPACITY_OPAQUE_F, OPACITY_OPAQUE_F);
    QVERIFY(!job3);

    // we only added the dabs, but we haven't completed them yet
    QVERIFY(!queue.hasPreparedDabs());
    QCOMPARE(queue.testingGetQueueSize(), 4);

    QList<KisDabRenderingJobSP > jobs;
    QList<KisRenderedDab> renderedDabs;


    {
        // we've completed job0
        job0->originalDevice = new KisFixedPaintDevice(cs);
        job0->postprocessedDevice = job0->originalDevice;

        jobs = queue.notifyJobFinished(job0->seqNo);
        QVERIFY(jobs.isEmpty());

        // now we should have at least one job in prepared state
        QVERIFY(queue.hasPreparedDabs());

        // take the prepared dabs
        renderedDabs = queue.takeReadyDabs();
        QCOMPARE(renderedDabs.size(), 1);

        // the list should be empty again
        QVERIFY(!queue.hasPreparedDabs());
        QCOMPARE(queue.testingGetQueueSize(), 3);
    }

    {
        // we've completed job1
        job1->originalDevice = new KisFixedPaintDevice(cs);
        job1->postprocessedDevice = job1->originalDevice;

        jobs = queue.notifyJobFinished(job1->seqNo);
        QVERIFY(jobs.isEmpty());

        // now we should have at least one job in prepared state
        QVERIFY(queue.hasPreparedDabs());

        // take the prepared dabs
        renderedDabs = queue.takeReadyDabs();
        QCOMPARE(renderedDabs.size(), 3);

        // since they are copies, they should be the same
        QCOMPARE(renderedDabs[1].device, renderedDabs[0].device);
        QCOMPARE(renderedDabs[2].device, renderedDabs[0].device);

        // the list should be empty again
        QVERIFY(!queue.hasPreparedDabs());

        // we delete all the painted jobs except the latest 'dab' job
        QCOMPARE(queue.testingGetQueueSize(), 1);
    }

    {
        // add one more cached job and take it
        cacheInterface->typeOverride = KisDabRenderingJob::Copy;
        KisDabRenderingJobSP job = queue.addDab(request2, OPACITY_OPAQUE_F, OPACITY_OPAQUE_F);
        QVERIFY(!job);

        // now we should have at least one job in prepared state
        QVERIFY(queue.hasPreparedDabs());

        // take the prepared dabs
        renderedDabs = queue.takeReadyDabs();
        QCOMPARE(renderedDabs.size(), 1);

        // the list should be empty again
        QVERIFY(!queue.hasPreparedDabs());

        // we delete all the painted jobs except the latest 'dab' job
        QCOMPARE(queue.testingGetQueueSize(), 1);
    }

    {
        // add a 'dab' job and complete it

        cacheInterface->typeOverride = KisDabRenderingJob::Dab;
        KisDabRenderingJobSP job = queue.addDab(request1, OPACITY_OPAQUE_F, OPACITY_OPAQUE_F);

        QVERIFY(job);
        QCOMPARE(job->seqNo, 5);
        QCOMPARE(job->generationInfo.info.pos(), request1.info.pos());
        QCOMPARE(job->type, KisDabRenderingJob::Dab);
        QVERIFY(!job->originalDevice);
        QVERIFY(!job->postprocessedDevice);

        // now the queue can be cleared from the completed dabs!
        QCOMPARE(queue.testingGetQueueSize(), 1);

        job->originalDevice = new KisFixedPaintDevice(cs);
        job->postprocessedDevice = job->originalDevice;

        jobs = queue.notifyJobFinished(job->seqNo);
        QVERIFY(jobs.isEmpty());

        // now we should have at least one job in prepared state
        QVERIFY(queue.hasPreparedDabs());

        // take the prepared dabs
        renderedDabs = queue.takeReadyDabs();
        QCOMPARE(renderedDabs.size(), 1);

        // the list should be empty again
        QVERIFY(!queue.hasPreparedDabs());

        // we do not delete the queue of jobs until the next 'dab'
        // job arrives
        QCOMPARE(queue.testingGetQueueSize(), 1);
    }

}

void KisDabRenderingQueueTest::testPostprocessedDabs()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();

    SurrogateCacheInterface *cacheInterface = new SurrogateCacheInterface();

    KisDabRenderingQueue queue(cs, testResourcesFactory);
    queue.setCacheInterface(cacheInterface);

    KoColor color;
    QPointF pos1(10,10);
    QPointF pos2(20,20);
    KisDabShape shape;
    KisPaintInformation pi1(pos1);
    KisPaintInformation pi2(pos2);

    KisDabCacheUtils::DabRequestInfo request1(color, pos1, shape, pi1, 1.0);
    KisDabCacheUtils::DabRequestInfo request2(color, pos2, shape, pi2, 1.0);

    cacheInterface->typeOverride = KisDabRenderingJob::Dab;
    KisDabRenderingJobSP job0 = queue.addDab(request1, OPACITY_OPAQUE_F, OPACITY_OPAQUE_F);

    QVERIFY(job0);
    QCOMPARE(job0->seqNo, 0);
    QCOMPARE(job0->generationInfo.info.pos(), request1.info.pos());
    QCOMPARE(job0->type, KisDabRenderingJob::Dab);
    QVERIFY(!job0->originalDevice);
    QVERIFY(!job0->postprocessedDevice);

    cacheInterface->typeOverride = KisDabRenderingJob::Dab;
    KisDabRenderingJobSP job1 = queue.addDab(request2, OPACITY_OPAQUE_F, OPACITY_OPAQUE_F);

    QVERIFY(job1);
    QCOMPARE(job1->seqNo, 1);
    QCOMPARE(job1->generationInfo.info.pos(), request2.info.pos());
    QCOMPARE(job1->type, KisDabRenderingJob::Dab);
    QVERIFY(!job1->originalDevice);
    QVERIFY(!job1->postprocessedDevice);

    cacheInterface->typeOverride = KisDabRenderingJob::Postprocess;
    KisDabRenderingJobSP job2 = queue.addDab(request2, OPACITY_OPAQUE_F, OPACITY_OPAQUE_F);
    QVERIFY(!job2);

    cacheInterface->typeOverride = KisDabRenderingJob::Postprocess;
    KisDabRenderingJobSP job3 = queue.addDab(request2, OPACITY_OPAQUE_F, OPACITY_OPAQUE_F);
    QVERIFY(!job3);

    // we only added the dabs, but we haven't completed them yet
    QVERIFY(!queue.hasPreparedDabs());
    QCOMPARE(queue.testingGetQueueSize(), 4);

    QList<KisDabRenderingJobSP > jobs;
    QList<KisRenderedDab> renderedDabs;


    {
        // we've completed job0
        job0->originalDevice = new KisFixedPaintDevice(cs);
        job0->postprocessedDevice = job0->originalDevice;

        jobs = queue.notifyJobFinished(job0->seqNo);
        QVERIFY(jobs.isEmpty());

        // now we should have at least one job in prepared state
        QVERIFY(queue.hasPreparedDabs());

        // take the prepared dabs
        renderedDabs = queue.takeReadyDabs();
        QCOMPARE(renderedDabs.size(), 1);

        // the list should be empty again
        QVERIFY(!queue.hasPreparedDabs());
        QCOMPARE(queue.testingGetQueueSize(), 3);
    }

    {
        // we've completed job1
        job1->originalDevice = new KisFixedPaintDevice(cs);
        job1->postprocessedDevice = job1->originalDevice;

        jobs = queue.notifyJobFinished(job1->seqNo);
        QCOMPARE(jobs.size(), 2);

        QCOMPARE(jobs[0]->seqNo, 2);
        QCOMPARE(jobs[1]->seqNo, 3);

        QVERIFY(jobs[0]->originalDevice);
        QVERIFY(!jobs[0]->postprocessedDevice);

        QVERIFY(jobs[1]->originalDevice);
        QVERIFY(!jobs[1]->postprocessedDevice);

        // pretend we have created a postprocessed device
        jobs[0]->postprocessedDevice = new KisFixedPaintDevice(cs);
        jobs[1]->postprocessedDevice = new KisFixedPaintDevice(cs);

        // now we should have at least one job in prepared state
        QVERIFY(queue.hasPreparedDabs());

        // take the prepared dabs
        renderedDabs = queue.takeReadyDabs();
        QCOMPARE(renderedDabs.size(), 1);

        // the list should be empty again
        QVERIFY(!queue.hasPreparedDabs());


        // return back two postprocessed dabs
        QList<KisDabRenderingJobSP > emptyJobs;
        emptyJobs = queue.notifyJobFinished(jobs[0]->seqNo);
        QVERIFY(emptyJobs.isEmpty());

        emptyJobs = queue.notifyJobFinished(jobs[1]->seqNo);
        QVERIFY(emptyJobs.isEmpty());


        // now we should have at least one job in prepared state
        QVERIFY(queue.hasPreparedDabs());

        // take the prepared dabs
        renderedDabs = queue.takeReadyDabs();
        QCOMPARE(renderedDabs.size(), 2);

        // the list should be empty again
        QVERIFY(!queue.hasPreparedDabs());

        // we delete all the painted jobs except the latest 'dab' job
        QCOMPARE(queue.testingGetQueueSize(), 1);
    }

    {
        // add one more postprocessed job and take it
        cacheInterface->typeOverride = KisDabRenderingJob::Postprocess;
        KisDabRenderingJobSP job = queue.addDab(request2, OPACITY_OPAQUE_F, OPACITY_OPAQUE_F);

        QVERIFY(job);
        QCOMPARE(job->seqNo, 4);
        QCOMPARE(job->generationInfo.info.pos(), request2.info.pos());
        ENTER_FUNCTION() << ppVar(job->type);

        QCOMPARE(job->type, KisDabRenderingJob::Postprocess);
        QVERIFY(job->originalDevice);
        QVERIFY(!job->postprocessedDevice);

        // the list should still be empty
        QVERIFY(!queue.hasPreparedDabs());

        // pretend we have created a postprocessed device
        job->postprocessedDevice = new KisFixedPaintDevice(cs);

        // return back the postprocessed dab
        QList<KisDabRenderingJobSP > emptyJobs;
        emptyJobs = queue.notifyJobFinished(job->seqNo);
        QVERIFY(emptyJobs.isEmpty());

        // now we should have at least one job in prepared state
        QVERIFY(queue.hasPreparedDabs());

        // take the prepared dabs
        renderedDabs = queue.takeReadyDabs();
        QCOMPARE(renderedDabs.size(), 1);

        // the list should be empty again
        QVERIFY(!queue.hasPreparedDabs());

        // we delete all the painted jobs except the latest 'dab' job
        QCOMPARE(queue.testingGetQueueSize(), 1);
    }

    {
        // add a 'dab' job and complete it. That will clear the queue!

        cacheInterface->typeOverride = KisDabRenderingJob::Dab;
        KisDabRenderingJobSP job = queue.addDab(request1, OPACITY_OPAQUE_F, OPACITY_OPAQUE_F);

        QVERIFY(job);
        QCOMPARE(job->seqNo, 5);
        QCOMPARE(job->generationInfo.info.pos(), request1.info.pos());
        QCOMPARE(job->type, KisDabRenderingJob::Dab);
        QVERIFY(!job->originalDevice);
        QVERIFY(!job->postprocessedDevice);

        // now the queue can be cleared from the completed dabs!
        QCOMPARE(queue.testingGetQueueSize(), 1);

        job->originalDevice = new KisFixedPaintDevice(cs);
        job->postprocessedDevice = job->originalDevice;

        jobs = queue.notifyJobFinished(job->seqNo);
        QVERIFY(jobs.isEmpty());

        // now we should have at least one job in prepared state
        QVERIFY(queue.hasPreparedDabs());

        // take the prepared dabs
        renderedDabs = queue.takeReadyDabs();
        QCOMPARE(renderedDabs.size(), 1);

        // the list should be empty again
        QVERIFY(!queue.hasPreparedDabs());

        // we do not delete the queue of jobs until the next 'dab'
        // job arrives
        QCOMPARE(queue.testingGetQueueSize(), 1);
    }

}

#include <../KisDabRenderingQueueCache.h>

void KisDabRenderingQueueTest::testRunningJobs()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();

    KisDabRenderingQueueCache *cacheInterface = new KisDabRenderingQueueCache();
    // we do *not* initialize any options yet!

    KisDabRenderingQueue queue(cs, testResourcesFactory);
    queue.setCacheInterface(cacheInterface);


    KoColor color(Qt::red, cs);
    QPointF pos1(10,10);
    QPointF pos2(20,20);
    KisDabShape shape;
    KisPaintInformation pi1(pos1);
    KisPaintInformation pi2(pos2);

    KisDabCacheUtils::DabRequestInfo request1(color, pos1, shape, pi1, 1.0);
    KisDabCacheUtils::DabRequestInfo request2(color, pos2, shape, pi2, 1.0);

    KisDabRenderingJobSP job0 = queue.addDab(request1, OPACITY_OPAQUE_F, OPACITY_OPAQUE_F);

    QVERIFY(job0);
    QCOMPARE(job0->seqNo, 0);
    QCOMPARE(job0->generationInfo.info.pos(), request1.info.pos());
    QCOMPARE(job0->type, KisDabRenderingJob::Dab);

    QVERIFY(!job0->originalDevice);
    QVERIFY(!job0->postprocessedDevice);

    KisDabRenderingJobRunner runner(job0, &queue, 0);
    runner.run();

    QVERIFY(job0->originalDevice);
    QVERIFY(job0->postprocessedDevice);
    QCOMPARE(job0->originalDevice, job0->postprocessedDevice);

    QVERIFY(!job0->originalDevice->bounds().isEmpty());

    KisDabRenderingJobSP job1 = queue.addDab(request2, OPACITY_OPAQUE_F, OPACITY_OPAQUE_F);
    QVERIFY(!job1);

    QList<KisRenderedDab> renderedDabs = queue.takeReadyDabs();
    QCOMPARE(renderedDabs.size(), 2);

    // we did the caching
    QVERIFY(renderedDabs[0].device == renderedDabs[1].device);

    QCOMPARE(renderedDabs[0].offset, QPoint(5,5));
    QCOMPARE(renderedDabs[1].offset, QPoint(15,15));
}

#include "../KisDabRenderingExecutor.h"
#include "KisFakeRunnableStrokeJobsExecutor.h"

void KisDabRenderingQueueTest::testExecutor()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();

    QScopedPointer<KisRunnableStrokeJobsInterface> runner(new KisFakeRunnableStrokeJobsExecutor());

    KisDabRenderingExecutor executor(cs, testResourcesFactory, runner.data());

    KoColor color(Qt::red, cs);
    QPointF pos1(10,10);
    QPointF pos2(20,20);
    KisDabShape shape;
    KisPaintInformation pi1(pos1);
    KisPaintInformation pi2(pos2);

    KisDabCacheUtils::DabRequestInfo request1(color, pos1, shape, pi1, 1.0);
    KisDabCacheUtils::DabRequestInfo request2(color, pos2, shape, pi2, 1.0);

    executor.addDab(request1, 0.5, 0.25);
    executor.addDab(request2, 0.125, 1.0);

    QList<KisRenderedDab> renderedDabs = executor.takeReadyDabs();
    QCOMPARE(renderedDabs.size(), 2);

    // we did the caching
    QVERIFY(renderedDabs[0].device == renderedDabs[1].device);

    QCOMPARE(renderedDabs[0].offset, QPoint(5,5));
    QCOMPARE(renderedDabs[1].offset, QPoint(15,15));

    QCOMPARE(renderedDabs[0].opacity, 0.5);
    QCOMPARE(renderedDabs[0].flow, 0.25);
    QCOMPARE(renderedDabs[1].opacity, 0.125);
    QCOMPARE(renderedDabs[1].flow, 1.0);

}

SIMPLE_TEST_MAIN(KisDabRenderingQueueTest)
