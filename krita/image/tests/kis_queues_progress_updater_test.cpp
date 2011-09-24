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

#include <qtest_kde.h>

#include "kis_queues_progress_updater.h"
#include "testutil.h"


void KisQueuesProgressUpdaterTest::testSlowProgress()
{
    TestUtil::TestProgressBar progressProxy;
    KisQueuesProgressUpdater updater(&progressProxy);

    updater.notifyJobDone(100);
    updater.updateProgress(200, "test task");

    QTest::qWait(100);

    QCOMPARE(progressProxy.min(), 0);
    QCOMPARE(progressProxy.max(), 0);
    QCOMPARE(progressProxy.value(), 0);
    QCOMPARE(progressProxy.format(), QString());

    QTest::qWait(500);

    QCOMPARE(progressProxy.min(), 0);
    QCOMPARE(progressProxy.max(), 300);
    QCOMPARE(progressProxy.value(), 100);
    QCOMPARE(progressProxy.format(), QString("test task"));

    updater.notifyJobDone(200);
    updater.updateProgress(0, "test task");

    QTest::qWait(500);

    QCOMPARE(progressProxy.min(), 0);
    QCOMPARE(progressProxy.max(), 300);
    QCOMPARE(progressProxy.value(), 300);
    QCOMPARE(progressProxy.format(), QString("test task"));
}

void KisQueuesProgressUpdaterTest::testFastProgress()
{
    /**
     * If the progress is too fast we don't even touch the bar
     */

    TestUtil::TestProgressBar progressProxy;
    KisQueuesProgressUpdater updater(&progressProxy);

    updater.notifyJobDone(100);
    updater.updateProgress(200, "test task");

    QTest::qWait(20);

    QCOMPARE(progressProxy.min(), 0);
    QCOMPARE(progressProxy.max(), 0);
    QCOMPARE(progressProxy.value(), 0);
    QCOMPARE(progressProxy.format(), QString());

    updater.notifyJobDone(100);
    updater.updateProgress(100, "test task");

    QTest::qWait(20);

    QCOMPARE(progressProxy.min(), 0);
    QCOMPARE(progressProxy.max(), 0);
    QCOMPARE(progressProxy.value(), 0);
    QCOMPARE(progressProxy.format(), QString());

    updater.notifyJobDone(100);
    updater.updateProgress(0, "test task");

    QTest::qWait(20);

    QCOMPARE(progressProxy.min(), 0);
    QCOMPARE(progressProxy.max(), 0);
    QCOMPARE(progressProxy.value(), 0);
    QCOMPARE(progressProxy.format(), QString());

    QTest::qWait(500);

    QCOMPARE(progressProxy.min(), 0);
    QCOMPARE(progressProxy.max(), 0);
    QCOMPARE(progressProxy.value(), 0);
    QCOMPARE(progressProxy.format(), QString());
}

QTEST_KDEMAIN(KisQueuesProgressUpdaterTest, GUI)
