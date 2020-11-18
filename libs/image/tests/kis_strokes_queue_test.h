/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_STROKES_QUEUE_TEST_H
#define __KIS_STROKES_QUEUE_TEST_H

#include <QtTest>
#include "kis_types.h"
#include "kis_stroke_job_strategy.h"

class KisStrokesQueueTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testSequentialJobs();
    void testConcurrentSequentialBarrier();
    void testExclusiveStrokes();
    void testBarrierStrokeJobs();
    void testStrokesOverlapping();
    void testImmediateCancel();
    void testOpenedStrokeCounter();
    void testAsyncCancelWhileOpenedStroke();
    void testStrokesLevelOfDetail();
    void testMultipleLevelOfDetailStrokes();
    void testMultipleLevelOfDetailAfterLegacy();
    void testMultipleLevelOfDetailMixedLegacy();
    void testLegacyInitializerStrategy();
    void testLegacyStrokeWithLegacyInitializerStrategy();
    void testLodUndoBase();
    void testLodUndoBase2();
    void testMutatedJobs();
    void testUniquelyConcurrentJobs();

private:
    struct LodStrokesQueueTester;
    static void checkJobsOverlapping(LodStrokesQueueTester &t, KisStrokeId id, KisStrokeJobData::Sequentiality first, KisStrokeJobData::Sequentiality second, bool allowed);
};

#endif /* __KIS_STROKES_QUEUE_TEST_H */
