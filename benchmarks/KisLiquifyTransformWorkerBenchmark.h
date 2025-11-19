/*
 *  SPDX-FileCopyrightText: 2025 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_LIQUIFY_TRANSFORM_WORKER_BENCHMARK_H
#define KIS_LIQUIFY_TRANSFORM_WORKER_BENCHMARK_H

#include <simpletest.h>


class KisLiquifyTransformWorkerBenchmark : public QObject
{
public:
    enum Operation {
        Translate,
        Scale,
        Rotate,
        Undo,
        RunOnQImage,
        RunOnDev,
        BenchmarkCopyConstructor,
        MovePointsOutsideOfTheGrid,
    };


    Q_OBJECT
private Q_SLOTS:

    void testInitialization();

    void testPointsTranslateBuildUp();
    void testPointsTranslateWash();

    void testPointsScaleBuildUp();
    void testPointsScaleWash();


    void testPointsRotateBuildUp();
    void testPointsRotateWash();

    void testPointsUndoBuildUp();
    void testPointsUndoWash();

    void testRunOnDev();
    void testRunOnQImage();

    void testSmallChangeOnDev();
    void testSmallChangeOnQImage();

    void testRenderingMask_data();
    void testRenderingMask();


    void testCopyConstructor();
    void testMovePointsOutsideOfTheGrid();

    void checkMath();

    void testCheckingIfPolygonsAreConvex();

};

#endif
