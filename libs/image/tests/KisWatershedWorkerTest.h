/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISWATERSHEDWORKERTEST_H
#define KISWATERSHEDWORKERTEST_H

#include <QtTest>

class KisWatershedWorkerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testWorker();

    void testWorkerSmall();
    void testWorkerSmallWithAllies();
};

#endif // KISWATERSHEDWORKERTEST_H
