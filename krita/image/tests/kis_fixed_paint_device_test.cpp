/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_fixed_paint_device_test.h"
#include <qtest_kde.h>

#include <QTime>

#include <KoStore.h>
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
#include "testutil.h"
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
          .toAscii());
}

void KisFixedPaintDeviceTest::testColorSpaceConversion()
{
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "tile.png");
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
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisFixedPaintDeviceSP dev = new KisFixedPaintDevice(cs);
    dev->convertFromQImage(image, 0);
    QImage result = dev->convertToQImage(0, 0, 0, 640, 441);

    QPoint errpoint;

    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("kis_fixed_paint_device_test_test_roundtrip_qimage.png");
        result.save("kis_fixed_paint_device_test_test_roundtrip_result.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
}

void KisFixedPaintDeviceTest::testBltFixed()
{
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisFixedPaintDeviceSP fdev = new KisFixedPaintDevice(cs);
    fdev->convertFromQImage(image, 0);

    // Without opacity
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    KisPainter gc(dev);
    gc.bltFixed(QPoint(0, 0), fdev, image.rect());

    QImage result = dev->convertToQImage(0, 0, 0, 640, 441);

    QPoint errpoint;

    if (!TestUtil::compareQImages(errpoint, image, result)) {
        fdev->convertToQImage(0).save("kis_fixed_paint_device_test_test_blt_fixed_expected.png");
        result.save("kis_fixed_paint_device_test_test_blt_fixed_result.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }

}

void KisFixedPaintDeviceTest::testBltFixedOpacity()
{
    // blt a semi-transparent image on a white paint device

    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa_transparent.png");
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisFixedPaintDeviceSP fdev = new KisFixedPaintDevice(cs);
    fdev->convertFromQImage(image, 0);

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->fill(0, 0, 640, 441, KoColor(Qt::white, cs).data());
    KisPainter gc(dev);
    gc.bltFixed(QPoint(0, 0), fdev, image.rect());

    QImage result = dev->convertToQImage(0, 0, 0, 640, 441);
    QImage checkResult(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa_transparent_result.png");
    QPoint errpoint;

    if (!TestUtil::compareQImages(errpoint, checkResult, result)) {
        checkResult.save("kis_fixed_paint_device_test_test_blt_fixed_opactiy_expected.png");
        result.save("kis_fixed_paint_device_test_test_blt_fixed_opacity_result.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
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
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    } 
    
    delete[] red;
}

void KisFixedPaintDeviceTest::testBltFixedSmall()
{
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "fixed_blit_small.png");
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
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
}

void KisFixedPaintDeviceTest::testBltPerformance()
{
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa_transparent.png");
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisFixedPaintDeviceSP fdev = new KisFixedPaintDevice(cs);
    fdev->convertFromQImage(image, 0);

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->fill(0, 0, 640, 441, KoColor(Qt::white, cs).data());


    QTime t;
    t.start();

    int x;
    for (x = 0; x < 1000; ++x) {
        KisPainter gc(dev);
        gc.bltFixed(QPoint(0, 0), fdev, image.rect());
    }

    qDebug() << x
    << "blits"
    << " done in "
    << t.elapsed()
    << "ms";


}

QTEST_KDEMAIN(KisFixedPaintDeviceTest, GUI)
#include "kis_fixed_paint_device_test.moc"


