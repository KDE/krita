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

#include "kis_brush_test.h"

#include <qtest_kde.h>
#include <QString>
#include <QDir>
#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include "testutil.h"
#include "../kis_gbr_brush.h"
#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_paint_information.h"
#include "kis_vec.h"
#include <kis_fixed_paint_device.h>

void KisBrushTest::testMaskGenerationNoColor()
{
    KisGbrBrush* brush = new KisGbrBrush(QString(FILES_DATA_DIR) + QDir::separator() + "brush.gbr");
    brush->load();
    Q_ASSERT(brush->valid());
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();

    KisVector2D v2d = KisVector2D::Zero();
    KisPaintInformation info(QPointF(100.0, 100.0), 0.5, 0, 0, v2d, 0, 0);

    // check masking an existing paint device
    KisFixedPaintDeviceSP fdev = new KisFixedPaintDevice(cs);
    fdev->setRect(QRect(0, 0, 100, 100));
    fdev->initialize();
    cs->setOpacity(fdev->data(), OPACITY_OPAQUE_U8, 100 * 100);

    QPoint errpoint;
    QImage result(QString(FILES_DATA_DIR) + QDir::separator() + "result_brush_1.png");
    QImage image = fdev->convertToQImage(0);

    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("kis_brush_test_1.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }

    brush->mask(fdev, 1.0, 1.0, 0.0, info);

    result = QImage(QString(FILES_DATA_DIR) + QDir::separator() + "result_brush_2.png");
    image = fdev->convertToQImage(0);
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("kis_brush_test_2.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

void KisBrushTest::testMaskGenerationSingleColor()
{
    KisGbrBrush* brush = new KisGbrBrush(QString(FILES_DATA_DIR) + QDir::separator() + "brush.gbr");
    brush->load();
    Q_ASSERT(brush->valid());
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();

    KisVector2D v2d = KisVector2D::Zero();
    KisPaintInformation info(QPointF(100.0, 100.0), 0.5, 0, 0, v2d, 0, 0);

    // check masking an existing paint device
    KisFixedPaintDeviceSP fdev = new KisFixedPaintDevice(cs);
    fdev->setRect(QRect(0, 0, 100, 100));
    fdev->initialize();
    cs->setOpacity(fdev->data(), OPACITY_OPAQUE_U8, 100 * 100);

    // Check creating a mask dab with a single color
    fdev = new KisFixedPaintDevice(cs);
    brush->mask(fdev, KoColor(Qt::black, cs), 1.0, 1.0, 0.0, info);

    QPoint errpoint;
    QImage result = QImage(QString(FILES_DATA_DIR) + QDir::separator() + "result_brush_3.png");
    QImage image = fdev->convertToQImage(0);
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("kis_brush_test_3.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

void KisBrushTest::testMaskGenerationDevColor()
{
    KisGbrBrush* brush = new KisGbrBrush(QString(FILES_DATA_DIR) + QDir::separator() + "brush.gbr");
    brush->load();
    Q_ASSERT(brush->valid());
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();

    KisVector2D v2d = KisVector2D::Zero();
    KisPaintInformation info(QPointF(100.0, 100.0), 0.5, 0, 0, v2d, 0, 0);

    // check masking an existing paint device
    KisFixedPaintDeviceSP fdev = new KisFixedPaintDevice(cs);
    fdev->setRect(QRect(0, 0, 100, 100));
    fdev->initialize();
    cs->setOpacity(fdev->data(), OPACITY_OPAQUE_U8, 100 * 100);

    // Check creating a mask dab with a color taken from a paint device
    KoColor red(Qt::red, cs);
    cs->setOpacity(red.data(), quint8(128), 1);
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->fill(0, 0, 100, 100, red.data());

    fdev = new KisFixedPaintDevice(cs);
    brush->mask(fdev, dev, 1.0, 1.0, 0.0, info);

    QPoint errpoint;
    QImage result = QImage(QString(FILES_DATA_DIR) + QDir::separator() + "result_brush_4.png");
    QImage image = fdev->convertToQImage(0);
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("kis_brush_test_4.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

void KisBrushTest::testMaskGenerationDefaultColor()
{
    KisGbrBrush* brush = new KisGbrBrush(QString(FILES_DATA_DIR) + QDir::separator() + "brush.gbr");
    brush->load();
    Q_ASSERT(brush->valid());
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();

    KisVector2D v2d = KisVector2D::Zero();
    KisPaintInformation info(QPointF(100.0, 100.0), 0.5, 0, 0, v2d, 0, 0);

    // check masking an existing paint device
    KisFixedPaintDeviceSP fdev = new KisFixedPaintDevice(cs);
    fdev->setRect(QRect(0, 0, 100, 100));
    fdev->initialize();
    cs->setOpacity(fdev->data(), OPACITY_OPAQUE_U8, 100 * 100);

    // check creating a mask dab with a default color
    fdev = new KisFixedPaintDevice(cs);
    brush->mask(fdev, 1.0, 1.0, 0.0, info);

    QPoint errpoint;
    QImage result = QImage(QString(FILES_DATA_DIR) + QDir::separator() + "result_brush_3.png");
    QImage image = fdev->convertToQImage(0);
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("kis_brush_test_5.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }

    delete brush;
}


void KisBrushTest::testImageGeneration()
{
    KisGbrBrush* brush = new KisGbrBrush(QString(FILES_DATA_DIR) + QDir::separator() + "testing_brush_512_bars.gbr");
    brush->load();
    QVERIFY(!brush->image().isNull());
    brush->prepareBrushPyramid();
    qsrand(1);

    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
    KisVector2D v2d = KisVector2D::Zero();
    KisPaintInformation info(QPointF(100.0, 100.0), 0.5, 0, 0, v2d, 0, 0);
    KisFixedPaintDeviceSP dab;

    for (int i = 0; i < 200; i++) {
        qreal scale = qreal(qrand()) / RAND_MAX * 2.0;
        qreal rotation = qreal(qrand()) / RAND_MAX * 2 * M_PI;
        qreal subPixelX = qreal(qrand()) / RAND_MAX * 0.5;
        QString testName =
            QString("brush_%1_sc_%2_rot_%3_sub_%4")
            .arg(i).arg(scale).arg(rotation).arg(subPixelX);

        dab = brush->paintDevice(cs, scale, rotation, info, subPixelX);

        /**
         * Compare first 10 images. Others are tested for asserts only
         */
        if (i < 10) {
            QImage result = dab->convertToQImage(0);
            TestUtil::checkQImage(result, "brush_masks", "", testName);
        }
    }
}

void KisBrushTest::benchmarkPyramidCreation()
{
    KisGbrBrush* brush = new KisGbrBrush(QString(FILES_DATA_DIR) + QDir::separator() + "testing_brush_512_bars.gbr");
    brush->load();
    QVERIFY(!brush->image().isNull());

    QBENCHMARK {
        brush->prepareBrushPyramid();
        brush->clearBrushPyramid();
    }
}

void KisBrushTest::benchmarkScaling()
{
    KisGbrBrush* brush = new KisGbrBrush(QString(FILES_DATA_DIR) + QDir::separator() + "testing_brush_512_bars.gbr");
    brush->load();
    QVERIFY(!brush->image().isNull());
    brush->prepareBrushPyramid();
    qsrand(1);

    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
    KisVector2D v2d = KisVector2D::Zero();
    KisPaintInformation info(QPointF(100.0, 100.0), 0.5, 0, 0, v2d, 0, 0);
    KisFixedPaintDeviceSP dab;

    QBENCHMARK {
        dab = brush->paintDevice(cs, qreal(qrand()) / RAND_MAX * 2.0, 0.0, info);
        //dab->convertToQImage(0).save(QString("dab_%1_new_smooth.png").arg(i++));
    }
}

void KisBrushTest::benchmarkRotation()
{
    KisGbrBrush* brush = new KisGbrBrush(QString(FILES_DATA_DIR) + QDir::separator() + "testing_brush_512_bars.gbr");
    brush->load();
    QVERIFY(!brush->image().isNull());
    brush->prepareBrushPyramid();
    qsrand(1);

    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
    KisVector2D v2d = KisVector2D::Zero();
    KisPaintInformation info(QPointF(100.0, 100.0), 0.5, 0, 0, v2d, 0, 0);
    KisFixedPaintDeviceSP dab;

    QBENCHMARK {
        dab = brush->paintDevice(cs, 1.0, qreal(qrand()) / RAND_MAX * 2 * M_PI, info);
    }
}

void KisBrushTest::benchmarkMaskScaling()
{
    KisGbrBrush* brush = new KisGbrBrush(QString(FILES_DATA_DIR) + QDir::separator() + "testing_brush_512_bars.gbr");
    brush->load();
    QVERIFY(!brush->image().isNull());
    brush->prepareBrushPyramid();
    qsrand(1);

    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
    KisVector2D v2d = KisVector2D::Zero();
    KisPaintInformation info(QPointF(100.0, 100.0), 0.5, 0, 0, v2d, 0, 0);
    KisFixedPaintDeviceSP dab = new KisFixedPaintDevice(cs);

    QBENCHMARK {
        KoColor c(Qt::black, cs);
        qreal scale = qreal(qrand()) / RAND_MAX * 2.0;
        brush->mask(dab, c, scale, scale, 0.0, info, 0.0, 0.0, 1.0);
    }
}

QTEST_KDEMAIN(KisBrushTest, GUI)
#include "kis_brush_test.moc"
