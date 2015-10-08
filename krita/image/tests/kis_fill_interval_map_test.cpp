/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_fill_interval_map_test.h"

#include <QTest>

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

QTEST_MAIN(KisFillIntervalMapTest)
