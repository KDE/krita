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
#include <QTest>

#include <QTime>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoStore.h>

#include "kis_paint_device_writer.h"
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
#include "config-limit-long-tests.h"
#include "kistest.h"


class KisFakePaintDeviceWriter : public KisPaintDeviceWriter {
public:
    KisFakePaintDeviceWriter(KoStore *store)
        : m_store(store)
    {
    }

    bool write(const QByteArray &data) override {
        return (m_store->write(data) == data.size());
    }

    bool write(const char* data, qint64 length) override {
        return (m_store->write(data, length) == length);
    }

    KoStore *m_store;
};

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
    QVERIFY(dev->read(readStore->device()));
    readStore->close();
    delete readStore;

    QVERIFY(dev->exactBounds() == QRect(0, 0, 100, 100));

    KoStore * writeStore =
        KoStore::createStore(QString(FILES_OUTPUT_DIR) + QDir::separator() + "store_test_out.kra", KoStore::Write);
    KisFakePaintDeviceWriter fakeWriter(writeStore);
    writeStore->open("built image/layers/layer0");
    QVERIFY(dev->write(fakeWriter));
    writeStore->close();
    delete writeStore;

    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    readStore =
        KoStore::createStore(QString(FILES_OUTPUT_DIR) + QDir::separator() + "store_test_out.kra", KoStore::Read);
    readStore->open("built image/layers/layer0");
    QVERIFY(dev2->read(readStore->device()));
    readStore->close();
    delete readStore;

    QVERIFY(dev2->exactBounds() == QRect(0, 0, 100, 100));

    QPoint pt;
    if (!TestUtil::comparePaintDevices(pt, dev, dev2)) {
        QFAIL(QString("Loading a saved image is not pixel perfect, first different pixel: %1,%2 ").arg(pt.x()).arg(pt.y()).toLatin1());
    }

}

void KisPaintDeviceTest::testGeometry()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    quint8* pixel = new quint8[cs->pixelSize()];
    cs->fromQColor(Qt::white, pixel);
    dev->fill(0, 0, 512, 512, pixel);

    QCOMPARE(dev->exactBounds(), QRect(0, 0, 512, 512));
    QCOMPARE(dev->extent(), QRect(0, 0, 512, 512));

    dev->moveTo(10, 10);

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

    QVERIFY(!dev->extent().isValid());
    QVERIFY(!dev->exactBounds().isValid());

    dev->clear();

    QVERIFY(!dev->extent().isValid());
    QVERIFY(!dev->exactBounds().isValid());

    QRect fillRect1(50, 100, 150, 100);
    dev->fill(fillRect1, KoColor(Qt::red, cs));

    QCOMPARE(dev->extent(), QRect(0, 64, 256, 192));
    QCOMPARE(dev->exactBounds(), fillRect1);

    dev->clear(QRect(100, 100, 100, 100));

    QCOMPARE(dev->extent(), QRect(0, 64, 256, 192));
    QCOMPARE(dev->exactBounds(), QRect(50, 100, 50, 100));

    dev->clear();

    QVERIFY(!dev->extent().isValid());
    QVERIFY(!dev->exactBounds().isValid());
}

void KisPaintDeviceTest::testCrop()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    quint8* pixel = new quint8[cs->pixelSize()];
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

    quint8* bytes = new quint8[cs->pixelSize() * image.width() * image.height()];
    memset(bytes, 0, image.width() * image.height() * dev->pixelSize());
    dev->readBytes(bytes, image.rect());

    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    dev2->writeBytes(bytes, image.rect());
    QVERIFY(dev2->exactBounds() == image.rect());

    dev2->convertToQImage(0, 0, 0, image.width(), image.height()).save("readwrite.png");


    QPoint pt;
    if (!TestUtil::comparePaintDevices(pt, dev, dev2)) {
        QFAIL(QString("Failed round trip using readBytes and writeBytes, first different pixel: %1,%2 ").arg(pt.x()).arg(pt.y()).toLatin1());
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

void KisPaintDeviceTest::testColorSpaceConversion()
{
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "tile.png");
    const KoColorSpace* srcCs = KoColorSpaceRegistry::instance()->rgb8();
    const KoColorSpace* dstCs = KoColorSpaceRegistry::instance()->lab16();
    KisPaintDeviceSP dev = new KisPaintDevice(srcCs);
    dev->convertFromQImage(image, 0);
    dev->moveTo(10, 10);   // Unalign with tile boundaries
    KUndo2Command* cmd = dev->convertTo(dstCs);

    QCOMPARE(dev->exactBounds(), QRect(10, 10, image.width(), image.height()));
    QCOMPARE(dev->pixelSize(), dstCs->pixelSize());
    QVERIFY(*dev->colorSpace() == *dstCs);

    cmd->redo();
    cmd->undo();

    QCOMPARE(dev->exactBounds(), QRect(10, 10, image.width(), image.height()));
    QCOMPARE(dev->pixelSize(), srcCs->pixelSize());
    QVERIFY(*dev->colorSpace() == *srcCs);

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
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
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
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }

    // Test Rough version
    dstDev->clear();
    dstDev->fastBitBltRough(srcDev, cloneRect);

    srcImage = srcDev->convertToQImage(0, cloneRect.x(), cloneRect.y(),
                                       cloneRect.width(), cloneRect.height());
    dstImage = dstDev->convertToQImage(0, cloneRect.x(), cloneRect.y(),
                                       cloneRect.width(), cloneRect.height());

    if (!TestUtil::compareQImages(errpoint, srcImage, dstImage)) {
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }

    srcDev->moveTo(10,10);
    QVERIFY(!dstDev->fastBitBltPossible(srcDev));
}

void KisPaintDeviceTest::testMakeClone()
{
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP srcDev = new KisPaintDevice(cs);
    srcDev->convertFromQImage(image, 0);
    srcDev->moveTo(10,10);

    const KoColorSpace * weirdCS = KoColorSpaceRegistry::instance()->lab16();
    KisPaintDeviceSP dstDev = new KisPaintDevice(weirdCS);
    dstDev->moveTo(1000,1000);

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
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
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

void KisPaintDeviceTest::testThumbnailDeviceWithOffset()
{
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);
    dev->setX(10);
    dev->setY(10);

    QImage thumb = dev->createThumbnail(640,441,QRect(10,10,640,441));

    image.save("oring.png");
    thumb.save("thumb.png");

    QPoint pt;
    QVERIFY(TestUtil::compareQImages(pt, thumb, image));
}

void KisPaintDeviceTest::testCaching()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    quint8* whitePixel = new quint8[cs->pixelSize()];
    cs->fromQColor(Qt::white, whitePixel);

    quint8* blackPixel = new quint8[cs->pixelSize()];
    cs->fromQColor(Qt::black, blackPixel);

    dev->fill(0, 0, 512, 512, whitePixel);
    QImage thumb1 = dev->createThumbnail(50, 50);
    QRect exactBounds1 = dev->exactBounds();

    dev->fill(0, 0, 768, 768, blackPixel);
    QImage thumb2 = dev->createThumbnail(50, 50);
    QRect exactBounds2 = dev->exactBounds();

    dev->moveTo(10, 10);
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

    quint8* whitePixel = new quint8[cs->pixelSize()];
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

    quint8* pixel = new quint8[cs->pixelSize()];
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

    dev->convertToQImage(0, 0, 0, 1000, 1000).save("planar.png");

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

    // check if one of the planes is Null.
    Q_ASSERT(planes.size() == 4);
    delete [] planes[2];
    planes[2] = 0;
    dev->writePlanarBytes(planes, 0, 0, 100, 100);
    dev->convertToQImage(0, 0, 0, 1000, 1000).save("planar_noR.png");

    dev->pixel(75, 50, &c1);

    QCOMPARE(c1.red(), 200);
    QCOMPARE(c1.green(), 200);
    QCOMPARE(c1.blue(), 155);
    QCOMPARE(c1.alpha(), 100);

    QVector<quint8*>::iterator i;
    for (i = planes.begin(); i != planes.end(); ++i)
    {
        delete [] *i;
    }
    swappedPlanes.clear();
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
#ifdef LIMIT_LONG_TESTS
    int steps = 10;
#else
    int steps = 1000;
#endif
    for (x = 0; x < steps; ++x) {
        KisPainter gc(dev);
        gc.bitBlt(QPoint(0, 0), fdev, image.rect());
    }

    dbgKrita << x
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

