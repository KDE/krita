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
                          0, 0, updater, filter, true);
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
        QFAIL(QString("Failed to mirror the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
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
        QFAIL(QString("Failed to mirror the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
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
        QFAIL(QString("Failed to mirror the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());

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
        QFAIL(QString("Failed to mirror the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
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
                          0, 0, updater, filter, true);
    tw.run();
    t.end();

    QRect rc = dev->exactBounds();

    QCOMPARE(rc.width(), qRound(image.width() * 2.4));
    QCOMPARE(rc.height(), qRound(image.height() * 2.4));

    QImage result = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    QPoint errpoint;
    image.load(QString(FILES_DATA_DIR) + QDir::separator() + "test_scaleup_result.png");
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("test_scaleup_source.png");
        result.save("test_scaleup_result.png");
        QFAIL(QString("Failed to scale the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
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
                          0, 0, updater, filter, true);
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
        QFAIL(QString("Failed to scale up the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
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
                          0, 0, updater, filter, true);
    tw.run();
    t.end();

    QRect rc = dev->exactBounds();

    QVERIFY(rc.width() == image.width());
    QVERIFY(rc.height() == image.height() * 2);

    QImage result = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    QPoint errpoint;
    image.load(QString(FILES_DATA_DIR) + QDir::separator() + "scaleupy_result.png");
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("test_y_scaleup_source.png");
        result.save("test_y_scaleup_result.png");
        QFAIL(QString("Failed to scale up the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
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
                          0, 0, updater, filter, true);
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
        QFAIL(QString("Failed to apply identity transformation to image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
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
                          0, 0, updater, filter, true);
    tw.run();
    t.end();

    QRect rc = dev->exactBounds();

    QVERIFY(rc.width() == qRound(image.width() * 0.123));
    QVERIFY(rc.height() == qRound(image.height() * 0.123));

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
        QFAIL(QString("Failed to scale down the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
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
                          0, 0, updater, filter, true);
    tw.run();
    t.end();

    QRect rc = dev->exactBounds();

    QVERIFY(rc.width() == qRound(image.width() * 0.123));
    QVERIFY(rc.height() == image.height() - 1); // the height is reduced by 1 because in the source image
                                                // at the bottom line most pixels (except 1 or 2) are
                                                // entirely transparent.
                                                // when scaling down the image by ~ 1/10, the few non-tranparent
                                                // pixels disappear when "mixed" with the transparent ones
                                                // around

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
    image.load(QString(FILES_DATA_DIR) + QDir::separator() + "scaledownx_result.png");
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("scaledownx_source.png");
        result.save("scaledownx_result.png");
        QFAIL(QString("Failed to scale down the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
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
                          0, 0, updater, filter, true);
    tw.run();
    t.end();

    QRect rc = dev->exactBounds();

    QVERIFY(rc.width() == image.width());
    QVERIFY(rc.height() == qRound(image.height() * 0.123));

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
    image.load(QString(FILES_DATA_DIR) + QDir::separator() + "scaledowny_result.png");
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("scaledowny_source.png");
        result.save("scaledowny_result.png");
        QFAIL(QString("Failed to scale down the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
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
                          0, 0, updater, filter, true);
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
        QFAIL(QString("Failed to shear the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
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
                          0, 0, updater, filter, true);
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
        QFAIL(QString("Failed to shear the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
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
                          updater, filter, true);
    tw.run();
    t.end();

    QPolygonF referencePolygon =
        tw.transform().map(QPolygonF(QRectF(fillRect)));

    QVERIFY(fuzzyCompareRects(dev->exactBounds(),
                              referencePolygon.boundingRect(), 3));
}


void KisTransformWorkerTest::testRotation()
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
                          34,
                          0, 0, updater, filter, true);
    tw.run();
    t.end();

    QRect rc = dev->exactBounds();

    QCOMPARE(rc.width(), 702);
    QCOMPARE(rc.height(), 629);

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
    image.load(QString(FILES_DATA_DIR) + QDir::separator() + "rotate_result.png");
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("rotate_source.png");
        result.save("rotate_result.png");
        QFAIL(QString("Failed to rotate the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
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
                          updater, filter, true);
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
                          0, 0, updater, filter, true);
    tw.run();
    t.end();

    QRect rc = dev->exactBounds();

#if 0
    // here you can check the input and result images
    image.save("test_scale_2000_2000_input.bmp");
    QImage result = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    result.save("test_scale_2000_2000_" + QString::number(SCALE) + "_result.bmp");
#endif

    QCOMPARE(rc.width(), qRound(image.width() * SCALE));
    QCOMPARE(rc.height(), qRound(image.height() * SCALE));
}

void KisTransformWorkerTest::rotateNone()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "transform_rotate_test.png");
    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    dev2->convertFromQImage(image, 0);

    KisPaintDeviceSP tmpdev1 = new KisPaintDevice(dev2->colorSpace());
    tmpdev1->setDefaultPixel(dev2->defaultPixel());

    int lastProgressReport = 0;
    int progressTotalSteps = 0;
    int progresStep = 0;
    QRect boundRect = dev2->exactBounds();

    KisTransaction t("mirror", dev2);
    QRect rc = KisTransformWorker::rotateNone(dev2, tmpdev1, boundRect, 0, lastProgressReport, progressTotalSteps, progresStep);
    t.end();

    QImage result = tmpdev1->convertToQImage(0, rc.x(), rc.y(), image.width(), image.height());

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        // They are the same, but should be mirrored
        image.save("rotate_none_test_1_source.png");
        result.save("rotate_none_1_result.png");
        QFAIL(QString("Failed to rotate none the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
}


void KisTransformWorkerTest::rotate90Left()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "transform_rotate_test.png");
    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    dev2->convertFromQImage(image, 0);

    KisPaintDeviceSP tmpdev1 = new KisPaintDevice(dev2->colorSpace());
    tmpdev1->setDefaultPixel(dev2->defaultPixel());

    int lastProgressReport = 0;
    int progressTotalSteps = 0;
    int progresStep = 0;
    QRect boundRect = dev2->exactBounds();

    KisTransaction t("rotate left 90", dev2);
    QRect rc = KisTransformWorker::rotateLeft90(dev2, tmpdev1, boundRect, 0, lastProgressReport, progressTotalSteps, progresStep);
    t.end();

    QImage result = tmpdev1->convertToQImage(0, rc.x(), rc.y(), image.width(), image.height());
    QTransform tf;
    QImage rotatedimage = image.transformed(tf.rotate(270));

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, rotatedimage, result)) {
        // They are the same, but should be mirrored
        image.save("rotate_90_left_test_1_source.png");
        rotatedimage.save("rotate_90_left_test_1_rotated_source.png");
        result.save("rotate_90_left_test_1_result.png");
        QFAIL(QString("Failed to rotate 90 left the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
}

void KisTransformWorkerTest::rotate90Right()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "transform_rotate_test.png");
    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    dev2->convertFromQImage(image, 0);

    int lastProgressReport = 0;
    int progressTotalSteps = 0;
    int progresStep = 0;
    QRect boundRect = dev2->exactBounds();

    KisPaintDeviceSP tmpdev1 = new KisPaintDevice(dev2->colorSpace());
    tmpdev1->setDefaultPixel(dev2->defaultPixel());

    KisTransaction t("rotate right 90", dev2);
    QRect rc = KisTransformWorker::rotateRight90(dev2, tmpdev1, boundRect, 0, lastProgressReport, progressTotalSteps, progresStep);
    t.end();

    QTransform tf;
    QImage rotatedimage = image.transformed(tf.rotate(90));

    QImage result = tmpdev1->convertToQImage(0, rc.x(), rc.y(), image.width(), image.height());

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, rotatedimage, result)) {
        // They are the same, but should be mirrored
        image.save("rotate_90_right_test_1_source.png");
        rotatedimage.save("rotate_90_right_test_1_rotated_source.png");
        result.save("rotate_90_right_1_result.png");
        QFAIL(QString("Failed to rotate 90 right the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
}

void KisTransformWorkerTest::rotate180()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "transform_rotate_test.png");
    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    dev2->convertFromQImage(image, 0);

    int lastProgressReport = 0;
    int progressTotalSteps = 0;
    int progresStep = 0;
    QRect boundRect = dev2->exactBounds();

    KisPaintDeviceSP tmpdev1 = new KisPaintDevice(dev2->colorSpace());
    tmpdev1->setDefaultPixel(dev2->defaultPixel());

    KisTransaction t("rotate 180", dev2);
    QRect rc = KisTransformWorker::rotate180(dev2, tmpdev1, boundRect, 0, lastProgressReport, progressTotalSteps, progresStep);
    t.end();

    QImage result = tmpdev1->convertToQImage(0, rc.x(), rc.y(), image.width(), image.height());

    QTransform tf;
    QImage rotatedimage = image.transformed(tf.rotate(180));

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, rotatedimage, result)) {
        // They are the same, but should be mirrored
        image.save("rotate_180_1_source.png");
        rotatedimage.save("rotate_180_1_rotated_source.png");
        result.save("rotate_180_1_result.png");
        QFAIL(QString("Failed to rotate 180 the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
}


QTEST_KDEMAIN(KisTransformWorkerTest, GUI)
#include "kis_transform_worker_test.moc"
