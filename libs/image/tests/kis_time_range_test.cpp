/*
 *  Copyright (c) 2018 Jouni Pentikäinen <joupent@gmail.com>
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
#include "kis_time_range_test.h"

#include <QTest>
#include "kis_time_range.h"

void KisTimeRangeTest::testTimeSpans()
{
    KisTimeSpan emptySpan;
    QCOMPARE(emptySpan.isEmpty(), true);
    QCOMPARE(emptySpan.duration(), 0);
    QCOMPARE(emptySpan.contains(0), false);

    KisTimeSpan span1(7, 17);
    KisTimeSpan span2(10, 20);
    KisTimeSpan span3(20, 30);

    QCOMPARE(span1.isEmpty(), false);
    QCOMPARE(span1.duration(), 11);
    QCOMPARE(span1.contains(6), false);
    QCOMPARE(span1.contains(7), true);
    QCOMPARE(span1.contains(17), true);
    QCOMPARE(span1.contains(18), false);

    QCOMPARE(emptySpan | span1, span1);
    QCOMPARE(span1 | emptySpan, span1);
    QCOMPARE(emptySpan & span1, KisTimeSpan());
    QCOMPARE(span1 & emptySpan, KisTimeSpan());

    QCOMPARE(span1 | span2, KisTimeSpan(7, 20));
    QCOMPARE(span2 | span1, KisTimeSpan(7, 20));
    QCOMPARE(span1 & span2, KisTimeSpan(10, 17));
    QCOMPARE(span2 & span1, KisTimeSpan(10, 17));

    QCOMPARE(span1 | span3, KisTimeSpan(7, 30));
    QCOMPARE(span3 | span1, KisTimeSpan(7, 30));
    QCOMPARE(span1 & span3, KisTimeSpan());
    QCOMPARE(span3 & span1, KisTimeSpan());
}

void KisTimeRangeTest::testFrameSets()
{
    KisFrameSet emptySet;
    QCOMPARE(emptySet.contains(0), false);
    QCOMPARE(emptySet.isEmpty(), true);
    QCOMPARE(emptySet.isInfinite(), false);
    QCOMPARE(emptySet, KisFrameSet());

    KisFrameSet set1 = KisFrameSet::between(7, 17);

    QCOMPARE(set1.isEmpty(), false);
    QCOMPARE(set1.isInfinite(), false);
    QCOMPARE(set1.start(), 7);
    QCOMPARE(set1.contains(6), false);
    QCOMPARE(set1.contains(7), true);
    QCOMPARE(set1.contains(17), true);
    QCOMPARE(set1.contains(18), false);

    KisFrameSet set2 = KisFrameSet::between(10, 20);

    QCOMPARE(set1 == set2, false);

    KisFrameSet intersection12 = set2 & set1;
    QCOMPARE(intersection12.isEmpty(), false);
    QCOMPARE(intersection12.isInfinite(), false);
    QCOMPARE(intersection12.start(), 10);
    QCOMPARE(intersection12.contains(9), false);
    QCOMPARE(intersection12.contains(10), true);
    QCOMPARE(intersection12.contains(17), true);
    QCOMPARE(intersection12.contains(18), false);

    KisFrameSet set3 = KisFrameSet::between(20, 30);

    QCOMPARE(set1 & set3, emptySet);

    KisFrameSet union13 = set3 | set1;
    QCOMPARE(union13.isEmpty(), false);
    QCOMPARE(union13.isInfinite(), false);
    QCOMPARE(union13.start(), 7);
    QCOMPARE(union13.contains(6), false);
    QCOMPARE(union13.contains(7), true);
    QCOMPARE(union13.contains(17), true);
    QCOMPARE(union13.contains(18), false);
    QCOMPARE(union13.contains(19), false);
    QCOMPARE(union13.contains(20), true);
    QCOMPARE(union13.contains(30), true);
    QCOMPARE(union13.contains(31), false);

    QCOMPARE(set1 - set3, set1);
    QCOMPARE(set3 - set1, set3);

    KisFrameSet set4 = KisFrameSet({KisTimeSpan(9,10), KisTimeSpan(13, 14)}, 16);
    KisFrameSet expected14 = KisFrameSet({KisTimeSpan(7,8), KisTimeSpan(11,12), KisTimeSpan(15,15)});
    KisFrameSet difference14 = set1 - set4;

    QCOMPARE(difference14.isEmpty(), false);
    QCOMPARE(difference14.isInfinite(), false);
    QCOMPARE(difference14, expected14);
    QCOMPARE(set4 - set1, KisFrameSet::infiniteFrom(18));

    KisFrameSet set5 = KisFrameSet::infiniteFrom(9);

    QCOMPARE(set5 & set1, KisFrameSet::between(9, 17));
    QCOMPARE(set5 | set1, KisFrameSet::infiniteFrom(7));
    QCOMPARE(set5 & set2, set2);
    QCOMPARE(set5 | set2, set5);
}

QTEST_MAIN(KisTimeRangeTest)
