/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_cage_transform_worker_test.h"

#include <QTest>

#include <KoProgressUpdater.h>
#include <KoUpdater.h>

#include <testutil.h>
#include <kistest.h>

#include <kis_cage_transform_worker.h>
#include <algorithm>

void testCage(bool clockwise, bool unityTransform, bool benchmarkPrepareOnly = false, int pixelPrecision = 8, bool testQImage = false)
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(TestUtil::fetchDataFileLazy("test_cage_transform.png"));

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    KisPaintDeviceSP srcDev = new KisPaintDevice(*dev);

    QVector<QPointF> origPoints;
    QVector<QPointF> transfPoints;

    QRectF bounds(dev->exactBounds());

    origPoints << bounds.topLeft();
    origPoints << 0.5 * (bounds.topLeft() + bounds.topRight());
    origPoints << 0.5 * (bounds.topLeft() + bounds.bottomRight());
    origPoints << 0.5 * (bounds.topRight() + bounds.bottomRight());
    origPoints << bounds.bottomRight();
    origPoints << bounds.bottomLeft();

    if (!clockwise) {
        std::reverse(origPoints.begin(), origPoints.end());
    }

    if (unityTransform) {
        transfPoints = origPoints;
    } else {
        transfPoints << bounds.topLeft();
        transfPoints << 0.5 * (bounds.topLeft() + bounds.topRight());
        transfPoints << 0.5 * (bounds.bottomLeft() + bounds.bottomRight());
        transfPoints << 0.5 * (bounds.bottomLeft() + bounds.bottomRight()) +
            (bounds.bottomLeft() - bounds.topLeft());
        transfPoints << bounds.bottomLeft() +
            (bounds.bottomLeft() - bounds.topLeft());
        transfPoints << bounds.bottomLeft();

        if (!clockwise) {
            std::reverse(transfPoints.begin(), transfPoints.end());
        }
    }

    KisCageTransformWorker worker(dev->region().boundingRect(),
                                  origPoints,
                                  updater,
                                  pixelPrecision);

    QImage result;
    QPointF srcQImageOffset(0, 0);
    QPointF dstQImageOffset;

    QBENCHMARK_ONCE {
        if (!testQImage) {
            worker.prepareTransform();
            if (!benchmarkPrepareOnly) {
                worker.setTransformedCage(transfPoints);
                worker.run(srcDev, dev);

            }
        } else {
            QImage srcImage = image;
            QImage image2 = QImage(image.size(), QImage::Format_ARGB32);
            QPainter gc(&image2);
            gc.drawImage(QPoint(), srcImage);
            gc.end();
            image = image2;

            KisCageTransformWorker qimageWorker(image,
                                                srcQImageOffset,
                                                origPoints,
                                                updater,
                                                pixelPrecision);
            qimageWorker.prepareTransform();
            qimageWorker.setTransformedCage(transfPoints);
            result = qimageWorker.runOnQImage(&dstQImageOffset);
        }
    }

    QString testName = QString("%1_%2")
        .arg(clockwise ? "clk" : "cclk")
        .arg(unityTransform ? "unity" : "normal");

    if (testQImage) {
        QVERIFY(TestUtil::checkQImage(result, "cage_transform_test", "cage_qimage", testName, 1, 1));
    } else if (!benchmarkPrepareOnly && pixelPrecision == 8) {

        result = dev->convertToQImage(0);
        QVERIFY(TestUtil::checkQImage(result, "cage_transform_test", "cage", testName, 1, 1));
    }
}

void KisCageTransformWorkerTest::testCageClockwise()
{
    testCage(true, false);
}

void KisCageTransformWorkerTest::testCageClockwisePrepareOnly()
{
    testCage(true, false, true);
}

void KisCageTransformWorkerTest::testCageClockwisePixePrecision4()
{
    testCage(true, false, false, 4);
}

void KisCageTransformWorkerTest::testCageClockwisePixePrecision8QImage()
{
    testCage(true, false, false, 8, true);
}

void KisCageTransformWorkerTest::testCageCounterclockwise()
{
    testCage(false, false);
}

void KisCageTransformWorkerTest::testCageClockwiseUnity()
{
    testCage(true, true);
}

void KisCageTransformWorkerTest::testCageCounterclockwiseUnity()
{
    testCage(false, true);
}

#include <QtGlobal>


QPointF generatePoint(const QRectF &rc)
{
    qreal cx = qreal(qrand()) / RAND_MAX;
    qreal cy = qreal(qrand()) / RAND_MAX;

    QPointF diff = rc.bottomRight() - rc.topLeft();

    QPointF pt = rc.topLeft() + QPointF(cx * diff.x(), cy * diff.y());
    return pt;
}

