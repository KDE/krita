/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include <QTest>

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
    m_device = new KisPaintDevice(m_colorSpace);
    m_color = KoColor(m_colorSpace);

    QColor qcolor(Qt::red);
    srand(31524744);

    int tilew = 38;
    int tileh = 56;

    m_color.fromQColor(QColor(0,0,0,0)); // default pixel
    m_device->fill( 0,0,GMP_IMAGE_WIDTH, GMP_IMAGE_HEIGHT,m_color.data() );

    // fill the image with red ellipses (like some random dabs)
    m_color.fromQColor(Qt::red);
    KisPainter painter(m_device);
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


}

void KisFloodFillBenchmark::benchmarkFlood()
{
    KoColor fg(m_colorSpace);
    KoColor bg(m_colorSpace);
    fg.fromQColor(Qt::blue);
    bg.fromQColor(Qt::black);

    QBENCHMARK
    {
        KisFillPainter fillPainter(m_device);
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
        fillPainter.fillColor(1, 1, m_device);

        fillPainter.deleteTransaction();
    }

    // uncomment this to see the output
    //QImage out = m_device->convertToQImage(m_colorSpace->profile(),0,0,GMP_IMAGE_WIDTH,GMP_IMAGE_HEIGHT);
    //out.save("fill_output.png");
}


void KisFloodFillBenchmark::cleanupTestCase()
{

}

QTEST_MAIN(KisFloodFillBenchmark)
