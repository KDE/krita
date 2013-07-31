/*
 *  Copyright (c) 2007 Boudewijn Rempt boud@valdyas.org
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

#include "kis_transform_worker_test.h"

#include <qtest_kde.h>
#include <KoProgressUpdater.h>
#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <QTransform>
#include <QVector>

#include "kis_types.h"
#include "kis_image.h"
#include "kis_filter_strategy.h"
#include "kis_paint_device.h"
#include "kis_transform_worker.h"
#include "testutil.h"
#include "kis_transaction.h"
#include "kis_random_accessor_ng.h"

void KisTransformWorkerTest::testCreation()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisFilterStrategy * filter = new KisBoxFilterStrategy();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    KisTransformWorker tw(dev, 1.0, 1.0,
                          1.0, 1.0,
                          0.0, 0.0,
                          1.5,
                          0, 0, updater, filter);
}

void KisTransformWorkerTest::testMirrorX()
{

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    dev2->convertFromQImage(image, 0);
    KisTransformWorker::mirrorX(dev2);
    KisTransformWorker::mirrorX(dev2);
    KisTransformWorker::mirrorX(dev2);
    QImage result = dev2->convertToQImage(0, 0, 0, image.width(), image.height());
    image = image.mirrored(true, false);

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        // They are the same, but should be mirrored
        image.save("mirror_test_1_source.png");
        result.save("mirror_test_1_result.png");
        QFAIL(QString("Failed to mirror the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }

}

void KisTransformWorkerTest::testMirrorY()
{

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    dev2->convertFromQImage(image, 0);
    KisTransformWorker::mirrorY(dev2);
    KisTransformWorker::mirrorY(dev2);
    KisTransformWorker::mirrorY(dev2);
    QImage result = dev2->convertToQImage(0, 0, 0, image.width(), image.height());
    image = image.mirrored(false, true  );

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        // They are the same, but should be mirrored
        image.save("mirror_test_2_source.png");
        result.save("mirror_test_2_result.png");
        QFAIL(QString("Failed to mirror the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }

}

void KisTransformWorkerTest::testOffset()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QString imageName("mirror_source.png");
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + imageName);
    QPoint bottomRight(image.width(), image.height());
    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);

    QVector<QPoint> offsetPoints;
    offsetPoints.append(QPoint(image.width() / 2, image.height() / 2));     // offset to 1/2 of image
    offsetPoints.append(QPoint(image.width() / 4, image.height() / 4)); // offset to 1/4 of image
    offsetPoints.append(QPoint(image.width() - image.width() / 4, image.height() - image.height() / 4)); // offset to 3/4 of image
    offsetPoints.append(QPoint(image.width() / 4, 0));      // offset with y == 0
    offsetPoints.append(QPoint(0, image.height() / 4));     // offset with x == 0

    QPoint errpoint;
    QPoint backOffsetPoint;
    QImage result;
    int test = 0;
    QPoint origin(0,0);
    foreach (QPoint offsetPoint, offsetPoints)
    {
        dev2->convertFromQImage(image, 0);
        KisTransformWorker::offset(dev2, offsetPoint, QRect(origin, image.size()) );
        backOffsetPoint = bottomRight - offsetPoint;
        KisTransformWorker::offset(dev2, backOffsetPoint , QRect(origin, image.size()) );
        result = dev2->convertToQImage(0, 0, 0, image.width(), image.height());
        if (!TestUtil::compareQImages(errpoint, image, result))
        {
            // They are the same, but should be mirrored
            image.save(QString("offset_test_%1_source.png").arg(test));
            result.save(QString("offset_test_%1_result.png").arg(test));
            QFAIL(QString("Failed to offset the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
        }
    }
}


void KisTransformWorkerTest::testMirrorTransactionX()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    dev2->convertFromQImage(image, 0);

    KisTransaction t("mirror", dev2);
    KisTransformWorker::mirrorX(dev2);
    t.end();

    QImage result = dev2->convertToQImage(0, 0, 0, image.width(), image.height());

    image = image.mirrored(true, false);

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        // They are the same, but should be mirrored
        image.save("mirror_test_3_source.png");
        result.save("mirror_test_3_result.png");
        QFAIL(QString("Failed to mirror the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());

    }
}
void KisTransformWorkerTest::testMirrorTransactionY()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    dev2->convertFromQImage(image, 0);

    KisTransaction t("mirror", dev2);
    KisTransformWorker::mirrorY(dev2);
    t.end();

    QImage result = dev2->convertToQImage(0, 0, 0, image.width(), image.height());

    image = image.mirrored(false, true);

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        // They are the same, but should be mirrored
        image.save("mirror_test_4_source.png");
        result.save("mirror_test_4_result.png");
        QFAIL(QString("Failed to mirror the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

void KisTransformWorkerTest::testScaleUp()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    KisFilterStrategy * filter = new KisBoxFilterStrategy();
    KisTransaction t("test", dev);
    KisTransformWorker tw(dev, 2.4, 2.4,
                          0.0, 0.0,
                          0.0, 0.0,
                          0.0,
                          0, 0, updater, filter);
    tw.run();
    t.end();

    QRect rc = dev->exactBounds();

    QCOMPARE(rc.width(), qCeil(image.width() * 2.4));
    QCOMPARE(rc.height(), qCeil(image.height() * 2.4));

    QImage result = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    QPoint errpoint;
    image.load(QString(FILES_DATA_DIR) + QDir::separator() + "test_scaleup_result.png");
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("test_scaleup_source.png");
        result.save("test_scaleup_result.png");
        QFAIL(QString("Failed to scale the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}


void KisTransformWorkerTest::testXScaleUp()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    KisFilterStrategy * filter = new KisBoxFilterStrategy();
    KisTransaction t("test", dev);
    KisTransformWorker tw(dev, 2.0, 1.0,
                          0.0, 0.0,
                          0.0, 0.0,
                          0.0,
                          0, 0, updater, filter);
    tw.run();
    t.end();

    QRect rc = dev->exactBounds();

    QVERIFY(rc.width() == image.width() * 2);
    QVERIFY(rc.height() == image.height());

    QImage result = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    QPoint errpoint;
    image.load(QString(FILES_DATA_DIR) + QDir::separator() + "scaleupx_result.png");
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("test_x_scaleup_source.png");
        result.save("test_x_scaleup_result.png");
        QFAIL(QString("Failed to scale up the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

void KisTransformWorkerTest::testYScaleUp()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    KisFilterStrategy * filter = new KisBoxFilterStrategy();

    KisTransaction t("test", dev);
    KisTransformWorker tw(dev, 1.0, 2.0,
                          0.0, 0.0,
                          0.0, 0.0,
                          0.0,
                          0, 0, updater, filter);
    tw.run();
    t.end();

    QRect rc = dev->exactBounds();

    QCOMPARE(rc.width(), image.width());
    QCOMPARE(rc.height(), image.height() * 2);

    QImage result = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    QPoint errpoint;
    image.load(QString(FILES_DATA_DIR) + QDir::separator() + "scaleupy_result.png");
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("test_y_scaleup_source.png");
        result.save("test_y_scaleup_result.png");
        QFAIL(QString("Failed to scale up the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

void KisTransformWorkerTest::testIdentity()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);
    KisFilterStrategy * filter = new KisBoxFilterStrategy();

    KisTransaction t("test", dev);
    KisTransformWorker tw(dev, 1.0, 1.0,
                          0.0, 0.0,
                          0.0, 0.0,
                          0.0,
                          0, 0, updater, filter);
    tw.run();
    t.end();

    QRect rc = dev->exactBounds();

    QVERIFY(rc.width() ==image.width());
    QVERIFY(rc.height() == image.height());

    QImage result = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("test_identity_source.png");
        result.save("test_identity_result.png");
        QFAIL(QString("Failed to apply identity transformation to image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

void KisTransformWorkerTest::testScaleDown()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);
    KisFilterStrategy * filter = new KisBoxFilterStrategy();

    KisTransaction t("test", dev);
    KisTransformWorker tw(dev, 0.123, 0.123,
                          0.0, 0.0,
                          0.0, 0.0,
                          0.0,
                          0, 0, updater, filter);
    tw.run();
    t.end();

    QRect rc = dev->exactBounds();

    QCOMPARE(rc.width(), qCeil(image.width() * 0.123));
    QCOMPARE(rc.height(), qCeil(image.height() * 0.123));

//    KisTransaction t2("test", dev);
//    KisRandomAccessorSP ac = dev->createRandomAccessorNG(rc.x(), rc.y());
//    for(int x = rc.x(); x < rc.width(); ++x) {
//        for(int y = rc.y(); y < rc.height(); ++y) {
//            ac->moveTo(x, y);
//            cs->setOpacity(ac->rawData(), 0.5, 1);
//        }
//    }
//    t2.end();

    QImage result = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    QPoint errpoint;
    image.load(QString(FILES_DATA_DIR) + QDir::separator() + "test_scaledown_result.png");
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("test_scaledown_source.png");
        result.save("test_scaledown_result.png");
        QFAIL(QString("Failed to scale down the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}


void KisTransformWorkerTest::testXScaleDown()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    KisFilterStrategy * filter = new KisBoxFilterStrategy();

    KisTransaction t("test", dev);
    KisTransformWorker tw(dev, 0.123, 1.0,
                          0.0, 0.0,
                          0.0, 0.0,
                          0.0,
                          0, 0, updater, filter);
    tw.run();
    t.end();

    QRect rc = dev->exactBounds();

    QCOMPARE(rc.width(), qCeil(image.width() * 0.123));
    QCOMPARE(rc.height(), image.height());

    QImage result = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    QPoint errpoint;
    image.load(QString(FILES_DATA_DIR) + QDir::separator() + "scaledownx_result.png");
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("scaledownx_source.png");
        result.save("scaledownx_result.png");
        QFAIL(QString("Failed to scale down the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

void KisTransformWorkerTest::testYScaleDown()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    KisFilterStrategy * filter = new KisBoxFilterStrategy();
    KisTransaction t("test", dev);
    KisTransformWorker tw(dev, 1.0, 0.123,
                          0.0, 0.0,
                          0.0, 0.0,
                          0.0,
                          0, 0, updater, filter);
    tw.run();
    t.end();

    QRect rc = dev->exactBounds();

    QCOMPARE(rc.width(), image.width());
    QCOMPARE(rc.height(), qCeil(image.height() * 0.123));

    QImage result = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    QPoint errpoint;
    image.load(QString(FILES_DATA_DIR) + QDir::separator() + "scaledowny_result.png");
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("scaledowny_source.png");
        result.save("scaledowny_result.png");
        QFAIL(QString("Failed to scale down the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

void KisTransformWorkerTest::testXShear()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    KisFilterStrategy * filter = new KisBoxFilterStrategy();

    KisTransaction t("test", dev);
    KisTransformWorker tw(dev, 1.0, 1.0,
                          1.0, 0.0,
                          300., 200.,
                          0.0,
                          0, 0, updater, filter);
    tw.run();
    t.end();

    QRect rc = dev->exactBounds();

    QVERIFY(rc.width() == 959);
    QVERIFY(rc.height() == image.height());

//    KisTransaction t2("test", dev);
//    KisRandomAccessorSP ac = dev->createRandomAccessorNG(rc.x(), rc.y());
//    for(int x = rc.x(); x < rc.width(); ++x) {
//        for(int y = rc.y(); y < rc.height(); ++y) {
//            ac->moveTo(x, y);
//            cs->setOpacity(ac->rawData(), 0.5, 1);
//        }
//    }
//    t2.end();

    QImage result = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    QPoint errpoint;
    image.load(QString(FILES_DATA_DIR) + QDir::separator() + "shearx_result.png");
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("shearx_source.png");
        result.save("shearx_result.png");
        QFAIL(QString("Failed to shear the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}


void KisTransformWorkerTest::testYShear()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    KisFilterStrategy * filter = new KisBoxFilterStrategy();

    KisTransaction t("test", dev);
    KisTransformWorker tw(dev, 1.0, 1.0,
                          0.0, 1.0,
                          300., 200.,
                          0.0,
                          0, 0, updater, filter);
    tw.run();
    t.end();

    QRect rc = dev->exactBounds();

    QVERIFY(rc.width() == image.width());
    QVERIFY(rc.height() == 959);

//    KisTransaction t2("test", dev);
//    KisRandomAccessorSP ac = dev->createRandomAccessorNG(rc.x(), rc.y());
//    for(int x = rc.x(); x < rc.width(); ++x) {
//        for(int y = rc.y(); y < rc.height(); ++y) {
//            ac->moveTo(x, y);
//            cs->setOpacity(ac->rawData(), 0.5, 1);
//        }
//    }
//    t2.end();

    QImage result = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    QPoint errpoint;
    image.load(QString(FILES_DATA_DIR) + QDir::separator() + "sheary_result.png");
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("sheary_source.png");
        result.save("sheary_result.png");
        QFAIL(QString("Failed to shear the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }

}


bool fuzzyCompareRects(const QRectF rc1, const QRectF rc2, qreal accuracy)
{
    bool result =
        qAbs(rc1.x() - rc2.x()) < accuracy &&
        qAbs(rc1.y() - rc2.y()) < accuracy &&
        qAbs(rc1.width() - rc2.width()) < 2 * accuracy &&
        qAbs(rc1.height() - rc2.height()) < 2 * accuracy;

    if(!result) {
        qDebug() << "Failed to fuzzy compare rects";
        qDebug() << "\t" << ppVar(accuracy);
        qDebug() << "\t" << "actual  " << rc1;
        qDebug() << "\t" << "expected" << rc2;
        qDebug() << "+---------------------------+";
    }

    return result;
}

void KisTransformWorkerTest::testMatrices()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();
    KisFilterStrategy *filter = new KisBoxFilterStrategy();

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    QRect fillRect(0,0,300,200);
    KoColor fillColor(Qt::white, cs);
    dev->fill(fillRect, fillColor);

    qreal scaleX = 1.5, scaleY = 1.5;
    qreal shearX = 1, shearY = 1.33;
    qreal shearOrigX = 150, shearOrigY = 100;
    qreal angle = M_PI/6;
    qreal transX = 77, transY = 33;

    KisTransaction t("test", dev);
    KisTransformWorker tw(dev, scaleX, scaleY,
                          shearX, shearY,
                          shearOrigX, shearOrigY,
                          angle,
                          transX, transY,
                          updater, filter);
    tw.run();
    t.end();

    QPolygonF referencePolygon =
        tw.transform().map(QPolygonF(QRectF(fillRect)));

    QVERIFY(fuzzyCompareRects(dev->exactBounds(),
                              referencePolygon.boundingRect(), 3));
}


void testRotationImpl(qreal angle, QString filePrefix)
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    KisFilterStrategy * filter = new KisBoxFilterStrategy();

    KisTransaction t("test", dev);
    KisTransformWorker tw(dev, 1.0, 1.0,
                          0.0, 0.0,
                          0.0, 0.0,
                          angle,
                          0, 0, updater, filter);
    tw.run();
    t.end();

    QRect rc = dev->exactBounds();

    QImage result = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    QPoint errpoint;

    QString resFileName = QString("%1_result.png").arg(filePrefix);
    QString refFileName = QString("%1_expected.png").arg(filePrefix);

    image = QImage();
    image.load(QString(FILES_DATA_DIR) + QDir::separator() + resFileName);
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        qDebug() << filePrefix;
        image.save(refFileName);
        result.save(resFileName);
        QFAIL(QString("Failed to rotate the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
}

void KisTransformWorkerTest::testRotation()
{
    testRotationImpl(M_PI/6, "rotation_30");
    testRotationImpl(M_PI/3, "rotation_60");
    testRotationImpl(M_PI/2,  "rotation_90");
    testRotationImpl(2*M_PI/3, "rotation_120");
    testRotationImpl(7*M_PI/6, "rotation_210");
    testRotationImpl(5*M_PI/3, "rotation_300");
}

void KisTransformWorkerTest::testRotationSpecialCases()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();
    KisFilterStrategy *filter = new KisBoxFilterStrategy();

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    QRect fillRect(0,0,600,300);
    KoColor fillColor(Qt::white, cs);
    dev->fill(fillRect, fillColor);

    qreal scaleX = 0.5, scaleY = 0.5;
    qreal shearX = 0, shearY = 0;
    qreal shearOrigX = 0, shearOrigY = 0;
    qreal angle = M_PI;
    qreal transX = 300, transY = 150;

    KisTransaction t("test", dev);
    KisTransformWorker tw(dev, scaleX, scaleY,
                          shearX, shearY,
                          shearOrigX, shearOrigY,
                          angle,
                          transX, transY,
                          updater, filter);
    tw.run();
    t.end();

    QCOMPARE(dev->exactBounds(), QRect(0,0,300,150));
}

void KisTransformWorkerTest::testScaleUp5times()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    QImage image(QSize(2000,2000), QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(Qt::green).rgba());

    int checkSize = 20;
    QImage tile(checkSize * 2, checkSize * 2, QImage::Format_ARGB32_Premultiplied);
    QPainter pt(&tile);
    pt.fillRect(tile.rect(), Qt::green);
    pt.fillRect(0, 0, checkSize, checkSize, Qt::white);
    pt.fillRect(checkSize, checkSize, checkSize, checkSize, Qt::white);
    pt.end();

    pt.begin(&image);
    pt.setBrush(QBrush(tile));
    pt.drawRect(image.rect());
    pt.end();

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    KisFilterStrategy * filter = new KisBicubicFilterStrategy();
    KisTransaction t("test", dev);

    qreal SCALE = 5.0;

    KisTransformWorker tw(dev, SCALE, SCALE,
                          0.0, 0.0,
                          0.0, 0.0,
                          0.0,
                          0, 0, updater, filter);
    tw.run();
    t.end();

    QRect rc = dev->exactBounds();

#if 0
    // here you can check the input and result images
    image.save("test_scale_2000_2000_input.bmp");
    QImage result = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    result.save("test_scale_2000_2000_" + QString::number(SCALE) + "_result.bmp");
#endif

    QCOMPARE(rc.width(), qCeil(image.width() * SCALE));
    QCOMPARE(rc.height(), qCeil(image.height() * SCALE));
}

void KisTransformWorkerTest::rotate90Left()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "transform_rotate_test.png");
    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    dev2->convertFromQImage(image, 0);

    QRect boundRect = dev2->exactBounds();

    KisTransaction t("rotate left 90", dev2);
    QRect rc = KisTransformWorker::rotateLeft90(dev2, boundRect, 0, 0);
    t.end();

    QImage result = dev2->convertToQImage(0, rc.x(), rc.y(), image.width(), image.height());
    QTransform tf;
    QImage rotatedimage = image.transformed(tf.rotate(270));

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, rotatedimage, result)) {
        // They are the same, but should be mirrored
        image.save("rotate_90_left_test_1_source.png");
        rotatedimage.save("rotate_90_left_test_1_rotated_source.png");
        result.save("rotate_90_left_test_1_result.png");
        QFAIL(QString("Failed to rotate 90 left the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

void KisTransformWorkerTest::rotate90Right()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "transform_rotate_test.png");
    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    dev2->convertFromQImage(image, 0);

    QRect boundRect = dev2->exactBounds();

    KisTransaction t("rotate right 90", dev2);
    QRect rc = KisTransformWorker::rotateRight90(dev2, boundRect, 0, 0);
    t.end();

    QTransform tf;
    QImage rotatedimage = image.transformed(tf.rotate(90));

    QImage result = dev2->convertToQImage(0, rc.x(), rc.y(), image.width(), image.height());

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, rotatedimage, result)) {
        // They are the same, but should be mirrored
        image.save("rotate_90_right_test_1_source.png");
        rotatedimage.save("rotate_90_right_test_1_rotated_source.png");
        result.save("rotate_90_right_1_result.png");
        QFAIL(QString("Failed to rotate 90 right the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

void KisTransformWorkerTest::rotate180()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "transform_rotate_test.png");
    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    dev2->convertFromQImage(image, 0);

    QRect boundRect = dev2->exactBounds();

    KisTransaction t("rotate 180", dev2);
    QRect rc = KisTransformWorker::rotate180(dev2, boundRect, 0, 0);
    t.end();

    QImage result = dev2->convertToQImage(0, rc.x(), rc.y(), image.width(), image.height());

    QTransform tf;
    QImage rotatedimage = image.transformed(tf.rotate(180));

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, rotatedimage, result)) {
        // They are the same, but should be mirrored
        image.save("rotate_180_1_source.png");
        rotatedimage.save("rotate_180_1_rotated_source.png");
        result.save("rotate_180_1_result.png");
        QFAIL(QString("Failed to rotate 180 the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

void generateTestImage(QString inputFileName, qreal scale, qreal rotation, qreal xshear, KisFilterStrategy *filter, bool saveImage = true)
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + inputFileName);
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    KisTransformWorker tw(dev, scale, scale,
                          xshear, 0.0,
                          0.0, 0.0,
                          rotation,
                          0, 0,
                          updater, filter);


    tw.run();

    if (saveImage) {
        QStringList tmp = inputFileName.split('.');
        QString filename =
            QString("transform_%1_%2_%3_%4_%5_new.png")
            .arg(tmp[0]).arg(scale).arg(rotation).arg(xshear).arg(filter->name());

        qDebug() << filename;

        dev->convertToQImage(0).save(filename);
    }
}

void KisTransformWorkerTest::benchmarkScale()
{
    QBENCHMARK {
        generateTestImage("hakonepa.png", 1.379,0.0,0.0,new KisBicubicFilterStrategy(), false);
    }
}

void KisTransformWorkerTest::benchmarkRotate()
{
    QBENCHMARK {
        generateTestImage("hakonepa.png", 1.0,M_PI/6.0,0.0,new KisBicubicFilterStrategy(), false);
    }
}

void KisTransformWorkerTest::benchmarkRotate1Q()
{
    QBENCHMARK {
        generateTestImage("hakonepa.png", 1.0,2 * M_PI/3.0,0.0,new KisBicubicFilterStrategy(), false);
    }
}

void KisTransformWorkerTest::benchmarkShear()
{
    QBENCHMARK {
        generateTestImage("hakonepa.png", 1.0,0.0,0.479,new KisBicubicFilterStrategy(), false);
    }
}

void KisTransformWorkerTest::benchmarkScaleRotateShear()
{
    QBENCHMARK {
        generateTestImage("hakonepa.png", 1.379,M_PI/6.0,0.479,new KisBicubicFilterStrategy(), false);
    }
}

void KisTransformWorkerTest::generateTestImages()
{
    QList<KisFilterStrategy*> filters;
    filters << new KisBoxFilterStrategy();
    filters << new KisHermiteFilterStrategy();
    filters << new KisBicubicFilterStrategy();
    filters << new KisBilinearFilterStrategy();
    filters << new KisBellFilterStrategy();
    filters << new KisBSplineFilterStrategy();
    filters << new KisLanczos3FilterStrategy();
    filters << new KisMitchellFilterStrategy();

    QStringList names;
    names << "hakonepa.png";
    //names << "star-chart-bars-full-600dpi.png";

    foreach(const QString &name, names) {
        foreach(KisFilterStrategy *filter, filters) {
            generateTestImage(name, 0.5,0.0,0.0,filter);
            generateTestImage(name, 0.734,0.0,0.0,filter);
            generateTestImage(name, 1.387,0.0,0.0,filter);
            generateTestImage(name, 2.0,0.0,0.0,filter);
            generateTestImage(name, 3.789,0.0,0.0,filter);

            generateTestImage(name, 1.0,M_PI/6,0.0,filter);

            generateTestImage(name, 1.0,0.0,0.5,filter);
        }
    }
}

QTEST_KDEMAIN(KisTransformWorkerTest, GUI)
#include "kis_transform_worker_test.moc"
