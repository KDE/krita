
/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "TestKoProgressUpdater.h"

#include <QtTest>

#include <testutil.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>


void TestKoProgressUpdater::test()
{
    TestUtil::TestProgressBar testProxy;

    KoProgressUpdater progress(&testProxy);
    progress.setUpdateInterval(1);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 0);
    QCOMPARE(testProxy.value(), 0);
    QCOMPARE(testProxy.format(), QString(""));

    progress.start(100, "Test Action: %p%");

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 0);
    QCOMPARE(testProxy.format(), QString("Test Action: %p%"));

    QPointer<KoUpdater> updater1 = progress.startSubtask(1, "");

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 0);
    QCOMPARE(testProxy.format(), QString("Test Action: %p%"));

    updater1->setProgress(50);
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 50);
    QCOMPARE(testProxy.format(), QString("Test Action: %p%"));

    QPointer<KoUpdater> updater2 = progress.startSubtask(4, "");
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 10);
    QCOMPARE(testProxy.format(), QString("Test Action: %p%"));

    updater2->setProgress(25);
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 30);
    QCOMPARE(testProxy.format(), QString("Test Action: %p%"));

    updater1->setProgress(100);
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 40);
    QCOMPARE(testProxy.format(), QString("Test Action: %p%"));

    updater2->setProgress(100);
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 99);
    QCOMPARE(testProxy.format(), QString("Test Action: %p%"));

    // both updaters are auto-killed right after completion
    QVERIFY(!updater1);
    QVERIFY(!updater2);

}

void TestKoProgressUpdater::testNamedSubtasks()
{
    TestUtil::TestProgressBar testProxy;

    KoProgressUpdater progress(&testProxy);
    progress.setUpdateInterval(1);
    progress.setAutoNestNames(true);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 0);
    QCOMPARE(testProxy.value(), 0);
    QCOMPARE(testProxy.format(), QString(""));
    QCOMPARE(testProxy.autoNestedName(), QString(""));

    progress.start(100, "Test Action");
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 0);
    QCOMPARE(testProxy.format(), QString("Test Action: %p%"));
    QCOMPARE(testProxy.autoNestedName(), QString("Test Action"));

    QPointer<KoUpdater> updater1 = progress.startSubtask(1, "subtask1");
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 0);
    QCOMPARE(testProxy.format(), QString("Test Action: subtask1: %p%"));
    QCOMPARE(testProxy.autoNestedName(), QString("Test Action: subtask1"));

    updater1->setProgress(50);
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 50);
    QCOMPARE(testProxy.format(), QString("Test Action: subtask1: %p%"));
    QCOMPARE(testProxy.autoNestedName(), QString("Test Action: subtask1"));

    QPointer<KoUpdater> updater2 = progress.startSubtask(4, "subtask2");
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 10);
    QCOMPARE(testProxy.format(), QString("Test Action: subtask1: %p%"));
    QCOMPARE(testProxy.autoNestedName(), QString("Test Action: subtask1"));

    updater1->setProgress(100);
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 20);
    QCOMPARE(testProxy.format(), QString("Test Action: subtask2: %p%"));
    QCOMPARE(testProxy.autoNestedName(), QString("Test Action: subtask2"));

    // tests subtask with an empty name!
    QPointer<KoUpdater> updater3 = progress.startSubtask(1, "");
    updater2->setProgress(100);
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 83);
    QCOMPARE(testProxy.format(), QString("Test Action: %p%"));
    QCOMPARE(testProxy.autoNestedName(), QString("Test Action"));
}

void TestKoProgressUpdater::testNamedSubtasksUnnamedParent()
{
    TestUtil::TestProgressBar testProxy;

    KoProgressUpdater progress(&testProxy);
    progress.setUpdateInterval(1);
    progress.setAutoNestNames(true);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 0);
    QCOMPARE(testProxy.value(), 0);
    QCOMPARE(testProxy.format(), QString(""));
    QCOMPARE(testProxy.autoNestedName(), QString(""));

    progress.start(100, "");
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 0);
    QCOMPARE(testProxy.format(), QString("%p%"));
    QCOMPARE(testProxy.autoNestedName(), QString(""));

    QPointer<KoUpdater> updater1 = progress.startSubtask(1, "subtask1");
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 0);
    QCOMPARE(testProxy.format(), QString("subtask1: %p%"));
    QCOMPARE(testProxy.autoNestedName(), QString("subtask1"));
}

