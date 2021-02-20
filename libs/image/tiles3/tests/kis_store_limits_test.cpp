/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

    int emergencyThreshold = MiB_TO_METRIC(config.tilesHardLimit());

    int hardLimitThreshold = emergencyThreshold - (emergencyThreshold / 8);
    int hardLimit = hardLimitThreshold - (hardLimitThreshold / 8);

    int softLimitThreshold = qBound(0, MiB_TO_METRIC(config.tilesSoftLimit()), hardLimitThreshold);
    int softLimit = softLimitThreshold - softLimitThreshold / 8;

    KisStoreLimits limits;

    QCOMPARE(limits.emergencyThreshold(), emergencyThreshold);
    QCOMPARE(limits.hardLimitThreshold(), hardLimitThreshold);
    QCOMPARE(limits.hardLimit(), hardLimit);
    QCOMPARE(limits.softLimitThreshold(), softLimitThreshold);
    QCOMPARE(limits.softLimit(), softLimit);
}

QTEST_MAIN(KisStoreLimitsTest)

