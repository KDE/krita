/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_UPDATE_SCHEDULER_TEST_H
#define KIS_UPDATE_SCHEDULER_TEST_H

#include <simpletest.h>

#include "kis_types.h"

class KisUpdateSchedulerTest : public QObject
{
    Q_OBJECT

private:
    KisImageSP buildTestingImage();

private Q_SLOTS:
    void testMerge();
    void benchmarkOverlappedMerge();
    void testLocking();
    void testExclusiveStrokes();
    void testEmptyStroke();
    void testLazyWaitCondition();
    void testBlockUpdates();

    void testTimeMonitor();

    void testLodSync();
};

#endif /* KIS_UPDATE_SCHEDULER_TEST_H */

