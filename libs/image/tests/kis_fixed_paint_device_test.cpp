/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_fixed_paint_device_test.h"
#include <QTest>

#include <QTime>

#include <KoColorSpace.h>
#include <KoColor.h>
#include <KoColorSpaceRegistry.h>

#include "kis_painter.h"
#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_fixed_paint_device.h"
#include "kis_layer.h"
#include "kis_paint_layer.h"
#include "kis_selection.h"
#include "kis_datamanager.h"
#include "kis_global.h"
#include <testutil.h>
#include <testimage.h>
#include "kis_transaction.h"
#include "kis_image.h"

void KisFixedPaintDeviceTest::testCreation()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisFixedPaintDeviceSP dev = new KisFixedPaintDevice(cs);

    dev = new KisFixedPaintDevice(cs);
    QVERIFY(dev->bounds() == QRect());
    QVERIFY(*dev->colorSpace() == *cs);
    QVERIFY(dev->pixelSize() == cs->pixelSize());

    dev->setRect(QRect(0, 0, 100, 100));
    QVERIFY(dev->bounds() == QRect(0, 0, 100, 100));
    dev->initialize();
    QVERIFY(dev->data() != 0);

    quint8* data = dev->data();
    for (uint i = 0; i < 100 * 100 * cs->pixelSize(); ++i) {
        QVERIFY(data[i] == 0);
    }

}

void logFailure(const QString & reason, const KoColorSpace * srcCs, const KoColorSpace * dstCs)
{
    QString profile1("no profile");
    QString profile2("no profile");
    if (srcCs->profile())
        profile1 = srcCs->profile()->name();
    if (dstCs->profile())
        profile2 = dstCs->profile()->name();

    QWARN(QString("Failed %1 %2 -> %3 %4 %5")
          .arg(srcCs->name())
          .arg(profile1)
          .arg(dstCs->name())
          .arg(profile2)
          .arg(reason)
          .toLatin1());
}

void KisFixedPaintDeviceTest::testColorSpaceConversion()
{
    QImage image(QString(FILES_DATA_DIR) + '/' + "tile.png");
    const KoColorSpace* srcCs = KoColorSpaceRegistry::instance()->rgb8();
    const KoColorSpace* dstCs = KoColorSpaceRegistry::instance()->lab16();
    KisFixedPaintDeviceSP dev = new KisFixedPaintDevice(srcCs);
    dev->convertFromQImage(image, 0);

    dev->convertTo(dstCs);

    QVERIFY(dev->bounds() == QRect(0, 0, image.width(), image.height()));
    QVERIFY(dev->pixelSize() == dstCs->pixelSize());
    QVERIFY(*dev->colorSpace() == *dstCs);

}


