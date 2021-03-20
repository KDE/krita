/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_fill_interval_map_test.h"

#include <simpletest.h>

#include <floodfill/kis_fill_interval_map.h>
#include <floodfill/kis_fill_interval_map_p.h>

void KisFillIntervalMapTest::test()
{
    const int row = 1;

    KisFillInterval i1( 0, 40, row);
    KisFillInterval i2(11, 40, row);
    KisFillInterval i3(11, 19, row);
    KisFillInterval i4(11, 20, row);

    KisFillIntervalMap map;

    // 0..10, 20..30, 40..50, ...
    for (int i = 0; i < 100; i += 20) {
        map.insertInterval(KisFillInterval(i, i + 10, row));
    }

    KisFillIntervalMap::Private::IteratorRange range;

    range = map.m_d->findFirstIntersectingInterval(i1);
    QCOMPARE(range.beginIt->start, 0);
    QCOMPARE(range.beginIt->end, 10);

    range = map.m_d->findFirstIntersectingInterval(i2);
    QCOMPARE(range.beginIt->start, 20);
    QCOMPARE(range.beginIt->end, 30);

    range = map.m_d->findFirstIntersectingInterval(i3);
    QCOMPARE(range.beginIt, range.endIt);

    range = map.m_d->findFirstIntersectingInterval(i4);
    QCOMPARE(range.beginIt->start, 20);
    QCOMPARE(range.beginIt->end, 30);
}

SIMPLE_TEST_MAIN(KisFillIntervalMapTest)
