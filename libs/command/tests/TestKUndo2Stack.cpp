/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "TestKUndo2Stack.h"

#include <QTime>

#include <kundo2stack.h>
#include <kundo2command.h>


void TestKUndo2Stack::testExcludeFromMerge()
{
    KUndo2Stack stack;

    stack.setUseCumulativeUndoRedo(true);

    KisCumulativeUndoData data;
    data.excludeFromMerge = 5;
    data.mergeTimeout = 1000;
    data.maxGroupSeparation = 1000;
    data.maxGroupDuration = 100000;

    stack.setCumulativeUndoData(data);

    int startTime = 0;

    for (int i = 0; i < 6; i++) {
        KUndo2Command *cmd = new KUndo2Command(kundo2_noi18n(QString::number(startTime)));

        cmd->setTimedID(1);
        cmd->setTime(QTime(0, 0, startTime, 0));
        cmd->setEndTime(QTime(0, 0, startTime, 500));

        stack.push(cmd);
        startTime++;
    }

    QCOMPARE(stack.count(), 6);
    QCOMPARE(stack.index(), stack.count());

//    qDebug() << "===";
//    for (int j = 0; j < stack.count(); j++) {
//        qDebug() << j << stack.command(j)->text();
//    }
//    qDebug() << "===";

    for (int i = 0; i < 50; i++) {
        KUndo2Command *cmd = new KUndo2Command(kundo2_noi18n(QString::number(startTime)));

        cmd->setTimedID(1);
        cmd->setTime(QTime(0, 0, startTime, 0));
        cmd->setEndTime(QTime(0, 0, startTime, 500));

        stack.push(cmd);
        startTime++;

        QCOMPARE(stack.count(), 6);
        QCOMPARE(stack.index(), stack.count());
    }

    // verify that all the merged commands are merged into
    // command "50";
    for (int i = 0; i < stack.count(); i++) {
        QCOMPARE(stack.command(i)->text().toString(), QString::number(i + 50));
    }
}

void TestKUndo2Stack::testMergeTimeout()
{
    KUndo2Stack stack;

    stack.setUseCumulativeUndoRedo(true);

    KisCumulativeUndoData data;
    data.excludeFromMerge = 1;
    data.mergeTimeout = 5000;
    data.maxGroupSeparation = 1000;
    data.maxGroupDuration = 50000;

    stack.setCumulativeUndoData(data);

    int startTime = 0;

    // group 1
    for (int i = 0; i < 7; i++) {
        KUndo2Command *cmd = new KUndo2Command(kundo2_noi18n(QString("group 1: %1").arg(startTime)));

        cmd->setTimedID(1);
        cmd->setTime(QTime(0, 0, startTime, 0));
        cmd->setEndTime(QTime(0, 0, startTime, 500));

        stack.push(cmd);
        startTime++;
    }

    QCOMPARE(stack.count(), 7);
    QCOMPARE(stack.index(), stack.count());

    {
        KUndo2Command *cmd = new KUndo2Command(kundo2_noi18n(QString("group 1: %1").arg(startTime)));

        cmd->setTimedID(1);
        cmd->setTime(QTime(0, 0, startTime, 0));
        cmd->setEndTime(QTime(0, 0, startTime, 500));

        stack.push(cmd);
        startTime++;
    }

    QCOMPARE(stack.count(), 7);
    QCOMPARE(stack.index(), stack.count());

    QCOMPARE(stack.command(0)->isMerged(), true);
    QCOMPARE(stack.command(1)->isMerged(), false);
    QCOMPARE(stack.command(2)->isMerged(), false);
    QCOMPARE(stack.command(3)->isMerged(), false);
    QCOMPARE(stack.command(4)->isMerged(), false);
    QCOMPARE(stack.command(5)->isMerged(), false);
    QCOMPARE(stack.command(6)->isMerged(), false);
}

