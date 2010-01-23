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

void KisCubicCurveTest::testCreation()
{
    KisCubicCurve cc1;
    QCOMPARE(cc1.points()[0], QPointF(0,0));
    QCOMPARE(cc1.points()[1], QPointF(1,1));
    QList<QPointF> pts;
    QPointF pt1(0.5, 0.4);
    QPointF pt2(0.8, 0.7);
    QPointF pt3(0.6, 0.7);
    pts.push_back(pt1);
    pts.push_back(pt2);
    pts.push_back(pt3);
    KisCubicCurve cc2(pts);
    QCOMPARE(cc2.points()[0], pt1);
    QCOMPARE(cc2.points()[1], pt3);
    QCOMPARE(cc2.points()[2], pt2);
}

void KisCubicCurveTest::testCopy()
{
    QList<QPointF> pts;
    QPointF pt1(0.0, 0.0);
    QPointF pt2(0.5, 0.7);
    QPointF pt3(1.0, 1.0);
    QPointF pt4(0.8, 0.6);
    pts.push_back(pt1);
    pts.push_back(pt2);
    pts.push_back(pt3);
    KisCubicCurve cc1(pts);
    KisCubicCurve cc2(cc1);
    QCOMPARE(cc1.points()[0], cc2.points()[0]);
    QCOMPARE(cc1.points()[1], cc2.points()[1]);
    QCOMPARE(cc1.points()[2], cc2.points()[2]);
    cc2.addPoint(pt4);
    QCOMPARE(cc1.points().size(), 3);
    QCOMPARE(cc2.points().size(), 4);
    QCOMPARE(cc1.points()[0], cc2.points()[0]);
    QCOMPARE(cc1.points()[1], cc2.points()[1]);
    QCOMPARE(cc1.points()[2], cc2.points()[3]);
    QCOMPARE(pt4, cc2.points()[2]);
}

QTEST_KDEMAIN(KisCubicCurveTest, GUI)
#include "kis_cubic_curve_test.moc"