void KisCageTransformWorkerTest::stressTestRandomCages()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(TestUtil::fetchDataFileLazy("test_cage_transform.png"));

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    KisPaintDeviceSP dstDev = new KisPaintDevice(cs);

    const int pixelPrecision = 8;
    QRectF bounds(dev->exactBounds());

    qsrand(1);

    for (int numPoints = 4; numPoints < 15; numPoints+=5) {
        for (int j = 0; j < 200; j++) {
            QVector<QPointF> origPoints;
            QVector<QPointF> transfPoints;

            dbgKrita << ppVar(j);

            for (int i = 0; i < numPoints; i++) {
                origPoints << generatePoint(bounds);
                transfPoints << generatePoint(bounds);
            }

            // no just hope it doesn't crash ;)
            KisCageTransformWorker worker(dev->region().boundingRect(),
                                          origPoints,
                                          updater,
                                          pixelPrecision);
            worker.prepareTransform();
            worker.setTransformedCage(transfPoints);
            worker.run(dev, dstDev);
        }
    }
}

#include "kis_green_coordinates_math.h"

void KisCageTransformWorkerTest::testUnityGreenCoordinates()
{
    QVector<QPointF> origPoints;
    QVector<QPointF> transfPoints;

    QRectF bounds(0,0,300,300);

    origPoints << bounds.topLeft();
    origPoints << 0.5 * (bounds.topLeft() + bounds.topRight());
    origPoints << 0.5 * (bounds.topLeft() + bounds.bottomRight());
    origPoints << 0.5 * (bounds.topRight() + bounds.bottomRight());
    origPoints << bounds.bottomRight();
    origPoints << bounds.bottomLeft();

    transfPoints = origPoints;

    QVector<QPointF> points;
    points << QPointF(10,10);
    points << QPointF(140,10);
    points << QPointF(140,140);
    points << QPointF(10,140);

    points << QPointF(10,160);
    points << QPointF(140,160);
    points << QPointF(140,290);
    points << QPointF(10,290);

    points << QPointF(160,160);
    points << QPointF(290,160);
    points << QPointF(290,290);
    points << QPointF(160,290);

    KisGreenCoordinatesMath cage;

    cage.precalculateGreenCoordinates(origPoints, points);
    cage.generateTransformedCageNormals(transfPoints);

    QVector<QPointF> newPoints;

    for (int i = 0; i < points.size(); i++) {
        newPoints << cage.transformedPoint(i, transfPoints);
        QCOMPARE(points[i], newPoints.last());
    }
}

#include "kis_algebra_2d.h"

void KisCageTransformWorkerTest::testTransformAsBase()
{
    QPointF t(1.0, 0.0);
    QPointF b1(1.0, 0.0);
    QPointF b2(2.0, 0.0);
    QPointF result;


    t = QPointF(1.0, 0.0);
    b1 = QPointF(1.0, 0.0);
    b2 = QPointF(2.0, 0.0);
    result = KisAlgebra2D::transformAsBase(t, b1, b2);
    QCOMPARE(result, QPointF(2.0, 0.0));

    t = QPointF(1.0, 0.0);
    b1 = QPointF(1.0, 0.0);
    b2 = QPointF(0.0, 1.0);
    result = KisAlgebra2D::transformAsBase(t, b1, b2);
    QCOMPARE(result, QPointF(0.0, 1.0));

    t = QPointF(1.0, 0.0);
    b1 = QPointF(1.0, 0.0);
    b2 = QPointF(0.0, 2.0);
    result = KisAlgebra2D::transformAsBase(t, b1, b2);
    QCOMPARE(result, QPointF(0.0, 2.0));

    t = QPointF(0.0, 1.0);
    b1 = QPointF(1.0, 0.0);
    b2 = QPointF(2.0, 0.0);
    result = KisAlgebra2D::transformAsBase(t, b1, b2);
    QCOMPARE(result, QPointF(0.0, 2.0));

    t = QPointF(0.0, 1.0);
    b1 = QPointF(1.0, 0.0);
    b2 = QPointF(0.0, 1.0);
    result = KisAlgebra2D::transformAsBase(t, b1, b2);
    QCOMPARE(result, QPointF(-1.0, 0.0));

    t = QPointF(0.0, 1.0);
    b1 = QPointF(1.0, 0.0);
    b2 = QPointF(0.0, 2.0);
    result = KisAlgebra2D::transformAsBase(t, b1, b2);
    QCOMPARE(result, QPointF(-2.0, 0.0));
}

void KisCageTransformWorkerTest::testAngleBetweenVectors()
{
    QPointF b1(1.0, 0.0);
    QPointF b2(2.0, 0.0);
    qreal result;

    b1 = QPointF(1.0, 0.0);
    b2 = QPointF(0.0, 1.0);
    result = KisAlgebra2D::angleBetweenVectors(b1, b2);
    QCOMPARE(result, M_PI_2);

    b1 = QPointF(1.0, 0.0);
    b2 = QPointF(std::sqrt(0.5), std::sqrt(0.5));
    result = KisAlgebra2D::angleBetweenVectors(b1, b2);
    QCOMPARE(result, M_PI / 4);

    QTransform t;
    t.rotateRadians(M_PI / 4);
    QCOMPARE(t.map(b1), b2);
}

KISTEST_MAIN(KisCageTransformWorkerTest)