//    dbgKrita<<"FILLING";
    device->fill(fillRect.left(), fillRect.top(),
                 fillRect.width(), fillRect.height(),fillPixel);
    referenceImage = device->convertToQImage(0);


    KisTransaction transaction1(device);
//    dbgKrita<<"CLEARING";
    device->clear(clearRect);

    transaction1.revert();

    resultImage = device->convertToQImage(0);
    QVERIFY(resultImage == referenceImage);

    KisPaintDeviceSP clone =  new KisPaintDevice(*device);

    KisTransaction transaction(clone);
//    dbgKrita<<"CLEARING";
    clone->clear(clearRect);

    transaction.revert();

    resultImage = clone->convertToQImage(0);
    QVERIFY(resultImage == referenceImage);

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
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
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

void KisPaintDeviceTest::testAmortizedExactBounds()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    QVERIFY(dev->exactBounds().isEmpty());

    QRect fillRect(60,60, 833, 833);
    QRect extent(0,0,896,896);

    dev->fill(fillRect, KoColor(Qt::white, cs));

    QEXPECT_FAIL("", "Expecting the extent, we somehow get the fillrect", Continue);
    QCOMPARE(dev->exactBounds(), extent);
    QCOMPARE(dev->extent(), extent);

    QCOMPARE(dev->exactBoundsAmortized(), fillRect);

    dev->setDirty();
    QEXPECT_FAIL("", "Expecting the fillRect, we somehow get the extent", Continue);
    QCOMPARE(dev->exactBoundsAmortized(), fillRect);

    dev->setDirty();
    QCOMPARE(dev->exactBoundsAmortized(), extent);

    QTest::qSleep(1100);
    QEXPECT_FAIL("", "Expecting the fillRect, we somehow get the extent", Continue);
    QCOMPARE(dev->exactBoundsAmortized(), fillRect);
}

void KisPaintDeviceTest::testNonDefaultPixelArea()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    QVERIFY(dev->exactBounds().isEmpty());
    QVERIFY(dev->nonDefaultPixelArea().isEmpty());

    KoColor defPixel(Qt::red, cs);
    dev->setDefaultPixel(defPixel);

    QCOMPARE(dev->exactBounds(), KisDefaultBounds::infiniteRect);
    QVERIFY(dev->nonDefaultPixelArea().isEmpty());

    QRect fillRect(10,11,18,14);

    dev->fill(fillRect, KoColor(Qt::white, cs));

    QCOMPARE(dev->exactBounds(), KisDefaultBounds::infiniteRect);
    QCOMPARE(dev->nonDefaultPixelArea(), fillRect);


    // non-default pixel variant should also handle weird pixels

    const quint8 weirdPixelData[4] = {0,10,0,0};
    KoColor weirdColor(weirdPixelData, cs);
    dev->setPixel(100,100,weirdColor);

    // such weird pixels should not change our opinion about
    // device's size
    QCOMPARE(dev->exactBounds(), KisDefaultBounds::infiniteRect);
    QCOMPARE(dev->nonDefaultPixelArea(), fillRect | QRect(100,100,1,1));
}

void KisPaintDeviceTest::testExactBoundsNonTransparent()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 1000, 1000, cs, "merge test");
    KisPaintLayerSP layer = new KisPaintLayer(image, "bla", 125);
    KisPaintDeviceSP dev = layer->paintDevice();

    QVERIFY(dev);

    QRect imageRect(0,0,1000,1000);

    KoColor defPixel(Qt::red, cs);
    dev->setDefaultPixel(defPixel);

    QCOMPARE(dev->exactBounds(), imageRect);
    QVERIFY(dev->nonDefaultPixelArea().isEmpty());

    KoColor fillPixel(Qt::white, cs);

    dev->fill(imageRect, KoColor(Qt::white, cs));
    QCOMPARE(dev->exactBounds(), imageRect);
    QCOMPARE(dev->nonDefaultPixelArea(), imageRect);

    dev->fill(QRect(1000,0, 1, 1000), KoColor(Qt::white, cs));
    QCOMPARE(dev->exactBounds(), QRect(0,0,1001,1000));
    QCOMPARE(dev->nonDefaultPixelArea(), QRect(0,0,1001,1000));

    dev->fill(QRect(0,1000, 1000, 1), KoColor(Qt::white, cs));
    QCOMPARE(dev->exactBounds(), QRect(0,0,1001,1001));
    QCOMPARE(dev->nonDefaultPixelArea(), QRect(0,0,1001,1001));

    dev->fill(QRect(0,-1, 1000, 1), KoColor(Qt::white, cs));
    QCOMPARE(dev->exactBounds(), QRect(0,-1,1001,1002));
    QCOMPARE(dev->nonDefaultPixelArea(), QRect(0,-1,1001,1002));

    dev->fill(QRect(-1,0, 1, 1000), KoColor(Qt::white, cs));
    QCOMPARE(dev->exactBounds(), QRect(-1,-1,1002,1002));
    QCOMPARE(dev->nonDefaultPixelArea(), QRect(-1,-1,1002,1002));
}

KisPaintDeviceSP createWrapAroundPaintDevice(const KoColorSpace *cs)
{
    struct TestingDefaultBounds : public KisDefaultBoundsBase {
        QRect bounds() const override {
            return QRect(0,0,20,20);
        }
        bool wrapAroundMode() const override {
            return true;
        }
        int currentLevelOfDetail() const override {
            return 0;
        }
        int currentTime() const override {
            return 0;
        }
        bool externalFrameActive() const override {
            return false;
        }
    };

    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    KisDefaultBoundsBaseSP bounds = new TestingDefaultBounds();
    dev->setDefaultBounds(bounds);

    return dev;
}

void checkReadWriteRoundTrip(KisPaintDeviceSP dev,
                             const QRect &rc)
{
    KisPaintDeviceSP deviceCopy = new KisPaintDevice(*dev.data());

    QRect readRect(10, 10, 20, 20);
    int bufSize = rc.width() * rc.height() * dev->pixelSize();

    QScopedArrayPointer<quint8> buf1(new quint8[bufSize]);

    deviceCopy->readBytes(buf1.data(), rc);

    deviceCopy->clear();
    QVERIFY(deviceCopy->extent().isEmpty());


    QScopedArrayPointer<quint8> buf2(new quint8[bufSize]);
    deviceCopy->writeBytes(buf1.data(), rc);
    deviceCopy->readBytes(buf2.data(), rc);

    QVERIFY(!memcmp(buf1.data(), buf2.data(), bufSize));
}


