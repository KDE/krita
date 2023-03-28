/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "TestKUndo2Stack.h"

#include <QTime>

#include <kundo2stack.h>
#include <kundo2command.h>


void TestKUndo2Stack::testMergeGroupByDepth()
{
    KUndo2Stack stack;

    stack.setUseCumulativeUndoRedo(true);
    stack.setCumulativeUndoData({5, 50000, 1000, 5000});

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

void TestKUndo2Stack::testMergeMultipleGroupsByDepth()
{
    KUndo2Stack stack;

    stack.setUseCumulativeUndoRedo(true);
    stack.setCumulativeUndoData({10, 50000, 1000, 5000});

    int startTime = 0;

    // group 1
    for (int i = 0; i < 5; i++) {
        KUndo2Command *cmd = new KUndo2Command(kundo2_noi18n(QString("group 1: %1").arg(startTime)));

        cmd->setTimedID(1);
        cmd->setTime(QTime(0, 0, startTime, 0));
        cmd->setEndTime(QTime(0, 0, startTime, 500));

        stack.push(cmd);
        startTime++;
    }

    QCOMPARE(stack.count(), 5);
    QCOMPARE(stack.index(), stack.count());

    // group 2
    for (int i = 0; i < 6; i++) {
        KUndo2Command *cmd = new KUndo2Command(kundo2_noi18n(QString("group 2: %1").arg(startTime)));

        cmd->setTimedID(2);
        cmd->setTime(QTime(0, 0, startTime, 0));
        cmd->setEndTime(QTime(0, 0, startTime, 500));

        stack.push(cmd);
        startTime++;
    }

    QCOMPARE(stack.count(), 11);
    QCOMPARE(stack.index(), stack.count());

    // group3: push out group1
    for (int i = 0; i < 4; i++) {
        KUndo2Command *cmd = new KUndo2Command(kundo2_noi18n(QString("group 3: %1").arg(startTime)));

        cmd->setTimedID(1);
        cmd->setTime(QTime(0, 0, startTime, 0));
        cmd->setEndTime(QTime(0, 0, startTime, 500));

        stack.push(cmd);
        startTime++;
        QCOMPARE(stack.count(), 11);
        QCOMPARE(stack.index(), stack.count());
    }

    // group3: push out group2
    for (int i = 0; i < 6; i++) {
        KUndo2Command *cmd = new KUndo2Command(kundo2_noi18n(QString("group 3: %1").arg(startTime)));

        cmd->setTimedID(1);
        cmd->setTime(QTime(0, 0, startTime, 0));
        cmd->setEndTime(QTime(0, 0, startTime, 500));

        stack.push(cmd);
        startTime++;
        QCOMPARE(stack.count(), 12);
        QCOMPARE(stack.index(), stack.count());
    }

    // group3: just add more commands to group3
    for (int i = 0; i < 5; i++) {
        KUndo2Command *cmd = new KUndo2Command(kundo2_noi18n(QString("group 3: %1").arg(startTime)));

        cmd->setTimedID(1);
        cmd->setTime(QTime(0, 0, startTime, 0));
        cmd->setEndTime(QTime(0, 0, startTime, 500));

        stack.push(cmd);
        startTime++;

        /// merged group1 + merged group2 + merged group3 +
        ///     10 separate commands -> 13 commands
        QCOMPARE(stack.count(), 13);
        QCOMPARE(stack.index(), stack.count());
    }

    // the last command of the first group
    QCOMPARE(stack.command(0)->text().toString(), "group 1: 4");
    QCOMPARE(stack.command(0)->isMerged(), true);

    // the last command of the second group
    QCOMPARE(stack.command(1)->text().toString(), "group 2: 10");
    QCOMPARE(stack.command(1)->isMerged(), true);

    // the merged part of the third group
    QCOMPARE(stack.command(2)->text().toString(), "group 3: 15");
    QCOMPARE(stack.command(2)->isMerged(), true);

    // first unmerged command of the third group
    QCOMPARE(stack.command(3)->text().toString(), "group 3: 16");
    QCOMPARE(stack.command(3)->isMerged(), false);
}

void TestKUndo2Stack::testMergeGroupByTimeout()
{
    KUndo2Stack stack;

    stack.setUseCumulativeUndoRedo(true);
    stack.setCumulativeUndoData({50, 4000, 1000, 5000});

    int startTime = 0;

    for (int i = 0; i < 3; i++) {
        KUndo2Command *cmd = new KUndo2Command(kundo2_noi18n(QString::number(startTime)));

        cmd->setTimedID(1);
        cmd->setTime(QTime(0, 0, startTime, 0));
        cmd->setEndTime(QTime(0, 0, startTime, 500));

        stack.push(cmd);
        startTime++;
    }

    // add two seconds!
    startTime += 2;

    QCOMPARE(stack.count(), 3);
    QCOMPARE(stack.index(), stack.count());

    /// fourth command is added without pushing out anything,
    /// since it touches the very first command
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

    /// fifth command pushes out the first one!
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

    QCOMPARE(stack.command(0)->isMerged(), true);
    // the time range is also merged
    QCOMPARE(stack.command(0)->time().msecsTo(stack.command(0)->endTime()), 1500);
}

void TestKUndo2Stack::testManageCleanIndex()
{
    KUndo2Stack stack;

    stack.setUseCumulativeUndoRedo(true);
    stack.setCumulativeUndoData({5, 50000, 1000, 5000});

    int startTime = 0;

    for (int i = 0; i < 6; i++) {
        KUndo2Command *cmd = new KUndo2Command(kundo2_noi18n(QString::number(startTime)));

        cmd->setTimedID(1);
        cmd->setTime(QTime(0, 0, startTime, 0));
        cmd->setEndTime(QTime(0, 0, startTime, 500));

        stack.push(cmd);
        startTime++;

        if (i == 2) {
            stack.setClean();
        }
    }

    QCOMPARE(stack.count(), 6);
    QCOMPARE(stack.index(), stack.count());
    QCOMPARE(stack.cleanIndex(), 3);

    /// the seventh command pushes out the first one!
    {
        KUndo2Command *cmd = new KUndo2Command(kundo2_noi18n(QString::number(startTime)));

        cmd->setTimedID(1);
        cmd->setTime(QTime(0, 0, startTime, 0));
        cmd->setEndTime(QTime(0, 0, startTime, 500));

        stack.push(cmd);
        startTime++;
    }

    QCOMPARE(stack.count(), 6);
    QCOMPARE(stack.index(), stack.count());
    QCOMPARE(stack.cleanIndex(), 2);

    {
        KUndo2Command *cmd = new KUndo2Command(kundo2_noi18n(QString::number(startTime)));

        cmd->setTimedID(1);
        cmd->setTime(QTime(0, 0, startTime, 0));
        cmd->setEndTime(QTime(0, 0, startTime, 500));

        stack.push(cmd);
        startTime++;
    }

    QCOMPARE(stack.count(), 6);
    QCOMPARE(stack.index(), stack.count());
    QCOMPARE(stack.cleanIndex(), 1);

    {
        KUndo2Command *cmd = new KUndo2Command(kundo2_noi18n(QString::number(startTime)));

        cmd->setTimedID(1);
        cmd->setTime(QTime(0, 0, startTime, 0));
        cmd->setEndTime(QTime(0, 0, startTime, 500));

        stack.push(cmd);
        startTime++;
    }

    QCOMPARE(stack.count(), 6);
    QCOMPARE(stack.index(), stack.count());
    QCOMPARE(stack.cleanIndex(), -1);
}

SIMPLE_TEST_MAIN(TestKUndo2Stack)
