/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_COMPOSITION_BENCHMARK_H
#define __KIS_COMPOSITION_BENCHMARK_H

#include <QtTest>

class KisCompositionBenchmark : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void checkRoundingAlphaDarken_05_03();
    void checkRoundingAlphaDarken_05_05();
    void checkRoundingAlphaDarken_05_07();
    void checkRoundingAlphaDarken_05_10();
    void checkRoundingAlphaDarken_05_10_08();
    void checkRoundingAlphaDarkenF32_05_03();
    void checkRoundingAlphaDarkenF32_05_05();
    void checkRoundingAlphaDarkenF32_05_07();
    void checkRoundingAlphaDarkenF32_05_10();
    void checkRoundingAlphaDarkenF32_05_10_08();

    void checkRoundingOver();
    void checkRoundingOverRgbaU16();
    void checkRoundingOverRgbaF32();

    void checkRoundingCopyRgbaU16();
    void checkRoundingCopyRgbaF32();

    void compareAlphaDarkenOps();
    void compareAlphaDarkenOpsNoMask();
    void compareRgbU16AlphaDarkenOps();
    void compareRgbF32AlphaDarkenOps();

    void compareOverOps();
    void compareOverOpsNoMask();
    void compareRgbU16OverOps();
    void compareRgbF32OverOps();

    void compareRgbU8CopyOps();
    void compareRgbU16CopyOps();
    void compareRgbF32CopyOps();

    void testRgb8CompositeAlphaDarkenLegacy();
    void testRgb8CompositeAlphaDarkenOptimized();

    void testRgb8CompositeOverLegacy();
    void testRgb8CompositeOverOptimized();

    void testRgb16CompositeAlphaDarkenLegacy();
    void testRgb16CompositeAlphaDarkenOptimized();

    void testRgb16CompositeOverLegacy();
    void testRgb16CompositeOverOptimized();

    void testRgb16CompositeCopyLegacy();
    void testRgb16CompositeCopyOptimized();

    void testRgbF32CompositeAlphaDarkenLegacy();
    void testRgbF32CompositeAlphaDarkenOptimized();

    void testRgbF32CompositeOverLegacy();
    void testRgbF32CompositeOverOptimized();

    void testRgbF32CompositeCopyLegacy();
    void testRgbF32CompositeCopyOptimized();

    void testRgb8CompositeAlphaDarkenReal_Aligned();
    void testRgb8CompositeOverReal_Aligned();

    void testRgb8CompositeCopyLegacy();
    void testRgb8CompositeCopyOptimized();

    void benchmarkMemcpy();

    void benchmarkUintFloat();
    void benchmarkUintIntFloat();
    void benchmarkFloatUint();
    void benchmarkFloatIntUint();
};

#endif /* __KIS_COMPOSITION_BENCHMARK_H */
