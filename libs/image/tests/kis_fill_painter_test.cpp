/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_fill_painter_test.h"

#include <testutil.h>

#include <simpletest.h>
#include "kis_fill_painter.h"

#include <floodfill/kis_scanline_fill.h>

#define THRESHOLD 10

void KisFillPainterTest::testCreation()
{
    KisFillPainter test;
}

void KisFillPainterTest::benchmarkFillPainter(const QPoint &startPoint, bool useCompositioning)
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    QImage srcImage(TestUtil::fetchDataFileLazy("heavy_labyrinth.png"));
    QVERIFY(!srcImage.isNull());

    QRect imageRect = srcImage.rect();

    dev->convertFromQImage(srcImage, 0, 0, 0);


    QBENCHMARK_ONCE {
        KisFillPainter gc(dev);
        gc.setFillThreshold(THRESHOLD);
        gc.setWidth(imageRect.width());
        gc.setHeight(imageRect.height());
        gc.setPaintColor(KoColor(Qt::red, dev->colorSpace()));
        gc.setUseCompositioning(useCompositioning);
        gc.fillColor(startPoint.x(), startPoint.y(), dev);
    }

    QImage resultImage =
        dev->convertToQImage(0,
                             imageRect.x(), imageRect.y(),
                             imageRect.width(), imageRect.height());

    QString testName = QString("heavy_labyrinth_%1_%2_%3")
        .arg(startPoint.x())
        .arg(startPoint.y())
        .arg(useCompositioning ? "composed" : "direct");


    QVERIFY(TestUtil::checkQImage(resultImage,
                                  "fill_painter",
                                  "general_",
                                  testName));
}

void KisFillPainterTest::benchmarkFillPainter()
{
    benchmarkFillPainter(QPoint(), false);
}

void KisFillPainterTest::benchmarkFillPainterOffset()
{
    benchmarkFillPainter(QPoint(5,5), false);
}

void KisFillPainterTest::benchmarkFillPainterOffsetCompositioning()
{
    benchmarkFillPainter(QPoint(5,5), true);
}

void KisFillPainterTest::benchmarkFillingScanlineColor()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    QImage srcImage(TestUtil::fetchDataFileLazy("heavy_labyrinth.png"));
    QVERIFY(!srcImage.isNull());

    QRect imageRect = srcImage.rect();

    dev->convertFromQImage(srcImage, 0, 0, 0);


    QBENCHMARK_ONCE {
        KisScanlineFill gc(dev, QPoint(), imageRect);
        gc.setThreshold(THRESHOLD);
        gc.fillColor(KoColor(Qt::red, dev->colorSpace()));
    }

    QImage resultImage =
        dev->convertToQImage(0,
                             imageRect.x(), imageRect.y(),
                             imageRect.width(), imageRect.height());

    QVERIFY(TestUtil::checkQImage(resultImage,
                                  "fill_painter",
                                  "scanline_",
                                  "heavy_labyrinth_top_left"));
}

void KisFillPainterTest::benchmarkFillingScanlineSelection()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    QImage srcImage(TestUtil::fetchDataFileLazy("heavy_labyrinth.png"));
    QVERIFY(!srcImage.isNull());

    QRect imageRect = srcImage.rect();

    dev->convertFromQImage(srcImage, 0, 0, 0);


    KisPixelSelectionSP pixelSelection = new KisPixelSelection();

    QBENCHMARK_ONCE {
        KisScanlineFill gc(dev, QPoint(), imageRect);
        gc.setThreshold(THRESHOLD);
        gc.fillSelection(pixelSelection);
    }

    QImage resultImage =
        pixelSelection->convertToQImage(0,
                                        imageRect.x(), imageRect.y(),
                                        imageRect.width(), imageRect.height());

    QVERIFY(TestUtil::checkQImage(resultImage,
                                  "fill_painter",
                                  "scanline_",
                                  "heavy_labyrinth_top_left_selection"));
}

void KisFillPainterTest::testPatternFill()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dst = new KisPaintDevice(cs);

    KisPaintDeviceSP pattern = new KisPaintDevice(cs);
    pattern->fill(QRect(0,0,32,32), KoColor(Qt::red, cs));
    pattern->fill(QRect(32,32,32,32), KoColor(Qt::red, cs));
    pattern->fill(QRect(32,0,32,32), KoColor(Qt::yellow, cs));
    pattern->fill(QRect(0,32,32,32), KoColor(Qt::white, cs));

    const QRect fillRect(-128,-128,384,384);
    KisFillPainter painter(dst);


    { // fill aligned
        const QRect patternRect = pattern->exactBounds();
        painter.fillRect(fillRect.x(), fillRect.y(), fillRect.width(), fillRect.height(), pattern, patternRect);
        dst->fill(QRect(0,0,10,10), KoColor(Qt::black, cs));

        QImage resultImage =
                dst->convertToQImage(0,
                                     fillRect.x(), fillRect.y(),
                                     fillRect.width(), fillRect.height());

        QVERIFY(TestUtil::checkQImage(resultImage,
                                      "fill_painter",
                                      "patterns_fill_",
                                      "null_origin"));
    }

    { // fill with offset
        dst->clear();
        pattern->setX(7);
        pattern->setY(-13);


        const QRect patternRect = pattern->exactBounds();

        painter.fillRect(fillRect.x(), fillRect.y(), fillRect.width(), fillRect.height(), pattern, patternRect);
        dst->fill(QRect(0,0,10,10), KoColor(Qt::black, cs));

        QImage resultImage =
                dst->convertToQImage(0,
                                     fillRect.x(), fillRect.y(),
                                     fillRect.width(), fillRect.height());

        QVERIFY(TestUtil::checkQImage(resultImage,
                                      "fill_painter",
                                      "patterns_fill_",
                                      "custom_origin"));
    }

}


SIMPLE_TEST_MAIN(KisFillPainterTest)
