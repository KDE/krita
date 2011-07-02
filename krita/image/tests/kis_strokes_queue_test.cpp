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


#define COMPARE_WALKER(item, walker)            \
    QCOMPARE(item->walker(), walker)
#define COMPARE_NAME(item, name)                                \
    QCOMPARE(getJobName(item->strokeJob()), QString(name))
#define VERIFY_EMPTY(item)                                      \
    QVERIFY(!item->isRunning())


void KisStrokesQueueTest::testSequentialJobs()
{
    KisStrokesQueue queue;
    queue.startStroke(new KisTestingStrokeStrategy("tri_", false));
    queue.addJob(0);
    queue.addJob(0);
    queue.addJob(0);
    queue.endStroke();

    KisTestableUpdaterContext context(2);
    QVector<KisUpdateJobItem*> jobs;

    queue.processQueue(context);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "tri_init");
    VERIFY_EMPTY(jobs[1]);

    context.clear();
    queue.processQueue(context);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "tri_dab");
    COMPARE_NAME(jobs[1], "tri_dab");

    context.clear();
    queue.processQueue(context);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "tri_dab");
    VERIFY_EMPTY(jobs[1]);

    context.clear();
    queue.processQueue(context);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "tri_finish");
    VERIFY_EMPTY(jobs[1]);
}

void KisStrokesQueueTest::testExclusiveStrokes()
{
    KisStrokesQueue queue;
    queue.startStroke(new KisTestingStrokeStrategy("excl_", true));
    queue.addJob(0);
    queue.addJob(0);
    queue.addJob(0);
    queue.endStroke();

    // well, this walker is not initialized... but who cares?
    KisBaseRectsWalkerSP walker = new KisMergeWalker(QRect());

    KisTestableUpdaterContext context(2);
    QVector<KisUpdateJobItem*> jobs;

    context.addMergeJob(walker);
    queue.processQueue(context);

    jobs = context.getJobs();
    COMPARE_WALKER(jobs[0], walker);
    VERIFY_EMPTY(jobs[1]);
    QCOMPARE(queue.needsExclusiveAccess(), true);

    context.clear();
    queue.processQueue(context);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "excl_init");
    VERIFY_EMPTY(jobs[1]);
    QCOMPARE(queue.needsExclusiveAccess(), true);

    context.clear();
    queue.processQueue(context);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "excl_dab");
    COMPARE_NAME(jobs[1], "excl_dab");
    QCOMPARE(queue.needsExclusiveAccess(), true);

    context.clear();
    context.addMergeJob(walker);
    queue.processQueue(context);

    COMPARE_WALKER(jobs[0], walker);
    VERIFY_EMPTY(jobs[1]);
    QCOMPARE(queue.needsExclusiveAccess(), true);

    context.clear();
    queue.processQueue(context);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "excl_dab");
    VERIFY_EMPTY(jobs[1]);
    QCOMPARE(queue.needsExclusiveAccess(), true);

    context.clear();
    queue.processQueue(context);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "excl_finish");
    VERIFY_EMPTY(jobs[1]);
    QCOMPARE(queue.needsExclusiveAccess(), true);

    context.clear();
    queue.processQueue(context);

    QCOMPARE(queue.needsExclusiveAccess(), false);
}

void KisStrokesQueueTest::testStrokesOverlapping()
{
    KisStrokesQueue queue;
    queue.startStroke(new KisTestingStrokeStrategy("1_", false, true));
    queue.addJob(0);

    // comment out this line to catch an assert
    queue.endStroke();

    queue.startStroke(new KisTestingStrokeStrategy("2_", false, true));
    queue.addJob(0);
    queue.endStroke();

    // uncomment this line to catch an assert
    // queue.addJob(0);

    KisTestableUpdaterContext context(2);
    QVector<KisUpdateJobItem*> jobs;

    queue.processQueue(context);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "1_dab");
    VERIFY_EMPTY(jobs[1]);

    context.clear();
    queue.processQueue(context);

    jobs = context.getJobs();
    COMPARE_NAME(jobs[0], "2_dab");
    VERIFY_EMPTY(jobs[1]);
}

QTEST_KDEMAIN(KisStrokesQueueTest, NoGUI)
#include "kis_strokes_queue_test.moc"
