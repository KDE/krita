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

#include "kis_gradient_painter_test.h"

#include <qtest_kde.h>
#include "kis_gradient_painter.h"

#include "kis_paint_device.h"
#include "kis_selection.h"

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "KoStopGradient.h"

#include "krita_utils.h"
#include "testutil.h"


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
                                   const QString &testName)
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
    selPainter.end();

    pixelSelection->invalidateOutlineCache();

    pixelSelection->convertToQImage(0, imageRect).save("sgt_selection.png");

    QLinearGradient testGradient;
    testGradient.setColorAt(0.0, Qt::white);
    testGradient.setColorAt(0.5, Qt::green);
    testGradient.setColorAt(1.0, Qt::black);
    testGradient.setSpread(QGradient::ReflectSpread);
    QScopedPointer<KoStopGradient> gradient(
        KoStopGradient::fromQGradient(&testGradient));

    KisGradientPainter gc(dev, selection);
    gc.setGradient(gradient.data());
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

    TestUtil::checkQImageExternal(dev->convertToQImage(0, imageRect),
                                  "shaped_gradient",
                                  "fill",
                                  testName, 1, 1, 0);
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

#include "kis_polygonal_gradient_shape_strategy.h"
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

    QRect rc = selectionPolygon.boundingRect().toAlignedRect();

    KisGradientShapeStrategy *strategy =
        new KisPolygonalGradientShapeStrategy(selectionPolygon, 2.0);

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
                qDebug() << ppVar(x) << ppVar(y) << ppVar(value) << ppVar(ref) << ppVar(relError);
            }
        }
    }

    qDebug() << ppVar(count(accum));
    qDebug() << ppVar(mean(accum));
    qDebug() << ppVar(variance(accum));
    qDebug() << ppVar((min)(accum));
    qDebug() << ppVar((max)(accum));

    qreal maxError = qMax(qAbs((min)(accum)), qAbs((max)(accum)));
    QVERIFY(maxError < maxRelError);
}

QTEST_KDEMAIN(KisGradientPainterTest, GUI)
#include "kis_gradient_painter_test.moc"
