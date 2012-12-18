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

#include "kis_paint_device_test.h"
#include <qtest_kde.h>

#include <QTime>

#include <KoStore.h>
#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_painter.h"
#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_layer.h"
#include "kis_paint_layer.h"
#include "kis_selection.h"
#include "kis_datamanager.h"
#include "kis_global.h"
#include "testutil.h"
#include "kis_transaction.h"
#include "kis_image.h"

void KisPaintDeviceTest::testCreation()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    QVERIFY(dev->objectName().isEmpty());

    dev = new KisPaintDevice(cs);
    QVERIFY(*dev->colorSpace() == *cs);
    QVERIFY(dev->x() == 0);
    QVERIFY(dev->y() == 0);
    QVERIFY(dev->pixelSize() == cs->pixelSize());
    QVERIFY(dev->channelCount() == cs->channelCount());
    QVERIFY(dev->dataManager() != 0);

    KisImageSP image = new KisImage(0, 1000, 1000, cs, "merge test");
    KisPaintLayerSP layer = new KisPaintLayer(image, "bla", 125);

    dev = new KisPaintDevice(layer.data(), cs);
    QVERIFY(*dev->colorSpace() == *cs);
    QVERIFY(dev->x() == 0);
    QVERIFY(dev->y() == 0);
    QVERIFY(dev->pixelSize() == cs->pixelSize());
    QVERIFY(dev->channelCount() == cs->channelCount());
    QVERIFY(dev->dataManager() != 0);

    // Let the layer go out of scope and see what happens
    {
        KisPaintLayerSP l2 = new KisPaintLayer(image, "blabla", 250);
        dev = new KisPaintDevice(l2.data(), cs);
    }

}


void KisPaintDeviceTest::testStore()
{

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    KoStore * readStore =
        KoStore::createStore(QString(FILES_DATA_DIR) + QDir::separator() + "store_test.kra", KoStore::Read);
    readStore->open("built image/layers/layer0");
    QVERIFY(dev->read(readStore));
    readStore->close();
    delete readStore;

    QVERIFY(dev->exactBounds() == QRect(0, 0, 100, 100));

    KoStore * writeStore =
        KoStore::createStore(QString(FILES_OUTPUT_DIR) + QDir::separator() + "store_test_out.kra", KoStore::Write);
    writeStore->open("built image/layers/layer0");
    QVERIFY(dev->write(writeStore));
    writeStore->close();
    delete writeStore;

    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    readStore =
        KoStore::createStore(QString(FILES_OUTPUT_DIR) + QDir::separator() + "store_test_out.kra", KoStore::Read);
    readStore->open("built image/layers/layer0");
    QVERIFY(dev2->read(readStore));
    readStore->close();
    delete readStore;

    QVERIFY(dev2->exactBounds() == QRect(0, 0, 100, 100));

    QPoint pt;
    if (!TestUtil::comparePaintDevices(pt, dev, dev2)) {
        QFAIL(QString("Loading a saved image is not pixel perfect, first different pixel: %1,%2 ").arg(pt.x()).arg(pt.y()).toAscii());
    }

}

void KisPaintDeviceTest::testGeometry()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    quint8* pixel = cs->allocPixelBuffer(1);
    cs->fromQColor(Qt::white, pixel);
    dev->fill(0, 0, 512, 512, pixel);

    QCOMPARE(dev->exactBounds(), QRect(0, 0, 512, 512));
    QCOMPARE(dev->extent(), QRect(0, 0, 512, 512));

    dev->move(10, 10);

    QCOMPARE(dev->exactBounds(), QRect(10, 10, 512, 512));
    QCOMPARE(dev->extent(), QRect(10, 10, 512, 512));

    dev->crop(50, 50, 50, 50);
    QCOMPARE(dev->exactBounds(), QRect(50, 50, 50, 50));
    QCOMPARE(dev->extent(), QRect(10, 10, 128, 128));

    QColor c;

    dev->clear(QRect(50, 50, 50, 50));
    dev->pixel(80, 80, &c);
    QVERIFY(c.alpha() == OPACITY_TRANSPARENT_U8);

    dev->fill(0, 0, 512, 512, pixel);
    dev->pixel(80, 80, &c);
    QVERIFY(c == Qt::white);
    QVERIFY(c.alpha() == OPACITY_OPAQUE_U8);

    dev->clear();
    dev->pixel(80, 80, &c);
    QVERIFY(c.alpha() == OPACITY_TRANSPARENT_U8);

    QVERIFY(dev->extent().isEmpty());
    QVERIFY(dev->exactBounds().isEmpty());

}

