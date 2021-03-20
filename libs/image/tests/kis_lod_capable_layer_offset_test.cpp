/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_lod_capable_layer_offset_test.h"

#include <simpletest.h>

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

SIMPLE_TEST_MAIN(KisLodCapableLayerOffsetTest)
