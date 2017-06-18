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

#include "kis_strokes_queue_test.h"
#include <QTest>

#include "scheduler_utils.h"
#include "kis_strokes_queue.h"
#include "kis_updater_context.h"
#include "kis_update_job_item.h"
#include "kis_merge_walker.h"


void KisStrokesQueueTest::testSequentialJobs()
{
    KisStrokesQueue queue;
    KisStrokeId id = queue.startStroke(new KisTestingStrokeStrategy("tri_", false));
    queue.addJob(id, new KisStrokeJobData(KisStrokeJobData::CONCURRENT));
    queue.addJob(id, new KisStrokeJobData(KisStrokeJobData::CONCURRENT));
    queue.addJob(id, new KisStrokeJobData(KisStrokeJobData::CONCURRENT));
    queue.endStroke(id);

    KisTestableUpdaterContext context(2);
    QVector<KisUpdateJobItem*> jobs;

    queue.processQueue(context, false);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "tri_init");
    VERIFY_EMPTY(jobs[1]);

    context.clear();
    queue.processQueue(context, false);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "tri_dab");
    COMPARE_NAME(jobs[1], "tri_dab");

    context.clear();
    queue.processQueue(context, false);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "tri_dab");
    VERIFY_EMPTY(jobs[1]);

    context.clear();
    queue.processQueue(context, false);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "tri_finish");
    VERIFY_EMPTY(jobs[1]);
}

void KisStrokesQueueTest::testConcurrentSequentialBarrier()
{
    KisStrokesQueue queue;
    KisStrokeId id = queue.startStroke(new KisTestingStrokeStrategy("tri_", false));
    queue.addJob(id, new KisStrokeJobData(KisStrokeJobData::CONCURRENT));
    queue.addJob(id, new KisStrokeJobData(KisStrokeJobData::CONCURRENT));
    queue.endStroke(id);

    // make the number of threads higher
    KisTestableUpdaterContext context(3);
    QVector<KisUpdateJobItem*> jobs;

    queue.processQueue(context, false);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "tri_init");
    VERIFY_EMPTY(jobs[1]);

    context.clear();
    queue.processQueue(context, false);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "tri_dab");
    COMPARE_NAME(jobs[1], "tri_dab");

    context.clear();
    queue.processQueue(context, false);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "tri_finish");
    VERIFY_EMPTY(jobs[1]);
}

void KisStrokesQueueTest::testExclusiveStrokes()
{
    KisStrokesQueue queue;
    KisStrokeId id = queue.startStroke(new KisTestingStrokeStrategy("excl_", true));
    queue.addJob(id, new KisStrokeJobData(KisStrokeJobData::CONCURRENT));
    queue.addJob(id, new KisStrokeJobData(KisStrokeJobData::CONCURRENT));
    queue.addJob(id, new KisStrokeJobData(KisStrokeJobData::CONCURRENT));
    queue.endStroke(id);

    // well, this walker is not initialized... but who cares?
    KisBaseRectsWalkerSP walker = new KisMergeWalker(QRect());

    KisTestableUpdaterContext context(2);
    QVector<KisUpdateJobItem*> jobs;

    context.addMergeJob(walker);
    queue.processQueue(context, false);

    jobs = context.getJobs();
    COMPARE_WALKER(jobs[0], walker);
    VERIFY_EMPTY(jobs[1]);
    QCOMPARE(queue.needsExclusiveAccess(), true);

    context.clear();
    queue.processQueue(context, false);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "excl_init");
    VERIFY_EMPTY(jobs[1]);
    QCOMPARE(queue.needsExclusiveAccess(), true);

    context.clear();
    queue.processQueue(context, false);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "excl_dab");
    COMPARE_NAME(jobs[1], "excl_dab");
    QCOMPARE(queue.needsExclusiveAccess(), true);

    context.clear();
    context.addMergeJob(walker);
    queue.processQueue(context, false);

    COMPARE_WALKER(jobs[0], walker);
    VERIFY_EMPTY(jobs[1]);
    QCOMPARE(queue.needsExclusiveAccess(), true);

    context.clear();
    queue.processQueue(context, false);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "excl_dab");
    VERIFY_EMPTY(jobs[1]);
    QCOMPARE(queue.needsExclusiveAccess(), true);

    context.clear();
    queue.processQueue(context, false);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "excl_finish");
    VERIFY_EMPTY(jobs[1]);
    QCOMPARE(queue.needsExclusiveAccess(), true);

    context.clear();
    queue.processQueue(context, false);

    QCOMPARE(queue.needsExclusiveAccess(), false);
}