void KisPaintDeviceTest::testReadBytesWrapAround()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = createWrapAroundPaintDevice(cs);

    KoColor c1(Qt::red, cs);
    KoColor c2(Qt::green, cs);

    dev->setPixel(3, 3, c1);
    dev->setPixel(18, 18, c2);

    const int pixelSize = dev->pixelSize();

    {
        QRect readRect(10, 10, 20, 20);
        QScopedArrayPointer<quint8> buf(new quint8[readRect.width() *
                                              readRect.height() *
                                              pixelSize]);
        dev->readBytes(buf.data(), readRect);
        //dev->convertToQImage(0, readRect.x(), readRect.y(), readRect.width(), readRect.height()).save("final1.png");

        QVERIFY(memcmp(buf.data() + (7 + readRect.width() * 7) * pixelSize, c2.data(), pixelSize));
        QVERIFY(!memcmp(buf.data() + (8 + readRect.width() * 8) * pixelSize, c2.data(), pixelSize));

        QVERIFY(memcmp(buf.data() + (12 + readRect.width() * 12) * pixelSize, c1.data(), pixelSize));
        QVERIFY(!memcmp(buf.data() + (13 + readRect.width() * 13) * pixelSize, c1.data(), pixelSize));

        checkReadWriteRoundTrip(dev, readRect);
    }

    {
        // check weird case when the read rect is larger than wrap rect
        QRect readRect(10, 10, 30, 30);
        QScopedArrayPointer<quint8> buf(new quint8[readRect.width() *
                                              readRect.height() *
                                              pixelSize]);
        dev->readBytes(buf.data(), readRect);
        //dev->convertToQImage(0, readRect.x(), readRect.y(), readRect.width(), readRect.height()).save("final2.png");

        QVERIFY(memcmp(buf.data() + (7 + readRect.width() * 7) * pixelSize, c2.data(), pixelSize));
        QVERIFY(!memcmp(buf.data() + (8 + readRect.width() * 8) * pixelSize, c2.data(), pixelSize));

        QVERIFY(memcmp(buf.data() + (12 + readRect.width() * 12) * pixelSize, c1.data(), pixelSize));
        QVERIFY(!memcmp(buf.data() + (13 + readRect.width() * 13) * pixelSize, c1.data(), pixelSize));

        QVERIFY(memcmp(buf.data() + (27 + readRect.width() * 7) * pixelSize, c2.data(), pixelSize));
        QVERIFY(!memcmp(buf.data() + (28 + readRect.width() * 8) * pixelSize, c2.data(), pixelSize));

        QVERIFY(memcmp(buf.data() + (7 + readRect.width() * 27) * pixelSize, c2.data(), pixelSize));
        QVERIFY(!memcmp(buf.data() + (8 + readRect.width() * 28) * pixelSize, c2.data(), pixelSize));

        QVERIFY(memcmp(buf.data() + (27 + readRect.width() * 27) * pixelSize, c2.data(), pixelSize));
        QVERIFY(!memcmp(buf.data() + (28 + readRect.width() * 28) * pixelSize, c2.data(), pixelSize));

        checkReadWriteRoundTrip(dev, readRect);
    }

    {
        // even more large
        QRect readRect(10, 10, 40, 40);
        QScopedArrayPointer<quint8> buf(new quint8[readRect.width() *
                                              readRect.height() *
                                              pixelSize]);
        dev->readBytes(buf.data(), readRect);
        //dev->convertToQImage(0, readRect.x(), readRect.y(), readRect.width(), readRect.height()).save("final3.png");

        QVERIFY(memcmp(buf.data() + (7 + readRect.width() * 7) * pixelSize, c2.data(), pixelSize));
        QVERIFY(!memcmp(buf.data() + (8 + readRect.width() * 8) * pixelSize, c2.data(), pixelSize));

        QVERIFY(memcmp(buf.data() + (12 + readRect.width() * 12) * pixelSize, c1.data(), pixelSize));
        QVERIFY(!memcmp(buf.data() + (13 + readRect.width() * 13) * pixelSize, c1.data(), pixelSize));

        QVERIFY(memcmp(buf.data() + (27 + readRect.width() * 7) * pixelSize, c2.data(), pixelSize));
        QVERIFY(!memcmp(buf.data() + (28 + readRect.width() * 8) * pixelSize, c2.data(), pixelSize));

        QVERIFY(memcmp(buf.data() + (7 + readRect.width() * 27) * pixelSize, c2.data(), pixelSize));
        QVERIFY(!memcmp(buf.data() + (8 + readRect.width() * 28) * pixelSize, c2.data(), pixelSize));

        QVERIFY(memcmp(buf.data() + (27 + readRect.width() * 27) * pixelSize, c2.data(), pixelSize));
        QVERIFY(!memcmp(buf.data() + (28 + readRect.width() * 28) * pixelSize, c2.data(), pixelSize));

        QVERIFY(memcmp(buf.data() + (32 + readRect.width() * 12) * pixelSize, c1.data(), pixelSize));
        QVERIFY(!memcmp(buf.data() + (33 + readRect.width() * 13) * pixelSize, c1.data(), pixelSize));

        QVERIFY(memcmp(buf.data() + (12 + readRect.width() * 32) * pixelSize, c1.data(), pixelSize));
        QVERIFY(!memcmp(buf.data() + (13 + readRect.width() * 33) * pixelSize, c1.data(), pixelSize));

        QVERIFY(memcmp(buf.data() + (32 + readRect.width() * 32) * pixelSize, c1.data(), pixelSize));
        QVERIFY(!memcmp(buf.data() + (33 + readRect.width() * 33) * pixelSize, c1.data(), pixelSize));

        checkReadWriteRoundTrip(dev, readRect);
    }

    {
        // check if the wrap rect contains the read rect entirely
        QRect readRect(1, 1, 10, 10);
        QScopedArrayPointer<quint8> buf(new quint8[readRect.width() *
                                              readRect.height() *
                                              pixelSize]);
        dev->readBytes(buf.data(), readRect);
        //dev->convertToQImage(0, readRect.x(), readRect.y(), readRect.width(), readRect.height()).save("final4.png");

        QVERIFY(memcmp(buf.data() + (1 + readRect.width() * 1) * pixelSize, c1.data(), pixelSize));
        QVERIFY(!memcmp(buf.data() + (2 + readRect.width() * 2) * pixelSize, c1.data(), pixelSize));

        checkReadWriteRoundTrip(dev, readRect);
    }

    {
        // check if the wrap happens only on vertical side of the rect
        QRect readRect(1, 1, 29, 10);
        QScopedArrayPointer<quint8> buf(new quint8[readRect.width() *
                                              readRect.height() *
                                              pixelSize]);
        dev->readBytes(buf.data(), readRect);
        //dev->convertToQImage(0, readRect.x(), readRect.y(), readRect.width(), readRect.height()).save("final5.png");

        QVERIFY(memcmp(buf.data() + (1 + readRect.width() * 1) * pixelSize, c1.data(), pixelSize));
        QVERIFY(!memcmp(buf.data() + (2 + readRect.width() * 2) * pixelSize, c1.data(), pixelSize));

        QVERIFY(memcmp(buf.data() + (21 + readRect.width() * 1) * pixelSize, c1.data(), pixelSize));
        QVERIFY(!memcmp(buf.data() + (22 + readRect.width() * 2) * pixelSize, c1.data(), pixelSize));

        checkReadWriteRoundTrip(dev, readRect);
    }

    {
        // check if the wrap happens only on horizontal side of the rect
        QRect readRect(1, 1, 10, 29);
        QScopedArrayPointer<quint8> buf(new quint8[readRect.width() *
                                              readRect.height() *
                                              pixelSize]);
        dev->readBytes(buf.data(), readRect);
        //dev->convertToQImage(0, readRect.x(), readRect.y(), readRect.width(), readRect.height()).save("final6.png");

        QVERIFY(memcmp(buf.data() + (1 + readRect.width() * 1) * pixelSize, c1.data(), pixelSize));
        QVERIFY(!memcmp(buf.data() + (2 + readRect.width() * 2) * pixelSize, c1.data(), pixelSize));

        QVERIFY(memcmp(buf.data() + (1 + readRect.width() * 21) * pixelSize, c1.data(), pixelSize));
        QVERIFY(!memcmp(buf.data() + (2 + readRect.width() * 22) * pixelSize, c1.data(), pixelSize));

        checkReadWriteRoundTrip(dev, readRect);
    }
}

#include "kis_random_accessor_ng.h"

void KisPaintDeviceTest::testWrappedRandomAccessor()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = createWrapAroundPaintDevice(cs);

    KoColor c1(Qt::red, cs);
    KoColor c2(Qt::green, cs);

    dev->setPixel(3, 3, c1);
    dev->setPixel(18, 18, c2);

    const int pixelSize = dev->pixelSize();

    int x;
    int y;

    x = 3;
    y = 3;
    KisRandomAccessorSP dstIt = dev->createRandomAccessorNG(x, y);

    QVERIFY(!memcmp(dstIt->rawData(), c1.data(), pixelSize));
    QCOMPARE(dstIt->numContiguousColumns(x), 17);
    QCOMPARE(dstIt->numContiguousRows(y), 17);

    x = 23;
    y = 23;
    dstIt->moveTo(x, y);
    QVERIFY(!memcmp(dstIt->rawData(), c1.data(), pixelSize));
    QCOMPARE(dstIt->numContiguousColumns(x), 17);
    QCOMPARE(dstIt->numContiguousRows(y), 17);

    x = 3;
    y = 23;
    dstIt->moveTo(x, y);
    QVERIFY(!memcmp(dstIt->rawData(), c1.data(), pixelSize));
    QCOMPARE(dstIt->numContiguousColumns(x), 17);
    QCOMPARE(dstIt->numContiguousRows(y), 17);

    x = 23;
    y = 3;
    dstIt->moveTo(x, y);
    QVERIFY(!memcmp(dstIt->rawData(), c1.data(), pixelSize));
    QCOMPARE(dstIt->numContiguousColumns(x), 17);
    QCOMPARE(dstIt->numContiguousRows(y), 17);

    x = -17;
    y = 3;
    dstIt->moveTo(x, y);
    QVERIFY(!memcmp(dstIt->rawData(), c1.data(), pixelSize));
    QCOMPARE(dstIt->numContiguousColumns(x), 17);
    QCOMPARE(dstIt->numContiguousRows(y), 17);

    x = 3;
    y = -17;
    dstIt->moveTo(x, y);
    QVERIFY(!memcmp(dstIt->rawData(), c1.data(), pixelSize));
    QCOMPARE(dstIt->numContiguousColumns(x), 17);
    QCOMPARE(dstIt->numContiguousRows(y), 17);

    x = -17;
    y = -17;
    dstIt->moveTo(x, y);
    QVERIFY(!memcmp(dstIt->rawData(), c1.data(), pixelSize));
    QCOMPARE(dstIt->numContiguousColumns(x), 17);
    QCOMPARE(dstIt->numContiguousRows(y), 17);
}

