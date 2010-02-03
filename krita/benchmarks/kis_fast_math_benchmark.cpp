/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
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

#include "kis_fast_math_benchmark.h"

#include <qtest_kde.h>

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

                atan2(y, x);
            }
        }
    }
}

QTEST_KDEMAIN(KisFastMathBenchmark, GUI)

#include "kis_fast_math_benchmark.moc"
