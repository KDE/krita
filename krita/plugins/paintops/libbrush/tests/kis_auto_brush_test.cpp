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
#include <KoCompositeOp.h>

void KisAutoBrushTest::testCreation()
{
    KisCircleMaskGenerator circle(10, 1.0, 1.0, 1.0, 2);
    KisRectangleMaskGenerator rect(10, 1.0, 1.0, 1.0, 2);
}

void KisAutoBrushTest::testMaskGeneration()
{
    KisCircleMaskGenerator* circle = new KisCircleMaskGenerator(10, 1.0, 1.0, 1.0, 2);
    KisBrushSP a = new KisAutoBrush(circle, 0.0, 0.0);
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    KisVector2D v2d = KisVector2D::Zero();
    KisPaintInformation info(QPointF(100.0, 100.0), 0.5, 0, 0, v2d, 0, 0);

    // check masking an existing paint device
    KisFixedPaintDeviceSP fdev = new KisFixedPaintDevice(cs);
    fdev->setRect(QRect(0, 0, 100, 100));
    fdev->initialize();
    cs->setOpacity(fdev->data(), OPACITY_OPAQUE_U8, 100 * 100);

    QPoint errpoint;
    QImage result(QString(FILES_DATA_DIR) + QDir::separator() + "result_autobrush_1.png");
    QImage image = fdev->convertToQImage(0);

    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("kis_autobrush_test_1.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }

    // Check creating a mask dab with a single color
    fdev = new KisFixedPaintDevice(cs);
    a->mask(fdev, KoColor(Qt::black, cs), 1.0, 1.0, 0.0, info);

    result = QImage(QString(FILES_DATA_DIR) + QDir::separator() + "result_autobrush_3.png");
    image = fdev->convertToQImage(0);
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("kis_autobrush_test_3.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }

    // Check creating a mask dab with a color taken from a paint device
    KoColor red(Qt::red, cs);
    cs->setOpacity(red.data(), quint8(128), 1);
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->fill(0, 0, 100, 100, red.data());

    fdev = new KisFixedPaintDevice(cs);
    a->mask(fdev, dev, 1.0, 1.0, 0.0, info);

    result = QImage(QString(FILES_DATA_DIR) + QDir::separator() + "result_autobrush_4.png");
    image = fdev->convertToQImage(0);
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("kis_autobrush_test_4.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }

}

void KisAutoBrushTest::testSizeRotation()
{
    {
        KisCircleMaskGenerator* circle = new KisCircleMaskGenerator(10, 0.5, 1.0, 1.0, 2);
        KisBrushSP a = new KisAutoBrush(circle, 0.0, 0.0);
        QCOMPARE(a->width(), 10);
        QCOMPARE(a->height(), 5);
        QCOMPARE(a->maskWidth(1.0,0.0), 11);
        QCOMPARE(a->maskHeight(1.0,0.0), 6);
        QCOMPARE(a->maskWidth(2.0,0.0), 21);
        QCOMPARE(a->maskHeight(2.0,0.0), 11);
        QCOMPARE(a->maskWidth(0.5,0.0), 6);
        QCOMPARE(a->maskHeight(0.5,0.0), 4);
        QCOMPARE(a->maskWidth(1.0,M_PI), 11);
        QCOMPARE(a->maskHeight(1.0,M_PI), 6);
        QCOMPARE(a->maskWidth(1.0,M_PI_2), 6);
        QCOMPARE(a->maskHeight(1.0,M_PI_2), 11);
        QCOMPARE(a->maskWidth(1.0,-M_PI_2), 6);
        QCOMPARE(a->maskHeight(1.0,-M_PI_2), 11);
        QCOMPARE(a->maskWidth(1.0,0.25*M_PI), 12);
        QCOMPARE(a->maskHeight(1.0,0.25*M_PI), 12);
        QCOMPARE(a->maskWidth(2.0,0.25*M_PI), 23);
        QCOMPARE(a->maskHeight(2.0,0.25*M_PI), 23);
        QCOMPARE(a->maskWidth(0.5,0.25*M_PI), 7);
        QCOMPARE(a->maskHeight(0.5,0.25*M_PI), 7);
    }
}

//#define SAVE_OUTPUT_IMAGES
void KisAutoBrushTest::testCopyMasking()
{
    int w = 64;
    int h = 64;

    int x = 0;
    int y = 0;
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    KoColor black(Qt::black, cs);
    KoColor red(Qt::red, cs);


    KisPaintDeviceSP tempDev = new KisPaintDevice(cs);
    tempDev->fill(0, 0, w+1, h+1, red.data()); // see the TODO
#ifdef SAVE_OUTPUT_IMAGES
    tempDev->convertToQImage(0).save("tempDev.png");
#endif

    KisCircleMaskGenerator * mask = new KisCircleMaskGenerator(w,1.0,0.5,0.5,2);
    KisAutoBrush brush(mask,0,0);

    KisFixedPaintDeviceSP maskDab = new KisFixedPaintDevice(cs);
    brush.mask(maskDab,black,1,1,0,KisPaintInformation()); // grows to w+1, h+1
    maskDab->convertTo(KoColorSpaceRegistry::instance()->alpha8());

#ifdef SAVE_OUTPUT_IMAGES
    maskDab->convertToQImage(0,0,0,64,64).save("maskDab.png");
#endif

    QRect rc = tempDev->exactBounds();
    //QRect maskRc = maskDab->bounds();

    //TODO: if rc != maskRc, bitBltWithFixedSelection works wrong
    //qDebug() << rc;
    //qDebug() << maskRc;


    KisFixedPaintDeviceSP dev2fixed = new KisFixedPaintDevice(cs);
    dev2fixed->setRect(rc);
    dev2fixed->initialize();
    tempDev->readBytes(dev2fixed->data(),rc);
    dev2fixed->convertToQImage(0).save("converted-tempDev-to-fixed.png");

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    KisPainter painter(dev);
    painter.setCompositeOp(COMPOSITE_COPY);
    painter.bltFixedWithFixedSelection(x, y, dev2fixed, maskDab, 0,0,0,0,rc.width(), rc.height() );
    //painter.bitBltWithFixedSelection(x, y, tempDev, maskDab, 0, 0, 0, 0, rc.width(), rc.height());

#ifdef SAVE_OUTPUT_IMAGES
    dev->convertToQImage(0).save("final.png");
#endif
}



QTEST_KDEMAIN(KisAutoBrushTest, GUI)
#include "kis_auto_brush_test.moc"
