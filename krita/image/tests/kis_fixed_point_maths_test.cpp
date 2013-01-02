/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_fixed_point_maths_test.h"

#include <qtest_kde.h>

#include "kis_fixed_point_maths.h"


void KisFixedPointMathsTest::testOperators()
{
    KisFixedPoint fp1(1);
    KisFixedPoint fp2(2);
    KisFixedPoint fp3(2.5);
    KisFixedPoint fp4(2.0);

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
    QCOMPARE(fp1 / fp2, KisFixedPoint(0.5));
    QCOMPARE(fp1 / fp3, KisFixedPoint(0.4));
    QCOMPARE(fp2 / fp3, KisFixedPoint(0.8));
}

void KisFixedPointMathsTest::testOperatorsNegative()
{
    KisFixedPoint fp1(-2.5);
    KisFixedPoint fp2(2);
    KisFixedPoint fp3(-3);

    QCOMPARE(fp1 + fp2, KisFixedPoint(-0.5));
    QCOMPARE(fp1 - fp2, KisFixedPoint(-4.5));
    QCOMPARE(fp1 * fp2, KisFixedPoint(-5));
    QCOMPARE(fp1 * fp3, KisFixedPoint(7.5));

    QCOMPARE(fp2 / fp1, KisFixedPoint(-0.8));
    QCOMPARE(fp1 / fp2, KisFixedPoint(-1.25));
    QCOMPARE(fp3 / fp1, KisFixedPoint(1.2));
}

void KisFixedPointMathsTest::testConversions()
{
    KisFixedPoint fp1(2.5);
    KisFixedPoint fp2(2.73);
    KisFixedPoint fp3(2.34);

    QCOMPARE(fp1.toInt(), 2);
    QCOMPARE(fp1.toIntRound(), 3);
    QCOMPARE(fp1.toIntFloor(), 2);
    QCOMPARE(fp1.toIntCeil(), 3);
    QCOMPARE(fp1.toFloat(), 2.5);

    QCOMPARE(fp2.toInt(), 2);
    QCOMPARE(fp2.toIntRound(), 3);
    QCOMPARE(fp2.toIntFloor(), 2);
    QCOMPARE(fp2.toIntCeil(), 3);
    QCOMPARE(fp2.toFloat(), 698.0/256.0);

    QCOMPARE(fp3.toInt(), 2);
    QCOMPARE(fp3.toIntRound(), 2);
    QCOMPARE(fp3.toIntFloor(), 2);
    QCOMPARE(fp3.toIntCeil(), 3);
    QCOMPARE(fp3.toFloat(), 599.0/256.0);
}

void KisFixedPointMathsTest::testConversionsNegative()
{
    KisFixedPoint fp1(-2.5);
    KisFixedPoint fp2(-2.73);
    KisFixedPoint fp3(-2.34);

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

QTEST_KDEMAIN(KisFixedPointMathsTest, GUI)
