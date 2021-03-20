/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "kis_time_span_test.h"
#include "kis_time_span.h"

void KisTimeSpanTest::testCreationMethods()
{
    {   // Test fromTimeToTime..
        const int start = 20;
        const int end = 100;
        KisTimeSpan span = KisTimeSpan::fromTimeToTime(start, end);
        QVERIFY(span.start() == start);
        QVERIFY(span.end() == end);
    }

    {   // Test fromTimeWithDuration..
        const int start = 20;
        const int end = 150;
        const int length = (end - start) + 1; // +1 since end is inclusive..
        KisTimeSpan span = KisTimeSpan::fromTimeWithDuration(start, length);
        QVERIFY(span.start() == start);
        QVERIFY(span.end() == end);
    }

    {   //Test infinite..

        const int start = 20;
        KisTimeSpan span = KisTimeSpan::infinite(start);
        QVERIFY(span.isInfinite());
        QVERIFY(span.start() == start);
    }

    {   // Test scenarios where time spans should be invalid.

        // Test invalid specifying from a time to a time less than the start time.
        QVERIFY(KisTimeSpan::fromTimeToTime(10, 5).isValid() == false);

        // Test invalid specifying a time with a negative length.
        QVERIFY(KisTimeSpan::fromTimeWithDuration(10, -5).isValid() == false);
    }


}

void KisTimeSpanTest::testTimeSpanOperators()
{
    const KisTimeSpan spanA = KisTimeSpan::fromTimeToTime(0, 25);
    const KisTimeSpan spanB = KisTimeSpan::fromTimeToTime(15, 30);

    {   // Union test..
        KisTimeSpan spanABU = spanA | spanB;
        QCOMPARE(spanABU.start(), spanA.start());
        QCOMPARE(spanABU.end(), spanB.end());

        const KisTimeSpan spanC = KisTimeSpan::fromTimeToTime(20, 60);
        spanABU |= spanC;

        QCOMPARE(spanABU.start(), spanA.start());
        QCOMPARE(spanABU.end(), spanC.end());
    }

    {   // Intersection test..
        KisTimeSpan spanABI = spanA & spanB; // Span should be 15 => 25

        QCOMPARE(spanABI.start(), spanB.start());
        QCOMPARE(spanABI.end(), spanA.end());

        const KisTimeSpan spanC = KisTimeSpan::fromTimeToTime(0, 20);
        spanABI &= spanC; // Span should be 15 => 20

        QCOMPARE(spanABI.start(), spanB.start());
        QCOMPARE(spanABI.end(), spanC.end());
    }
}

SIMPLE_TEST_MAIN(KisTimeSpanTest)