void KisPaintDeviceTest::testClear()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    QVERIFY(dev->extent() == QRect(2147483647, 2147483647, 0, 0));
    QVERIFY(dev->exactBounds() == QRect(2147483647, 2147483647, 0, 0));

    dev->clear();

    QVERIFY(dev->extent() == QRect(2147483647, 2147483647, 0, 0));
    QVERIFY(dev->exactBounds() == QRect(2147483647, 2147483647, 0, 0));

    dev->clear(QRect(100, 100, 100, 100));

    // XXX: This is strange!
    QVERIFY(dev->extent() == QRect(64, 64, 192, 192));
    QVERIFY(dev->exactBounds() == QRect(64, 64, 192, 192));

    dev->clear();

    QVERIFY(dev->extent() == QRect(2147483647, 2147483647, 0, 0));
    QVERIFY(dev->exactBounds() == QRect(2147483647, 2147483647, 0, 0));

}

void KisPaintDeviceTest::testCrop()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    quint8* pixel = cs->allocPixelBuffer(1);
    cs->fromQColor(Qt::white, pixel);
    dev->fill(-14, 8, 433, 512, pixel);

    QVERIFY(dev->exactBounds() == QRect(-14, 8, 433, 512));

    // Crop inside
    dev->crop(50, 50, 150, 150);
    QVERIFY(dev->exactBounds() == QRect(50, 50, 150, 150));

    // Crop outside, pd should not grow
    dev->crop(0, 0, 1000, 1000);
    QVERIFY(dev->exactBounds() == QRect(50, 50, 150, 150));
}

void KisPaintDeviceTest::testRoundtripReadWrite()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "tile.png");
    dev->convertFromQImage(image, 0);
    quint8* bytes = cs->allocPixelBuffer(image.width() * image.height());
    memset(bytes, 0, image.width() * image.height() * dev->pixelSize());
    dev->readBytes(bytes, image.rect());

    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    dev2->writeBytes(bytes, image.rect());
    QVERIFY(dev2->exactBounds() == image.rect());

    dev2->convertToQImage(0, 0, 0, image.width(), image.height()).save("readwrite.png");


    QPoint pt;
    if (!TestUtil::comparePaintDevices(pt, dev, dev2)) {
        QFAIL(QString("Failed round trip using readBytes and writeBytes, first different pixel: %1,%2 ").arg(pt.x()).arg(pt.y()).toAscii());
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

void KisPaintDeviceTest::testColorSpaceConversion()
{
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "tile.png");
    const KoColorSpace* srcCs = KoColorSpaceRegistry::instance()->rgb8();
    const KoColorSpace* dstCs = KoColorSpaceRegistry::instance()->lab16();
    KisPaintDeviceSP dev = new KisPaintDevice(srcCs);
    dev->convertFromQImage(image, 0);
    dev->move(10, 10);   // Unalign with tile boundaries
    KUndo2Command* cmd = dev->convertTo(dstCs);

    QVERIFY(dev->exactBounds() == QRect(10, 10, image.width(), image.height()));
    QVERIFY(dev->pixelSize() == dstCs->pixelSize());
    QVERIFY(*dev->colorSpace() == *dstCs);

    delete cmd;
}


void KisPaintDeviceTest::testRoundtripConversion()
{
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);
    QImage result = dev->convertToQImage(0, 0, 0, 640, 441);

    QPoint errpoint;

    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("kis_paint_device_test_test_roundtrip_qimage.png");
        result.save("kis_paint_device_test_test_roundtrip_result.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
}

