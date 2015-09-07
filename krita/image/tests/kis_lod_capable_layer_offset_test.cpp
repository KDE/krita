/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_lod_capable_layer_offset_test.h"

#include <qtest_kde.h>

#include "testing_timed_default_bounds.h"
#include "kis_lod_capable_layer_offset.h"

bool checkOffset(const KisLodCapableLayerOffset &o,
                  int x, int y)
{
    bool result = true;

    if (!(o.x() == x && o.y() == y)) {
        qDebug() << "Failed to compare an offset:";
        qDebug() << ppVar(o.x()) << ppVar(x);
        qDebug() << ppVar(o.y()) << ppVar(y);
        result = false;
    }

    return result;
}

void KisLodCapableLayerOffsetTest::test()
{
    TestUtil::TestingTimedDefaultBounds *bounds =
        new TestUtil::TestingTimedDefaultBounds();
    KisDefaultBoundsBaseSP _sp(bounds);


    KisLodCapableLayerOffset refOffset(bounds);
    refOffset.setX(12);
    refOffset.setY(16);

    QVERIFY(checkOffset(refOffset, 12, 16));

    bounds->testingSetLod(1);

    QVERIFY(checkOffset(refOffset, 0, 0));

    refOffset.syncLodOffset();

    QVERIFY(checkOffset(refOffset, 6, 8));

    refOffset.setX(2);
    refOffset.setY(6);

    QVERIFY(checkOffset(refOffset, 2, 6));

    bounds->testingSetLod(0);

    QVERIFY(checkOffset(refOffset, 12, 16));

    bounds->testingSetLod(1);

    refOffset.syncLodOffset();

    QVERIFY(checkOffset(refOffset, 6, 8));


    KisLodCapableLayerOffset copy(refOffset);
    bounds->testingSetLod(0);
    QVERIFY(checkOffset(copy, 12, 16));
    bounds->testingSetLod(1);
    QVERIFY(checkOffset(copy, 6, 8));


    KisLodCapableLayerOffset copy2(bounds);
    copy2 = refOffset;

    bounds->testingSetLod(0);
    QVERIFY(checkOffset(copy2, 12, 16));
    bounds->testingSetLod(1);
    QVERIFY(checkOffset(copy2, 6, 8));
}

QTEST_KDEMAIN(KisLodCapableLayerOffsetTest, GUI)