#include "kis_iterator_ng.h"

static bool nextRowGeneral(KisHLineIteratorSP it, int y, const QRect &rc) {
    it->nextRow();
    return y < rc.height();
}

static bool nextRowGeneral(KisVLineIteratorSP it, int y, const QRect &rc) {
    it->nextColumn();
    return y < rc.width();
}

template <class T>
bool checkXY(const QPoint &pt, const QPoint &realPt) {
    Q_UNUSED(pt);
    Q_UNUSED(realPt);
    return false;
}

template <>
bool checkXY<KisHLineIteratorSP>(const QPoint &pt, const QPoint &realPt) {
    return pt == realPt;
}

template <>
bool checkXY<KisVLineIteratorSP>(const QPoint &pt, const QPoint &realPt) {
    return pt.x() == realPt.y() && pt.y() == realPt.x();
}
#include <kis_wrapped_rect.h>
template <class T>
bool checkConseqPixels(int value, const QPoint &pt, const KisWrappedRect &wrappedRect) {
    Q_UNUSED(value);
    Q_UNUSED(pt);
    Q_UNUSED(wrappedRect);
    return false;
}

template <>
bool checkConseqPixels<KisHLineIteratorSP>(int value, const QPoint &pt, const KisWrappedRect &wrappedRect) {
    int x = KisWrappedRect::xToWrappedX(pt.x(), wrappedRect.wrapRect());
    int borderX = wrappedRect.originalRect().x() + wrappedRect.wrapRect().width();
    int conseq = x >= borderX ? wrappedRect.wrapRect().right() - x + 1 : borderX - x;
    conseq = qMin(conseq, wrappedRect.originalRect().right() - pt.x() + 1);

    return value == conseq;
}

template <>
bool checkConseqPixels<KisVLineIteratorSP>(int value, const QPoint &pt, const KisWrappedRect &wrappedRect) {
    Q_UNUSED(pt);
    Q_UNUSED(wrappedRect);
    return value == 1;
}

template <class IteratorSP>
IteratorSP createIterator(KisPaintDeviceSP dev, const QRect &rc) {
    Q_UNUSED(dev);
    Q_UNUSED(rc);
    return 0;
}

template <>
KisHLineIteratorSP createIterator(KisPaintDeviceSP dev,
                                  const QRect &rc) {
    return dev->createHLineIteratorNG(rc.x(), rc.y(), rc.width());
}

template <>
KisVLineIteratorSP createIterator(KisPaintDeviceSP dev,
                                  const QRect &rc) {
    return dev->createVLineIteratorNG(rc.x(), rc.y(), rc.height());
}

template <class IteratorSP>
void testWrappedLineIterator(QString testName, const QRect &rect)
{
    testName = QString("%1_%2_%3_%4_%5")
        .arg(testName)
        .arg(rect.x())
        .arg(rect.y())
        .arg(rect.width())
        .arg(rect.height());

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = createWrapAroundPaintDevice(cs);

    // test rect fits the wrap rect in both dimensions
    IteratorSP it = createIterator<IteratorSP>(dev, rect);

    int y = 0;
    do {
        int x = 0;
        do {
            quint8 *data = it->rawData();

            data[0] = 10 * x;
            data[1] = 10 * y;
            data[2] = 0;
            data[3] = 255;

            x++;
        } while (it->nextPixel());
    } while (nextRowGeneral(it, ++y, rect));

    QRect rc = dev->defaultBounds()->bounds() | dev->exactBounds();
    QImage result = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());

    QVERIFY(TestUtil::checkQImage(result, "paint_device_test", "wrapped_iterators", testName));
}

template <class IteratorSP>
void testWrappedLineIterator(const QString &testName)
{
    testWrappedLineIterator<IteratorSP>(testName, QRect(10,10,20,20));
    testWrappedLineIterator<IteratorSP>(testName, QRect(10,10,10,20));
    testWrappedLineIterator<IteratorSP>(testName, QRect(10,10,20,10));
    testWrappedLineIterator<IteratorSP>(testName, QRect(10,10,10,10));
    testWrappedLineIterator<IteratorSP>(testName, QRect(0,0,20,20));
}

void KisPaintDeviceTest::testWrappedHLineIterator()
{
    testWrappedLineIterator<KisHLineIteratorSP>("hline_iterator");
}

void KisPaintDeviceTest::testWrappedVLineIterator()
{
    testWrappedLineIterator<KisVLineIteratorSP>("vline_iterator");
}

template <class IteratorSP>
void testWrappedLineIteratorReadMoreThanBounds(QString testName)
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = createWrapAroundPaintDevice(cs);
    KisPaintDeviceSP dst = new KisPaintDevice(cs);

    // fill device with a gradient
    QRect bounds = dev->defaultBounds()->bounds();
    for (int y = bounds.y(); y < bounds.y() + bounds.height(); y++) {
        for (int x = bounds.x(); x < bounds.x() + bounds.width(); x++) {
            QColor c((10 * x) % 255, (10 * y) % 255, 0, 255);
            dev->setPixel(x, y, c);
        }
    }

    // test rect doesn't fit the wrap rect in both dimensions
    const QRect &rect(bounds.adjusted(-6,-6,8,8));
    KisRandomAccessorSP dstIt = dst->createRandomAccessorNG(rect.x(), rect.y());
    IteratorSP it = createIterator<IteratorSP>(dev, rect);

    for (int y = rect.y(); y < rect.y() + rect.height(); y++) {
        for (int x = rect.x(); x < rect.x() + rect.width(); x++) {
            quint8 *data = it->rawData();

            QVERIFY(checkConseqPixels<IteratorSP>(it->nConseqPixels(), QPoint(x, y), KisWrappedRect(rect, bounds)));

            dstIt->moveTo(x, y);
            memcpy(dstIt->rawData(), data, cs->pixelSize());

            QVERIFY(checkXY<IteratorSP>(QPoint(it->x(), it->y()), QPoint(x,y)));

            bool stepDone = it->nextPixel();
            QCOMPARE(stepDone, x < rect.x() + rect.width() - 1);
        }
        if (!nextRowGeneral(it, y, rect)) break;
    }

    testName = QString("%1_%2_%3_%4_%5")
        .arg(testName)
        .arg(rect.x())
        .arg(rect.y())
        .arg(rect.width())
        .arg(rect.height());

    QRect rc = rect;
    QImage result = dst->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    QImage ref = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());

    QVERIFY(TestUtil::checkQImage(result, "paint_device_test", "wrapped_iterators_huge", testName, 1));
}

void KisPaintDeviceTest::testWrappedHLineIteratorReadMoreThanBounds()
{
    testWrappedLineIteratorReadMoreThanBounds<KisHLineIteratorSP>("hline_iterator");
}

void KisPaintDeviceTest::testWrappedVLineIteratorReadMoreThanBounds()
{
    testWrappedLineIteratorReadMoreThanBounds<KisVLineIteratorSP>("vline_iterator");
}

void KisPaintDeviceTest::testMoveWrapAround()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = createWrapAroundPaintDevice(cs);

    KoColor c1(Qt::red, cs);
    KoColor c2(Qt::green, cs);

    dev->setPixel(3, 3, c1);
    dev->setPixel(18, 18, c2);

    // QRect rc = dev->defaultBounds()->bounds();

    //dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height()).save("move0.png");
    QCOMPARE(dev->exactBounds(), QRect(3,3,16,16));
    dev->moveTo(QPoint(10,10));
    QCOMPARE(dev->exactBounds(), QRect(8,8,6,6));
    //dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height()).save("move1.png");

}

