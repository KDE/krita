/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_store_limits_test.h"
#include <QTest>

#include "kis_debug.h"

#include "kis_image_config.h"
#include "tiles3/swap/kis_tile_data_swapper_p.h"

void KisStoreLimitsTest::testLimits()
{
    KisImageConfig config(false);
    config.setMemoryHardLimitPercent(50);
    config.setMemorySoftLimitPercent(25);
    config.setMemoryPoolLimitPercent(10);

    const int totalRAM = KisImageConfig::totalRAM();

    // values are shifted because of the pooler part
    const int halfRAMMetric = MiB_TO_METRIC(int(totalRAM * 0.4));
    const int quarterRAMMetric = MiB_TO_METRIC(int(totalRAM * 0.15));

    KisStoreLimits limits;

    QCOMPARE(limits.emergencyThreshold(), halfRAMMetric);
    QCOMPARE(limits.hardLimitThreshold(), halfRAMMetric * 7 / 8);
    QCOMPARE(limits.hardLimit(), (halfRAMMetric * 7 / 8) * 7 / 8);
    QCOMPARE(limits.softLimitThreshold(), quarterRAMMetric);
    QCOMPARE(limits.softLimit(), quarterRAMMetric * 7 / 8);
}

QTEST_MAIN(KisStoreLimitsTest)

