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

#include "kis_stroke_test.h"
#include <qtest_kde.h>

#include "kis_stroke.h"
#include "scheduler_utils.h"

void KisStrokeTest::testRegularStroke()
{
    KisStroke stroke(new KisTestingStrokeStrategy());
    QQueue<KisStrokeJob*> &queue = stroke.testingGetQueue();

    QCOMPARE(queue.size(), 1);
    SCOMPARE(getJobName(queue[0]), "init");
    QCOMPARE(stroke.isEnded(), false);

    stroke.addJob(0);

    QCOMPARE(queue.size(), 2);
    SCOMPARE(getJobName(queue[0]), "init");
    SCOMPARE(getJobName(queue[1]), "dab");
    QCOMPARE(stroke.isEnded(), false);

    stroke.endStroke();

    QCOMPARE(queue.size(), 3);
    SCOMPARE(getJobName(queue[0]), "init");
    SCOMPARE(getJobName(queue[1]), "dab");
    SCOMPARE(getJobName(queue[2]), "finish");
    QCOMPARE(stroke.isEnded(), true);

    // uncomment this line to catch an assert:
    // stroke.addJob(0);

    KisStrokeJob* job;

    job = stroke.popOneJob();
    delete job;
    QCOMPARE(queue.size(), 2);
    SCOMPARE(getJobName(queue[0]), "dab");
    SCOMPARE(getJobName(queue[1]), "finish");

    job = stroke.popOneJob();
    delete job;
    QCOMPARE(queue.size(), 1);
    SCOMPARE(getJobName(queue[0]), "finish");

    job = stroke.popOneJob();
    delete job;
    QCOMPARE(queue.size(), 0);

    job = stroke.popOneJob();
    QCOMPARE(job, (KisStrokeJob*)0);
}

void KisStrokeTest::testCancelStrokeCase1()
{
    KisStroke stroke(new KisTestingStrokeStrategy());
    QQueue<KisStrokeJob*> &queue = stroke.testingGetQueue();

    stroke.addJob(0);

    // "not initialized, has jobs"

    QCOMPARE(queue.size(), 2);
    SCOMPARE(getJobName(queue[0]), "init");
    SCOMPARE(getJobName(queue[1]), "dab");
    QCOMPARE(stroke.isEnded(), false);

    stroke.cancelStroke();

    QCOMPARE(queue.size(), 0);
    QCOMPARE(stroke.isEnded(), true);

    stroke.clearQueue();
}

void KisStrokeTest::testCancelStrokeCase2and3()
{
    KisStroke stroke(new KisTestingStrokeStrategy());
    QQueue<KisStrokeJob*> &queue = stroke.testingGetQueue();

    stroke.addJob(0);
    delete stroke.popOneJob();

    // "initialized, has jobs"

    QCOMPARE(queue.size(), 1);
    SCOMPARE(getJobName(queue[0]), "dab");
    QCOMPARE(stroke.isEnded(), false);

    stroke.cancelStroke();

    QCOMPARE(queue.size(), 1);
    SCOMPARE(getJobName(queue[0]), "cancel");
    QCOMPARE(stroke.isEnded(), true);

    stroke.clearQueue();
}

void KisStrokeTest::testCancelStrokeCase5()
{
    KisStroke stroke(new KisTestingStrokeStrategy());
    QQueue<KisStrokeJob*> &queue = stroke.testingGetQueue();

    // initialized, no jobs, not finished

    stroke.addJob(0);
    delete stroke.popOneJob(); // init
    delete stroke.popOneJob(); // dab

    QCOMPARE(stroke.isEnded(), false);

    stroke.cancelStroke();
    QCOMPARE(queue.size(), 1);
    SCOMPARE(getJobName(queue[0]), "cancel");
    QCOMPARE(stroke.isEnded(), true);

    delete stroke.popOneJob(); // cancel
}

void KisStrokeTest::testCancelStrokeCase4()
{
    KisStroke stroke(new KisTestingStrokeStrategy());
    QQueue<KisStrokeJob*> &queue = stroke.testingGetQueue();

    stroke.addJob(0);
    stroke.endStroke();
    delete stroke.popOneJob(); // init
    delete stroke.popOneJob(); // dab
    delete stroke.popOneJob(); // finish

    QCOMPARE(stroke.isEnded(), true);

    stroke.cancelStroke();
    QCOMPARE(queue.size(), 0);
    QCOMPARE(stroke.isEnded(), true);
}

QTEST_KDEMAIN(KisStrokeTest, NoGUI)
#include "kis_stroke_test.moc"
