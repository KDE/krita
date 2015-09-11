/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_warp_transform_worker_test.h"

#include <qtest_kde.h>
#include "testutil.h"

#include "kis_warptransform_worker.h"

#include <KoProgressUpdater.h>


void KisWarpTransformWorkerTest::test()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
//    QImage image(TestUtil::fetchDataFileLazy("test_transform_quality.png"));
    QImage image(TestUtil::fetchDataFileLazy("test_transform_quality_second.png"));

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    QVector<QPointF> origPoints;
    QVector<QPointF> transfPoints;
    qreal alpha = 1.0;

    QRectF bounds(dev->exactBounds());

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


    KisWarpTransformWorker worker(KisWarpTransformWorker::RIGID_TRANSFORM,
                                  dev,
                                  origPoints,
                                  transfPoints,
                                  alpha,
                                  updater);

    QBENCHMARK_ONCE {
        worker.run();
    }

    QImage result = dev->convertToQImage(0);

    TestUtil::checkQImage(result, "warp_transform_test", "simple", "tr");
}

void KisWarpTransformWorkerTest::testQImage()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

//    QImage image(TestUtil::fetchDataFileLazy("test_transform_quality.png"));
    QImage image(TestUtil::fetchDataFileLazy("test_transform_quality_second.png"));
    image = image.convertToFormat(QImage::Format_ARGB32);

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

#include "kis_four_point_interpolator_forward.h"

void KisWarpTransformWorkerTest::testForwardInterpolator()
{
    QPolygonF src;

    src << QPointF(0, 0);
    src << QPointF(100, 0);
    src << QPointF(100, 100);
    src << QPointF(0, 100);

    QPolygonF dst;

    dst << QPointF(0, 0);
    dst << QPointF(100, 10);
    dst << QPointF(100, 120);
    dst << QPointF(0, 100);

    KisFourPointInterpolatorForward interp(src, dst);

    QCOMPARE(interp.map(QPointF(0,50)), QPointF(0,50));
    QCOMPARE(interp.map(QPointF(50,0)), QPointF(50,5));
    QCOMPARE(interp.map(QPointF(100,0)), QPointF(100,10));
    QCOMPARE(interp.map(QPointF(100,50)), QPointF(100,65));

    QCOMPARE(interp.map(QPointF(100,100)), QPointF(100,120));
    QCOMPARE(interp.map(QPointF(50,100)), QPointF(50,110));
    QCOMPARE(interp.map(QPointF(50,50)), QPointF(50,57.5));
}


#include "kis_four_point_interpolator_backward.h"

void KisWarpTransformWorkerTest::testBackwardInterpolatorXShear()
{
    QPolygonF src;

    src << QPointF(0, 0);
    src << QPointF(100, 0);
    src << QPointF(100, 100);
    src << QPointF(0, 100);

    QPolygonF dst;

    dst << QPointF(0, 0);
    dst << QPointF(100, 0);
    dst << QPointF(120, 100);
    dst << QPointF(10, 100);

    KisFourPointInterpolatorBackward interp(src, dst);

    QCOMPARE(interp.map(QPointF(10,100)), QPointF(0,100));
    QCOMPARE(interp.map(QPointF(5,50)), QPointF(0,50));
    QCOMPARE(interp.map(QPointF(110,50)), QPointF(100,50));
    QCOMPARE(interp.map(QPointF(57.5,50)), QPointF(50,50));
}

void KisWarpTransformWorkerTest::testBackwardInterpolatorYShear()
{
    QPolygonF src;

    src << QPointF(0, 0);
    src << QPointF(100, 0);
    src << QPointF(100, 100);
    src << QPointF(0, 100);

    QPolygonF dst;

    dst << QPointF(0, 0);
    dst << QPointF(100, 10);
    dst << QPointF(100, 120);
    dst << QPointF(0, 100);

    KisFourPointInterpolatorBackward interp(src, dst);

    QCOMPARE(interp.map(QPointF(100,10)), QPointF(100,0));
    QCOMPARE(interp.map(QPointF(50,5)), QPointF(50,0));
    QCOMPARE(interp.map(QPointF(50,110)), QPointF(50,100));
    QCOMPARE(interp.map(QPointF(50,57.5)), QPointF(50,50));
}

void KisWarpTransformWorkerTest::testBackwardInterpolatorXYShear()
{
    QPolygonF src;

    src << QPointF(0, 0);
    src << QPointF(100, 0);
    src << QPointF(100, 100);
    src << QPointF(0, 100);

    QPolygonF dst;

    dst << QPointF(0, 0);
    dst << QPointF(100, 10);
    dst << QPointF(140, 120);
    dst << QPointF(20, 100);


    KisFourPointInterpolatorBackward interp(src, dst);

    QCOMPARE(interp.map(QPointF(100,10)), QPointF(100,0));
    QCOMPARE(interp.map(QPointF(50,5)), QPointF(50,0));
    QCOMPARE(interp.map(QPointF(80,110)), QPointF(50,100));
    QCOMPARE(interp.map(QPointF(120,65)), QPointF(100,50));
    QCOMPARE(interp.map(QPointF(10,50)), QPointF(0,50));
}

void KisWarpTransformWorkerTest::testBackwardInterpolatorRoundTrip()
{
    QPolygonF src;

    src << QPointF(0, 0);
    src << QPointF(100, 0);
    src << QPointF(100, 100);
    src << QPointF(0, 100);

    QPolygonF dst;

    dst << QPointF(100, 100);
    dst << QPointF(20, 140);
    dst << QPointF(10, 80);
    dst << QPointF(15, 5);

    KisFourPointInterpolatorForward f(src, dst);
    KisFourPointInterpolatorBackward b(src, dst);

    for (qreal y = 0; y <= 100; y += 1.0) {
        for (qreal x = 0; x <= 100; x += 1.0) {

            QPointF pt(x, y);

            QPointF fwdPt = f.map(pt);
            QPointF bwdPt = b.map(fwdPt);

            //dbgKrita << "R:" << ppVar(pt) << ppVar(fwdPt) << ppVar(bwdPt) << (bwdPt - pt);
            QVERIFY((bwdPt - pt).manhattanLength() < 1e-3);
        }
    }
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


QTEST_KDEMAIN(KisWarpTransformWorkerTest, GUI)
