/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_transform_worker_test.h"

#include <QTest>
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
#include <testutil.h>
#include "kistest.h"
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

void testMirror(const QRect &imageRect, const QRect &mirrorRect, Qt::Orientation orientation)
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    qreal axis = QRectF(mirrorRect).center().x();

    KisRandomAccessorSP it = dev->createRandomAccessorNG();

    int i = 0;
    for (int y = imageRect.y(); y < imageRect.y() + imageRect.height(); y++) {
        for (int x = imageRect.x(); x < imageRect.x() + imageRect.width(); x++) {
            it->moveTo(x, y);

            *reinterpret_cast<quint32*>(it->rawData()) = 0xFFFFFFFF - i++;
        }
    }

    QCOMPARE(dev->exactBounds(), imageRect);


    QImage srcImage = dev->convertToQImage(0, mirrorRect.x(), mirrorRect.y(), mirrorRect.width(), mirrorRect.height());
    QImage mirroredImage = srcImage.mirrored(orientation == Qt::Horizontal, orientation == Qt::Vertical);
    QImage result;

    //srcImage.save("input.png");
    //mirroredImage.save("mirror_expected.png");

    QBENCHMARK_ONCE {
        KisTransformWorker::mirror(dev, axis, orientation);
    }
    result = dev->convertToQImage(0, mirrorRect.x(), mirrorRect.y(), mirrorRect.width(), mirrorRect.height());
    QCOMPARE(result, mirroredImage);

    //result.save("mirror1.png");

    KisTransformWorker::mirror(dev, axis, orientation);
    result = dev->convertToQImage(0, mirrorRect.x(), mirrorRect.y(), mirrorRect.width(), mirrorRect.height());
    QCOMPARE(result, srcImage);

    //result.save("mirror2.png");

    KisTransformWorker::mirror(dev, axis, orientation);
    result = dev->convertToQImage(0, mirrorRect.x(), mirrorRect.y(), mirrorRect.width(), mirrorRect.height());
    QCOMPARE(result, mirroredImage);

    //result.save("mirror3.png");
}

void KisTransformWorkerTest::testMirrorX_Even()
{
    testMirror(QRect(10,10,30,30), QRect(1,1,70,70), Qt::Horizontal);
}

void KisTransformWorkerTest::testMirrorX_Odd()
{
    testMirror(QRect(10,10,30,30), QRect(1,1,71,71), Qt::Horizontal);
}

void KisTransformWorkerTest::testMirrorY_Even()
{
    testMirror(QRect(10,10,30,30), QRect(1,1,70,70), Qt::Vertical);
}

void KisTransformWorkerTest::testMirrorY_Odd()
{
    testMirror(QRect(10,10,30,30), QRect(1,1,71,71), Qt::Vertical);
}

void KisTransformWorkerTest::benchmarkMirrorX()
{
    testMirror(QRect(10,10,4000,4000), QRect(1,1,7000,7000), Qt::Horizontal);
}

void KisTransformWorkerTest::benchmarkMirrorY()
{
    testMirror(QRect(10,10,4000,4000), QRect(1,1,7000,7000), Qt::Vertical);
}