void TestKUndo2Stack::testGroupSeparation()
{
    KUndo2Stack stack;

    stack.setUseCumulativeUndoRedo(true);

    KisCumulativeUndoData data;
    data.excludeFromMerge = 1;
    data.mergeTimeout = 6000;
    data.maxGroupSeparation = 1000;
    data.maxGroupDuration = 50000;

    stack.setCumulativeUndoData(data);

    int startTime = 0;

    for (int i = 0; i < 3; i++) {
        KUndo2Command *cmd = new KUndo2Command(kundo2_noi18n(QString::number(startTime)));

        cmd->setTimedID(1);
        cmd->setTime(QTime(0, 0, startTime, 0));
        cmd->setEndTime(QTime(0, 0, startTime, 500));

        stack.push(cmd);
        startTime++;
    }

    QCOMPARE(stack.count(), 3);
    QCOMPARE(stack.index(), stack.count());

    // add one second to split groups
    startTime += 1;

    for (int i = 0; i < 2; i++) {
        KUndo2Command *cmd = new KUndo2Command(kundo2_noi18n(QString::number(startTime)));

        cmd->setTimedID(1);
        cmd->setTime(QTime(0, 0, startTime, 0));
        cmd->setEndTime(QTime(0, 0, startTime, 500));

        stack.push(cmd);
        startTime++;
    }

    QCOMPARE(stack.count(), 5);
    QCOMPARE(stack.index(), stack.count());

    // add five seconds to make all the previous commands outdated
    startTime += 6;

    {
        KUndo2Command *cmd = new KUndo2Command(kundo2_noi18n(QString::number(startTime)));

        cmd->setTimedID(1);
        cmd->setTime(QTime(0, 0, startTime, 0));
        cmd->setEndTime(QTime(0, 0, startTime, 500));

        stack.push(cmd);
        startTime++;
    }

    // all the first commands were collapsed
    QCOMPARE(stack.count(), 3);
    QCOMPARE(stack.index(), stack.count());

    // two separate groups
    QCOMPARE(stack.command(0)->isMerged(), true);
    QCOMPARE(stack.command(1)->isMerged(), true);
    QCOMPARE(stack.command(2)->isMerged(), false);
}

void TestKUndo2Stack::testMaxGroupDuration()
{
    KUndo2Stack stack;

    stack.setUseCumulativeUndoRedo(true);

    KisCumulativeUndoData data;
    data.excludeFromMerge = 1;
    data.mergeTimeout = 1000;
    data.maxGroupSeparation = 1000;
    data.maxGroupDuration = 3000;

    stack.setCumulativeUndoData(data);

    int startTime = 0;

    for (int i = 0; i < 3; i++) {
        KUndo2Command *cmd = new KUndo2Command(kundo2_noi18n(QString::number(startTime)));

        cmd->setTimedID(1);
        cmd->setTime(QTime(0, 0, startTime, 0));
        cmd->setEndTime(QTime(0, 0, startTime, 500));

        stack.push(cmd);
        startTime++;
    }

    QCOMPARE(stack.count(), 3);
    QCOMPARE(stack.index(), stack.count());

    QCOMPARE(stack.command(0)->isMerged(), false);
    QCOMPARE(stack.command(1)->isMerged(), false);
    QCOMPARE(stack.command(2)->isMerged(), false);

    for (int i = 0; i < 3; i++) {
        KUndo2Command *cmd = new KUndo2Command(kundo2_noi18n(QString::number(startTime)));

        cmd->setTimedID(1);
        cmd->setTime(QTime(0, 0, startTime, 0));
        cmd->setEndTime(QTime(0, 0, startTime, 500));

        stack.push(cmd);
        startTime++;
    }

    QCOMPARE(stack.count(), 4);
    QCOMPARE(stack.index(), stack.count());

    QCOMPARE(stack.command(0)->isMerged(), true);
    QCOMPARE(stack.command(1)->isMerged(), false);
    QCOMPARE(stack.command(2)->isMerged(), false);
    QCOMPARE(stack.command(3)->isMerged(), false);

    startTime += 5;

    {
        KUndo2Command *cmd = new KUndo2Command(kundo2_noi18n(QString::number(startTime)));

        cmd->setTimedID(1);
        cmd->setTime(QTime(0, 0, startTime, 0));
        cmd->setEndTime(QTime(0, 0, startTime, 500));

        stack.push(cmd);
        startTime++;
    }

    QCOMPARE(stack.count(), 3);
    QCOMPARE(stack.index(), stack.count());

    // two separate groups, even though they have the same separation
    QCOMPARE(stack.command(0)->isMerged(), true);
    QCOMPARE(stack.command(1)->isMerged(), true);
    QCOMPARE(stack.command(2)->isMerged(), false);
}

