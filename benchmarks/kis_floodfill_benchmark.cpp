/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <simpletest.h>

#include <kundo2command.h>
#include "kis_benchmark_values.h"

#include "kis_random_accessor_ng.h"

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColor.h>
#include <KoCompositeOpRegistry.h>

#include <kis_image.h>

#include "kis_floodfill_benchmark.h"

#include <kis_fill_painter.h>

void KisFloodFillBenchmark::initTestCase()
{
    m_colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    m_deviceStandardFloodFill = new KisPaintDevice(m_colorSpace);
    m_color = KoColor(m_colorSpace);

    QColor qcolor(Qt::red);
    srand(31524744);

    int tilew = 38;
    int tileh = 56;

    m_color.fromQColor(QColor(0,0,0,0)); // default pixel
    m_deviceStandardFloodFill->fill( 0,0,GMP_IMAGE_WIDTH, GMP_IMAGE_HEIGHT,m_color.data() );

    // fill the image with red ellipses (like some random dabs)
    m_color.fromQColor(Qt::red);
    KisPainter painter(m_deviceStandardFloodFill);
    painter.setFillStyle(KisPainter::FillStyleForegroundColor);
    painter.setPaintColor(m_color);

    int x = 0;
    int y = 0;
    for (int i = 0; i < 100;i++){
        x = rand() % GMP_IMAGE_WIDTH;
        y = rand() % GMP_IMAGE_HEIGHT;
        // plus 10 so that we don't fill the ellipse
        painter.paintEllipse(x+ 10, y+ 10, tilew, tileh);
    }

    // copy to other tests
    m_deviceWithoutSelectionAsBoundary = new KisPaintDevice(m_colorSpace);
    m_deviceWithSelectionAsBoundary = new KisPaintDevice(m_colorSpace);


    KisPainter::copyAreaOptimized(QPoint(), m_deviceStandardFloodFill,
                                  m_deviceWithoutSelectionAsBoundary, m_deviceWithoutSelectionAsBoundary->exactBounds());
    KisPainter::copyAreaOptimized(QPoint(), m_deviceStandardFloodFill,
                                  m_deviceWithSelectionAsBoundary, m_deviceWithSelectionAsBoundary->exactBounds());

    //m_deviceWithoutSelectionAsBoundary = m_deviceStandardFloodFill->

    const KoColorSpace* alphacs = KoColorSpaceRegistry::instance()->alpha8();
    KoColor defaultSelected = KoColor(alphacs);
    defaultSelected.fromQColor(QColor(255, 255, 255));
    m_existingSelection = new KisPaintDevice(alphacs);
    m_existingSelection->fill(0, 0, GMP_IMAGE_WIDTH, GMP_IMAGE_HEIGHT, defaultSelected.data());

}

void KisFloodFillBenchmark::benchmarkFlood()
{
    KoColor fg(m_colorSpace);
    KoColor bg(m_colorSpace);
    fg.fromQColor(Qt::blue);
    bg.fromQColor(Qt::black);

    QBENCHMARK
    {
        KisFillPainter fillPainter(m_deviceStandardFloodFill);
        //setupPainter(&fillPainter);
        fillPainter.setPaintColor( fg );
        fillPainter.setBackgroundColor( bg );

        fillPainter.beginTransaction(kundo2_noi18n("Flood Fill"));

        //fillPainter.setProgress(updater->startSubtask());
        fillPainter.setOpacity(OPACITY_OPAQUE_U8);
        // default
        fillPainter.setFillThreshold(15);
        fillPainter.setCompositeOp(COMPOSITE_OVER);
        fillPainter.setCareForSelection(true);
        fillPainter.setWidth(GMP_IMAGE_WIDTH);
        fillPainter.setHeight(GMP_IMAGE_HEIGHT);

        // fill twice
        fillPainter.fillColor(1, 1, m_deviceStandardFloodFill);

        fillPainter.deleteTransaction();
    }

    // uncomment this to see the output
    //QImage out = m_device->convertToQImage(m_colorSpace->profile(),0,0,GMP_IMAGE_WIDTH,GMP_IMAGE_HEIGHT);
    //out.save("fill_output.png");
}

void KisFloodFillBenchmark::benchmarkFloodWithoutSelectionAsBoundary()
{
    KoColor fg(m_colorSpace);
    KoColor bg(m_colorSpace);
    fg.fromQColor(Qt::blue);
    bg.fromQColor(Qt::black);

    QBENCHMARK
    {
        KisFillPainter fillPainter(m_deviceWithoutSelectionAsBoundary);
        fillPainter.setPaintColor( fg );
        fillPainter.setBackgroundColor( bg );

        fillPainter.beginTransaction(kundo2_noi18n("Flood Fill"));

        fillPainter.setOpacity(OPACITY_OPAQUE_U8);
        // default
        fillPainter.setFillThreshold(15);
        fillPainter.setCompositeOp(COMPOSITE_OVER);
        fillPainter.setCareForSelection(true);
        fillPainter.setWidth(GMP_IMAGE_WIDTH);
        fillPainter.setHeight(GMP_IMAGE_HEIGHT);
        fillPainter.setUseSelectionAsBoundary(false);

        fillPainter.createFloodSelection(1, 1, m_deviceWithoutSelectionAsBoundary, m_existingSelection);

        fillPainter.deleteTransaction();
    }
}

void KisFloodFillBenchmark::benchmarkFloodWithSelectionAsBoundary()
{
    KoColor fg(m_colorSpace);
    KoColor bg(m_colorSpace);
    fg.fromQColor(Qt::blue);
    bg.fromQColor(Qt::black);

    QBENCHMARK
    {
        KisFillPainter fillPainter(m_deviceWithSelectionAsBoundary);
        fillPainter.setPaintColor( fg );
        fillPainter.setBackgroundColor( bg );

        fillPainter.beginTransaction(kundo2_noi18n("Flood Fill"));

        fillPainter.setOpacity(OPACITY_OPAQUE_U8);
        // default
        fillPainter.setFillThreshold(15);
        fillPainter.setCompositeOp(COMPOSITE_OVER);
        fillPainter.setCareForSelection(true);
        fillPainter.setWidth(GMP_IMAGE_WIDTH);
        fillPainter.setHeight(GMP_IMAGE_HEIGHT);
        fillPainter.setUseSelectionAsBoundary(true);

        fillPainter.createFloodSelection(1, 1, m_deviceWithSelectionAsBoundary, m_existingSelection);

        fillPainter.deleteTransaction();
    }
}


void KisFloodFillBenchmark::cleanupTestCase()
{

}

SIMPLE_TEST_MAIN(KisFloodFillBenchmark)
