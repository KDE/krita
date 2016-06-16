/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_quickop_test.h"

#include <QTest>

#include "../kis_multipoint_painter.h"

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoCompositeOpRegistry.h>

#include "kis_debug.h"
#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_painter.h"

#include "kis_paint_device_debug_utils.h"


void KisQuickopTest::test()
{
    KisMultipointPainter painter;

    QVector<KisMultipointPainter::Point> points;

    points << KisMultipointPainter::Point(QPointF(100, 100), 33.3, 44.4, 1.0);
    points << KisMultipointPainter::Point(QPointF(150, 150), 33.3, 44.4, 1.0);
    points << KisMultipointPainter::Point(QPointF(170, 150), 33.3, 44.4, 1.0);

    painter.setPoints(points);

    QCOMPARE(painter.boundingRect(), QRect(66,55,139,141));

    KisMultipointPainter::CompositeRow row;

    row = painter.getCompositeRow(80);
    QCOMPARE(row.rows.size(), 1);

    row = painter.getCompositeRow(130);
    QCOMPARE(row.rows.size(), 3);

    row = painter.getCompositeRow(170);
    QCOMPARE(row.rows.size(), 2);

    row = painter.getCompositeRow(205);
    QCOMPARE(row.rows.size(), 0);
}

void KisQuickopTest::testMerged()
{
    KisMultipointPainter painter;

    QVector<KisMultipointPainter::Point> points;

    points << KisMultipointPainter::Point(QPointF(135, 125), 15.0, 15.0, 1.0);
    points << KisMultipointPainter::Point(QPointF(145, 145), 15.0, 15.0, 1.0);
    points << KisMultipointPainter::Point(QPointF(135, 165), 15.0, 15.0, 1.0);

    painter.setPoints(points);

    QCOMPARE(painter.boundingRect(), QRect(120,110,41,71));

    painter.calcMergedAreas();

    QVector<KisMultipointPainter::MergedArea> areas = painter.fetchMergedAreas();

    qDebug() << ppVar(areas.size());

    foreach (const KisMultipointPainter::MergedArea &a, areas) {
        qDebug() << "===";
        qDebug() << ppVar(a.xStart) << ppVar(a.xEnd);
        qDebug() << ppVar(a.yStart) << ppVar(a.yEnd);
        //qDebug() << ppVar(a.baseUnitValues.size());
        qDebug() << ppVar(a.dxValues.size());
        qDebug() << ppVar(a.dyValues.size());
        qDebug() << ppVar(a.activePoints.size());
    }

}

void KisQuickopTest::testPaint()
{
    QVector<QVector<KisMultipointPainter::Point> > points;

    points.resize(1);

    const int numSteps = 2;
    const int stride = numSteps / points.size();

    for (int i = 0; i < numSteps; i++) {
        qreal x = 100 + 25 * i/* + 46*cos(2*3.14/210 * i)*/;
        qreal y = 100 + 25 * i/* + 97*sin(2*3.14/170 * i)*/;
        qreal opacity = 1.0;//pow2(sin(1 * 3.14 / numSteps * i));


        QVector<KisMultipointPainter::Point> &pts =
            points[i / stride];

        qreal size = 40.0;
        //qreal size = 500;

        pts << KisMultipointPainter::Point(QPointF(x,y), size, size, opacity);
    }

    QRect imageRect(0,0,700,700);

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KoColor color(Qt::black, cs);

    KisPaintDeviceSP projection = new KisPaintDevice(cs);
    KisPainter gc(projection);

    for (int i = 0; i < points.size(); i++) {
        KisMultipointPainter painter;
        painter.setPoints(points[i]);

        ///
        painter.calcMergedAreas();
        ///

        QRect bounds = painter.boundingRect();
        qDebug() << ppVar(bounds);

        KisPaintDeviceSP dev = new KisPaintDevice(cs);

        QTime elapsedCycle;
        elapsedCycle.start();

        painter.paintPoints3(projection, color);

        qDebug() << ppVar(elapsedCycle.elapsed() / points[i].size());
    }

    KIS_DUMP_DEVICE_2(projection, imageRect, "projection", "dd");
}


QTEST_MAIN(KisQuickopTest)