void KisPaintDeviceTest::testFastBitBlt()
{
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dstDev = new KisPaintDevice(cs);
    KisPaintDeviceSP srcDev = new KisPaintDevice(cs);
    srcDev->convertFromQImage(image, 0);

    QRect cloneRect(100,100,200,200);
    QPoint errpoint;


    QVERIFY(dstDev->fastBitBltPossible(srcDev));
    dstDev->fastBitBlt(srcDev, cloneRect);

    QImage srcImage = srcDev->convertToQImage(0, cloneRect.x(), cloneRect.y(),
                                               cloneRect.width(), cloneRect.height());
    QImage dstImage = dstDev->convertToQImage(0, cloneRect.x(), cloneRect.y(),
                                              cloneRect.width(), cloneRect.height());

    if (!TestUtil::compareQImages(errpoint, srcImage, dstImage)) {
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }

    // Test Rough version
    dstDev->clear();
    dstDev->fastBitBltRough(srcDev, cloneRect);

    srcImage = srcDev->convertToQImage(0, cloneRect.x(), cloneRect.y(),
                                       cloneRect.width(), cloneRect.height());
    dstImage = dstDev->convertToQImage(0, cloneRect.x(), cloneRect.y(),
                                       cloneRect.width(), cloneRect.height());

    if (!TestUtil::compareQImages(errpoint, srcImage, dstImage)) {
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }

    srcDev->move(10,10);
    QVERIFY(!dstDev->fastBitBltPossible(srcDev));
}

void KisPaintDeviceTest::testMakeClone()
{
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP srcDev = new KisPaintDevice(cs);
    srcDev->convertFromQImage(image, 0);
    srcDev->move(10,10);

    const KoColorSpace * weirdCS = KoColorSpaceRegistry::instance()->lab16();
    KisPaintDeviceSP dstDev = new KisPaintDevice(weirdCS);
    dstDev->move(1000,1000);

    QVERIFY(!dstDev->fastBitBltPossible(srcDev));

    QRect cloneRect(100,100,200,200);
    QPoint errpoint;

    dstDev->makeCloneFrom(srcDev, cloneRect);

    QVERIFY(*dstDev->colorSpace() == *srcDev->colorSpace());
    QCOMPARE(dstDev->pixelSize(), srcDev->pixelSize());
    QCOMPARE(dstDev->x(), srcDev->x());
    QCOMPARE(dstDev->y(), srcDev->y());
    QCOMPARE(dstDev->exactBounds(), cloneRect);

    QImage srcImage = srcDev->convertToQImage(0, cloneRect.x(), cloneRect.y(),
                                              cloneRect.width(), cloneRect.height());
    QImage dstImage = dstDev->convertToQImage(0, cloneRect.x(), cloneRect.y(),
                                              cloneRect.width(), cloneRect.height());
    if (!TestUtil::compareQImages(errpoint, dstImage, srcImage)) {
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
}

void KisPaintDeviceTest::testThumbnail()
{
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);
    {
        KisPaintDeviceSP thumb = dev->createThumbnailDevice(50, 50);
        QRect rc = thumb->exactBounds();
        QVERIFY(rc.width() <= 50);
        QVERIFY(rc.height() <= 50);
    }
    {
        QImage thumb = dev->createThumbnail(50, 50);
        QVERIFY(!thumb.isNull());
        QVERIFY(thumb.width() <= 50);
        QVERIFY(thumb.height() <= 50);
        image.save("kis_paint_device_test_test_thumbnail.png");
    }
}

void KisPaintDeviceTest::testCaching()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    quint8* whitePixel = cs->allocPixelBuffer(1);
    cs->fromQColor(Qt::white, whitePixel);

    quint8* blackPixel = cs->allocPixelBuffer(1);
    cs->fromQColor(Qt::black, blackPixel);

    dev->fill(0, 0, 512, 512, whitePixel);
    QImage thumb1 = dev->createThumbnail(50, 50);
    QRect exactBounds1 = dev->exactBounds();

    dev->fill(0, 0, 768, 768, blackPixel);
    QImage thumb2 = dev->createThumbnail(50, 50);
    QRect exactBounds2 = dev->exactBounds();

    dev->move(10, 10);
    QImage thumb3 = dev->createThumbnail(50, 50);
    QRect exactBounds3 = dev->exactBounds();

    dev->crop(50, 50, 50, 50);
    QImage thumb4 = dev->createThumbnail(50, 50);
    QRect exactBounds4 = dev->exactBounds();

    QVERIFY(thumb1 != thumb2);
    QVERIFY(thumb2 == thumb3); // Cache miss, but image is the same
    QVERIFY(thumb3 != thumb4);
    QVERIFY(thumb4 != thumb1);

    QCOMPARE(exactBounds1, QRect(0,0,512,512));
    QCOMPARE(exactBounds2, QRect(0,0,768,768));
    QCOMPARE(exactBounds3, QRect(10,10,768,768));
    QCOMPARE(exactBounds4, QRect(50,50,50,50));
}

