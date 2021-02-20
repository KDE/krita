/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_fixed_point_maths_test.h"

#include <QTest>

#include "kis_fixed_point_maths.h"


void KisFixedPointMathsTest::testOperators()
{
    KisFixedPoint fp1(1);
    KisFixedPoint fp2(2);
    KisFixedPoint fp3(qreal(2.5));
    KisFixedPoint fp4(qreal(2.0));

    QVERIFY(fp1 < fp2);
    QVERIFY(fp2 < fp3);

    QVERIFY(fp2 > fp1);
    QVERIFY(fp3 > fp2);

    QVERIFY(fp1 != fp2);
    QVERIFY(fp2 == fp4);

    QCOMPARE(fp1 + fp2, KisFixedPoint(3));
    QCOMPARE(fp1 - fp2, KisFixedPoint(-1));
    QCOMPARE(fp1 * fp2, KisFixedPoint(2));
    QCOMPARE(fp2 * fp3, KisFixedPoint(5));
    QCOMPARE(fp1 / fp2, KisFixedPoint(qreal(0.5)));
    QCOMPARE(fp1 / fp3, KisFixedPoint(qreal(0.4)));
    QCOMPARE(fp2 / fp3, KisFixedPoint(qreal(0.8)));
}

void KisFixedPointMathsTest::testOperatorsNegative()
{
    KisFixedPoint fp1(qreal(-2.5));
    KisFixedPoint fp2(2);
    KisFixedPoint fp3(-3);

    QCOMPARE(fp1 + fp2, KisFixedPoint(qreal(-0.5)));
    QCOMPARE(fp1 - fp2, KisFixedPoint(qreal(-4.5)));
    QCOMPARE(fp1 * fp2, KisFixedPoint(-5));
    QCOMPARE(fp1 * fp3, KisFixedPoint(qreal(7.5)));

    QCOMPARE(fp2 / fp1, KisFixedPoint(qreal(-0.8)));
    QCOMPARE(fp1 / fp2, KisFixedPoint(qreal(-1.25)));
    QCOMPARE(fp3 / fp1, KisFixedPoint(qreal(1.2)));
}

void KisFixedPointMathsTest::testConversions()
{
    KisFixedPoint fp1(qreal(2.5));
    KisFixedPoint fp2(qreal(2.73));
    KisFixedPoint fp3(qreal(2.34));

    QCOMPARE(fp1.toInt(), 2);
    QCOMPARE(fp1.toIntRound(), 3);
    QCOMPARE(fp1.toIntFloor(), 2);
    QCOMPARE(fp1.toIntCeil(), 3);
    QCOMPARE(fp1.toFloat(), qreal(2.5));

    QCOMPARE(fp2.toInt(), 2);
    QCOMPARE(fp2.toIntRound(), 3);
    QCOMPARE(fp2.toIntFloor(), 2);
    QCOMPARE(fp2.toIntCeil(), 3);
    QCOMPARE(fp2.toFloat(), qreal(698.0/256.0));

    QCOMPARE(fp3.toInt(), 2);
    QCOMPARE(fp3.toIntRound(), 2);
    QCOMPARE(fp3.toIntFloor(), 2);
    QCOMPARE(fp3.toIntCeil(), 3);
    QCOMPARE(fp3.toFloat(), qreal(599.0/256.0));
}

void KisFixedPointMathsTest::testConversionsNegative()
{
    KisFixedPoint fp1(qreal(-2.5));
    KisFixedPoint fp2(qreal(-2.73));
    KisFixedPoint fp3(qreal(-2.34));

    QCOMPARE(fp1.toInt(), -2);
    QCOMPARE(fp1.toIntRound(), -3);
    QCOMPARE(fp1.toIntFloor(), -3);
    QCOMPARE(fp1.toIntCeil(), -2);

    QCOMPARE(fp2.toInt(), -2);
    QCOMPARE(fp2.toIntRound(), -3);
    QCOMPARE(fp2.toIntFloor(), -3);
    QCOMPARE(fp2.toIntCeil(), -2);

    QCOMPARE(fp3.toInt(), -2);
    QCOMPARE(fp3.toIntRound(), -2);
    QCOMPARE(fp3.toIntFloor(), -3);
    QCOMPARE(fp3.toIntCeil(), -2);
}

QTEST_MAIN(KisFixedPointMathsTest)
