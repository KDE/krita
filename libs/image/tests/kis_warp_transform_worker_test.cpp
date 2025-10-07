/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_warp_transform_worker_test.h"

#include <simpletest.h>
#include <testutil.h>

#include "kis_warptransform_worker.h"

#include <KoProgressUpdater.h>

struct WarpTransformWorkerData {

    WarpTransformWorkerData() {
        TestUtil::TestProgressBar bar;
        KoProgressUpdater pu(&bar);
        updater = pu.startSubtask();

        const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
        // QImage image(TestUtil::fetchDataFileLazy("test_transform_quality.png"));
        QImage image(TestUtil::fetchDataFileLazy("test_transform_quality_second.png"));

        dev = new KisPaintDevice(cs);
        dev->convertFromQImage(image, 0);

        alpha = 1.0;

        bounds = dev->exactBounds();

        origPoints << bounds.topLeft();
        origPoints << bounds.topRight();
        origPoints << bounds.bottomRight();
        origPoints << bounds.bottomLeft();

        origPoints << 0.5 * (bounds.bottomLeft() + bounds.bottomRight());
        origPoints << 0.5 * (bounds.bottomLeft() + bounds.bottomRight()) + QPointF(-20, 0);


        transfPoints << bounds.topLeft();
        transfPoints << bounds.bottomLeft() + 0.6 * (bounds.topRight() - bounds.bottomLeft());
        transfPoints << bounds.topLeft() + 0.8 * (bounds.bottomRight() - bounds.topLeft());
        transfPoints << bounds.bottomLeft() + QPointF(200, 0);

        transfPoints << 0.5 * (bounds.bottomLeft() + bounds.bottomRight()) + QPointF(40,20);
        transfPoints << 0.5 * (bounds.bottomLeft() + bounds.bottomRight()) + QPointF(-20, 0) + QPointF(-40,20);
    }


    KisPaintDeviceSP dev;
    QVector<QPointF> origPoints;
    QVector<QPointF> transfPoints;
    qreal alpha;
    KoUpdaterPtr updater;
    QRectF bounds;
};


void KisWarpTransformWorkerTest::test()
{
    WarpTransformWorkerData d;
    KisPaintDeviceSP srcDev = new KisPaintDevice(*d.dev);

    KisWarpTransformWorker worker(KisWarpTransformWorker::RIGID_TRANSFORM,
                                  d.origPoints,
                                  d.transfPoints,
                                  d.alpha,
                                  d.updater);

    QBENCHMARK_ONCE {
        worker.run(srcDev, d.dev);
    }

    QImage result = d.dev->convertToQImage(0);

    TestUtil::checkQImage(result, "warp_transform_test", "simple", "tr");
}

void KisWarpTransformWorkerTest::testQImage()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

//    QImage image(TestUtil::fetchDataFileLazy("test_transform_quality.png"));
    QImage image(TestUtil::fetchDataFileLazy("test_transform_quality_second.png"));
    image.convertTo(QImage::Format_ARGB32);

    dbgKrita << ppVar(image.format());


    QVector<QPointF> origPoints;
    QVector<QPointF> transfPoints;
    qreal alpha = 1.0;

    QRectF bounds(image.rect());

    origPoints << bounds.topLeft();
    origPoints << bounds.topRight();
    origPoints << bounds.bottomRight();
    origPoints << bounds.bottomLeft();

    origPoints << 0.5 * (bounds.bottomLeft() + bounds.bottomRight());
    origPoints << 0.5 * (bounds.bottomLeft() + bounds.bottomRight()) + QPointF(-20, 0);


    transfPoints << bounds.topLeft();
    transfPoints << bounds.bottomLeft() + 0.6 * (bounds.topRight() - bounds.bottomLeft());
    transfPoints << bounds.topLeft() + 0.8 * (bounds.bottomRight() - bounds.topLeft());
    transfPoints << bounds.bottomLeft() + QPointF(200, 0);

    transfPoints << 0.5 * (bounds.bottomLeft() + bounds.bottomRight()) + QPointF(40,20);
    transfPoints << 0.5 * (bounds.bottomLeft() + bounds.bottomRight()) + QPointF(-20, 0) + QPointF(-40,20);


    QImage result;
    QPointF newOffset;

    QBENCHMARK_ONCE {
        result = KisWarpTransformWorker::transformQImage(
            KisWarpTransformWorker::RIGID_TRANSFORM,
            origPoints, transfPoints, alpha,
            image, QPointF(), &newOffset);
    }

    dbgKrita << ppVar(newOffset);

    TestUtil::checkQImage(result, "warp_transform_test", "qimage", "tr");
}