void KisStrokesQueueTest::testBarrierStrokeJobs()
{
    KisStrokesQueue queue;
    KisStrokeId id = queue.startStroke(new KisTestingStrokeStrategy("nor_", false));
    queue.addJob(id, new KisStrokeJobData(KisStrokeJobData::CONCURRENT));
    queue.addJob(id, new KisStrokeJobData(KisStrokeJobData::BARRIER));
    queue.addJob(id, new KisStrokeJobData(KisStrokeJobData::CONCURRENT));
    queue.endStroke(id);

    // yes, this walker is not initialized again... but who cares?
    KisBaseRectsWalkerSP walker = new KisMergeWalker(QRect());
    bool externalJobsPending = false;

    KisTestableUpdaterContext context(3);
    QVector<KisUpdateJobItem*> jobs;

    queue.processQueue(context, externalJobsPending);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "nor_init");
    VERIFY_EMPTY(jobs[1]);
    VERIFY_EMPTY(jobs[2]);

    context.clear();

    queue.processQueue(context, externalJobsPending);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "nor_dab");
    VERIFY_EMPTY(jobs[1]);
    VERIFY_EMPTY(jobs[2]);

    // Now some updates has come...
    context.addMergeJob(walker);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "nor_dab");
    COMPARE_WALKER(jobs[1], walker);
    VERIFY_EMPTY(jobs[2]);

    // No difference for the queue
    queue.processQueue(context, externalJobsPending);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "nor_dab");
    COMPARE_WALKER(jobs[1], walker);
    VERIFY_EMPTY(jobs[2]);

    // Even more updates has come...
    externalJobsPending = true;

    // Still no difference for the queue
    queue.processQueue(context, externalJobsPending);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "nor_dab");
    COMPARE_WALKER(jobs[1], walker);
    VERIFY_EMPTY(jobs[2]);

    // Now clear the context
    context.clear();

    // And still no difference for the queue
    queue.processQueue(context, externalJobsPending);

    jobs = context.getJobs();
    VERIFY_EMPTY(jobs[0]);
    VERIFY_EMPTY(jobs[1]);
    VERIFY_EMPTY(jobs[2]);

    // Process the last update...
    context.addMergeJob(walker);
    externalJobsPending = false;

    // Yep, the queue is still waiting
    queue.processQueue(context, externalJobsPending);

    jobs = context.getJobs();
    COMPARE_WALKER(jobs[0], walker);
    VERIFY_EMPTY(jobs[1]);
    VERIFY_EMPTY(jobs[2]);

    context.clear();

    // Finally, we can do our work. Barrier job is executed alone
    queue.processQueue(context, externalJobsPending);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "nor_dab");
    VERIFY_EMPTY(jobs[1]);
    VERIFY_EMPTY(jobs[2]);

    // Barrier job has finished
    context.clear();

    jobs = context.getJobs();
    VERIFY_EMPTY(jobs[0]);
    VERIFY_EMPTY(jobs[1]);
    VERIFY_EMPTY(jobs[2]);

    // fetch the last (concurrent) one
    queue.processQueue(context, externalJobsPending);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "nor_dab");
    VERIFY_EMPTY(jobs[1]);
    VERIFY_EMPTY(jobs[2]);

    context.clear();

    // finish the stroke
    queue.processQueue(context, externalJobsPending);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "nor_finish");
    VERIFY_EMPTY(jobs[1]);
    VERIFY_EMPTY(jobs[2]);

    context.clear();
}

