/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#include "kis_cubic_curve_test.h"

#include <qtest_kde.h>

#include "kis_cubic_curve.h"

KisCubicCurveTest::KisCubicCurveTest() : pt0(0,0), pt1(0.1, 0.0), pt2(0.5, 0.7), pt3(0.8, 0.6), pt4(0.9, 1.0), pt5(1,1)
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
    cc1.setPoint(0,pt4);
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

QTEST_KDEMAIN(KisCubicCurveTest, GUI)
#include "kis_cubic_curve_test.moc"
