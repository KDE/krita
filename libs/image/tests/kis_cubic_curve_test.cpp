/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_cubic_curve_test.h"

#include <simpletest.h>

#include "kis_cubic_curve.h"

KisCubicCurveTest::KisCubicCurveTest() : pt0(0, 0), pt1(0.1, 0.0), pt2(0.5, 0.7), pt3(0.8, 0.6), pt4(0.9, 1.0), pt5(1, 1)
{
}

void KisCubicCurveTest::testCreation()
{
    KisCubicCurve cc1;
    QCOMPARE(cc1.points()[0], pt0);
    QCOMPARE(cc1.points()[1], pt5);
    QList<QPointF> pts;
    pts.push_back(pt1);
    pts.push_back(pt3);
    pts.push_back(pt2);
    KisCubicCurve cc2(pts);
    QCOMPARE(cc2.points()[0], pt1);
    QCOMPARE(cc2.points()[1], pt2);
    QCOMPARE(cc2.points()[2], pt3);
}

void KisCubicCurveTest::testCopy()
{
    QList<QPointF> pts;
    pts.push_back(pt1);
    pts.push_back(pt2);
    pts.push_back(pt4);
    KisCubicCurve cc1(pts);
    KisCubicCurve cc2(cc1);
    QVERIFY(cc1 == cc2);
    QCOMPARE(cc1.points()[0], cc2.points()[0]);
    QCOMPARE(cc1.points()[1], cc2.points()[1]);
    QCOMPARE(cc1.points()[2], cc2.points()[2]);
    cc2.addPoint(pt3);
    QCOMPARE(cc1.points().size(), 3);
    QCOMPARE(cc2.points().size(), 4);
    QCOMPARE(cc1.points()[0], cc2.points()[0]);
    QCOMPARE(cc1.points()[1], cc2.points()[1]);
    QCOMPARE(cc1.points()[2], cc2.points()[3]);
    QCOMPARE(pt3, cc2.points()[2]);
}

void KisCubicCurveTest::testEdition()
{
    KisCubicCurve cc1;
    cc1.addPoint(pt3);
    QCOMPARE(cc1.points().size(), 3);
    QCOMPARE(cc1.points()[0], pt0);
    QCOMPARE(cc1.points()[1], pt3);
    QCOMPARE(cc1.points()[2], pt5);
    cc1.setPoint(0, pt4);
    QCOMPARE(cc1.points().size(), 3);
    QCOMPARE(cc1.points()[0], pt3);
    QCOMPARE(cc1.points()[1], pt4);
    QCOMPARE(cc1.points()[2], pt5);
    int pos = cc1.addPoint(pt2);
    QCOMPARE(pos, 0);
    QCOMPARE(cc1.points().size(), 4);
    QCOMPARE(cc1.points()[0], pt2);
    QCOMPARE(cc1.points()[1], pt3);
    QCOMPARE(cc1.points()[2], pt4);
    QCOMPARE(cc1.points()[3], pt5);
    cc1.removePoint(2);
    QCOMPARE(cc1.points().size(), 3);
    QCOMPARE(cc1.points()[0], pt2);
    QCOMPARE(cc1.points()[1], pt3);
    QCOMPARE(cc1.points()[2], pt5);
}

void KisCubicCurveTest::testComparison()
{
    QList<QPointF> pts;
    pts.push_back(pt1);
    pts.push_back(pt2);
    pts.push_back(pt4);
    KisCubicCurve cc1(pts);
    KisCubicCurve cc2(pts);
    QVERIFY(cc1 == cc2);
    cc2.setPoint(0, pt1);
    QVERIFY(cc1 == cc2);
    cc2.removePoint(2);
    QVERIFY(!(cc1 == cc2));
    cc2.addPoint(pt4);
    QVERIFY(cc1 == cc2);
    cc2.addPoint(pt5);
    QVERIFY(!(cc1 == cc2));
    QList<QPointF> pts2;
    pts2.push_back(pt1);
    pts2.push_back(pt2);
    pts2.push_back(pt3);
    KisCubicCurve cc3(pts2);
    QVERIFY(!(cc1 == cc3));

}

void KisCubicCurveTest::testSerialization()
{
    QList<QPointF> pts;
    pts.push_back(pt1);
    pts.push_back(pt2);
    pts.push_back(pt4);
    KisCubicCurve cc1(pts);
    QString s = cc1.toString();
    QCOMPARE(s, QString("0.1,0;0.5,0.7;0.9,1;"));
    KisCubicCurve cc2;
    cc2.fromString(s);
    QVERIFY(cc1 == cc2);
}

void KisCubicCurveTest::testValue()
{
    KisCubicCurve cc;
    for(int i = 0; i < 256; ++i)
    {
        qreal x = i/255.0;
        QCOMPARE(cc.value(x), x);
    }
}

void KisCubicCurveTest::testNull()
{
    KisCubicCurve cc;
    QVERIFY(cc.isIdentity());

    cc.addPoint(QPointF(0.2, 0.3));
    QVERIFY(!cc.isIdentity());

    QList<QPointF> points;
    points << QPointF();
    points << QPointF(0.2,0.2);
    points << QPointF(1.0,1.0);

    cc.setPoints(points);
    QVERIFY(cc.isIdentity());
}


void KisCubicCurveTest::testTransfer()
{
    KisCubicCurve cc;
    QCOMPARE(cc.uint16Transfer().size(), 256);
    qreal denom = 1 / 255.0;
    for(int i = 0; i < 256; ++i)
    {
        QCOMPARE(cc.uint16Transfer()[i], quint16( cc.value(i * denom) * 0xFFFF) );
    }
    QCOMPARE(cc.floatTransfer().size(), 256);
    for(int i = 0; i < 256; ++i)
    {
        QCOMPARE(cc.floatTransfer()[i], i * denom);
    }
    QCOMPARE(cc.uint16Transfer(1024).size(), 1024);
    denom = 1 / 1023.0;
    for(int i = 0; i < 1024; ++i)
    {
        QCOMPARE(cc.uint16Transfer(1024)[i], quint16( cc.value(i * denom) * 0xFFFF) );
    }
    QCOMPARE(cc.floatTransfer(1024).size(), 1024);
    for(int i = 0; i < 1024; ++i)
    {
        QCOMPARE(cc.floatTransfer(1024)[i], i * denom);
    }
}

SIMPLE_TEST_MAIN(KisCubicCurveTest)