void KisStrokesQueueTest::testStrokesOverlapping()
{
    KisStrokesQueue queue;
    KisStrokeId id = queue.startStroke(new KisTestingStrokeStrategy("1_", false, true));
    queue.addJob(id, 0);

    // comment out this line to catch an assert
    queue.endStroke(id);

    id = queue.startStroke(new KisTestingStrokeStrategy("2_", false, true));
    queue.addJob(id, 0);
    queue.endStroke(id);

    // uncomment this line to catch an assert
    // queue.addJob(id, 0);

    KisTestableUpdaterContext context(2);
    QVector<KisUpdateJobItem*> jobs;

    queue.processQueue(context, false);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "1_dab");
    VERIFY_EMPTY(jobs[1]);

    context.clear();
    queue.processQueue(context, false);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "2_dab");
    VERIFY_EMPTY(jobs[1]);
}

void KisStrokesQueueTest::testImmediateCancel()
{
    KisStrokesQueue queue;
    KisTestableUpdaterContext context(2);

    KisStrokeId id = queue.startStroke(new KisTestingStrokeStrategy("1_", false, false));
    queue.cancelStroke(id);

    // this should not crash
    queue.processQueue(context, false);
}

void KisStrokesQueueTest::testOpenedStrokeCounter()
{
    KisStrokesQueue queue;

    QVERIFY(!queue.hasOpenedStrokes());
    KisStrokeId id0 = queue.startStroke(new KisTestingStrokeStrategy("0"));
    QVERIFY(queue.hasOpenedStrokes());
    KisStrokeId id1 = queue.startStroke(new KisTestingStrokeStrategy("1"));
    QVERIFY(queue.hasOpenedStrokes());
    queue.endStroke(id0);
    QVERIFY(queue.hasOpenedStrokes());
    queue.endStroke(id1);
    QVERIFY(!queue.hasOpenedStrokes());

    KisTestableUpdaterContext context(2);
    queue.processQueue(context, false); context.clear();
    queue.processQueue(context, false); context.clear();
    queue.processQueue(context, false); context.clear();
    queue.processQueue(context, false); context.clear();
}

void KisStrokesQueueTest::testAsyncCancelWhileOpenedStroke()
{
    KisStrokesQueue queue;
    KisStrokeId id = queue.startStroke(new KisTestingStrokeStrategy("nor_", false));
    queue.addJob(id, 0);
    queue.addJob(id, 0);
    queue.addJob(id, 0);

    // no async cancelling until the stroke is ended by the owner
    QVERIFY(!queue.tryCancelCurrentStrokeAsync());

    queue.endStroke(id);

    QVERIFY(queue.tryCancelCurrentStrokeAsync());

    bool externalJobsPending = false;

    KisTestableUpdaterContext context(3);
    QVector<KisUpdateJobItem*> jobs;

    queue.processQueue(context, externalJobsPending);

    // no? really?
    jobs = context.getJobs();
    VERIFY_EMPTY(jobs[0]);
    VERIFY_EMPTY(jobs[1]);
    VERIFY_EMPTY(jobs[2]);
}

struct KisStrokesQueueTest::LodStrokesQueueTester {

    LodStrokesQueueTester(bool real = false)
        : fakeContext(2),
          realContext(2),
          context(!real ? fakeContext : realContext)
    {
        queue.setSuspendUpdatesStrokeStrategyFactory(
            []() {
                return KisSuspendResumePair(
                    new KisTestingStrokeStrategy("susp_u_", false, true, true),
                    QList<KisStrokeJobData*>());
            });

        queue.setResumeUpdatesStrokeStrategyFactory(
            []() {
                return KisSuspendResumePair(
                    new KisTestingStrokeStrategy("resu_u_", false, true, true),
                    QList<KisStrokeJobData*>());
            });
        queue.setLod0ToNStrokeStrategyFactory(
            [](bool forgettable) {
                Q_UNUSED(forgettable);
                return KisSuspendResumePair(
                    new KisTestingStrokeStrategy("sync_u_", false, true, true),
                    QList<KisStrokeJobData*>());
            });
    }