#include "kis_lock_free_cache.h"

#define NUM_TYPES 3

// high-concurrency
#define NUM_CYCLES 500000
#define NUM_THREADS 4


struct TestingCache : KisLockFreeCache<int> {
    int calculateNewValue() const override {
        return m_realValue;
    }

    QAtomicInt m_realValue;
};

class CacheStressJob : public QRunnable
{
public:
    CacheStressJob(TestingCache &cache)
        : m_cache(cache),
          m_oldValue(0)
    {
    }

    void run() override {
        for(qint32 i = 0; i < NUM_CYCLES; i++) {
            qint32 type = i % NUM_TYPES;

            switch(type) {
            case 0:
                m_cache.m_realValue.ref();
                m_oldValue = m_cache.m_realValue;
                m_cache.invalidate();
                break;
            case 1:
            {
                int newValue = m_cache.getValue();
                Q_ASSERT(newValue >= m_oldValue);
                Q_UNUSED(newValue);
            }
                break;
            case 3:
                QTest::qSleep(3);
                break;
            }
        }
    }

private:
    TestingCache &m_cache;
    int m_oldValue;
};

void KisPaintDeviceTest::testCacheState()
{
    TestingCache cache;

    QList<CacheStressJob*> jobsList;
    CacheStressJob *job;

    for(qint32 i = 0; i < NUM_THREADS; i++) {
        //job = new CacheStressJob(value, cacheValue, cacheState);
        job = new CacheStressJob(cache);
        job->setAutoDelete(true);
        jobsList.append(job);
    }

    QThreadPool pool;
    pool.setMaxThreadCount(NUM_THREADS);

    Q_FOREACH (job, jobsList) {
        pool.start(job);
    }

    pool.waitForDone();
}

struct TestingLodDefaultBounds : public KisDefaultBoundsBase {
    TestingLodDefaultBounds(const QRect &bounds = QRect(0,0,100,100))
        : m_lod(0), m_bounds(bounds) {}

    QRect bounds() const override {
        return m_bounds;
    }
    bool wrapAroundMode() const override {
        return false;
    }

    int currentLevelOfDetail() const override {
        return m_lod;
    }

    int currentTime() const override {
        return 0;
    }
    bool externalFrameActive() const override {
        return false;
    }

    void testingSetLevelOfDetail(int lod) {
        m_lod = lod;
    }

private:
    int m_lod;
    QRect m_bounds;
};

void fillGradientDevice(KisPaintDeviceSP dev, const QRect &rect, bool flat = false)
{
    if (flat) {
        dev->fill(rect, KoColor(Qt::red, dev->colorSpace()));
    } else {
        // fill device with a gradient
        KisSequentialIterator it(dev, rect);
        while (it.nextPixel()) {
            QColor c((10 * it.x()) & 0xFF, (10 * it.y()) & 0xFF, 0, 255);
            KoColor color(c, dev->colorSpace());
            memcpy(it.rawData(), color.data(), dev->pixelSize());
        }
    }
}
#include "kis_lod_transform.h"
void KisPaintDeviceTest::testLodTransform()
{
    const int lod = 2; // round to 4
    KisLodTransform t(lod);

    QRect rc1(-16, -16, 8, 8);
    QRect rc2(-16, -16, 7, 7);
    QRect rc3(-15, -15, 7, 7);

    QCOMPARE(t.alignedRect(rc1, lod), rc1);
    QCOMPARE(t.alignedRect(rc2, lod), rc1);
    QCOMPARE(t.alignedRect(rc3, lod), rc1);
}

#include "krita_utils.h"
void syncLodCache(KisPaintDeviceSP dev, int levelOfDetail)
{
    KisPaintDevice::LodDataStruct* s = dev->createLodDataStruct(levelOfDetail);

    QRegion region = dev->regionForLodSyncing();
    Q_FOREACH(QRect rect2, KritaUtils::splitRegionIntoPatches(region, KritaUtils::optimalPatchSize())) {
        dev->updateLodDataStruct(s, rect2);
    }

    dev->uploadLodDataStruct(s);
}

void KisPaintDeviceTest::testLodDevice()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    TestingLodDefaultBounds *bounds = new TestingLodDefaultBounds();
    dev->setDefaultBounds(bounds);

    // fill device with a gradient
    // QRect rect = dev->defaultBounds()->bounds();
    // fillGradientDevice(dev, rect);
    fillGradientDevice(dev, QRect(50,50,30,30));

    QCOMPARE(dev->exactBounds(), QRect(50,50,30,30));

    QImage result;

    qDebug() << ppVar(dev->exactBounds());
    result = dev->convertToQImage(0,0,0,100,100);
    /*QVERIFY*/(TestUtil::checkQImage(result, "paint_device_test",
                                  "lod", "initial"));

    bounds->testingSetLevelOfDetail(1);
    syncLodCache(dev, 1);
    QCOMPARE(dev->exactBounds(), QRect(25,25,15,15));

    qDebug() << ppVar(dev->exactBounds());
    result = dev->convertToQImage(0,0,0,100,100);
    /*QVERIFY*/(TestUtil::checkQImage(result, "paint_device_test",
                                  "lod", "lod1"));

    bounds->testingSetLevelOfDetail(2);
    QCOMPARE(dev->exactBounds(), QRect(25,25,15,15));

    qDebug() << ppVar(dev->exactBounds());
    result = dev->convertToQImage(0,0,0,100,100);
    /*QVERIFY*/(TestUtil::checkQImage(result, "paint_device_test",
                                  "lod", "lod1"));

    syncLodCache(dev, 2);
    QCOMPARE(dev->exactBounds(), QRect(12,12,8,8));

    qDebug() << ppVar(dev->exactBounds());
    result = dev->convertToQImage(0,0,0,100,100);
    /*QVERIFY*/(TestUtil::checkQImage(result, "paint_device_test",
                                  "lod", "lod2"));

    bounds->testingSetLevelOfDetail(0);

    dev->setX(20);
    dev->setY(10);

    bounds->testingSetLevelOfDetail(1);
    syncLodCache(dev, 1);

    QCOMPARE(dev->exactBounds(), QRect(35,30,15,15));


    qDebug() << ppVar(dev->exactBounds()) << ppVar(dev->x()) << ppVar(dev->y());
    result = dev->convertToQImage(0,0,0,100,100);
    /*QVERIFY*/(TestUtil::checkQImage(result, "paint_device_test",
                                  "lod", "lod1-offset-6-14"));
}

void KisPaintDeviceTest::benchmarkLod1Generation()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    TestingLodDefaultBounds *bounds = new TestingLodDefaultBounds(QRect(0,0,6000,4000));
    dev->setDefaultBounds(bounds);

    // fill device with a gradient
    QRect rect = dev->defaultBounds()->bounds();
    fillGradientDevice(dev, rect, true);

    QBENCHMARK {
        bounds->testingSetLevelOfDetail(1);
        syncLodCache(dev, 1);
    }
}

void KisPaintDeviceTest::benchmarkLod2Generation()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    TestingLodDefaultBounds *bounds = new TestingLodDefaultBounds(QRect(0,0,6000,4000));
    dev->setDefaultBounds(bounds);

    // fill device with a gradient
    QRect rect = dev->defaultBounds()->bounds();
    fillGradientDevice(dev, rect, true);

    QBENCHMARK {
        bounds->testingSetLevelOfDetail(2);
        syncLodCache(dev, 2);
    }
}

void KisPaintDeviceTest::benchmarkLod3Generation()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    TestingLodDefaultBounds *bounds = new TestingLodDefaultBounds(QRect(0,0,3000,2000));
    dev->setDefaultBounds(bounds);

    // fill device with a gradient
    QRect rect = dev->defaultBounds()->bounds();
    fillGradientDevice(dev, rect, true);

    QBENCHMARK {
        bounds->testingSetLevelOfDetail(3);
        syncLodCache(dev, 3);
    }
}

void KisPaintDeviceTest::benchmarkLod4Generation()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    TestingLodDefaultBounds *bounds = new TestingLodDefaultBounds(QRect(0,0,3000,2000));
    dev->setDefaultBounds(bounds);

    // fill device with a gradient
    QRect rect = dev->defaultBounds()->bounds();
    fillGradientDevice(dev, rect, true);

    QBENCHMARK {
        bounds->testingSetLevelOfDetail(4);
        syncLodCache(dev, 4);
    }
}

