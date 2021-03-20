/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TESTKOPROGRESSUPDATER_H
#define TESTKOPROGRESSUPDATER_H

#include <QObject>

class TestKoProgressUpdater : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void test();
    void testNamedSubtasks();
    void testNamedSubtasksUnnamedParent();
    void testPersistentSubtask();

    void testDestructionNonpersistentSubtasks();
    void testUndefinedStateTasks();

    void testNonStandardRange();
};

#endif // TESTKOPROGRESSUPDATER_H
