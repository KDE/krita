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

#include "kis_queues_progress_updater_test.h"

#include <QTest>

#include "kis_queues_progress_updater.h"
#include "testutil.h"


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

QTEST_MAIN(KisQueuesProgressUpdaterTest)
