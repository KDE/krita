/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_gbr_brush_test.h"

#include <simpletest.h>
#include <QString>
#include <QDir>
#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <testutil.h>
#include "../kis_gbr_brush.h"
#include "kis_types.h"
#include "kis_paint_device.h"
#include "brushengine/kis_paint_information.h"
#include <kis_fixed_paint_device.h>
#include "kis_qimage_pyramid.h"
#include <KisGlobalResourcesInterface.h>

void KisGbrBrushTest::testMaskGenerationSingleColor()
{
    QScopedPointer<KisGbrBrush> brush(new KisGbrBrush(QString(FILES_DATA_DIR) + '/' + "brush.gbr"));
    brush->load(KisGlobalResourcesInterface::instance());
    Q_ASSERT(brush->valid());
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();

    KisPaintInformation info(QPointF(100.0, 100.0), 0.5);

    // check masking an existing paint device
    KisFixedPaintDeviceSP fdev = new KisFixedPaintDevice(cs);
    fdev->setRect(QRect(0, 0, 100, 100));
    fdev->initialize();
    cs->setOpacity(fdev->data(), OPACITY_OPAQUE_U8, 100 * 100);

    // Check creating a mask dab with a single color
    fdev = new KisFixedPaintDevice(cs);
    brush->mask(fdev, KoColor(Qt::black, cs), KisDabShape(), info);

    QPoint errpoint;
    QImage result = QImage(QString(FILES_DATA_DIR) + '/' + "result_brush_3.png");
    QImage image = fdev->convertToQImage(0);
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("kis_gbr_brush_test_3.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

void KisGbrBrushTest::testMaskGenerationDevColor()
{
    QScopedPointer<KisGbrBrush> brush(new KisGbrBrush(QString(FILES_DATA_DIR) + '/' + "brush.gbr"));
    brush->load(KisGlobalResourcesInterface::instance());
    Q_ASSERT(brush->valid());
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();

    KisPaintInformation info(QPointF(100.0, 100.0), 0.5);

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
    brush->mask(fdev, dev, KisDabShape(), info);

    QPoint errpoint;
    QImage result = QImage(QString(FILES_DATA_DIR) + '/' + "result_brush_4.png");
    QImage image = fdev->convertToQImage(0);
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("kis_gbr_brush_test_4.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

void KisGbrBrushTest::testImageGeneration()
{
    QScopedPointer<KisGbrBrush> brush(new KisGbrBrush(QString(FILES_DATA_DIR) + '/' + "testing_brush_512_bars.gbr"));
    bool res = brush->load(KisGlobalResourcesInterface::instance());
    Q_UNUSED(res);
    Q_ASSERT(res);
    QVERIFY(!brush->brushTipImage().isNull());
    qsrand(1);

    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintInformation info(QPointF(100.0, 100.0), 0.5);
    KisFixedPaintDeviceSP dab;

    for (int i = 0; i < 200; i++) {
        qreal scale = qreal(qrand()) / RAND_MAX * 2.0;
        qreal rotation = qreal(qrand()) / RAND_MAX * 2 * M_PI;
        qreal subPixelX = qreal(qrand()) / RAND_MAX * 0.5;
        QString testName =
            QString("brush_%1_sc_%2_rot_%3_sub_%4")
            .arg(i).arg(scale).arg(rotation).arg(subPixelX);

        dab = brush->paintDevice(cs, KisDabShape(scale, 1.0, rotation), info, subPixelX);

        /**
         * Compare first 10 images. Others are tested for asserts only
         */
        if (i < 10) {
            QImage result = dab->convertToQImage(0);
            TestUtil::checkQImage(result, "brush_masks", "", testName);
        }
    }
}

#include "KisSharedQImagePyramid.h"

void KisGbrBrushTest::benchmarkPyramidCreation()
{
    QScopedPointer<KisGbrBrush> brush(new KisGbrBrush(QString(FILES_DATA_DIR) + '/' + "testing_brush_512_bars.gbr"));
    brush->load(KisGlobalResourcesInterface::instance());
    QVERIFY(!brush->brushTipImage().isNull());

    QBENCHMARK {
        KisSharedQImagePyramid sharedPyramid;
        QVERIFY(sharedPyramid.pyramid(brush.data())); // avoid compiler elimination of unused code!
    }
}

void KisGbrBrushTest::benchmarkScaling()
{
    QScopedPointer<KisGbrBrush> brush(new KisGbrBrush(QString(FILES_DATA_DIR) + '/' + "testing_brush_512_bars.gbr"));
    brush->load(KisGlobalResourcesInterface::instance());
    QVERIFY(!brush->brushTipImage().isNull());
    qsrand(1);

    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintInformation info(QPointF(100.0, 100.0), 0.5);
    KisFixedPaintDeviceSP dab;

    {
        // warm up the pyramid!
        dab = brush->paintDevice(cs, KisDabShape(qreal(qrand()) / RAND_MAX * 2.0, 1.0, 0.0), info);
        QVERIFY(dab); // avoid compiler elimination of unused code!
        dab.clear();
    }

    QBENCHMARK {
        dab = brush->paintDevice(cs, KisDabShape(qreal(qrand()) / RAND_MAX * 2.0, 1.0, 0.0), info);
        //dab->convertToQImage(0).save(QString("dab_%1_new_smooth.png").arg(i++));
    }
}

void KisGbrBrushTest::benchmarkRotation()
{
    QScopedPointer<KisGbrBrush> brush(new KisGbrBrush(QString(FILES_DATA_DIR) + '/' + "testing_brush_512_bars.gbr"));
    brush->load(KisGlobalResourcesInterface::instance());
    QVERIFY(!brush->brushTipImage().isNull());
    qsrand(1);

    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintInformation info(QPointF(100.0, 100.0), 0.5);
    KisFixedPaintDeviceSP dab;

    QBENCHMARK {
        dab = brush->paintDevice(cs, KisDabShape(1.0, 1.0, qreal(qrand()) / RAND_MAX * 2 * M_PI), info);
    }
}

void KisGbrBrushTest::benchmarkMaskScaling()
{
    QScopedPointer<KisGbrBrush> brush(new KisGbrBrush(QString(FILES_DATA_DIR) + '/' + "testing_brush_512_bars.gbr"));
    brush->load(KisGlobalResourcesInterface::instance());
    QVERIFY(!brush->brushTipImage().isNull());
    qsrand(1);

    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintInformation info(QPointF(100.0, 100.0), 0.5);
    KisFixedPaintDeviceSP dab = new KisFixedPaintDevice(cs);

    QBENCHMARK {
        KoColor c(Qt::black, cs);
        qreal scale = qreal(qrand()) / RAND_MAX * 2.0;
        brush->mask(dab, c, KisDabShape(scale, 1.0, 0.0), info, 0.0, 0.0, 1.0);
    }
}

void KisGbrBrushTest::testPyramidLevelRounding()
{
    QSize imageSize(41, 41);
    QImage image(imageSize, QImage::Format_ARGB32);
    image.fill(0);

    KisQImagePyramid pyramid(image);

    qreal baseScale;
    int baseLevel;

    baseLevel = pyramid.findNearestLevel(1.0, &baseScale);
    QCOMPARE(baseScale, 1.0);
    QCOMPARE(baseLevel, 3);

    baseLevel = pyramid.findNearestLevel(2.0, &baseScale);
    QCOMPARE(baseScale, 2.0);
    QCOMPARE(baseLevel, 2);

    baseLevel = pyramid.findNearestLevel(4.0, &baseScale);
    QCOMPARE(baseScale, 4.0);
    QCOMPARE(baseLevel, 1);

    baseLevel = pyramid.findNearestLevel(0.5, &baseScale);
    QCOMPARE(baseScale, 0.5);
    QCOMPARE(baseLevel, 4);

    baseLevel = pyramid.findNearestLevel(0.25, &baseScale);
    QCOMPARE(baseScale, 0.25);
    QCOMPARE(baseLevel, 5);

    baseLevel = pyramid.findNearestLevel(0.25 + 1e-7, &baseScale);
    QCOMPARE(baseScale, 0.25);
    QCOMPARE(baseLevel, 5);
}

static QSize dabTransformHelper(KisDabShape const& shape)
{
    QSize const testSize(150, 150);
    qreal const subPixelX = 0.0,
                subPixelY = 0.0;
    return KisQImagePyramid::imageSize(testSize, shape, subPixelX, subPixelY);
}

void KisGbrBrushTest::testPyramidDabTransform()
{
    QCOMPARE(dabTransformHelper(KisDabShape(1.0, 1.0, 0.0)),      QSize(150, 150));
    QCOMPARE(dabTransformHelper(KisDabShape(1.0, 0.5, 0.0)),      QSize(150,  75));
    QCOMPARE(dabTransformHelper(KisDabShape(1.0, 1.0, M_PI / 4)), QSize(213, 213));
    QCOMPARE(dabTransformHelper(KisDabShape(1.0, 0.5, M_PI / 4)), QSize(160, 160));
}

// see comment in KisQImagePyramid::appendPyramidLevel
void KisGbrBrushTest::testQPainterTransformationBorder()
{
    QImage image1(10, 10, QImage::Format_ARGB32);
    QImage image2(12, 12, QImage::Format_ARGB32);

    image1.fill(0);
    image2.fill(0);

    {
        QPainter gc(&image1);
        gc.fillRect(QRect(0, 0, 10, 10), Qt::black);
    }

    {
        QPainter gc(&image2);
        gc.fillRect(QRect(1, 1, 10, 10), Qt::black);
    }

    image1.save("src1.png");
    image2.save("src2.png");

    {
        QImage canvas(100, 100, QImage::Format_ARGB32);
        canvas.fill(0);
        QPainter gc(&canvas);
        QTransform transform;
        transform.rotate(15);
        gc.setTransform(transform);
        gc.setRenderHints(QPainter::SmoothPixmapTransform);
        gc.drawImage(QPointF(50, 50), image1);
        gc.end();
        canvas.save("canvas1.png");
    }
    {
        QImage canvas(100, 100, QImage::Format_ARGB32);
        canvas.fill(0);
        QPainter gc(&canvas);
        QTransform transform;
        transform.rotate(15);
        gc.setTransform(transform);
        gc.setRenderHints(QPainter::SmoothPixmapTransform);
        gc.drawImage(QPointF(50, 50), image2);
        gc.end();
        canvas.save("canvas2.png");
    }
}

SIMPLE_TEST_MAIN(KisGbrBrushTest)
