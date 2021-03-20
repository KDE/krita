/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "kis_fast_math_benchmark.h"

#include <simpletest.h>

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

SIMPLE_TEST_MAIN(KisFastMathBenchmark)