void KisTransformWorkerTest::testOffset()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QString imageName("mirror_source.png");
    QImage image(QString(FILES_DATA_DIR) + '/' + imageName);
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
    Q_FOREACH (QPoint offsetPoint, offsetPoints)
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
    QImage image(QString(FILES_DATA_DIR) + '/' + "mirror_source.png");
    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    dev2->convertFromQImage(image, 0);

    KisTransaction t(kundo2_noi18n("mirror"), dev2);
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
    QImage image(QString(FILES_DATA_DIR) + '/' + "mirror_source.png");
    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    dev2->convertFromQImage(image, 0);

    KisTransaction t(kundo2_noi18n("mirror"), dev2);
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
    QImage image(QString(FILES_DATA_DIR) + '/' + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    KisFilterStrategy * filter = new KisBoxFilterStrategy();
    KisTransaction t(dev);
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
    image.load(QString(FILES_DATA_DIR) + '/' + "test_scaleup_result.png");
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
    QImage image(QString(FILES_DATA_DIR) + '/' + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    KisFilterStrategy * filter = new KisBoxFilterStrategy();
    KisTransaction t(dev);
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
    image.load(QString(FILES_DATA_DIR) + '/' + "scaleupx_result.png");
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
    QImage image(QString(FILES_DATA_DIR) + '/' + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    KisFilterStrategy * filter = new KisBoxFilterStrategy();

    KisTransaction t(dev);
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
    image.load(QString(FILES_DATA_DIR) + '/' + "scaleupy_result.png");
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
    QImage image(QString(FILES_DATA_DIR) + '/' + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);
    KisFilterStrategy * filter = new KisBoxFilterStrategy();

    KisTransaction t(dev);
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
    QImage image(QString(FILES_DATA_DIR) + '/' + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);
    KisFilterStrategy * filter = new KisBoxFilterStrategy();

    KisTransaction t(dev);
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
    image.load(QString(FILES_DATA_DIR) + '/' + "test_scaledown_result.png");
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
    QImage image(QString(FILES_DATA_DIR) + '/' + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    KisFilterStrategy * filter = new KisBoxFilterStrategy();

    KisTransaction t(dev);
    KisTransformWorker tw(dev, 0.123, 1.0,
                          0.0, 0.0,
                          0.0, 0.0,
                          0.0,
                          0, 0, updater, filter);
    tw.run();
    t.end();

    QRect rc = dev->exactBounds();
    QImage result = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    QPoint errpoint;
    image.load(QString(FILES_DATA_DIR) + '/' + "scaledownx_result.png");
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
    QImage image(QString(FILES_DATA_DIR) + '/' + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    KisFilterStrategy * filter = new KisBoxFilterStrategy();
    KisTransaction t(dev);
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
    image.load(QString(FILES_DATA_DIR) + '/' + "scaledowny_result.png");
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
    QImage image(QString(FILES_DATA_DIR) + '/' + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    KisFilterStrategy * filter = new KisBoxFilterStrategy();

    KisTransaction t(dev);
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
    image.load(QString(FILES_DATA_DIR) + '/' + "shearx_result.png");
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
    QImage image(QString(FILES_DATA_DIR) + '/' + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    KisFilterStrategy * filter = new KisBoxFilterStrategy();

    KisTransaction t(dev);
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
    image.load(QString(FILES_DATA_DIR) + '/' + "sheary_result.png");
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
        dbgKrita << "Failed to fuzzy compare rects";
        dbgKrita << "\t" << ppVar(accuracy);
        dbgKrita << "\t" << "actual  " << rc1;
        dbgKrita << "\t" << "expected" << rc2;
        dbgKrita << "+---------------------------+";
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

    KisTransaction t(dev);
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


void testRotationImpl(qreal angle, QString filePrefix, bool useUniformColor = false, const QString &filterId = "NearestNeighbor")
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    QImage image;

    if (!useUniformColor) {
        image = QImage(QString(FILES_DATA_DIR) + '/' + "mirror_source.png");
        dev->convertFromQImage(image, 0);
    } else {
        dev->fill(QRect(120, 130, 374, 217), KoColor(QColor(150, 180, 230), cs));
    }

    KisFilterStrategy * filter = KisFilterStrategyRegistry::instance()->value(filterId);
    Q_ASSERT(filter);

    KisTransaction t(dev);
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
    if (!TestUtil::compareQImagesPremultiplied(errpoint, image, result, 2, 1)) {
        dbgKrita << filePrefix;
        image.save(refFileName);
        result.save(resFileName);
        QFAIL(QString("Failed to rotate the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
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
    testRotationImpl(M_PI/6, "rotation_30_uniform_blin", true, "Bilinear");
    testRotationImpl(M_PI/6, "rotation_30_uniform_bcub", true, "Bicubic");
    testRotationImpl(M_PI/6, "rotation_30_uniform_lanc", true, "Lanczos3");
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

    KisTransaction t(dev);
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
    KisTransaction t(dev);

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
    QImage image(QString(FILES_DATA_DIR) + '/' + "transform_rotate_test.png");
    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    dev2->convertFromQImage(image, 0);

    QRect boundRect = dev2->exactBounds();

    KisTransaction t(kundo2_noi18n("rotate left 90"), dev2);
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
    QImage image(QString(FILES_DATA_DIR) + '/' + "transform_rotate_test.png");
    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    dev2->convertFromQImage(image, 0);

    QRect boundRect = dev2->exactBounds();

    KisTransaction t(kundo2_noi18n("rotate right 90"), dev2);
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
    QImage image(QString(FILES_DATA_DIR) + '/' + "transform_rotate_test.png");
    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    dev2->convertFromQImage(image, 0);

    QRect boundRect = dev2->exactBounds();

    KisTransaction t(kundo2_noi18n("rotate 180"), dev2);
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
    QImage image(QString(FILES_DATA_DIR) + '/' + inputFileName);
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

        dbgKrita << filename;

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

    Q_FOREACH (const QString &name, names) {
        Q_FOREACH (KisFilterStrategy *filter, filters) {
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

#include "kis_perspectivetransform_worker.h"


void KisTransformWorkerTest::testPartialProcessing()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(TestUtil::fetchDataFileLazy("test_transform_quality.png"));
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    KisTransaction t(dev);

    QTransform transform = QTransform::fromScale(2.0, 1.1);
    transform.shear(1.1, 0);
    transform.rotateRadians(M_PI / 18);

    KisPerspectiveTransformWorker tw(0, transform, updater);
    tw.runPartialDst(dev, dev, QRect(1200, 1200, 150, 150));
    tw.runPartialDst(dev, dev, QRect(1350, 1200, 150, 150));
    tw.runPartialDst(dev, dev, QRect(1200, 1350, 150, 150));
    tw.runPartialDst(dev, dev, QRect(1350, 1350, 150, 150));

    t.end();

    QImage result = dev->convertToQImage(0);
    TestUtil::checkQImage(result, "transform_test", "partial", "single");
}

KISTEST_MAIN(KisTransformWorkerTest)
