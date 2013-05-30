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
#include <qtest_kde.h>

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

    // Finally, we can do our work
    queue.processQueue(context, externalJobsPending);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "nor_dab");
    COMPARE_NAME(jobs[1], "nor_dab");
    VERIFY_EMPTY(jobs[2]);

    context.clear();

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

QTEST_KDEMAIN(KisStrokesQueueTest, NoGUI)
#include "kis_strokes_queue_test.moc"
