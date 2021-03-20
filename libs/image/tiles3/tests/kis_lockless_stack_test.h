/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_LOCKLESS_STACK_TEST_H
#define KIS_LOCKLESS_STACK_TEST_H

#include <simpletest.h>

class KisAbstractIntStack;

class KisLocklessStackTest : public QObject
{
    Q_OBJECT

private:
    void runStressTest(KisAbstractIntStack &stack);

private Q_SLOTS:
    void testOperations();
    void stressTestLockless();
    void stressTestQStack();

    void stressTestClear();


    void stressTestBulkPop();
};

#endif /* KIS_LOCKLESS_STACK_TEST_H */

