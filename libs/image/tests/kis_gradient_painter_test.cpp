/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_gradient_painter_test.h"

#include <simpletest.h>
#include "kis_gradient_painter.h"

#include "kis_paint_device.h"
#include "kis_selection.h"

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoCompositeOpRegistry.h>

#include <resources/KoStopGradient.h>

#include "krita_utils.h"
#include <testutil.h>


void KisGradientPainterTest::testSimplifyPath()
{
    QPolygonF selectionPolygon;
    selectionPolygon << QPointF(100, 100);
    selectionPolygon << QPointF(200, 100);
    selectionPolygon << QPointF(202, 100);
    selectionPolygon << QPointF(200, 200);
    selectionPolygon << QPointF(100, 200);
    selectionPolygon << QPointF(100, 102);

    QPainterPath path;
    path.addPolygon(selectionPolygon);

    QPainterPath simplifiedPath;
    simplifiedPath = KritaUtils::trySimplifyPath(path, 10.0);

    QPainterPath ref;
    ref.moveTo(100,100);
    ref.lineTo(200,100);
    ref.lineTo(200,200);
    ref.lineTo(100,200);

    QCOMPARE(simplifiedPath, ref);
}

void testShapedGradientPainterImpl(const QPolygonF &selectionPolygon,
                                   const QString &testName,
                                   const QPolygonF &selectionErasePolygon = QPolygonF())
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    QRect imageRect(0,0,300,300);

    KisSelectionSP selection = new KisSelection();
    KisPixelSelectionSP pixelSelection = selection->pixelSelection();

    KisPainter selPainter(pixelSelection);
    selPainter.setFillStyle(KisPainter::FillStyleForegroundColor);
    selPainter.setPaintColor(KoColor(Qt::white, pixelSelection->colorSpace()));
    selPainter.paintPolygon(selectionPolygon);

    if (!selectionErasePolygon.isEmpty()) {
        selPainter.setCompositeOp(COMPOSITE_ERASE);
        selPainter.setPaintColor(KoColor(Qt::white, pixelSelection->colorSpace()));
        selPainter.paintPolygon(selectionErasePolygon);
    }

    selPainter.end();

    pixelSelection->invalidateOutlineCache();

    pixelSelection->convertToQImage(0, imageRect).save("sgt_selection.png");

    QLinearGradient testGradient;
    testGradient.setColorAt(0.0, Qt::white);
    testGradient.setColorAt(0.5, Qt::green);
    testGradient.setColorAt(1.0, Qt::black);
    testGradient.setSpread(QGradient::ReflectSpread);
    QSharedPointer<KoStopGradient> gradient(KoStopGradient::fromQGradient(&testGradient));

    KisGradientPainter gc(dev, selection);
    gc.setGradient(gradient);
    gc.setGradientShape(KisGradientPainter::GradientShapePolygonal);

    gc.paintGradient(selectionPolygon.boundingRect().topLeft(),
                     selectionPolygon.boundingRect().bottomRight(),
                     KisGradientPainter::GradientRepeatNone,
                     0,
                     false,
                     imageRect.x(),
                     imageRect.y(),
                     imageRect.width(),
                     imageRect.height());

    QVERIFY(TestUtil::checkQImageExternal(dev->convertToQImage(0, imageRect),
                                          "shaped_gradient",
                                          "fill",
                                          testName, 1, 1, 0));
}

void KisGradientPainterTest::testShapedGradientPainterRect()
{
    QPolygonF selectionPolygon;

    selectionPolygon << QPointF(100, 100);
    selectionPolygon << QPointF(200, 100);
    selectionPolygon << QPointF(202, 100);
    selectionPolygon << QPointF(200, 200);
    selectionPolygon << QPointF(100, 200);

    testShapedGradientPainterImpl(selectionPolygon, "rect_shape");
}

void KisGradientPainterTest::testShapedGradientPainterRectPierced()
{
    QPolygonF selectionPolygon;

    selectionPolygon << QPointF(100, 100);
    selectionPolygon << QPointF(200, 100);
    selectionPolygon << QPointF(200, 200);
    selectionPolygon << QPointF(100, 200);

    QPolygonF selectionErasePolygon;
    selectionErasePolygon << QPointF(150, 150);
    selectionErasePolygon << QPointF(155, 150);
    selectionErasePolygon << QPointF(155, 155);
    selectionErasePolygon << QPointF(150, 155);

    testShapedGradientPainterImpl(selectionPolygon, "rect_shape_pierced", selectionErasePolygon);
}

void KisGradientPainterTest::testShapedGradientPainterNonRegular()
{
    QPolygonF selectionPolygon;
    selectionPolygon << QPointF(100, 100);
    selectionPolygon << QPointF(200, 120);
    selectionPolygon << QPointF(170, 140);
    selectionPolygon << QPointF(200, 180);
    selectionPolygon << QPointF(30, 220);

    testShapedGradientPainterImpl(selectionPolygon, "nonregular_shape");
}

void KisGradientPainterTest::testShapedGradientPainterNonRegularPierced()
{
    QPolygonF selectionPolygon;
    selectionPolygon << QPointF(100, 100);
    selectionPolygon << QPointF(200, 120);
    selectionPolygon << QPointF(170, 140);
    selectionPolygon << QPointF(200, 180);
    selectionPolygon << QPointF(30, 220);

    QPolygonF selectionErasePolygon;
    selectionErasePolygon << QPointF(150, 150);
    selectionErasePolygon << QPointF(155, 150);
    selectionErasePolygon << QPointF(155, 155);
    selectionErasePolygon << QPointF(150, 155);

    testShapedGradientPainterImpl(selectionPolygon, "nonregular_shape_pierced", selectionErasePolygon);
}

