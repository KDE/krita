/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_fast_math_benchmark.h"

#include <QTest>

#include <kis_fast_math.h>

const int COUNT = 1000;

void KisFastMathBenchmark::benchmarkFastAtan2()
{
    QBENCHMARK{
        for (int i = 0 ; i < COUNT; ++i) {
            double x = i;
            for (int j = 0 ; j < COUNT; ++j) {

                double y = j;

                KisFastMath::atan2(y, x);
            }
        }
    }
}

void KisFastMathBenchmark::benchmarkLibCAtan2()
{
    QBENCHMARK{
        for (int i = 0 ; i < COUNT; ++i) {
            double x = i;
            for (int j = 0 ; j < COUNT; ++j) {

                double y = j;

                double result = atan2(y, x);
                Q_UNUSED(result);
            }
        }
    }
}

QTEST_MAIN(KisFastMathBenchmark)

