/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <compositeops/KoVcMultiArchBuildSupport.h> //MSVC requires that Vc come first
#include "kis_auto_brush_test.h"

#include <QTest>
#include <testutil.h>
#include "../kis_auto_brush.h"
#include "kis_mask_generator.h"
#include "kis_paint_device.h"
#include "kis_fill_painter.h"
#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoCompositeOpRegistry.h>
#include <kis_fixed_paint_device.h>
#include <brushengine/kis_paint_information.h>

void KisAutoBrushTest::testCreation()
{
    KisCircleMaskGenerator circle(10, 1.0, 1.0, 1.0, 2, true);
    KisRectangleMaskGenerator rect(10, 1.0, 1.0, 1.0, 2, true);
}

void KisAutoBrushTest::testMaskGeneration()
{
    KisCircleMaskGenerator* circle = new KisCircleMaskGenerator(10, 1.0, 1.0, 1.0, 2, false);
    KisBrushSP a(new KisAutoBrush(circle, 0.0, 0.0));
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    KisPaintInformation info(QPointF(100.0, 100.0), 0.5);

    // check masking an existing paint device
    KisFixedPaintDeviceSP fdev = new KisFixedPaintDevice(cs);
    fdev->setRect(QRect(0, 0, 100, 100));
    fdev->initialize();
    cs->setOpacity(fdev->data(), OPACITY_OPAQUE_U8, 100 * 100);

    QPoint errpoint;
    QImage result(QString(FILES_DATA_DIR) + '/' + "result_autobrush_1.png");
    QImage image = fdev->convertToQImage(0);

    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("kis_autobrush_test_1.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }

    // Check creating a mask dab with a single color
    fdev = new KisFixedPaintDevice(cs);
    a->mask(fdev, KoColor(Qt::black, cs), KisDabShape(), info);

    result = QImage(QString(FILES_DATA_DIR) + '/' + "result_autobrush_3.png");
    image = fdev->convertToQImage(0);
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("kis_autobrush_test_3.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }

    // Check creating a mask dab with a color taken from a paint device
    KoColor red(Qt::red, cs);
    cs->setOpacity(red.data(), quint8(128), 1);
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->fill(0, 0, 100, 100, red.data());

    fdev = new KisFixedPaintDevice(cs);
    a->mask(fdev, dev, KisDabShape(), info);

    result = QImage(QString(FILES_DATA_DIR) + '/' + "result_autobrush_4.png");
    image = fdev->convertToQImage(0);
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("kis_autobrush_test_4.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }

}

static void dabSizeHelper(KisBrushSP const& brush,
    QString const& name, KisDabShape const& shape, int expectedWidth, int expectedHeight)
{
    qDebug() << name;
    QCOMPARE(brush->maskWidth(shape, 0.0, 0.0, KisPaintInformation()), expectedWidth);
    QCOMPARE(brush->maskHeight(shape, 0.0, 0.0, KisPaintInformation()), expectedHeight);
}

void KisAutoBrushTest::testDabSize()
{
    KisCircleMaskGenerator* circle = new KisCircleMaskGenerator(10, 0.5, 1.0, 1.0, 2, false);
    KisBrushSP a(new KisAutoBrush(circle, 0.0, 0.0));
    QCOMPARE(a->width(), 10);
    QCOMPARE(a->height(), 5);

    dabSizeHelper(a, "Identity",  KisDabShape(),                        10,  5);
    dabSizeHelper(a, "Double",    KisDabShape(2.0, 1.0, 0.0),           20, 10);
    dabSizeHelper(a, "Halve",     KisDabShape(0.5, 1.0, 0.0),            5,  3);
    dabSizeHelper(a, "180 deg",   KisDabShape(1.0, 1.0, M_PI),          10,  5);
    dabSizeHelper(a, "90 deg",    KisDabShape(1.0, 1.0, M_PI_2),         6, 10); // ceil rule
    dabSizeHelper(a, "-90 deg",   KisDabShape(1.0, 1.0, -M_PI_2),        6, 11); // ceil rule
    dabSizeHelper(a, "45 deg",    KisDabShape(1.0, 1.0, 0.25 * M_PI),   11, 11);
    dabSizeHelper(a, "2x, 45d",   KisDabShape(2.0, 1.0, 0.25 * M_PI),   22, 22);
    dabSizeHelper(a, "0.5x, 45d", KisDabShape(0.5, 1.0, 0.25 * M_PI),    6, 6);
    dabSizeHelper(a, "0.5x, 45d", KisDabShape(0.5, 1.0, 0.25 * M_PI),    6, 6);
    dabSizeHelper(a, "0.5y",      KisDabShape(1.0, 0.5, 0.0),           10, 3);
}

//#define SAVE_OUTPUT_IMAGES
void KisAutoBrushTest::testCopyMasking()
{
    int w = 64;
    int h = 64;
    int x = 0;
    int y = 0;
    QRect rc(x, y, w, h);

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    KoColor black(Qt::black, cs);
    KoColor red(Qt::red, cs);


    KisPaintDeviceSP tempDev = new KisPaintDevice(cs);
    tempDev->fill(0, 0, w, h, red.data());
#ifdef SAVE_OUTPUT_IMAGES
    tempDev->convertToQImage(0).save("tempDev.png");
#endif

    KisCircleMaskGenerator * mask = new KisCircleMaskGenerator(w, 1.0, 0.5, 0.5, 2, true);
    KisAutoBrush brush(mask, 0, 0);

    KisFixedPaintDeviceSP maskDab = new KisFixedPaintDevice(cs);
    brush.mask(maskDab, black, KisDabShape(), KisPaintInformation());
    maskDab->convertTo(KoColorSpaceRegistry::instance()->alpha8());

#ifdef SAVE_OUTPUT_IMAGES
    maskDab->convertToQImage(0, 0, 0, 64, 64).save("maskDab.png");
#endif

    QCOMPARE(tempDev->exactBounds(), rc);
    QCOMPARE(maskDab->bounds(), rc);

    KisFixedPaintDeviceSP dev2fixed = new KisFixedPaintDevice(cs);
    dev2fixed->setRect(rc);
    dev2fixed->initialize();
    tempDev->readBytes(dev2fixed->data(), rc);
    dev2fixed->convertToQImage(0).save("converted-tempDev-to-fixed.png");

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    KisPainter painter(dev);
    painter.setCompositeOp(COMPOSITE_COPY);
    painter.bltFixedWithFixedSelection(x, y, dev2fixed, maskDab, 0, 0, 0, 0, rc.width(), rc.height());
    //painter.bitBltWithFixedSelection(x, y, tempDev, maskDab, 0, 0, 0, 0, rc.width(), rc.height());

#ifdef SAVE_OUTPUT_IMAGES
    dev->convertToQImage(0).save("final.png");
#endif
}

void KisAutoBrushTest::testClone()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    KisCircleMaskGenerator* circle = new KisCircleMaskGenerator(10, 0.7, 0.85, 0.5, 2, true);
    KisBrushSP brush(new KisAutoBrush(circle, 0.5, 0.0));

    KisPaintInformation info(QPointF(100.0, 100.0), 0.5);

    KisFixedPaintDeviceSP fdev1 = new KisFixedPaintDevice(cs);
    brush->mask(fdev1, KoColor(Qt::black, cs), KisDabShape(0.8, 1.0, 8.0), info);
    QImage res1 = fdev1->convertToQImage(0);

    KisBrushSP clone = brush->clone().dynamicCast<KisBrush>();

    KisFixedPaintDeviceSP fdev2 = new KisFixedPaintDevice(cs);
    clone->mask(fdev2, KoColor(Qt::black, cs), KisDabShape(0.8, 1.0, 8.0), info);
    QImage res2 = fdev2->convertToQImage(0);

    QCOMPARE(res1, res2);
}

QTEST_MAIN(KisAutoBrushTest)
