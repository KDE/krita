/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_LAZY_BRUSH_TEST_H
#define __KIS_LAZY_BRUSH_TEST_H

#include <QtTest>

class KisLazyBrushTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    //void test();
    void testGraph();
    void testGraphMultilabel();
    void testGraphStandardIterators();
    void testGraphConcepts();
    void testCutOnGraph();
    void testCutOnGraphDevice();
    void testCutOnGraphDeviceMulti();
    void testLoG();

    void testSplitIntoConnectedComponents();

    void testEstimateTransparentPixels();

    void multiwayCutBenchmark();
};

#endif /* __KIS_LAZY_BRUSH_TEST_H */