void KisPaintDeviceTest::testRegion()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    quint8* whitePixel = cs->allocPixelBuffer(1);
    cs->fromQColor(Qt::white, whitePixel);

    dev->fill(0, 0, 10, 10, whitePixel);
    dev->fill(70, 70, 10, 10, whitePixel);
    dev->fill(129, 0, 10, 10, whitePixel);
    dev->fill(0, 1030, 10, 10, whitePixel);

    QRegion referenceRegion;
    referenceRegion += QRect(0,0,64,64);
    referenceRegion += QRect(64,64,64,64);
    referenceRegion += QRect(128,0,64,64);
    referenceRegion += QRect(0,1024,64,64);

    QCOMPARE(dev->exactBounds(), QRect(0,0,139,1040));
    QCOMPARE(dev->region(), referenceRegion);
}

void KisPaintDeviceTest::testPixel()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    QColor c = Qt::red;
    quint8 opacity = 125;

    c.setAlpha(opacity);
    dev->setPixel(5, 5, c);

    QColor c2;

    dev->pixel(5, 5, &c2);

    QVERIFY(c == c2);
    QVERIFY(opacity == c2.alpha());

}

void KisPaintDeviceTest::testPlanarReadWrite()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    quint8* pixel = cs->allocPixelBuffer(1);
    cs->fromQColor(QColor(255, 200, 155, 100), pixel);
    dev->fill(0, 0, 5000, 5000, pixel);
    delete[] pixel;

    QColor c1;
    dev->pixel(5, 5, &c1);

    QVector<quint8*> planes = dev->readPlanarBytes(500, 500, 100, 100);
    QVector<quint8*> swappedPlanes;

    QCOMPARE((int)planes.size(), (int)dev->channelCount());

    for (int i = 0; i < 100*100; i++) {
        // BGRA encoded
        QVERIFY(planes.at(2)[i] == 255);
        QVERIFY(planes.at(1)[i] == 200);
        QVERIFY(planes.at(0)[i] == 155);
        QVERIFY(planes.at(3)[i] == 100);
    }

    for (uint i = 1; i < dev->channelCount() + 1; ++i) {
        swappedPlanes.append(planes[dev->channelCount() - i]);
    }

    dev->writePlanarBytes(swappedPlanes, 0, 0, 100, 100);

    dev->convertToQImage(0, 0, 0, 5000, 5000).save("planar.png");

    qDeleteAll(planes);
    swappedPlanes.clear();

    dev->pixel(5, 5, &c1);

    QVERIFY(c1.red() == 200);
    QVERIFY(c1.green() == 255);
    QVERIFY(c1.blue() == 100);
    QVERIFY(c1.alpha() == 155);

    dev->pixel(75, 50, &c1);

    QVERIFY(c1.red() == 200);
    QVERIFY(c1.green() == 255);
    QVERIFY(c1.blue() == 100);
    QVERIFY(c1.alpha() == 155);
}

void KisPaintDeviceTest::testBltPerformance()
{
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa_transparent.png");
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP fdev = new KisPaintDevice(cs);
    fdev->convertFromQImage(image, 0);

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->fill(0, 0, 640, 441, KoColor(Qt::white, cs).data());

    QTime t;
    t.start();

    int x;
    for (x = 0; x < 1000; ++x) {
        KisPainter gc(dev);
        gc.bitBlt(QPoint(0, 0), fdev, image.rect());
    }

    qDebug() << x
    << "blits"
    << " done in "
    << t.elapsed()
    << "ms";


}

void KisPaintDeviceTest::testDeviceDuplication()
{
    QRect fillRect(0,0,64,64);
    quint8 fillPixel[4]={255,255,255,255};
    QRect clearRect(10,10,20,20);
    QImage referenceImage;
    QImage resultImage;

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP device = new KisPaintDevice(cs);

//    qDebug()<<"FILLING";
    device->fill(fillRect.left(), fillRect.top(),
                 fillRect.width(), fillRect.height(),fillPixel);
    referenceImage = device->convertToQImage(0);


    KisTransaction transaction1(0, device);
//    qDebug()<<"CLEARING";
    device->clear(clearRect);

    transaction1.revert();

    resultImage = device->convertToQImage(0);
    QVERIFY(resultImage == referenceImage);

    KisPaintDeviceSP clone =  new KisPaintDevice(*device);

    KisTransaction transaction(0, clone);
//    qDebug()<<"CLEARING";
    clone->clear(clearRect);

    transaction.revert();

    resultImage = clone->convertToQImage(0);
    QVERIFY(resultImage == referenceImage);

}

