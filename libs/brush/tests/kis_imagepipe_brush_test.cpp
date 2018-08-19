/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_imagepipe_brush_test.h"

#include <QTest>
#include <QPainter>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoCompositeOpRegistry.h>
#include <kis_fixed_paint_device.h>
#include <brushengine/kis_paint_information.h>

#include "kis_imagepipe_brush.h"
#include <kis_paint_device.h>
#include <kis_painter.h>

#define COMPARE_ALL(brush, method)                                      \
    Q_FOREACH (KisGbrBrush *child, brush->brushes()) {           \
        if(brush->method() != child->method()) {                        \
            dbgKrita << "Failing method:" << #method                    \
                     << "brush index:"                                  \
                     << brush->brushes().indexOf(child);      \
            QCOMPARE(brush->method(), child->method());                 \
        }                                                               \
    }

inline void KisImagePipeBrushTest::checkConsistency(KisImagePipeBrush *brush)
{
    qreal scale = 0.5; Q_UNUSED(scale);
    KisGbrBrush *firstBrush = brush->brushes().first();

    /**
     * This set of values is supposed to be constant, so
     * it is just set to the corresponding values of the
     * first brush
     */
    QCOMPARE(brush->width(), firstBrush->width());
    QCOMPARE(brush->height(), firstBrush->height());
    QCOMPARE(brush->boundary(), firstBrush->boundary());

    /**
     * These values should be spread over the children brushes
     */
    COMPARE_ALL(brush, maskAngle);
    COMPARE_ALL(brush, scale);
    COMPARE_ALL(brush, angle);
    COMPARE_ALL(brush, spacing);

    /**
     * Check mask size values, they depend on current brush
     */

    KisPaintInformation info;

    KisBrush *oldBrush = brush->testingGetCurrentBrush(info);
    QVERIFY(oldBrush);

    qreal realScale = 1;
    qreal realAngle = 0;
    qreal subPixelX = 0;
    qreal subPixelY = 0;

    int maskWidth = brush->maskWidth(KisDabShape(realScale, 1.0, realAngle), subPixelX, subPixelY, info);
    int maskHeight = brush->maskHeight(KisDabShape(realScale, 1.0, realAngle), subPixelX, subPixelY, info);

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisFixedPaintDeviceSP dev = brush->testingGetCurrentBrush(info)->paintDevice(
        cs, KisDabShape(realScale, 1.0, realAngle), info, subPixelX, subPixelY);

    QCOMPARE(maskWidth, dev->bounds().width());
    QCOMPARE(maskHeight, dev->bounds().height());

    KisBrush *newBrush = brush->testingGetCurrentBrush(info);
    QCOMPARE(oldBrush, newBrush);
}


void KisImagePipeBrushTest::testLoading()
{
    QScopedPointer<KisImagePipeBrush> brush(new KisImagePipeBrush(QString(FILES_DATA_DIR) + QDir::separator() + "C_Dirty_Spot.gih"));
    brush->load();
    QVERIFY(brush->valid());

    checkConsistency(brush.data());
}

void KisImagePipeBrushTest::testChangingBrushes()
{
    QScopedPointer<KisImagePipeBrush> brush(new KisImagePipeBrush(QString(FILES_DATA_DIR) + QDir::separator() + "C_Dirty_Spot.gih"));
    brush->load();
    QVERIFY(brush->valid());

    qreal rotation = 0;
    KisPaintInformation info(QPointF(100.0, 100.0), 0.5, 0, 0, rotation);

    for (int i = 0; i < 100; i++) {
        checkConsistency(brush.data());
        brush->testingSelectNextBrush(info);
    }
}

void checkIncrementalPainting(KisBrush *brush, const QString &prefix)
{
    qreal realScale = 1;
    qreal realAngle = 0;

    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
    KoColor fillColor(Qt::red, cs);

    KisFixedPaintDeviceSP fixedDab = new KisFixedPaintDevice(cs);

    qreal rotation = 0;
    qreal subPixelX = 0.0;
    qreal subPixelY = 0.0;
    KisPaintInformation info(QPointF(100.0, 100.0), 0.5, 0, 0, rotation);

    for (int i = 0; i < 20; i++) {
        int maskWidth = brush->maskWidth(KisDabShape(realScale, 1.0, realAngle), subPixelX, subPixelY, info);
        int maskHeight = brush->maskHeight(KisDabShape(realScale, 1.0, realAngle), subPixelX, subPixelY, info);
        QRect fillRect(0, 0, maskWidth, maskHeight);

        fixedDab->setRect(fillRect);
        fixedDab->lazyGrowBufferWithoutInitialization();

        brush->mask(fixedDab, fillColor, KisDabShape(realScale, 1.0, realAngle), info);
        QCOMPARE(fixedDab->bounds(), fillRect);

        QImage result = fixedDab->convertToQImage(0);
        result.save(QString("fixed_dab_%1_%2.png").arg(prefix).arg(i));
    }
}

