/*
 *  Copyright (c) 2010 Lukáš Tvrdý lukast.dev@gmail.com
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

#include <qtest_kde.h>

#include <QImage>

#include "kis_painter_benchmark.h"
#include "kis_benchmark_values.h"

#include "kis_paint_device.h"
#include "kis_fixed_paint_device.h"

#include "kis_selection.h"
#include "kis_pixel_selection.h"
#include "kis_iterators_pixel.h"

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColor.h>

#include <kis_image.h>
#include <kis_painter.h>
#include <kis_types.h>


#define CYCLES 20

void KisPainterBenchmark::initTestCase()
{
    m_colorSpace = KoColorSpaceRegistry::instance()->rgb8();    
    
    m_color = KoColor(m_colorSpace);
    m_color.fromQColor(Qt::red);
}

void KisPainterBenchmark::cleanupTestCase()
{
}


void KisPainterBenchmark::benchmarkBitBlt()
{
    KisPaintDeviceSP src = new KisPaintDevice(m_colorSpace);
    KisPaintDeviceSP dst = new KisPaintDevice(m_colorSpace);
    src->fill(0,0,TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT, m_color.data());
    dst->fill(0,0,TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT, m_color.data());
    
    KisPainter gc(dst);
    
    QPoint pos(0,0);
    QRect rc(0,0,TEST_IMAGE_WIDTH, TEST_IMAGE_WIDTH);
    
    QBENCHMARK{
        for (int i = 0; i < CYCLES ; i++){
            gc.bitBlt(pos,src,rc);
        }
    }

}


void KisPainterBenchmark::benchmarkBitBltSelection()
{
    KisPaintDeviceSP src = new KisPaintDevice(m_colorSpace);
    KisPaintDeviceSP dst = new KisPaintDevice(m_colorSpace);
    src->fill(0,0,TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT, m_color.data());
    dst->fill(0,0,TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT, m_color.data());

    KisSelectionSP selection = new KisSelection();
    selection->getOrCreatePixelSelection()->select(QRect(0, 0, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT));
    selection->updateProjection();

    
    KisPainter gc(dst);
    gc.setSelection(selection);
    
    QPoint pos(0,0);
    QRect rc(0,0,TEST_IMAGE_WIDTH, TEST_IMAGE_WIDTH);
    
    QBENCHMARK{
        for (int i = 0; i < CYCLES ; i++){
            gc.bitBlt(pos,src,rc);
        }
    }

    
}


void KisPainterBenchmark::benchmarkFixedBitBlt()
{
    QImage img(TEST_IMAGE_WIDTH,TEST_IMAGE_HEIGHT,QImage::Format_ARGB32);
    img.fill(255);

    KisFixedPaintDeviceSP fdev = new KisFixedPaintDevice(m_colorSpace);
    fdev->convertFromQImage(img, "");

    KisPaintDeviceSP dst = new KisPaintDevice(m_colorSpace);
    KisPainter gc(dst);
    QPoint pos(0, 0);
    QRect rc = img.rect();

    QBENCHMARK{
        for (int i = 0; i < CYCLES ; i++){
            gc.bltFixed(pos,fdev,rc);
        }
    }
}


void KisPainterBenchmark::benchmarkFixedBitBltSelection()
{
    QImage img(TEST_IMAGE_WIDTH,TEST_IMAGE_HEIGHT,QImage::Format_ARGB32);
    img.fill(128);

    KisFixedPaintDeviceSP fdev = new KisFixedPaintDevice(m_colorSpace);
    fdev->convertFromQImage(img, "");

    KisPaintDeviceSP dst = new KisPaintDevice(m_colorSpace);

    KisSelectionSP selection = new KisSelection();
    selection->getOrCreatePixelSelection()->select(QRect(0, 0, TEST_IMAGE_WIDTH , TEST_IMAGE_HEIGHT));
    selection->updateProjection();

    KisPainter gc(dst);
    gc.setSelection(selection);

    QPoint pos(0, 0);
    QRect rc = img.rect();

    QBENCHMARK{
        for (int i = 0; i < CYCLES ; i++){
            gc.bltFixed(pos,fdev,rc);
        }
    }

}



QTEST_KDEMAIN(KisPainterBenchmark, NoGUI)
#include "kis_painter_benchmark.moc"
