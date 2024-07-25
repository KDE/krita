/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TESTKUNDO2STACK_H
#define TESTKUNDO2STACK_H

#include <simpletest.h>
#include <QObject>

class TestKUndo2Stack : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testExcludeFromMerge();
    void testMergeTimeout();
    void testGroupSeparation();
    void testMaxGroupDuration();
    void testCleanIndexAfterMerge();
    void testCleanIndexBeforeMerge();
};

#endif // TESTKUNDO2STACK_H
