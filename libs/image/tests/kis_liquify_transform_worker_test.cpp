/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_liquify_transform_worker_test.h"

#include <QTest>

#include <KoColor.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>

#include <testutil.h>
#include <kis_liquify_transform_worker.h>
#include <kis_algebra_2d.h>


void KisLiquifyTransformWorkerTest::testPoints()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(TestUtil::fetchDataFileLazy("test_transform_quality_second.png"));

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    KisPaintDeviceSP srcDev = new KisPaintDevice(*dev);

    const int pixelPrecision = 8;

    KisLiquifyTransformWorker worker(dev->exactBounds(),
                                     updater,
                                     pixelPrecision);


    QBENCHMARK_ONCE {
        worker.translatePoints(QPointF(100,100),
                               QPointF(50, 0),
                               50, false, 0.2);

        worker.scalePoints(QPointF(400,100),
                           0.9,
                           50, false, 0.2);

        worker.undoPoints(QPointF(400,100),
                           1.0,
                           50);

        worker.scalePoints(QPointF(400,300),
                           0.5,
                           50, false, 0.2);

        worker.scalePoints(QPointF(100,300),
                           -0.5,
                           30, false, 0.2);

        worker.rotatePoints(QPointF(100,500),
                            M_PI / 4,
                            50, false, 0.2);
    }

    worker.run(srcDev, dev);

    QImage result = dev->convertToQImage(0);
    TestUtil::checkQImage(result, "liquify_transform_test", "liquify_dev", "unity");
}

void KisLiquifyTransformWorkerTest::testPointsQImage()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(TestUtil::fetchDataFileLazy("test_transform_quality_second.png"));

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    KisPaintDeviceSP srcDev = new KisPaintDevice(*dev);

    const int pixelPrecision = 8;

    KisLiquifyTransformWorker worker(dev->exactBounds(),
                                     updater,
                                     pixelPrecision);


    worker.translatePoints(QPointF(100,100),
                           QPointF(50, 0),
                           50, false, 0.2);

    QRect rc = dev->exactBounds();
    dev->setX(50);
    dev->setY(50);
    worker.run(srcDev, dev);
    rc |= dev->exactBounds();

    QImage resultDev = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    TestUtil::checkQImage(resultDev, "liquify_transform_test", "liquify_qimage", "refDevice");

    QTransform imageToThumbTransform =
        QTransform::fromScale(0.5, 0.5);

    QImage srcImage(image);
    image = QImage(image.size(), QImage::Format_ARGB32);
    QPainter gc(&image);
    gc.setTransform(imageToThumbTransform);
    gc.drawImage(QPoint(), srcImage);

    QPointF newOffset;
    QImage result = worker.runOnQImage(image, QPointF(10, 10), imageToThumbTransform, &newOffset);
    dbgKrita << ppVar(newOffset);


    TestUtil::checkQImage(result, "liquify_transform_test", "liquify_qimage", "resultImage");
}

void KisLiquifyTransformWorkerTest::testIdentityTransform()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();

    QRect rc(0,0,13,23);

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->fill(rc, KoColor(Qt::blue, cs));

    KisPaintDeviceSP srcDev = new KisPaintDevice(*dev);

    const int pixelPrecision = 8;

    KisLiquifyTransformWorker worker(dev->exactBounds(),
                                     updater,
                                     pixelPrecision);

    worker.run(srcDev, dev);

    QImage result = dev->convertToQImage(0, rc);
    TestUtil::checkQImage(result, "liquify_transform_test", "liquify_dev", "identity");
}

QTEST_MAIN(KisLiquifyTransformWorkerTest)
