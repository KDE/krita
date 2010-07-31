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
#include <qtest_kde.h>

#include "kis_debug.h"

#include "kis_image_config.h"
#include "tiles3/swap/kis_tile_data_swapper_p.h"

void KisStoreLimitsTest::testLimits()
{
    KisImageConfig config;
    config.setMemoryHardLimit(16);
    config.setMemorySoftLimit(8);

    KisStoreLimits limits;

    QCOMPARE(limits.emergencyThreshold(), 4096);
    QCOMPARE(limits.hardLimitThreshold(), 3584);
    QCOMPARE(limits.hardLimit(), 3136);
    QCOMPARE(limits.softLimitThreshold(), 2048);
    QCOMPARE(limits.softLimit(), 1792);
}

QTEST_KDEMAIN(KisStoreLimitsTest, NoGUI)
#include "kis_store_limits_test.moc"