#include "kis_keyframe_channel.h"
#include "kis_raster_keyframe_channel.h"
#include "kis_paint_device_frames_interface.h"
#include "testing_timed_default_bounds.h"

void KisPaintDeviceTest::testFramesLeaking()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    TestUtil::TestingTimedDefaultBounds *bounds = new TestUtil::TestingTimedDefaultBounds();
    dev->setDefaultBounds(bounds);

    KisRasterKeyframeChannel *channel = dev->createKeyframeChannel(KisKeyframeChannel::Content);
    QVERIFY(channel);

    KisPaintDeviceFramesInterface *i = dev->framesInterface();
    QVERIFY(i);

    QCOMPARE(i->frames().size(), 1);

    KisPaintDeviceFramesInterface::TestingDataObjects o;

    // Itinial state: one frame, m_data is kept separate
    o = i->testingGetDataObjects();
    QVERIFY(o.m_data);
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    QCOMPARE(o.m_frames.size(), 1);
    QVERIFY(o.m_currentData == o.m_frames[0]);

    // add keyframe at position 10
    channel->addKeyframe(10);

    // two frames, m_data has a default empty value
    o = i->testingGetDataObjects();
    QVERIFY(o.m_data);
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    QCOMPARE(o.m_frames.size(), 2);
    QVERIFY(o.m_currentData == o.m_frames[0]);

    // add keyframe at position 20
    channel->addKeyframe(20);

    // three frames, m_data is default, current frame is 0
    o = i->testingGetDataObjects();
    QVERIFY(o.m_data);
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    QCOMPARE(o.m_frames.size(), 3);
    QVERIFY(o.m_currentData == o.m_frames[0]);

    // switch to frame 10
    bounds->testingSetTime(10);

    // three frames, m_data is default, current frame is 10
    o = i->testingGetDataObjects();
    QVERIFY(o.m_data);
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    QVERIFY(o.m_currentData == o.m_frames[1]);
    QCOMPARE(o.m_frames.size(), 3);

    // switch to frame 20
    bounds->testingSetTime(20);

    // three frames, m_data is default, current frame is 20
    o = i->testingGetDataObjects();
    QVERIFY(o.m_data);
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    QVERIFY(o.m_currentData == o.m_frames[2]);
    QCOMPARE(o.m_frames.size(), 3);

    // switch to frame 15
    bounds->testingSetTime(15);

    // three frames, m_data is default, current frame is 10
    o = i->testingGetDataObjects();
    QVERIFY(o.m_data);
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    QVERIFY(o.m_currentData == o.m_frames[1]);
    QCOMPARE(o.m_frames.size(), 3);

    KisKeyframeSP key;

    // deletion of frame 0 is forbidden
    key = channel->keyframeAt(0);
    QVERIFY(key);
    QVERIFY(channel->deleteKeyframe(key));

    // delete keyframe at position 11
    key = channel->activeKeyframeAt(11);
    QVERIFY(key);
    QCOMPARE(key->time(), 10);
    QVERIFY(channel->deleteKeyframe(key));

    // two frames, m_data is default, current frame is 0
    o = i->testingGetDataObjects();
    QVERIFY(o.m_data);
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    //QVERIFY(o.m_currentData == o.m_frames[0]);
    QCOMPARE(o.m_frames.size(), 2);

    // deletion of frame 0 is forbidden
    key = channel->activeKeyframeAt(11);
    QVERIFY(key);
    QCOMPARE(key->time(), 0);
    QVERIFY(channel->deleteKeyframe(key));

    // nothing changed
    o = i->testingGetDataObjects();
    QVERIFY(o.m_data);
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    //QVERIFY(o.m_currentData == o.m_frames[0]);
    QCOMPARE(o.m_frames.size(), 2);

    // delete keyframe at position 20
    key = channel->activeKeyframeAt(20);
    QVERIFY(key);
    QCOMPARE(key->time(), 20);
    QVERIFY(channel->deleteKeyframe(key));

    // one keyframe is left at position 0, m_data is default
    o = i->testingGetDataObjects();
    QVERIFY(o.m_data);
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    //QVERIFY(o.m_currentData == o.m_frames[0]);
    QCOMPARE(o.m_frames.size(), 1);

    // ensure all the objects in the list of all objects are unique
    QList<KisPaintDeviceData*> allObjects = i->testingGetDataObjectsList();
    QSet<KisPaintDeviceData*> uniqueObjects;
    Q_FOREACH (KisPaintDeviceData *obj, allObjects) {
        if (!obj) continue;
        QVERIFY(!uniqueObjects.contains(obj));
        uniqueObjects.insert(obj);
    }
}

void KisPaintDeviceTest::testFramesUndoRedo()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    TestUtil::TestingTimedDefaultBounds *bounds = new TestUtil::TestingTimedDefaultBounds();
    dev->setDefaultBounds(bounds);

    KisRasterKeyframeChannel *channel = dev->createKeyframeChannel(KisKeyframeChannel::Content);
    QVERIFY(channel);

    KisPaintDeviceFramesInterface *i = dev->framesInterface();
    QVERIFY(i);

    QCOMPARE(i->frames().size(), 1);

    KisPaintDeviceFramesInterface::TestingDataObjects o;

    // Itinial state: one frame, m_data shared
    o = i->testingGetDataObjects();
    QVERIFY(o.m_data); // default m_data should always be present
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    QCOMPARE(o.m_frames.size(), 1);
    QVERIFY(o.m_currentData == o.m_frames[0]);

    // add a keyframe

    KUndo2Command cmdAdd;
    int frameId = -1;
    const int time = 1;

    channel->addKeyframe(time, &cmdAdd);
    frameId = channel->frameIdAt(time);
    //int frameId = i->createFrame(false, 0, QPoint(), &cmdAdd);

    QCOMPARE(frameId, 1);

    o = i->testingGetDataObjects();
    QVERIFY(o.m_data); // default m_data should always be present
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    QCOMPARE(o.m_frames.size(), 2);
    QVERIFY(o.m_currentData == o.m_frames[0]);

    cmdAdd.undo();

    o = i->testingGetDataObjects();
    QVERIFY(o.m_data); // default m_data should always be present
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    QCOMPARE(o.m_frames.size(), 1);
    QVERIFY(o.m_currentData == o.m_frames[0]);

    cmdAdd.redo();

    o = i->testingGetDataObjects();
    QVERIFY(o.m_data); // default m_data should always be present
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    QCOMPARE(o.m_frames.size(), 2);
    QVERIFY(o.m_currentData == o.m_frames[0]);


    KUndo2Command cmdRemove;
    KisKeyframeSP keyframe = channel->keyframeAt(time);
    QVERIFY(keyframe);

    channel->deleteKeyframe(keyframe, &cmdRemove);

    //i->deleteFrame(1, &cmdRemove);

    o = i->testingGetDataObjects();
    QVERIFY(o.m_data); // default m_data should always be present
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    QCOMPARE(o.m_frames.size(), 1);
    QVERIFY(o.m_currentData == o.m_frames[0]);

    cmdRemove.undo();

    o = i->testingGetDataObjects();
    QVERIFY(o.m_data); // default m_data should always be present
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    QCOMPARE(o.m_frames.size(), 2);
    QVERIFY(o.m_currentData == o.m_frames[0]);

    cmdRemove.redo();

    o = i->testingGetDataObjects();
    QVERIFY(o.m_data); // default m_data should always be present
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    QCOMPARE(o.m_frames.size(), 1);
    QVERIFY(o.m_currentData == o.m_frames[0]);
}