#include "kis_polygonal_gradient_shape_strategy.h"

void KisGradientPainterTest::testFindShapedExtremums()
{
    QPolygonF selectionPolygon;
    selectionPolygon << QPointF(100, 100);
    selectionPolygon << QPointF(200, 120);
    selectionPolygon << QPointF(170, 140);
    selectionPolygon << QPointF(200, 180);
    selectionPolygon << QPointF(30, 220);

    QPolygonF selectionErasePolygon;
    selectionErasePolygon << QPointF(101, 101);
    selectionErasePolygon << QPointF(190, 120);
    selectionErasePolygon << QPointF(160, 140);
    selectionErasePolygon << QPointF(200, 180);
    selectionErasePolygon << QPointF(30, 220);

    QPainterPath path;
    path.addPolygon(selectionPolygon);
    path.closeSubpath();
    path.addPolygon(selectionErasePolygon);
    path.closeSubpath();

    QPointF center =
        KisPolygonalGradientShapeStrategy::testingCalculatePathCenter(
            4, path, 2.0, true);

    dbgKrita << ppVar(center);

    QVERIFY(path.contains(center));
}

void KisGradientPainterTest::testSplitDisjointPaths()
{
    QPainterPath path;

    // small bug: the smaller rect is also merged
    path.addRect(QRectF(323, 123, 4, 4));
    path.addRect(QRectF(300, 100, 50, 50));
    path.addRect(QRectF(320, 120, 10, 10));

    path.addRect(QRectF(200, 100, 50, 50));
    path.addRect(QRectF(240, 120, 70, 10));

    path.addRect(QRectF(100, 100, 50, 50));
    path.addRect(QRectF(120, 120, 10, 10));

    path = path.simplified();

    {
        QImage srcImage(450, 250, QImage::Format_ARGB32);
        srcImage.fill(0);
        QPainter gc(&srcImage);
        gc.fillPath(path, Qt::red);
        //srcImage.save("src_disjoint_paths.png");
    }

    QList<QPainterPath> result = KritaUtils::splitDisjointPaths(path);

    {
        QImage dstImage(450, 250, QImage::Format_ARGB32);
        dstImage.fill(0);
        QPainter gc(&dstImage);

        QVector<QBrush> brushes;
        brushes << Qt::red;
        brushes << Qt::green;
        brushes << Qt::blue;
        brushes << Qt::cyan;
        brushes << Qt::magenta;
        brushes << Qt::yellow;
        brushes << Qt::black;
        brushes << Qt::white;

        int index = 0;
        Q_FOREACH (const QPainterPath &p, result) {
            gc.fillPath(p, brushes[index]);
            index = (index + 1) % brushes.size();
        }


        TestUtil::checkQImageExternal(dstImage,
                                      "shaped_gradient",
                                      "test",
                                      "disjoint_paths");
    }
}

#include "kis_cached_gradient_shape_strategy.h"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>

using namespace boost::accumulators;

void KisGradientPainterTest::testCachedStrategy()
{
    QPolygonF selectionPolygon;
    selectionPolygon << QPointF(100, 100);
    selectionPolygon << QPointF(200, 120);
    selectionPolygon << QPointF(170, 140);
    selectionPolygon << QPointF(200, 180);
    selectionPolygon << QPointF(30, 220);

    QPainterPath selectionPath;
    selectionPath.addPolygon(selectionPolygon);

    QRect rc = selectionPolygon.boundingRect().toAlignedRect();

    KisGradientShapeStrategy *strategy =
        new KisPolygonalGradientShapeStrategy(selectionPath, 2.0);

    KisCachedGradientShapeStrategy cached(rc, 4, 4, strategy);

    accumulator_set<qreal, stats<tag::variance, tag::max, tag::min> > accum;
    const qreal maxRelError = 5.0 / 256;


    for (int y = rc.y(); y <= rc.bottom(); y++) {
        for (int x = rc.x(); x <= rc.right(); x++) {
            if (!selectionPolygon.containsPoint(QPointF(x, y), Qt::OddEvenFill)) continue;

            qreal ref = strategy->valueAt(x, y);
            qreal value = cached.valueAt(x, y);

            if (ref == 0.0) continue;

            qreal relError = (ref - value)/* / ref*/;
            accum(relError);

            if (relError > maxRelError) {
                //dbgKrita << ppVar(x) << ppVar(y) << ppVar(value) << ppVar(ref) << ppVar(relError);
            }
        }
    }

    dbgKrita << ppVar(count(accum));
    dbgKrita << ppVar(mean(accum));
    dbgKrita << ppVar(variance(accum));
    dbgKrita << ppVar((min)(accum));
    dbgKrita << ppVar((max)(accum));

    qreal varError = variance(accum);
    QVERIFY(varError < maxRelError);

    qreal maxError = qMax(qAbs((min)(accum)), qAbs((max)(accum)));
    QVERIFY(maxError < 2 * maxRelError);
}

SIMPLE_TEST_MAIN(KisGradientPainterTest)
