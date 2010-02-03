/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#include "kis_fast_math_test.h"

#include <qtest_kde.h>
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


QTEST_KDEMAIN(KisFastMathTest, GUI)
#include "kis_fast_math_test.moc"
