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

void KisBrushTest::testCreation()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    QImage img(512, 512, QImage::Format_ARGB32);

    KisGbrBrush a(QString(FILES_DATA_DIR) + QDir::separator() + "brush.gbr");
    KisGbrBrush b(dev, 0, 0, 10, 10);
    KisGbrBrush c(img, "bla");
    KisGbrBrush d(QString(FILES_DATA_DIR) + QDir::separator() + "brush.gih");
}

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
    cs->setAlpha(fdev->data(), OPACITY_OPAQUE, 100 * 100);

    QPoint errpoint;
    QImage result(QString(FILES_DATA_DIR) + QDir::separator() + "result_brush_1.png");
    QImage image = fdev->convertToQImage();

    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("kis_brush_test_1.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }

    brush->mask(fdev, 1.0, 1.0, 0.0, info);

    result = QImage(QString(FILES_DATA_DIR) + QDir::separator() + "result_brush_2.png");
    image = fdev->convertToQImage();
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("kis_brush_test_2.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
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
    cs->setAlpha(fdev->data(), OPACITY_OPAQUE, 100 * 100);

    // Check creating a mask dab with a single color
    fdev = new KisFixedPaintDevice(cs);
    brush->mask(fdev, KoColor(Qt::black, cs), 1.0, 1.0, 0.0, info);

    QPoint errpoint;
    QImage result = QImage(QString(FILES_DATA_DIR) + QDir::separator() + "result_brush_3.png");
    QImage image = fdev->convertToQImage();
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("kis_brush_test_3.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
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
    cs->setAlpha(fdev->data(), OPACITY_OPAQUE, 100 * 100);

    // Check creating a mask dab with a color taken from a paint device
    KoColor red(Qt::red, cs);
    cs->setAlpha(red.data(), 128, 1);
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->fill(0, 0, 100, 100, red.data());

    fdev = new KisFixedPaintDevice(cs);
    brush->mask(fdev, dev, 1.0, 1.0, 0.0, info);

    QPoint errpoint;
    QImage result = QImage(QString(FILES_DATA_DIR) + QDir::separator() + "result_brush_4.png");
    QImage image = fdev->convertToQImage();
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("kis_brush_test_4.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
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
    cs->setAlpha(fdev->data(), OPACITY_OPAQUE, 100 * 100);

    // check creating a mask dab with a default color
    fdev = new KisFixedPaintDevice(cs);
    brush->mask(fdev, 1.0, 1.0, 0.0, info);

    QPoint errpoint;
    QImage result = QImage(QString(FILES_DATA_DIR) + QDir::separator() + "result_brush_3.png");
    QImage image = fdev->convertToQImage();
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("kis_brush_test_5.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }

    delete brush;
}


void KisBrushTest::testImageGeneration()
{
    KisGbrBrush* brush = new KisGbrBrush(QString(FILES_DATA_DIR) + QDir::separator() + "brush.gbr");
    brush->load();
    Q_ASSERT(brush->valid());
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
    KisVector2D v2d = KisVector2D::Zero();
    KisPaintInformation info(QPointF(100.0, 100.0), 0.5, 0, 0, v2d, 0, 0);

    KisFixedPaintDeviceSP fdev = brush->image(cs, 1.0, 0.0, info);

    fdev->convertToQImage().save("bla.png");

    delete brush;
}

QTEST_KDEMAIN(KisBrushTest, GUI)
#include "kis_brush_test.moc"
