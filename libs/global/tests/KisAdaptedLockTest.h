/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISADAPTEDLOCKTEST_H
#define KISADAPTEDLOCKTEST_H

#include <QObject>

class KisAdaptedLockTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testConstructorDefault();
    void testConstructorTryLock();
    void testConstructorDeferLock();
    void testConstructorAdoptLock();
    void testMoveConstructor();
    void testInPlaceMoveConstructor();
    void testMoveAssignmentOperator();
};

#endif // KISADAPTEDLOCKTEST_H