void KisFixedPaintDeviceTest::testRoundtripQImageConversion()
{
    QImage image(QString(FILES_DATA_DIR) + '/' + "hakonepa.png");
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisFixedPaintDeviceSP dev = new KisFixedPaintDevice(cs);
    dev->convertFromQImage(image, 0);
    QImage result = dev->convertToQImage(0, 0, 0, 640, 441);

    QPoint errpoint;

    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("kis_fixed_paint_device_test_test_roundtrip_qimage.png");
        result.save("kis_fixed_paint_device_test_test_roundtrip_result.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

void KisFixedPaintDeviceTest::testBltFixed()
{
    QImage image(QString(FILES_DATA_DIR) + '/' + "hakonepa.png");
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisFixedPaintDeviceSP fdev = new KisFixedPaintDevice(cs);
    fdev->convertFromQImage(image, 0);

    // Without opacity
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    KisPainter gc(dev);
    gc.bltFixed(QPoint(0, 0), fdev, image.rect());

    QImage result = dev->convertToQImage(0, 0, 0, 640, 441);

    QPoint errpoint;

    if (!TestUtil::compareQImages(errpoint, image, result, 1)) {
        fdev->convertToQImage(0).save("kis_fixed_paint_device_test_test_blt_fixed_expected.png");
        result.save("kis_fixed_paint_device_test_test_blt_fixed_result.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }

}

void KisFixedPaintDeviceTest::testBltFixedOpacity()
{
    // blt a semi-transparent image on a white paint device

    QImage image(QString(FILES_DATA_DIR) + '/' + "hakonepa_transparent.png");
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisFixedPaintDeviceSP fdev = new KisFixedPaintDevice(cs);
    fdev->convertFromQImage(image, 0);

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->fill(0, 0, 640, 441, KoColor(Qt::white, cs).data());
    KisPainter gc(dev);
    gc.bltFixed(QPoint(0, 0), fdev, image.rect());

    QImage result = dev->convertToQImage(0, 0, 0, 640, 441);
    QImage checkResult(QString(FILES_DATA_DIR) + '/' + "hakonepa_transparent_result.png");
    QPoint errpoint;

    if (!TestUtil::compareQImages(errpoint, checkResult, result, 1)) {
        checkResult.save("kis_fixed_paint_device_test_test_blt_fixed_opactiy_expected.png");
        result.save("kis_fixed_paint_device_test_test_blt_fixed_opacity_result.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

void KisFixedPaintDeviceTest::testSilly()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisFixedPaintDeviceSP dev = new KisFixedPaintDevice(cs);
    dev->initialize();
    dev->initialize();

}
void KisFixedPaintDeviceTest::testClear()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisFixedPaintDeviceSP dev = new KisFixedPaintDevice(cs);
    dev->clear(QRect(0, 0, 100, 100));
    QVERIFY(dev->bounds() == QRect(0, 0, 100, 100));
    QVERIFY(cs->opacityU8(dev->data() + (50 * 50 * cs->pixelSize())) == OPACITY_TRANSPARENT_U8);
}

void KisFixedPaintDeviceTest::testFill()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    quint8* red = new quint8[cs->pixelSize()];
    memcpy(red, KoColor(Qt::red, cs).data(), cs->pixelSize());
    cs->setOpacity(red, quint8(128), 1);

    KisFixedPaintDeviceSP dev = new KisFixedPaintDevice(cs);
    dev->fill(0, 0, 100, 100, red);

    QVERIFY(dev->bounds() == QRect(0, 0, 100, 100));
    QVERIFY(cs->opacityU8(dev->data()) == 128);
    QVERIFY(memcmp(dev->data(), red, cs->pixelSize()) == 0);
 
    //Compare fill will normal paint device
    dev = new KisFixedPaintDevice(cs);
    dev->setRect(QRect(0, 0, 150, 150));
    dev->initialize();
    dev->fill(50, 50, 50, 50, red);
    
    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    dev2->fill(50, 50, 50, 50, red);
    
    QImage image = dev->convertToQImage(0);
    QImage checkImage = dev2->convertToQImage(0, 0, 0, 150, 150);
    QPoint errpoint;

    if (!TestUtil::compareQImages(errpoint, image, checkImage)) {
        image.save("kis_fixed_paint_device_filled_result.png");
        checkImage.save("kis_fixed_paint_device_filled_result_expected.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    } 
    
    delete[] red;
}

void KisFixedPaintDeviceTest::testBltFixedSmall()
{
    QImage image(QString(FILES_DATA_DIR) + '/' + "fixed_blit_small.png");
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisFixedPaintDeviceSP fdev = new KisFixedPaintDevice(cs);
    fdev->convertFromQImage(image, 0);

    // Without opacity
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    KisPainter gc(dev);
    gc.bltFixed(QPoint(0, 0), fdev, image.rect());

    QImage result = dev->convertToQImage(0, 0, 0, 51, 51);

    QPoint errpoint;

    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("kis_fixed_paint_device_test_blt_small_image.png");
        result.save("kis_fixed_paint_device_test_blt_small_result.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

void KisFixedPaintDeviceTest::testBltPerformance()
{
    QImage image(QString(FILES_DATA_DIR) + '/' + "hakonepa_transparent.png");
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisFixedPaintDeviceSP fdev = new KisFixedPaintDevice(cs);
    fdev->convertFromQImage(image, 0);

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->fill(0, 0, 640, 441, KoColor(Qt::white, cs).data());


    QElapsedTimer t;
    t.start();

    int x;
    for (x = 0; x < 100; ++x) {
        KisPainter gc(dev);
        gc.bltFixed(QPoint(0, 0), fdev, image.rect());
    }

    dbgKrita << x
    << "blits"
    << " done in "
    << t.elapsed()
    << "ms";
}

inline void setPixel(KisFixedPaintDeviceSP dev, int x, int y, quint8 alpha)
{
    KoColor c(Qt::black, dev->colorSpace());
    c.setOpacity(alpha);

    dev->fill(x, y, 1, 1, c.data());
}

inline quint8 pixel(KisFixedPaintDeviceSP dev, int x, int y)
{
    KoColor c(Qt::black, dev->colorSpace());


    dev->readBytes(c.data(), x, y, 1, 1);
    return c.opacityU8();
}

void KisFixedPaintDeviceTest::testMirroring_data()
{
    QTest::addColumn<QRect>("rc");
    QTest::addColumn<bool>("mirrorHorizontally");
    QTest::addColumn<bool>("mirrorVertically");

    QTest::newRow("4, false, false") << (QRect(99,99,4,4)) << false << false;
    QTest::newRow("4, false, true") << (QRect(99,99,4,4)) << false << true;
    QTest::newRow("4, true, false") << (QRect(99,99,4,4)) << true << false;
    QTest::newRow("4, true, true") << (QRect(99,99,4,4)) << true << true;

    QTest::newRow("5, false, false") << (QRect(99,99,5,5)) << false << false;
    QTest::newRow("5, false, true") << (QRect(99,99,5,5)) << false << true;
    QTest::newRow("5, true, false") << (QRect(99,99,5,5)) << true << false;
    QTest::newRow("5, true, true") << (QRect(99,99,5,5)) << true << true;
}


void KisFixedPaintDeviceTest::testMirroring()
{
    QFETCH(QRect, rc);
    QFETCH(bool, mirrorHorizontally);
    QFETCH(bool, mirrorVertically);

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisFixedPaintDeviceSP dev = new KisFixedPaintDevice(cs);
    dev->setRect(rc);
    dev->initialize();

    KoColor c(Qt::black, cs);

    qsrand(1);
    int value = 0;

    for (int i = rc.x(); i < rc.x() + rc.width(); i++) {
        for (int j = rc.y(); j < rc.y() + rc.height(); j++) {
            setPixel(dev, i, j, value);
            value = qrand() % 255;
        }
        value = qrand() % 255;
    }

    //dev->convertToQImage(0).save("0_a.png");
    dev->mirror(mirrorHorizontally, mirrorVertically);
    //dev->convertToQImage(0).save("0_b.png");

    int startX;
    int endX;
    int incX;

    int startY;
    int endY;
    int incY;

    if (mirrorHorizontally) {
        startX = rc.x() + rc.width() - 1;
        endX = rc.x() - 1;
        incX = -1;
    } else {
        startX = rc.x();
        endX = rc.x() + rc.width();
        incX = 1;
    }

    if (mirrorVertically) {
        startY = rc.y() + rc.height() - 1;
        endY = rc.y() - 1;
        incY = -1;
    } else {
        startY = rc.y();
        endY = rc.y() + rc.height();
        incY = 1;
    }

    qsrand(1);
    value = 0;

    for (int i = startX; i != endX ; i += incX) {
        for (int j = startY; j != endY; j += incY) {
            QCOMPARE(pixel(dev, i, j), (quint8)value);
            value = qrand() % 255;
        }
        value = qrand() % 255;
    }
}

QTEST_MAIN(KisFixedPaintDeviceTest)