void KisImagePipeBrushTest::testSimpleDabApplication()
{
    QScopedPointer<KisImagePipeBrush> brush(new KisImagePipeBrush(QString(FILES_DATA_DIR) + QDir::separator() + "C_Dirty_Spot.gih"));
    brush->load();
    QVERIFY(brush->valid());

    checkConsistency(brush.data());
    checkIncrementalPainting(brush.data(), "simple");
}

void KisImagePipeBrushTest::testColoredDab()
{
    QScopedPointer<KisImagePipeBrush> brush(new KisImagePipeBrush(QString(FILES_DATA_DIR) + QDir::separator() + "G_Sparks.gih"));
    brush->load();
    QVERIFY(brush->valid());

    checkConsistency(brush.data());

    QCOMPARE(brush->useColorAsMask(), false);
    QCOMPARE(brush->hasColor(), true);
    QCOMPARE(brush->brushType(), PIPE_IMAGE);

    // let it be the mask (should be revertible)
    brush->setUseColorAsMask(true);

    QCOMPARE(brush->useColorAsMask(), true);
    QCOMPARE(brush->hasColor(), true);
    QCOMPARE(brush->brushType(), PIPE_MASK);

    // revert back
    brush->setUseColorAsMask(false);

    QCOMPARE(brush->useColorAsMask(), false);
    QCOMPARE(brush->hasColor(), true);
    QCOMPARE(brush->brushType(), PIPE_IMAGE);

    // convert to the mask (irreversible)
    brush->makeMaskImage();

    QCOMPARE(brush->useColorAsMask(), false);
    QCOMPARE(brush->hasColor(), false);
    QCOMPARE(brush->brushType(), PIPE_MASK);

    checkConsistency(brush.data());
}

void KisImagePipeBrushTest::testColoredDabWash()
{
    QScopedPointer<KisImagePipeBrush> brush(new KisImagePipeBrush(QString(FILES_DATA_DIR) + QDir::separator() + "G_Sparks.gih"));
    brush->load();
    QVERIFY(brush->valid());

    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();

    qreal rotation = 0;
    KisPaintInformation info(QPointF(100.0, 100.0), 0.5, 0, 0, rotation);

    KisPaintDeviceSP layer = new KisPaintDevice(cs);
    KisPainter painter(layer);
    painter.setCompositeOp(COMPOSITE_ALPHA_DARKEN);

    const QVector<KisGbrBrush*> gbrs = brush->brushes();

    KisFixedPaintDeviceSP dab = gbrs.at(0)->paintDevice(cs, KisDabShape(2.0, 1.0, 0.0), info);
    painter.bltFixed(0, 0, dab, 0, 0, dab->bounds().width(), dab->bounds().height());
    painter.bltFixed(80, 60, dab, 0, 0, dab->bounds().width(), dab->bounds().height());

    painter.end();

    QRect rc = layer->exactBounds();

    QImage result = layer->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());


#if 0
    // if you want to see the result on white background, set #if 1
    QImage bg(result.size(), result.format());
    bg.fill(Qt::white);
    QPainter qPainter(&bg);
    qPainter.drawImage(0, 0, result);
    result = bg;
#endif
    result.save("z_spark_alpha_darken.png");
}



#include "kis_text_brush.h"

void KisImagePipeBrushTest::testTextBrushNoPipes()
{
    QScopedPointer<KisTextBrush> brush(new KisTextBrush());

    brush->setPipeMode(false);
    brush->setFont(QApplication::font());
    brush->setText("The_Quick_Brown_Fox_Jumps_Over_The_Lazy_Dog");
    brush->updateBrush();

    checkIncrementalPainting(brush.data(), "text_no_incremental");
}

void KisImagePipeBrushTest::testTextBrushPiped()
{
    QScopedPointer<KisTextBrush> brush(new KisTextBrush());

    brush->setPipeMode(true);
    brush->setFont(QApplication::font());
    brush->setText("The_Quick_Brown_Fox_Jumps_Over_The_Lazy_Dog");
    brush->updateBrush();

    checkIncrementalPainting(brush.data(), "text_incremental");
}

QTEST_MAIN(KisImagePipeBrushTest)
