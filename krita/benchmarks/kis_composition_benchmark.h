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

#include <QtTest/QtTest>

class KisCompositionBenchmark : public QObject
{
    Q_OBJECT
private slots:
    void checkRoundingAlphaDarken();
    void checkRoundingOver();

    void compareAlphaDarkenOps();
    void compareAlphaDarkenOpsNoMask();
    void compareOverOps();
    void compareOverOpsNoMask();

    void testRgb8CompositeAlphaDarkenLegacy();
    void testRgb8CompositeAlphaDarkenOptimized();

    void testRgb8CompositeOverLegacy();
    void testRgb8CompositeOverOptimized();

    void testRgb8CompositeAlphaDarkenReal_Aligned();
    void testRgb8CompositeOverReal_Aligned();

    void benchmarkMemcpy();

    void benchmarkUintFloat();
    void benchmarkUintIntFloat();
    void benchmarkFloatUint();
    void benchmarkFloatIntUint();
};

#endif /* __KIS_COMPOSITION_BENCHMARK_H */
