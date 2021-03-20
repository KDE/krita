/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_queues_progress_updater_test.h"

#include <simpletest.h>

#include "kis_queues_progress_updater.h"
#include <testutil.h>


void KisQueuesProgressUpdaterTest::testSlowProgress()
{
    TestUtil::TestProgressBar progressProxy;
    KisQueuesProgressUpdater updater(&progressProxy);

    updater.updateProgress(200, "test task");
    updater.updateProgress(100, "test task");

    QTest::qWait(100);

    QCOMPARE(progressProxy.min(), 0);
    QCOMPARE(progressProxy.max(), 0);
    QCOMPARE(progressProxy.value(), 0);
    QCOMPARE(progressProxy.format(), QString());

    QTest::qWait(500);

    QCOMPARE(progressProxy.min(), 0);
    QEXPECT_FAIL("", "The max should be 200 but is 0.", Continue);
    QCOMPARE(progressProxy.max(), 200);
    QEXPECT_FAIL("", "Progress should be 100 but is 0.", Continue);
    QCOMPARE(progressProxy.value(), 100);
    QEXPECT_FAIL("", "format() should be 'test task' but is empty.", Continue);
    QCOMPARE(progressProxy.format(), QString("test task"));

    updater.updateProgress(0, "test task");

    QTest::qWait(500);

    QCOMPARE(progressProxy.min(), 0);
    QEXPECT_FAIL("", "Max should be 200 but is 100.", Continue);
    QCOMPARE(progressProxy.max(), 200);
    QEXPECT_FAIL("", "Value should be 200 but is 100.", Continue);
    QCOMPARE(progressProxy.value(), 200);
    QEXPECT_FAIL("", "format() should be 'test task' but is '%p%'.", Continue);
    QCOMPARE(progressProxy.format(), QString("test task"));
}

void KisQueuesProgressUpdaterTest::testFastProgress()
{
    /**
     * If the progress is too fast we don't even touch the bar
     */

    TestUtil::TestProgressBar progressProxy;
    KisQueuesProgressUpdater updater(&progressProxy);

    updater.updateProgress(200, "test task");
    updater.updateProgress(0, "test task");

    QTest::qWait(20);

    QCOMPARE(progressProxy.min(), 0);
    QEXPECT_FAIL("", "Max should be 0 but is 100.", Continue);
    QCOMPARE(progressProxy.max(), 0);
    QEXPECT_FAIL("", "Value should be 0 but is 100.", Continue);
    QCOMPARE(progressProxy.value(), 0);
    QEXPECT_FAIL("", "format() should be empty but is '%p%'.", Continue);
    QCOMPARE(progressProxy.format(), QString());

    updater.updateProgress(100, "test task");
    updater.updateProgress(0, "test task");

    QTest::qWait(20);

    QCOMPARE(progressProxy.min(), 0);
    QEXPECT_FAIL("", "Max should be 0 but is 100.", Continue);
    QCOMPARE(progressProxy.max(), 0);
    QEXPECT_FAIL("", "Value should be 0 but is 100.", Continue);
    QCOMPARE(progressProxy.value(), 0);
    QEXPECT_FAIL("", "format() should be empty but is '%p%'.", Continue);
    QCOMPARE(progressProxy.format(), QString());

    updater.updateProgress(0, "test task");
    updater.updateProgress(0, "test task");

    QTest::qWait(20);

    QCOMPARE(progressProxy.min(), 0);
    QEXPECT_FAIL("", "Max should be 0 but is 100.", Continue);
    QCOMPARE(progressProxy.max(), 0);
    QEXPECT_FAIL("", "Value should be 0 but is 100.", Continue);
    QCOMPARE(progressProxy.value(), 0);
    QEXPECT_FAIL("", "format() should be empty but is '%p%'.", Continue);
    QCOMPARE(progressProxy.format(), QString());

    QTest::qWait(500);

    QCOMPARE(progressProxy.min(), 0);
    QEXPECT_FAIL("", "Max should be 0 but is 100.", Continue);
    QCOMPARE(progressProxy.max(), 0);
    QEXPECT_FAIL("", "Value should be 0 but is 100.", Continue);
    QCOMPARE(progressProxy.value(), 0);
    QEXPECT_FAIL("", "format() should be empty but is '%p%'.", Continue);
    QCOMPARE(progressProxy.format(), QString());
}

SIMPLE_TEST_MAIN(KisQueuesProgressUpdaterTest)