void KisPaintDeviceTest::testFramesUndoRedoWithChannel()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    TestUtil::TestingTimedDefaultBounds *bounds = new TestUtil::TestingTimedDefaultBounds();
    dev->setDefaultBounds(bounds);

    KisRasterKeyframeChannel *channel = dev->createKeyframeChannel(KisKeyframeChannel::Content);
    QVERIFY(channel);

    KisPaintDeviceFramesInterface *i = dev->framesInterface();
    QVERIFY(i);

    QCOMPARE(i->frames().size(), 1);

    KisPaintDeviceFramesInterface::TestingDataObjects o;

    // Itinial state: one frame, m_data shared
    o = i->testingGetDataObjects();
    QVERIFY(o.m_data); // default m_data should always be present
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    QCOMPARE(o.m_frames.size(), 1);
    QVERIFY(o.m_currentData == o.m_frames[0]);


    // add a keyframe

    KUndo2Command cmdAdd;

    KisKeyframeSP frame = channel->addKeyframe(10, &cmdAdd);

    QVERIFY(channel->keyframeAt(10));

    o = i->testingGetDataObjects();
    QVERIFY(o.m_data); // default m_data should always be present
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    QCOMPARE(o.m_frames.size(), 2);
    QVERIFY(o.m_currentData == o.m_frames[0]);

    cmdAdd.undo();

    QVERIFY(!channel->keyframeAt(10));

    o = i->testingGetDataObjects();
    QVERIFY(o.m_data); // default m_data should always be present
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    QCOMPARE(o.m_frames.size(), 1);
    QVERIFY(o.m_currentData == o.m_frames[0]);

    cmdAdd.redo();

    QVERIFY(channel->keyframeAt(10));

    o = i->testingGetDataObjects();
    QVERIFY(o.m_data); // default m_data should always be present
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    QCOMPARE(o.m_frames.size(), 2);
    QVERIFY(o.m_currentData == o.m_frames[0]);


    KUndo2Command cmdRemove;
    channel->deleteKeyframe(frame, &cmdRemove);

    QVERIFY(!channel->keyframeAt(10));

    o = i->testingGetDataObjects();
    QVERIFY(o.m_data); // default m_data should always be present
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    QCOMPARE(o.m_frames.size(), 1);
    QVERIFY(o.m_currentData == o.m_frames[0]);

    cmdRemove.undo();

    QVERIFY(channel->keyframeAt(10));

    o = i->testingGetDataObjects();
    QVERIFY(o.m_data); // default m_data should always be present
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    QCOMPARE(o.m_frames.size(), 2);
    QVERIFY(o.m_currentData == o.m_frames[0]);

    cmdRemove.redo();

    QVERIFY(!channel->keyframeAt(10));

    o = i->testingGetDataObjects();
    QVERIFY(o.m_data); // default m_data should always be present
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    QCOMPARE(o.m_frames.size(), 1);
    QVERIFY(o.m_currentData == o.m_frames[0]);

    cmdRemove.undo();

    QVERIFY(channel->keyframeAt(10));

    o = i->testingGetDataObjects();
    QVERIFY(o.m_data); // default m_data should always be present
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    QCOMPARE(o.m_frames.size(), 2);
    QVERIFY(o.m_currentData == o.m_frames[0]);


    KUndo2Command cmdMove;
    channel->moveKeyframe(frame, 12, &cmdMove);

    QVERIFY(!channel->keyframeAt(10));
    QVERIFY(channel->keyframeAt(12));

    o = i->testingGetDataObjects();
    QVERIFY(o.m_data); // default m_data should always be present
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    QCOMPARE(o.m_frames.size(), 2);
    QVERIFY(o.m_currentData == o.m_frames[0]);

    cmdMove.undo();

    QVERIFY(channel->keyframeAt(10));
    QVERIFY(!channel->keyframeAt(12));

    o = i->testingGetDataObjects();
    QVERIFY(o.m_data); // default m_data should always be present
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    QCOMPARE(o.m_frames.size(), 2);
    QVERIFY(o.m_currentData == o.m_frames[0]);

    cmdMove.redo();

    QVERIFY(!channel->keyframeAt(10));
    QVERIFY(channel->keyframeAt(12));

    o = i->testingGetDataObjects();
    QVERIFY(o.m_data); // default m_data should always be present
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    QCOMPARE(o.m_frames.size(), 2);
    QVERIFY(o.m_currentData == o.m_frames[0]);
}

void fillRect(KisPaintDeviceSP dev, int time, const QRect &rc, TestUtil::TestingTimedDefaultBounds *bounds)
{
    KUndo2Command parentCommand;
    KisRasterKeyframeChannel *channel = dev->keyframeChannel();
    KisKeyframeSP frame = channel->addKeyframe(time, &parentCommand);

    const int oldTime = bounds->currentTime();
    bounds->testingSetTime(time);

    KoColor color(Qt::red, dev->colorSpace());
    dev->fill(rc, color);

    bounds->testingSetTime(oldTime);
}

bool checkRect(KisPaintDeviceSP dev, int time, const QRect &rc, TestUtil::TestingTimedDefaultBounds *bounds)
{
    const int oldTime = bounds->currentTime();
    bounds->testingSetTime(time);

    bool result = dev->exactBounds() == rc;

    if (!result) {
        qDebug() << "Failed to check frame:" << ppVar(time) << ppVar(rc) << ppVar(dev->exactBounds());
    }

    bounds->testingSetTime(oldTime);

    return result;
}

void testCrossDeviceFrameCopyImpl(bool useChannel)
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev1 = new KisPaintDevice(cs);
    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);

    const KoColorSpace *cs3 = KoColorSpaceRegistry::instance()->rgb16();
    KisPaintDeviceSP dev3 = new KisPaintDevice(cs3);

    TestUtil::TestingTimedDefaultBounds *bounds = new TestUtil::TestingTimedDefaultBounds();
    dev1->setDefaultBounds(bounds);
    dev2->setDefaultBounds(bounds);
    dev3->setDefaultBounds(bounds);

    KisRasterKeyframeChannel *channel1 = dev1->createKeyframeChannel(KisKeyframeChannel::Content);
    KisPaintDeviceFramesInterface *i1 = dev1->framesInterface();
    QVERIFY(channel1);
    QVERIFY(i1);

    KisRasterKeyframeChannel *channel2 = dev2->createKeyframeChannel(KisKeyframeChannel::Content);
    KisPaintDeviceFramesInterface *i2 = dev2->framesInterface();
    QVERIFY(channel2);
    QVERIFY(i2);

    KisRasterKeyframeChannel *channel3 = dev3->createKeyframeChannel(KisKeyframeChannel::Content);
    KisPaintDeviceFramesInterface *i3 = dev3->framesInterface();
    QVERIFY(channel3);
    QVERIFY(i3);

    fillRect(dev1, 10, QRect(100,100,100,100), bounds);
    fillRect(dev2, 20, QRect(200,200,100,100), bounds);
    fillRect(dev3, 30, QRect(300,300,100,100), bounds);

    QCOMPARE(dev1->exactBounds(), QRect());

    const int dstFrameId1 = channel1->frameIdAt(10);
    const int srcFrameId2 = channel2->frameIdAt(20);
    const int srcFrameId3 = channel3->frameIdAt(30);

    KUndo2Command cmd1;
    if (!useChannel) {
        dev1->framesInterface()->uploadFrame(srcFrameId2, dstFrameId1, dev2);
    } else {
        KisKeyframeSP k = channel1->copyExternalKeyframe(channel2, 20, 10, &cmd1);
    }

    QCOMPARE(dev1->exactBounds(), QRect());
    QVERIFY(checkRect(dev1, 10, QRect(200,200,100,100), bounds));

    if (useChannel) {
        cmd1.undo();
        QVERIFY(checkRect(dev1, 10, QRect(100,100,100,100), bounds));
    }

    KUndo2Command cmd2;
    if (!useChannel) {
        dev1->framesInterface()->uploadFrame(srcFrameId3, dstFrameId1, dev3);
    } else {
        KisKeyframeSP k = channel1->copyExternalKeyframe(channel3, 30, 10, &cmd2);
    }

    QCOMPARE(dev1->exactBounds(), QRect());
    QVERIFY(checkRect(dev1, 10, QRect(300,300,100,100), bounds));

    if (useChannel) {
        cmd2.undo();
        QVERIFY(checkRect(dev1, 10, QRect(100,100,100,100), bounds));
    }
}

void KisPaintDeviceTest::testCrossDeviceFrameCopyDirect()
{
    testCrossDeviceFrameCopyImpl(false);
}

void KisPaintDeviceTest::testCrossDeviceFrameCopyChannel()
{
    testCrossDeviceFrameCopyImpl(true);
}

#include "kis_surrogate_undo_adapter.h"

