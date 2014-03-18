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

#include "kis_fill_painter_test.h"

#include "testutil.h"

#include <qtest_kde.h>
#include "kis_fill_painter.h"

#include <floodfill/kis_scanline_fill.h>


void KisFillPainterTest::testCreation()
{
    KisFillPainter test;
}

void KisFillPainterTest::benchmarkFillingLegacy()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    QImage srcImage(TestUtil::fetchDataFileLazy("heavy_labyrinth.png"));
    QVERIFY(!srcImage.isNull());

    QRect imageRect = srcImage.rect();

    dev->convertFromQImage(srcImage, 0, 0, 0);


    QBENCHMARK_ONCE {
        KisFillPainter gc(dev);
        gc.setFillThreshold(50);
        gc.setWidth(imageRect.width());
        gc.setHeight(imageRect.height());
        gc.setPaintColor(KoColor(Qt::red, dev->colorSpace()));
        gc.fillColor(0,0, dev);
    }

    QImage resultImage =
        dev->convertToQImage(0,
                             imageRect.x(), imageRect.y(),
                             imageRect.width(), imageRect.height());

    TestUtil::checkQImage(resultImage,
                          "fill_painter",
                          "legacy_",
                          "heavy_labyrinth_top_left");
}

void KisFillPainterTest::benchmarkFillingScanline()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    QImage srcImage(TestUtil::fetchDataFileLazy("heavy_labyrinth.png"));
    QVERIFY(!srcImage.isNull());

    QRect imageRect = srcImage.rect();

    dev->convertFromQImage(srcImage, 0, 0, 0);


    QBENCHMARK_ONCE {
        KisScanlineFill gc(dev, QPoint(), imageRect);
        gc.run();
    }

    QImage resultImage =
        dev->convertToQImage(0,
                             imageRect.x(), imageRect.y(),
                             imageRect.width(), imageRect.height());

    TestUtil::checkQImage(resultImage,
                          "fill_painter",
                          "scanline_",
                          "heavy_labyrinth_top_left");
}


QTEST_KDEMAIN(KisFillPainterTest, GUI)
#include "kis_fill_painter_test.moc"
