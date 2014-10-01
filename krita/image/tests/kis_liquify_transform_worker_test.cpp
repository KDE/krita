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

#include "kis_liquify_transform_worker_test.h"

#include <qtest_kde.h>

#include <KoUpdater.h>

#include "testutil.h"

#include <kis_liquify_transform_worker.h>

#include <kis_cage_transform_worker.h>
#include <algorithm>

#include <kis_algebra_2d.h>


void testCage(bool clockwise, bool unityTransform, bool benchmarkPrepareOnly = false, int pixelPrecision = 8, bool testQImage = false)
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(TestUtil::fetchDataFileLazy("test_cage_transform.png"));

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

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

    KisCageTransformWorker worker(dev,
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
                worker.run();

            }
        } else {
            QImage srcImage(image);
            image = QImage(image.size(), QImage::Format_ARGB32);
            QPainter gc(&image);
            gc.drawImage(QPoint(), srcImage);

            image.convertToFormat(QImage::Format_ARGB32);

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
        QVERIFY(TestUtil::checkQImage(result, "cage_transform_test", "cage_qimage", testName));
    } else if (!benchmarkPrepareOnly && pixelPrecision == 8) {

        result = dev->convertToQImage(0);
        QVERIFY(TestUtil::checkQImage(result, "cage_transform_test", "cage", testName));
    }
}

QPointF translatePoint(const QPointF &src,
                       const QPointF &base,
                       const QPointF &offset,
                       qreal sigma)
{
    qreal dist = KisAlgebra2D::norm(src - base);
    if (dist > 3 * sigma) return src;
    const qreal lambda = exp(-pow2(dist) / 2 / pow2(sigma));
    return src + lambda * offset;
}

QPointF scalePoint(const QPointF &src,
                   const QPointF &base,
                   qreal scale,
                   qreal sigma)
{
    QPointF diff = src - base;
    qreal dist = KisAlgebra2D::norm(diff);
    if (dist > 3 * sigma) return src;
    const qreal lambda = exp(-pow2(dist) / 2 / pow2(sigma));

    return base + (1.0 + scale * lambda) * diff;
}

QPointF rotatePoint(const QPointF &src,
                    const QPointF &base,
                    qreal angle,
                    qreal sigma)
{
    QPointF diff = src - base;
    qreal dist = KisAlgebra2D::norm(diff);
    if (dist > 4 * sigma) return src;
    const qreal lambda = exp(-pow2(dist) / 2 / pow2(sigma));

    angle *= lambda;

    qreal sinA = std::sin(angle);
    qreal cosA = std::cos(angle);

    qreal x =  cosA * diff.x() + sinA * diff.y();
    qreal y = -sinA * diff.x() + cosA * diff.y();

    QPointF result = base + QPointF(x, y);

    return result;
}

QPointF undoPoint(const QPointF &src,
                  const QPointF &base,
                  const QPointF &refPoint,
                  qreal sigma)
{
    QPointF diff = src - base;
    qreal dist = KisAlgebra2D::norm(diff);
    if (dist > 3 * sigma) return src;
    const qreal lambda = exp(-pow2(dist) / 2 / pow2(sigma));

    return refPoint * lambda + src * (1.0 - lambda);
}

void KisLiquifyTransformWorkerTest::testPoints()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(TestUtil::fetchDataFileLazy("test_transform_quality_second.png"));

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    const int pixelPrecision = 8;

    KisLiquifyTransformWorker worker(dev,
                                     updater,
                                     pixelPrecision);


    QBENCHMARK_ONCE {
        worker.translatePoints(QPointF(100,100),
                               QPointF(50, 0),
                               50);

        worker.scalePoints(QPointF(400,100),
                           0.9,
                           50);

        worker.undoPoints(QPointF(400,100),
                           1.0,
                           50);

        worker.scalePoints(QPointF(400,300),
                           0.5,
                           50);

        worker.scalePoints(QPointF(100,300),
                           -0.5,
                           30);

        worker.rotatePoints(QPointF(100,500),
                            M_PI / 4,
                            50);
    }

    worker.run();

    QImage result = dev->convertToQImage(0);
    TestUtil::checkQImage(result, "liquify_transform_test", "liquify_dev", "unity");
}

QTEST_KDEMAIN(KisLiquifyTransformWorkerTest, GUI)