void KisPaintDeviceTest::testSharedDataManager()
{
    QRect fillRect(0,0,100,100);
    quint8 fillPixel[4]={255,255,255,255};
    QRect clearRect(10,10,20,20);
    QImage srcImage;
    QImage dstImage;

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP srcDevice = new KisPaintDevice(cs);

    srcDevice->setX(10);
    srcDevice->setY(20);

    srcDevice->fill(fillRect.left(), fillRect.top(),
                 fillRect.width(), fillRect.height(),fillPixel);

    KisPaintDeviceSP dstDevice = new KisPaintDevice(srcDevice->dataManager(), srcDevice);


    QVERIFY(srcDevice->extent() == dstDevice->extent());
    QVERIFY(srcDevice->exactBounds() == dstDevice->exactBounds());
    QVERIFY(srcDevice->defaultBounds() == dstDevice->defaultBounds());
    QVERIFY(srcDevice->x() == dstDevice->x());
    QVERIFY(srcDevice->y() == dstDevice->y());

    srcImage = srcDevice->convertToQImage(0);
    dstImage = dstDevice->convertToQImage(0);
    QVERIFY(srcImage == dstImage);

    srcDevice->clear(clearRect);

    srcImage = srcDevice->convertToQImage(0);
    dstImage = dstDevice->convertToQImage(0);
    QVERIFY(srcImage == dstImage);
}

void KisPaintDeviceTest::testTranslate()
{
    QRect fillRect(0,0,64,64);
    quint8 fillPixel[4]={255,255,255,255};

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP device = new KisPaintDevice(cs);

    device->fill(fillRect.left(), fillRect.top(),
                 fillRect.width(), fillRect.height(),fillPixel);

    device->setX(-10);
    device->setY(10);
    QCOMPARE(device->exactBounds(), QRect(-10,10,64,64));
    QCOMPARE(device->extent(), QRect(-10,10,64,64));
}

void KisPaintDeviceTest::testOpacity()
{
    // blt a semi-transparent image on a white paint device

    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa_transparent.png");
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP fdev = new KisPaintDevice(cs);
    fdev->convertFromQImage(image, 0);

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->fill(0, 0, 640, 441, KoColor(Qt::white, cs).data());
    KisPainter gc(dev);
    gc.bitBlt(QPoint(0, 0), fdev, image.rect());

    QImage result = dev->convertToQImage(0, 0, 0, 640, 441);
    QImage checkResult(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa_transparent_result.png");
    QPoint errpoint;

    if (!TestUtil::compareQImages(errpoint, checkResult, result, 1)) {
        checkResult.save("kis_paint_device_test_test_blt_fixed_opactiy_expected.png");
        result.save("kis_paint_device_test_test_blt_fixed_opacity_result.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
}

void KisPaintDeviceTest::testExactBoundsWeirdNullAlphaCase()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    QVERIFY(dev->exactBounds().isEmpty());

    dev->fill(QRect(10,10,10,10), KoColor(Qt::white, cs));

    QCOMPARE(dev->exactBounds(), QRect(10,10,10,10));

    const quint8 weirdPixelData[4] = {0,10,0,0};
    KoColor weirdColor(weirdPixelData, cs);
    dev->setPixel(6,6,weirdColor);

    // such weird pixels should not change our opinion about
    // device's size
    QCOMPARE(dev->exactBounds(), QRect(10,10,10,10));
}

void KisPaintDeviceTest::benchmarkExactBoundsNullDefaultPixel()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    QVERIFY(dev->exactBounds().isEmpty());

    QRect fillRect(60,60, 1930, 1930);

    dev->fill(fillRect, KoColor(Qt::white, cs));

    QRect measuredRect;

    QBENCHMARK {
        // invalidate the cache
        dev->setDirty();
        measuredRect = dev->exactBounds();
    }

    QCOMPARE(measuredRect, fillRect);
}


QTEST_KDEMAIN(KisPaintDeviceTest, GUI)
#include "kis_paint_device_test.moc"


