/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
    void checkRoundingOverRgbaF32();

    void compareAlphaDarkenOps();
    void compareAlphaDarkenOpsNoMask();
    void compareRgbF32AlphaDarkenOps();
    void compareOverOps();
    void compareOverOpsNoMask();
    void compareRgbF32OverOps();

    void testRgb8CompositeAlphaDarkenLegacy();
    void testRgb8CompositeAlphaDarkenOptimized();

    void testRgb8CompositeOverLegacy();
    void testRgb8CompositeOverOptimized();

    void testRgbF32CompositeAlphaDarkenLegacy();
    void testRgbF32CompositeAlphaDarkenOptimized();

    void testRgbF32CompositeOverLegacy();
    void testRgbF32CompositeOverOptimized();

    void testRgb8CompositeAlphaDarkenReal_Aligned();
    void testRgb8CompositeOverReal_Aligned();

    void testRgb8CompositeCopyLegacy();

    void benchmarkMemcpy();

    void benchmarkUintFloat();
    void benchmarkUintIntFloat();
    void benchmarkFloatUint();
    void benchmarkFloatIntUint();
};

#endif /* __KIS_COMPOSITION_BENCHMARK_H */