    KisStrokesQueue queue;

    KisTestableUpdaterContext fakeContext;
    KisUpdaterContext realContext;
    KisUpdaterContext &context;
    QVector<KisUpdateJobItem*> jobs;

    void processQueueNoAdd() {
        if (&context != &fakeContext) return;

        fakeContext.clear();

        jobs = fakeContext.getJobs();
        VERIFY_EMPTY(jobs[0]);
        VERIFY_EMPTY(jobs[1]);
    }

    void processQueue() {
        processQueueNoAdd();
        queue.processQueue(context, false);

        if (&context == &realContext) {
            context.waitForDone();
        }
    }

    void checkOnlyJob(const QString &name) {
        KIS_ASSERT(&context == &fakeContext);

        jobs = fakeContext.getJobs();
        COMPARE_NAME(jobs[0], name);
        VERIFY_EMPTY(jobs[1]);
        QCOMPARE(queue.needsExclusiveAccess(), false);
    }

    void checkOnlyExecutedJob(const QString &name) {
        realContext.waitForDone();
        QVERIFY(!globalExecutedDabs.isEmpty());
        QCOMPARE(globalExecutedDabs[0], name);

        QCOMPARE(globalExecutedDabs.size(), 1);
        globalExecutedDabs.clear();
    }
};


void KisStrokesQueueTest::testStrokesLevelOfDetail()
{
    LodStrokesQueueTester t;
    KisStrokesQueue &queue = t.queue;

    // create a stroke with LOD0 + LOD2
    queue.setDesiredLevelOfDetail(2);
    KisStrokeId id2 = queue.startStroke(new KisTestingStrokeStrategy("lod_", false, true));
    queue.addJob(id2, new KisTestingStrokeJobData(KisStrokeJobData::CONCURRENT));
    queue.endStroke(id2);

    // create a update with LOD == 0 (default one)
    // well, this walker is not initialized... but who cares?
    KisBaseRectsWalkerSP walker = new KisMergeWalker(QRect());

    KisTestableUpdaterContext context(2);
    QVector<KisUpdateJobItem*> jobs;

    context.addMergeJob(walker);
    queue.processQueue(context, false);

    jobs = context.getJobs();
    COMPARE_WALKER(jobs[0], walker);
    VERIFY_EMPTY(jobs[1]);
    QCOMPARE(queue.needsExclusiveAccess(), false);

    context.clear();

    jobs = context.getJobs();
    VERIFY_EMPTY(jobs[0]);
    VERIFY_EMPTY(jobs[1]);

    context.clear();
    queue.processQueue(context, false);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "clone2_lod_dab");
    VERIFY_EMPTY(jobs[1]);
    QCOMPARE(queue.needsExclusiveAccess(), false);

    // walker of a different LOD must not be allowed
    QCOMPARE(context.isJobAllowed(walker), false);

    context.clear();
    context.addMergeJob(walker);
    queue.processQueue(context, false);

    jobs = context.getJobs();
    COMPARE_WALKER(jobs[0], walker);
    COMPARE_NAME(jobs[1], "susp_u_init");
    QCOMPARE(queue.needsExclusiveAccess(), false);

    context.clear();
    queue.processQueue(context, false);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "lod_dab");
    VERIFY_EMPTY(jobs[1]);
    QCOMPARE(queue.needsExclusiveAccess(), false);

    context.clear();
    queue.processQueue(context, false);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "resu_u_init");
    VERIFY_EMPTY(jobs[1]);
    QCOMPARE(queue.needsExclusiveAccess(), false);

    context.clear();
}

#include <kundo2command.h>
#include <kis_post_execution_undo_adapter.h>
struct TestUndoCommand : public KUndo2Command
{
    TestUndoCommand(const QString &text) : KUndo2Command(kundo2_noi18n(text)) {}

