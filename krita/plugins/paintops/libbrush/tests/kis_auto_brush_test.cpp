/*
 *  Copyright (c) 2007 Boudewijn Rempt boud@valdyas.org
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include "kis_auto_brush_test.h"

#include <qtest_kde.h>
#include "testutil.h"
#include "../kis_auto_brush.h"
#include "kis_mask_generator.h"
#include "kis_paint_device.h"
#include "kis_fill_painter.h"
#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

void KisAutoBrushTest::testCreation()
{
    KisCircleMaskGenerator circle(10, 10, 1.0, 1.0);
    KisRectangleMaskGenerator rect(10, 10, 1.0, 1.0);
}

void KisAutoBrushTest::testMaskGeneration()
{
    KisCircleMaskGenerator* circle = new KisCircleMaskGenerator(10, 10, 1.0, 1.0);
    KisBrushSP a = new KisAutoBrush(circle, 0.0);
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    KisVector2D v2d = KisVector2D::Zero();
    KisPaintInformation info(QPointF(100.0, 100.0), 0.5, 0, 0, v2d, 0, 0);

    // check masking an existing paint device
    KisFixedPaintDeviceSP fdev = new KisFixedPaintDevice(cs);
    fdev->setRect(QRect(0, 0, 100, 100));
    fdev->initialize();
    cs->setAlpha(fdev->data(), OPACITY_OPAQUE, 100 * 100);

    QPoint errpoint;
    QImage result(QString(FILES_DATA_DIR) + QDir::separator() + "result_autobrush_1.png");
    QImage image = fdev->convertToQImage();

    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("kis_autobrush_test_1.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }

    a->mask(fdev, 1.0, 1.0, 0.0, info);

    result = QImage(QString(FILES_DATA_DIR) + QDir::separator() + "result_autobrush_2.png");
    image = fdev->convertToQImage();
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("kis_autobrush_test_2.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }

    // Check creating a mask dab with a single color
    fdev = new KisFixedPaintDevice(cs);
    a->mask(fdev, KoColor(Qt::black, cs), 1.0, 1.0, 0.0, info);

    result = QImage(QString(FILES_DATA_DIR) + QDir::separator() + "result_autobrush_3.png");
    image = fdev->convertToQImage();
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("kis_autobrush_test_3.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }

    // Check creating a mask dab with a color taken from a paint device
    KoColor red(Qt::red, cs);
    cs->setAlpha(red.data(), 128, 1);
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->fill(0, 0, 100, 100, red.data());

    fdev = new KisFixedPaintDevice(cs);
    a->mask(fdev, dev, 1.0, 1.0, 0.0, info);

    result = QImage(QString(FILES_DATA_DIR) + QDir::separator() + "result_autobrush_4.png");
    image = fdev->convertToQImage();
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("kis_autobrush_test_4.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }

    // check creating a mask dab with a default color
    fdev = new KisFixedPaintDevice(cs);
    a->mask(fdev, 1.0, 1.0, 0.0, info);

    result = QImage(QString(FILES_DATA_DIR) + QDir::separator() + "result_autobrush_3.png");
    image = fdev->convertToQImage();
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("kis_autobrush_test_5.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }

}

void KisAutoBrushTest::testSizeRotation()
{
    {
        KisCircleMaskGenerator* circle = new KisCircleMaskGenerator(10, 0.5, 1.0, 1.0, 2);
        KisBrushSP a = new KisAutoBrush(circle, 0.0);
        QCOMPARE(a->width(), 10);
        QCOMPARE(a->height(), 5);
        QCOMPARE(a->maskWidth(1.0,0.0), 11);
        QCOMPARE(a->maskHeight(1.0,0.0), 6);
        QCOMPARE(a->maskWidth(2.0,0.0), 21);
        QCOMPARE(a->maskHeight(2.0,0.0), 11);
        QCOMPARE(a->maskWidth(0.5,0.0), 6);
        QCOMPARE(a->maskHeight(0.5,0.0), 3);
        QCOMPARE(a->maskWidth(1.0,M_PI), 11);
        QCOMPARE(a->maskHeight(1.0,M_PI), 6);
        QCOMPARE(a->maskWidth(1.0,M_PI_2), 6);
        QCOMPARE(a->maskHeight(1.0,M_PI_2), 11);
        QCOMPARE(a->maskWidth(1.0,-M_PI_2), 7);
        QCOMPARE(a->maskHeight(1.0,-M_PI_2), 11);
        QCOMPARE(a->maskWidth(1.0,0.25*M_PI), 12);
        QCOMPARE(a->maskHeight(1.0,0.25*M_PI), 12);
        QCOMPARE(a->maskWidth(2.0,0.25*M_PI), 23);
        QCOMPARE(a->maskHeight(2.0,0.25*M_PI), 23);
        QCOMPARE(a->maskWidth(0.5,0.25*M_PI), 7);
        QCOMPARE(a->maskHeight(0.5,0.25*M_PI), 7);
    }
}


QTEST_KDEMAIN(KisAutoBrushTest, GUI)
#include "kis_auto_brush_test.moc"