#include "kis_grid_interpolation_tools.h"

void KisWarpTransformWorkerTest::testGridSize()
{
    QCOMPARE(GridIterationTools::calcGridDimension(1, 7, 4), 3);
    QCOMPARE(GridIterationTools::calcGridDimension(1, 8, 4), 3);
    QCOMPARE(GridIterationTools::calcGridDimension(1, 9, 4), 4);
    QCOMPARE(GridIterationTools::calcGridDimension(0, 7, 4), 3);
    QCOMPARE(GridIterationTools::calcGridDimension(1, 8, 4), 3);
    QCOMPARE(GridIterationTools::calcGridDimension(4, 9, 4), 3);
    QCOMPARE(GridIterationTools::calcGridDimension(0, 9, 4), 4);
    QCOMPARE(GridIterationTools::calcGridDimension(-1, 9, 4), 5);

    QCOMPARE(GridIterationTools::calcGridDimension(0, 300, 8), 39);
}

void KisWarpTransformWorkerTest::testBackwardInterpolatorExtrapolation()
{
    QPolygonF src;

    src << QPointF(0, 0);
    src << QPointF(100, 0);
    src << QPointF(100, 100);
    src << QPointF(0, 100);

    QPolygonF dst(src);
    std::rotate(dst.begin(), dst.begin() + 1, dst.end());
    KisFourPointInterpolatorBackward interp(src, dst);

    // standard checks
    QCOMPARE(interp.map(QPointF(0,0)), QPointF(0,100));
    QCOMPARE(interp.map(QPointF(100,0)), QPointF(0,0));
    QCOMPARE(interp.map(QPointF(100,100)), QPointF(100,0));
    QCOMPARE(interp.map(QPointF(0,100)), QPointF(100,100));

    // extrapolate!
    QCOMPARE(interp.map(QPointF(-10,0)), QPointF(0,110));
    QCOMPARE(interp.map(QPointF(0,-10)), QPointF(-10,100));
    QCOMPARE(interp.map(QPointF(-10,-10)), QPointF(-10,110));

    QCOMPARE(interp.map(QPointF(110,0)), QPointF(0,-10));
    QCOMPARE(interp.map(QPointF(100,-10)), QPointF(-10,0));
    QCOMPARE(interp.map(QPointF(110,-10)), QPointF(-10,-10));

    QCOMPARE(interp.map(QPointF(110,100)), QPointF(100, -10));
    QCOMPARE(interp.map(QPointF(100,110)), QPointF(110, 0));
    QCOMPARE(interp.map(QPointF(110,110)), QPointF(110,-10));

    QCOMPARE(interp.map(QPointF(-10,100)), QPointF(100, 110));
    QCOMPARE(interp.map(QPointF(0,110)), QPointF(110, 100));
    QCOMPARE(interp.map(QPointF(-10,110)), QPointF(110,110));
}
#include "krita_utils.h"
void KisWarpTransformWorkerTest::testNeedChangeRects()
{
    WarpTransformWorkerData d;
    KisWarpTransformWorker worker(KisWarpTransformWorker::RIGID_TRANSFORM,
                                  d.origPoints,
                                  d.transfPoints,
                                  d.alpha,
                                  d.updater);

    QCOMPARE(KisAlgebra2D::sampleRectWithPoints(d.bounds.toAlignedRect()).size(), 9);
    QCOMPARE(worker.approxChangeRect(d.bounds.toAlignedRect()), QRect(-44,-44, 982,986));
}


SIMPLE_TEST_MAIN(KisWarpTransformWorkerTest)
