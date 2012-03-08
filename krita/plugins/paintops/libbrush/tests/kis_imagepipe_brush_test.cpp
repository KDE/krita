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

#include <qtest_kde.h>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_imagepipe_brush.h"
#include "kis_qimage_mask.h"

#define COMPARE_ALL(brush, method)                                      \
    foreach(KisGbrBrush *child, brush->testingGetBrushes()) {           \
        if(brush->method() != child->method()) {                        \
            qDebug() << "Failing method:" << #method                    \
                     << "brush index:"                                  \
                     << brush->testingGetBrushes().indexOf(child);      \
            QCOMPARE(brush->method(), child->method());                 \
        }                                                               \
    }

inline void KisImagePipeBrushTest::checkConsistency(KisImagePipeBrush *brush)
{
    qreal scale = 0.5;
    KisGbrBrush *firstBrush = brush->testingGetBrushes().first();

    /**
     * This set of values is supposed to be constant, so
     * it is just set to the corresponding values of the
     * first brush
     */
    QCOMPARE(brush->width(), firstBrush->width());
    QCOMPARE(brush->height(), firstBrush->height());
    QCOMPARE(brush->xSpacing(scale), firstBrush->xSpacing(scale));
    QCOMPARE(brush->ySpacing(scale), firstBrush->ySpacing(scale));
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

    QVERIFY(brush->currentBrush());

    qreal realScale = 1;
    qreal realAngle = 0;
    qreal subPixelX = 0;
    qreal subPixelY = 0;

    int maskWidth = brush->maskWidth(realScale, realAngle);
    int maskHeight = brush->maskHeight(realScale, realAngle);
    KisQImagemaskSP outputMask = brush->currentBrush()->createMask(realScale, subPixelX, subPixelY);

    QCOMPARE(maskWidth, outputMask->width());
    QCOMPARE(maskHeight, outputMask->height());
}


void KisImagePipeBrushTest::testLoading()
{
    KisImagePipeBrush *brush = new KisImagePipeBrush(QString(FILES_DATA_DIR) + QDir::separator() + "C_Dirty_Spot.gih");
    brush->load();
    QVERIFY(brush->valid());

    checkConsistency(brush);

    delete brush;
}

void KisImagePipeBrushTest::testChangingBrushes()
{
    KisImagePipeBrush *brush = new KisImagePipeBrush(QString(FILES_DATA_DIR) + QDir::separator() + "C_Dirty_Spot.gih");
    brush->load();
    QVERIFY(brush->valid());

    KisVector2D movement = KisVector2D::Zero();
    qreal rotation = 0;
    KisPaintInformation info(QPointF(100.0, 100.0), 0.5, 0, 0, movement, rotation, 0);

    for (int i = 0; i < 100; i++) {
        checkConsistency(brush);
        brush->selectNextBrush(info);
    }

    delete brush;
}

void KisImagePipeBrushTest::testDabApplication()
{
    KisImagePipeBrush *brush = new KisImagePipeBrush(QString(FILES_DATA_DIR) + QDir::separator() + "C_Dirty_Spot.gih");
    brush->load();
    QVERIFY(brush->valid());

    checkConsistency(brush);

    qreal realScale = 1;
    qreal realAngle = 0;
    qreal subPixelX = 0;
    qreal subPixelY = 0;

    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
    KoColor fillColor(Qt::red, cs);

    KisFixedPaintDeviceSP fixedDab = new KisFixedPaintDevice(cs);

    KisVector2D movement = KisVector2D::Zero();
    qreal rotation = 0;
    KisPaintInformation info(QPointF(100.0, 100.0), 0.5, 0, 0, movement, rotation, 0);

    for (int i = 0; i < 20; i++) {
        int maskWidth = brush->maskWidth(realScale, realAngle);
        int maskHeight = brush->maskHeight(realScale, realAngle);
        QRect fillRect(0, 0, maskWidth, maskHeight);

        fixedDab->setRect(fillRect);
        fixedDab->fill(fillRect.x(), fillRect.y(), fillRect.width(), fillRect.height(), fillColor.data());

        brush->mask(fixedDab, realScale, realScale, realAngle, info);
        fixedDab->convertToQImage(0).save(QString("fixed_dab_%1.png").arg(i));
    }

    delete brush;
}

QTEST_KDEMAIN(KisImagePipeBrushTest, GUI)
