/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_marker_painter_test.h"

#include <simpletest.h>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_paint_device_debug_utils.h"
#include "kis_paint_device.h"
#include "kis_marker_painter.h"

#include "kis_algebra_2d.h"
#include <testutil.h>


void KisMarkerPainterTest::testFillHalfBrushDiff()
{
    QRectF rc(10,10,10,10);
    KisAlgebra2D::RightHalfPlane p(QPointF(10,13), QPointF(20,17));

    QCOMPARE(cutOffRect(rc, p), QRectF(10,13,10,7));


    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    QRect fullRect(0,0,20,20);

    QPoint center(10,10);
    qreal radius = 5;
    QPointF p3(10,5);
    QPointF p2(12,10);
    QPointF p1(15,10);
    KoColor color(Qt::blue, cs);

    KisMarkerPainter painter(dev, color);
    painter.fillHalfBrushDiff(p1, p2, p3,
                              center, radius);

    //KIS_DUMP_DEVICE_2(dev, fullRect, "fill_half_brush_raw_points", "dd");
    QImage result = dev->convertToQImage(0, fullRect);
    QVERIFY(TestUtil::checkQImage(result, "marker_painter", "half_brush_fill", ""));
}

void KisMarkerPainterTest::testFillFullCircle()
{
    QRectF rc(10,10,10,10);
    KisAlgebra2D::RightHalfPlane p(QPointF(10,13), QPointF(20,17));

    QCOMPARE(cutOffRect(rc, p), QRectF(10,13,10,7));


    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    QRect fullRect(0,0,20,20);

    QPoint center(10,10);
    qreal radius = 5;
    KoColor color(Qt::blue, cs);

    KisMarkerPainter painter(dev, color);
    painter.fillFullCircle(center, radius);

    //KIS_DUMP_DEVICE_2(dev, fullRect, "fill_full_circle", "dd");
    QImage result = dev->convertToQImage(0, fullRect);
    QVERIFY(TestUtil::checkQImage(result, "marker_painter", "fill_full_circle", ""));
}

void KisMarkerPainterTest::testFillCirclesDiffSingle()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    QRect fullRect(0,0,60,60);
    KoColor color(Qt::blue, cs);


    KisMarkerPainter painter(dev, color);
    painter.fillCirclesDiff(QPointF(10,30), 20,
                            QPointF(30,30), 20);

    //KIS_DUMP_DEVICE_2(dev, fullRect, "fill_single_diff", "dd");
    QImage result = dev->convertToQImage(0, fullRect);
    QVERIFY(TestUtil::checkQImage(result, "marker_painter", "fill_single_diff", ""));
}

void KisMarkerPainterTest::testFillCirclesDiff()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    QRect fullRect(0,0,100,100);
    KoColor color(Qt::blue, cs);

    const int x0 = 20;
    const int x1 = 80;
    const int y0 = 50;
    const int step = 2;

    const qreal omega = 2 * M_PI / (x1 - x0);
    QPointF p0(x0, y0);

    KisMarkerPainter painter(dev, color);
    painter.fillFullCircle(p0, 10);

    for (int x = x0 + step; x < x1; x += step) {
        const qreal y = 20 * std::sin(omega * (x - x0));



        QPointF p1(x, y0 + y);
        painter.fillCirclesDiff(p0, 10,
                                p1, 10);

        p0 = p1;
    }

    //KIS_DUMP_DEVICE_2(dev, fullRect, "fill_stroke", "dd");
    QImage result = dev->convertToQImage(0, fullRect);
    QVERIFY(TestUtil::checkQImage(result, "marker_painter", "fill_stroke", ""));
}

SIMPLE_TEST_MAIN(KisMarkerPainterTest)