void KisPaintDeviceTest::testLazyFrameCreation()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    TestUtil::TestingTimedDefaultBounds *bounds = new TestUtil::TestingTimedDefaultBounds();
    dev->setDefaultBounds(bounds);

    KisRasterKeyframeChannel *channel = dev->createKeyframeChannel(KisKeyframeChannel::Content);
    QVERIFY(channel);

    KisPaintDeviceFramesInterface *i = dev->framesInterface();
    QVERIFY(i);

    QCOMPARE(i->frames().size(), 1);

    bounds->testingSetTime(10);

    QCOMPARE(i->frames().size(), 1);

    KisSurrogateUndoAdapter undoAdapter;

    {
        KisTransaction transaction1(dev);
        transaction1.commit(&undoAdapter);
    }

    QCOMPARE(i->frames().size(), 2);

    undoAdapter.undoAll();

    QCOMPARE(i->frames().size(), 1);

    undoAdapter.redoAll();

    QCOMPARE(i->frames().size(), 2);
}

void KisPaintDeviceTest::testCopyPaintDeviceWithFrames()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    TestUtil::TestingTimedDefaultBounds *bounds = new TestUtil::TestingTimedDefaultBounds();
    dev->setDefaultBounds(bounds);

    KisRasterKeyframeChannel *channel = dev->createKeyframeChannel(KisKeyframeChannel::Content);
    QVERIFY(channel);

    KisPaintDeviceFramesInterface *i = dev->framesInterface();
    QVERIFY(i);

    QCOMPARE(i->frames().size(), 1);

    KisPaintDeviceFramesInterface::TestingDataObjects o;

    // Itinial state: one frame, m_data shared
    o = i->testingGetDataObjects();
    QVERIFY(o.m_data); // m_data should always be present
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    QCOMPARE(o.m_frames.size(), 1);
    QVERIFY(o.m_currentData == o.m_frames[0]);


    // add a keyframe

    KUndo2Command cmdAdd;

    KisKeyframeSP frame = channel->addKeyframe(10, &cmdAdd);

    QVERIFY(channel->keyframeAt(10));

    o = i->testingGetDataObjects();
    QVERIFY(o.m_data); // m_data should always be present
    QVERIFY(!o.m_lodData);
    QVERIFY(!o.m_externalFrameData);
    QCOMPARE(o.m_frames.size(), 2);
    //QVERIFY(o.m_currentData == o.m_frames[0]);

    KisPaintDeviceSP newDev = new KisPaintDevice(*dev, KritaUtils::CopyAllFrames);

    QVERIFY(channel->keyframeAt(0));
    QVERIFY(channel->keyframeAt(10));
}

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_smallint.hpp>

#include "KoCompositeOpRegistry.h"


using namespace boost::accumulators;

accumulator_set<qreal, stats<tag::variance, tag::max, tag::min> > accum;


void KisPaintDeviceTest::testCompositionAssociativity()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();

    qsrand(500);

    boost::mt11213b _rnd0(qrand());
    boost::mt11213b _rnd1(qrand());
    boost::mt11213b _rnd2(qrand());
    boost::mt11213b _rnd3(qrand());

    boost::uniform_smallint<int> rnd0(0, 255);
    boost::uniform_smallint<int> rnd1(0, 255);
    boost::uniform_smallint<int> rnd2(0, 255);
    boost::uniform_smallint<int> rnd3(0, 255);

    QList<KoCompositeOp*> allCompositeOps = cs->compositeOps();

    Q_FOREACH (const KoCompositeOp *op, allCompositeOps) {

        accumulator_set<qreal, stats<tag::variance, tag::max, tag::min> > accum;

        const int numIterations = 10000;

        for (int j = 0; j < numIterations; j++) {
            KoColor c1(QColor(rnd0(_rnd0), rnd1(_rnd1), rnd2(_rnd2), rnd3(_rnd3)), cs);
            KoColor c2(QColor(rnd0(_rnd0), rnd1(_rnd1), rnd2(_rnd2), rnd3(_rnd3)), cs);
            KoColor c3(QColor(rnd0(_rnd0), rnd1(_rnd1), rnd2(_rnd2), rnd3(_rnd3)), cs);
            //KoColor c4(QColor(rnd0(_rnd0), rnd1(_rnd1), rnd2(_rnd2), rnd3(_rnd3)), cs);
            //KoColor c5(QColor(rnd0(_rnd0), rnd1(_rnd1), rnd2(_rnd2), rnd3(_rnd3)), cs);

            KoColor r1(QColor(Qt::transparent), cs);
            KoColor r2(QColor(Qt::transparent), cs);
            KoColor r3(QColor(Qt::transparent), cs);

            op->composite(r1.data(), 0, c1.data(), 0, 0,0, 1,1, 255);
            op->composite(r1.data(), 0, c2.data(), 0, 0,0, 1,1, 255);
            op->composite(r1.data(), 0, c3.data(), 0, 0,0, 1,1, 255);
            //op->composite(r1.data(), 0, c4.data(), 0, 0,0, 1,1, 255);
            //op->composite(r1.data(), 0, c5.data(), 0, 0,0, 1,1, 255);

            op->composite(r3.data(), 0, c2.data(), 0, 0,0, 1,1, 255);
            op->composite(r3.data(), 0, c3.data(), 0, 0,0, 1,1, 255);
            //op->composite(r3.data(), 0, c4.data(), 0, 0,0, 1,1, 255);
            //op->composite(r3.data(), 0, c5.data(), 0, 0,0, 1,1, 255);

            op->composite(r2.data(), 0, c1.data(), 0, 0,0, 1,1, 255);
            op->composite(r2.data(), 0, r3.data(), 0, 0,0, 1,1, 255);

            const quint8 *p1 = r1.data();
            const quint8 *p2 = r2.data();

            if (memcmp(p1, p2, 4) != 0) {
                for (int i = 0; i < 4; i++) {
                    accum(qAbs(p1[i] - p2[i]));
                }
            }

        }

        qDebug("Errors for op %25s err rate %7.2f var %7.2f max %7.2f",
               op->id().toLatin1().data(),
               (qreal(count(accum)) / (4 * numIterations)),
               variance(accum),
               count(accum) > 0 ? (max)(accum) : 0);
    }
}

#include <kundo2stack.h>

struct FillWorker : public QRunnable
{
    FillWorker(KisPaintDeviceSP dev, const QRect &fillRect, bool clear)
        : m_dev(dev), m_fillRect(fillRect), m_clear(clear)
    {
        setAutoDelete(true);
    }

    void run() {
        if (m_clear) {
            m_dev->clear(m_fillRect);
        } else {
            const KoColor fillColor(Qt::red, m_dev->colorSpace());
            const int pixelSize = m_dev->colorSpace()->pixelSize();

            KisSequentialIterator it(m_dev, m_fillRect);
            while (it.nextPixel()) {
                memcpy(it.rawData(), fillColor.data(), pixelSize);
            }
        }
    }

private:
    KisPaintDeviceSP m_dev;
    QRect m_fillRect;
    bool m_clear;
};

#include <malloc.h>

void KisPaintDeviceTest::stressTestMemoryFragmentation()
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    KUndo2Stack undoStack;

#ifdef LIMIT_LONG_TESTS
    const int numCycles = 3;
    undoStack.setUndoLimit(1);
#else
    const int numCycles = 200;
    undoStack.setUndoLimit(10);
#endif

    const int numThreads = 16;
    const int desiredWidth = 10000;
    const int patchSize = 81;
    const int numSidePatches = desiredWidth / patchSize;

    QThreadPool pool;
    pool.setMaxThreadCount(numThreads);


    for (int i = 0; i < numCycles; i++) {
        qDebug() << "iteration"<< i;
        // KisTransaction t(dev);

        for (int y = 0; y < numSidePatches; y++) {
            for (int x = 0; x < numSidePatches; x++) {
                const QRect workerRect(x * patchSize, y * patchSize, patchSize, patchSize);
                pool.start(new FillWorker(dev, workerRect, (i + x + y) & 0x1));
            }
        }

        pool.waitForDone();
        // undoStack.push(t.endAndTake());

        struct mallinfo info = mallinfo();
        qDebug() << "Iteration:" << i;
        qDebug() << "Allocated on heap:" << (info.arena >> 20) << "MiB";
        qDebug() << "Mmaped regions:" << info.hblks << (info.hblkhd >> 20) << "MiB";
        qDebug() << "Free fastbin chunks:" << info.smblks << (info.fsmblks >> 10)  << "KiB";
        qDebug() << "Allocated in ordinary blocks" << (info.uordblks >> 20) << "MiB";
        qDebug() << "Free in ordinary blockes" << info.ordblks << (info.fordblks >> 20) << "MiB";
        qDebug() << "========================================";
    }

    undoStack.clear();
}

KISTEST_MAIN(KisPaintDeviceTest)