void TestKoProgressUpdater::testPersistentSubtask()
{
    TestUtil::TestProgressBar testProxy;

    KoProgressUpdater progress(&testProxy);
    progress.setUpdateInterval(1);
    progress.setAutoNestNames(true);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 0);
    QCOMPARE(testProxy.value(), 0);
    QCOMPARE(testProxy.format(), QString(""));
    QCOMPARE(testProxy.autoNestedName(), QString(""));

    progress.start(100, "");
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 0);
    QCOMPARE(testProxy.format(), QString("%p%"));
    QCOMPARE(testProxy.autoNestedName(), QString(""));

    QPointer<KoUpdater> updater1 = progress.startSubtask(1, "subtask1");
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 0);
    QCOMPARE(testProxy.format(), QString("subtask1: %p%"));
    QCOMPARE(testProxy.autoNestedName(), QString("subtask1"));

    QPointer<KoUpdater> updater2_persistent = progress.startSubtask(1, "subtask2", true);
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 0);
    QCOMPARE(testProxy.format(), QString("subtask1: %p%"));
    QCOMPARE(testProxy.autoNestedName(), QString("subtask1"));

    updater1->setProgress(100);
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 50);
    QCOMPARE(testProxy.format(), QString("subtask2: %p%"));
    QCOMPARE(testProxy.autoNestedName(), QString("subtask2"));

    updater2_persistent->setValue(100);
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 99);
    QCOMPARE(testProxy.format(), QString("%p%"));
    QCOMPARE(testProxy.autoNestedName(), QString(""));

    // normal subtask is killed, persistent is still alive
    QVERIFY(!updater1);
    QVERIFY(updater2_persistent);

    progress.removePersistentSubtask(updater2_persistent);
    QTest::qWait(15);

    // persistent subtask is killed only after explicit removal
    QVERIFY(!updater2_persistent);
}

void TestKoProgressUpdater::testDestructionNonpersistentSubtasks()
{
    TestUtil::TestProgressBar testProxy;

    KoProgressUpdater progress(&testProxy);
    progress.setUpdateInterval(1);
    progress.setAutoNestNames(true);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 0);
    QCOMPARE(testProxy.value(), 0);
    QCOMPARE(testProxy.format(), QString(""));
    QCOMPARE(testProxy.autoNestedName(), QString(""));

    progress.start(100, "");
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 0);
    QCOMPARE(testProxy.format(), QString("%p%"));
    QCOMPARE(testProxy.autoNestedName(), QString(""));

    QPointer<KoUpdater> updater1 = progress.startSubtask(1, "subtask1");
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 0);
    QCOMPARE(testProxy.format(), QString("subtask1: %p%"));
    QCOMPARE(testProxy.autoNestedName(), QString("subtask1"));


    /// This test checks a weird effect. We can create a progress updater
    /// on some other KoUpdater, which might be non-persistent, that is be
    /// removed right after the completion.
    ///
    /// HACK ALERT:
    /// We use a hacky approach, we just crete a custom constructor for the
    /// progress updater that connects to an updater using QPointer interface.
    /// Ideally, we should just make KoProgressProxy inherit QObject, but that
    /// would introduce virtual inheritance in KoProgressBar, which we would like
    /// to avoid.

    // create a progress updater over a self-destructing updater
    KoProgressUpdater subtaskProgress(updater1);
    subtaskProgress.setUpdateInterval(1);
    subtaskProgress.setAutoNestNames(true);

    subtaskProgress.start(100, "");
    QTest::qWait(15);

    // nothing changed!
    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 0);
    QCOMPARE(testProxy.format(), QString("subtask1: %p%"));
    QCOMPARE(testProxy.autoNestedName(), QString("subtask1"));

    QPointer<KoUpdater> subUpdater1 = subtaskProgress.startSubtask();
    QTest::qWait(15);

    // nothing changed!
    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 0);
    QCOMPARE(testProxy.format(), QString("subtask1: %p%"));
    QCOMPARE(testProxy.autoNestedName(), QString("subtask1"));

    QPointer<KoUpdater> subUpdater2 = subtaskProgress.startSubtask();
    QTest::qWait(15);

    // nothing changed!
    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 0);
    QCOMPARE(testProxy.format(), QString("subtask1: %p%"));
    QCOMPARE(testProxy.autoNestedName(), QString("subtask1"));

    subUpdater1->setProgress(50);
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 25);
    QCOMPARE(testProxy.format(), QString("subtask1: %p%"));
    QCOMPARE(testProxy.autoNestedName(), QString("subtask1"));


    subUpdater1->setProgress(100);
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 50);
    QCOMPARE(testProxy.format(), QString("subtask1: %p%"));
    QCOMPARE(testProxy.autoNestedName(), QString("subtask1"));

    subUpdater2->setProgress(100);
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 99);
    QCOMPARE(testProxy.format(), QString("%p%"));
    QCOMPARE(testProxy.autoNestedName(), QString(""));

    // main updater has been killed already since all the children
    // updaters are completed
    QVERIFY(!updater1);

    // subupdater are killed as well
    QVERIFY(!subUpdater1);
    QVERIFY(!subUpdater2);
}

