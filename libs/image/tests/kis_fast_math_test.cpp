/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_fast_math_test.h"

#include <simpletest.h>
#include "kis_fast_math.h"

void KisFastMathTest::testAtan2()
{
    const int COUNT = 1000;

    for (int i = 0 ; i < COUNT; ++i) {
        double x = i;
        for (int j = 0 ; j < COUNT; ++j) {

            double y = j;

            double v1 = atan2(y, x);
            double v2 = KisFastMath::atan2(y, x);

            QVERIFY(fabs(v1 - v2) < 0.0001);
        }
    }
}


SIMPLE_TEST_MAIN(KisFastMathTest)