void TestKUndo2Stack::testCleanIndexAfterMerge()
{
    KUndo2Stack stack;

    stack.setUseCumulativeUndoRedo(true);

    KisCumulativeUndoData data;
    data.excludeFromMerge = 3;
    data.mergeTimeout = 5000;
    data.maxGroupSeparation = 1000;
    data.maxGroupDuration = 3000;

    stack.setCumulativeUndoData(data);

    int startTime = 0;

    for (int i = 0; i < 3; i++) {
        KUndo2Command *cmd = new KUndo2Command(kundo2_noi18n(QString::number(startTime)));

        cmd->setTimedID(1);
        cmd->setTime(QTime(0, 0, startTime, 0));
        cmd->setEndTime(QTime(0, 0, startTime, 500));

        stack.push(cmd);
        startTime++;
    }

    QCOMPARE(stack.count(), 3);
    QCOMPARE(stack.index(), stack.count());

    QCOMPARE(stack.command(0)->isMerged(), false);
    QCOMPARE(stack.command(1)->isMerged(), false);
    QCOMPARE(stack.command(2)->isMerged(), false);

    for (int i = 0; i < 2; i++) {
        KUndo2Command *cmd = new KUndo2Command(kundo2_noi18n(QString::number(startTime)));

        cmd->setTimedID(1);
        cmd->setTime(QTime(0, 0, startTime, 0));
        cmd->setEndTime(QTime(0, 0, startTime, 500));

        stack.push(cmd);

        if (i == 0) {
            stack.setClean();
        }

        startTime++;
    }

    QCOMPARE(stack.count(), 5);
    QCOMPARE(stack.index(), stack.count());
    QCOMPARE(stack.cleanIndex() - 1, 3);

    QCOMPARE(stack.command(0)->isMerged(), false);
    QCOMPARE(stack.command(1)->isMerged(), false);
    QCOMPARE(stack.command(2)->isMerged(), false);
    QCOMPARE(stack.command(3)->isMerged(), false);
    QCOMPARE(stack.command(4)->isMerged(), false);

    startTime += 10;

    {
        KUndo2Command *cmd = new KUndo2Command(kundo2_noi18n(QString::number(startTime)));

        cmd->setTimedID(1);
        cmd->setTime(QTime(0, 0, startTime, 0));
        cmd->setEndTime(QTime(0, 0, startTime, 500));

        stack.push(cmd);
        startTime++;
    }

    QCOMPARE(stack.count(), 4);
    QCOMPARE(stack.index(), stack.count());

    // the clean index got adjusted!
    QCOMPARE(stack.cleanIndex() - 1, 1);

    QCOMPARE(stack.command(0)->isMerged(), true);
    QCOMPARE(stack.command(1)->isMerged(), false);
    QCOMPARE(stack.command(2)->isMerged(), false);
    QCOMPARE(stack.command(3)->isMerged(), false);
}

void TestKUndo2Stack::testCleanIndexBeforeMerge()
{
    KUndo2Stack stack;

    stack.setUseCumulativeUndoRedo(true);

    KisCumulativeUndoData data;
    data.excludeFromMerge = 1;
    data.mergeTimeout = 5000;
    data.maxGroupSeparation = 1000;
    data.maxGroupDuration = 3000;

    stack.setCumulativeUndoData(data);

    int startTime = 0;

    {
        KUndo2Command *cmd = new KUndo2Command(kundo2_noi18n(QString::number(startTime)));

        cmd->setTimedID(1);
        cmd->setTime(QTime(0, 0, startTime, 0));
        cmd->setEndTime(QTime(0, 0, startTime, 500));

        stack.push(cmd);
        startTime++;
    }

    // set the first command as clean
    stack.setClean();

    startTime += 3;

    for (int i = 0; i < 3; i++) {
        KUndo2Command *cmd = new KUndo2Command(kundo2_noi18n(QString::number(startTime)));

        cmd->setTimedID(1);
        cmd->setTime(QTime(0, 0, startTime, 0));
        cmd->setEndTime(QTime(0, 0, startTime, 500));

        stack.push(cmd);
        startTime++;
    }

    QCOMPARE(stack.count(), 4);
    QCOMPARE(stack.index(), stack.count());
    QCOMPARE(stack.cleanIndex() - 1, 0);

    QCOMPARE(stack.command(0)->isMerged(), false);
    QCOMPARE(stack.command(1)->isMerged(), false);
    QCOMPARE(stack.command(2)->isMerged(), false);
    QCOMPARE(stack.command(3)->isMerged(), false);

    startTime += 10;

    {
        KUndo2Command *cmd = new KUndo2Command(kundo2_noi18n(QString::number(startTime)));

        cmd->setTimedID(1);
        cmd->setTime(QTime(0, 0, startTime, 0));
        cmd->setEndTime(QTime(0, 0, startTime, 500));

        stack.push(cmd);
        startTime++;
    }

    QCOMPARE(stack.count(), 3);
    QCOMPARE(stack.index(), stack.count());

    // the index is unchanged!
    QCOMPARE(stack.cleanIndex() - 1, 0);

    QCOMPARE(stack.command(0)->isMerged(), false);
    QCOMPARE(stack.command(1)->isMerged(), true);
    QCOMPARE(stack.command(2)->isMerged(), false);
}

SIMPLE_TEST_MAIN(TestKUndo2Stack)