void TestKoProgressUpdater::testUndefinedStateTasks()
{
    TestUtil::TestProgressBar testProxy;

    KoProgressUpdater progress(&testProxy);
    progress.setUpdateInterval(1);
    progress.setAutoNestNames(true);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 0);
    QCOMPARE(testProxy.value(), 0);
    QCOMPARE(testProxy.format(), QString(""));
    QCOMPARE(testProxy.autoNestedName(), QString(""));

    progress.start(100, "");
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 0);
    QCOMPARE(testProxy.format(), QString("%p%"));
    QCOMPARE(testProxy.autoNestedName(), QString(""));

    QPointer<KoUpdater> updater1 = progress.startSubtask(1, "subtask1");
    updater1->setProgress(50);
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 50);
    QCOMPARE(testProxy.format(), QString("subtask1: %p%"));
    QCOMPARE(testProxy.autoNestedName(), QString("subtask1"));

    QPointer<KoUpdater> updater2 = progress.startSubtask(1, "subtask2");
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 25);
    QCOMPARE(testProxy.format(), QString("subtask1: %p%"));
    QCOMPARE(testProxy.autoNestedName(), QString("subtask1"));

    updater2->setRange(0,0);
    updater2->setValue(0);
    QTest::qWait(15);

    // now we are in undefined state without showing percents!
    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 0);
    QCOMPARE(testProxy.value(), 0);
    QCOMPARE(testProxy.format(), QString("subtask1"));
    QCOMPARE(testProxy.autoNestedName(), QString("subtask1"));


    updater1->setProgress(100);
    QTest::qWait(15);

    // still undefined
    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 0);
    QCOMPARE(testProxy.value(), 0);
    QCOMPARE(testProxy.format(), QString("subtask2"));
    QCOMPARE(testProxy.autoNestedName(), QString("subtask2"));

    updater2->setRange(0,100);
    updater2->setValue(0);
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 99);
    QCOMPARE(testProxy.value(), 50);
    QCOMPARE(testProxy.format(), QString("subtask2: %p%"));
    QCOMPARE(testProxy.autoNestedName(), QString("subtask2"));
}

void TestKoProgressUpdater::testNonStandardRange()
{
    TestUtil::TestProgressBar testProxy;

    KoProgressUpdater progress(&testProxy);
    progress.setUpdateInterval(1);
    progress.setAutoNestNames(true);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 0);
    QCOMPARE(testProxy.value(), 0);
    QCOMPARE(testProxy.format(), QString(""));
    QCOMPARE(testProxy.autoNestedName(), QString(""));

    progress.start(50, "");
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 49);
    QCOMPARE(testProxy.value(), 0);
    QCOMPARE(testProxy.format(), QString("%p%"));
    QCOMPARE(testProxy.autoNestedName(), QString(""));

    QPointer<KoUpdater> updater1 = progress.startSubtask(1, "subtask1");
    updater1->setProgress(50);
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 49);
    QCOMPARE(testProxy.value(), 25);
    QCOMPARE(testProxy.format(), QString("subtask1: %p%"));
    QCOMPARE(testProxy.autoNestedName(), QString("subtask1"));

    updater1->setProgress(100);
    QTest::qWait(15);

    QCOMPARE(testProxy.min(), 0);
    QCOMPARE(testProxy.max(), 49);
    QCOMPARE(testProxy.value(), 49);
    QCOMPARE(testProxy.format(), QString("%p%"));
    QCOMPARE(testProxy.autoNestedName(), QString(""));

    QVERIFY(!updater1);
}


QTEST_MAIN(TestKoProgressUpdater)
