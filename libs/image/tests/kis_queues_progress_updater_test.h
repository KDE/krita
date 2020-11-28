/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_QUEUES_PROGRESS_UPDATER_TEST_H
#define __KIS_QUEUES_PROGRESS_UPDATER_TEST_H

#include <QtTest>

class KisQueuesProgressUpdaterTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testSlowProgress();
    void testFastProgress();
};

#endif /* __KIS_QUEUES_PROGRESS_UPDATER_TEST_H */