    void undo() override {
        ENTER_FUNCTION();
        undoCount++;
    }

    void redo() override {
        ENTER_FUNCTION();
        redoCount++;
    }

    int undoCount = 0;
    int redoCount = 0;
};

void KisStrokesQueueTest::testLodUndoBase()
{
    LodStrokesQueueTester t;
    KisStrokesQueue &queue = t.queue;

    // create a stroke with LOD0 + LOD2
    queue.setDesiredLevelOfDetail(2);
    KisStrokeId id1 = queue.startStroke(new KisTestingStrokeStrategy("str1_", false, true));
    queue.addJob(id1, new KisTestingStrokeJobData(KisStrokeJobData::CONCURRENT));
    queue.endStroke(id1);

    KisStrokeId id2 = queue.startStroke(new KisTestingStrokeStrategy("str2_", false, true));
    queue.addJob(id2, new KisTestingStrokeJobData(KisStrokeJobData::CONCURRENT));
    queue.endStroke(id2);

    t.processQueue();
    t.checkOnlyJob("clone2_str1_dab");


    QSharedPointer<TestUndoCommand> undoStr1(new TestUndoCommand("str1_undo"));
    queue.lodNPostExecutionUndoAdapter()->addCommand(undoStr1);

    t.processQueue();
    t.checkOnlyJob("clone2_str2_dab");

    QSharedPointer<TestUndoCommand> undoStr2(new TestUndoCommand("str2_undo"));
    queue.lodNPostExecutionUndoAdapter()->addCommand(undoStr2);

    t.processQueue();
    t.checkOnlyJob("susp_u_init");

    t.processQueue();
    t.checkOnlyJob("str1_dab");

    t.processQueue();
    t.checkOnlyJob("str2_dab");

    t.processQueue();
    t.checkOnlyJob("resu_u_init");
}

void KisStrokesQueueTest::testLodUndoBase2()
{
    LodStrokesQueueTester t(true);
    KisStrokesQueue &queue = t.queue;

    // create a stroke with LOD0 + LOD2
    queue.setDesiredLevelOfDetail(2);
    KisStrokeId id1 = queue.startStroke(new KisTestingStrokeStrategy("str1_", false, true, false, true));
    queue.addJob(id1, new KisTestingStrokeJobData(KisStrokeJobData::CONCURRENT));
    queue.endStroke(id1);

    KisStrokeId id2 = queue.startStroke(new KisTestingStrokeStrategy("str2_", false, true, false, true));
    queue.addJob(id2, new KisTestingStrokeJobData(KisStrokeJobData::CONCURRENT));
    queue.endStroke(id2);

    t.processQueue();
    t.checkOnlyExecutedJob("sync_u_init");

    t.processQueue();
    t.checkOnlyExecutedJob("clone2_str1_dab");

    QSharedPointer<TestUndoCommand> undoStr1(new TestUndoCommand("str1_undo"));
    queue.lodNPostExecutionUndoAdapter()->addCommand(undoStr1);

    t.processQueue();
    t.checkOnlyExecutedJob("clone2_str2_dab");

    QSharedPointer<TestUndoCommand> undoStr2(new TestUndoCommand("str2_undo"));
    queue.lodNPostExecutionUndoAdapter()->addCommand(undoStr2);

    t.processQueue();
    t.checkOnlyExecutedJob("susp_u_init");

    queue.tryUndoLastStrokeAsync();
    t.processQueue();

    while (queue.currentStrokeName() == kundo2_noi18n("str2_undo")) {
        //queue.debugPrintStrokes();
        t.processQueue();
    }

    QCOMPARE(undoStr2->undoCount, 1);

    t.checkOnlyExecutedJob("str1_dab");

    t.processQueue();
    t.checkOnlyExecutedJob("str2_cancel");

    t.processQueue();
    t.checkOnlyExecutedJob("resu_u_init");
}


QTEST_MAIN(KisStrokesQueueTest)
