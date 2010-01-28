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

#include "kis_rect_iterator_benchmark.h"
#include "kis_benchmark_values.h"

#include "kis_iterators_pixel.h"
#include "kis_paint_device.h"

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColor.h>

#include <qtest_kde.h>


void KisRectIteratorBenchmark::initTestCase()
{
    m_colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    m_device = new KisPaintDevice(m_colorSpace);
    m_color = new KoColor(m_colorSpace);
    // some random color
    m_color->fromQColor(QColor(0,120,250));
    m_device->fill(0,0,TEST_IMAGE_WIDTH,TEST_IMAGE_HEIGHT,m_color->data());
}

void KisRectIteratorBenchmark::cleanupTestCase()
{
    delete m_color;
    delete m_device;
}


void KisRectIteratorBenchmark::benchmarkCreation()
{
    QBENCHMARK{
        KisRectIteratorPixel it = m_device->createRectIterator(0, 0, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
    }
}

void KisRectIteratorBenchmark::benchmarkWriteBytes()
{
    KisRectIteratorPixel it = m_device->createRectIterator(0, 0, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
    QBENCHMARK{
            while (!it.isDone()) {
                memcpy(it.rawData(), m_color->data(), m_colorSpace->pixelSize());
                ++it;
            }
    }
}
    

void KisRectIteratorBenchmark::benchmarkReadBytes()
{
    KisRectIteratorPixel it = m_device->createRectIterator(0, 0, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);

    QBENCHMARK{
            while (!it.isDone()) {
                memcpy(m_color->data(), it.rawData(), m_colorSpace->pixelSize());
                ++it;
            }
    }
}


void KisRectIteratorBenchmark::benchmarkConstReadBytes()
{
    KisRectConstIteratorPixel it = m_device->createRectConstIterator(0, 0, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);

    QBENCHMARK{
            while (!it.isDone()) {
                memcpy(m_color->data(), it.rawData(), m_colorSpace->pixelSize());
                ++it;
            }
    }
}

void KisRectIteratorBenchmark::benchmarkReadWriteBytes(){
    KoColor c(m_colorSpace);
    c.fromQColor(QColor(250,120,0));
    KisPaintDevice dab(m_colorSpace);
    dab.fill(0,0,TEST_IMAGE_WIDTH,TEST_IMAGE_HEIGHT, c.data());
    
    KisRectIteratorPixel writeIterator = m_device->createRectIterator(0, 0, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
    KisRectConstIteratorPixel constReadIterator = dab.createRectConstIterator(0, 0, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);

    QBENCHMARK
    {
        while (!constReadIterator.isDone()) {
            memcpy(writeIterator.rawData(), constReadIterator.rawData(), m_colorSpace->pixelSize());
            ++constReadIterator;
            ++writeIterator;
        }
    }
}

    

void KisRectIteratorBenchmark::benchmarkNoMemCpy()
{
    KisRectIteratorPixel it = m_device->createRectIterator(0, 0, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);

    QBENCHMARK{
            while (!it.isDone()) {
                ++it;
            }
    }
}


void KisRectIteratorBenchmark::benchmarkConstNoMemCpy()
{
    KisRectConstIteratorPixel it = m_device->createRectConstIterator(0, 0, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);

    QBENCHMARK{
            while (!it.isDone()) {
                ++it;
            }
    }
}



void KisRectIteratorBenchmark::benchmarkTwoIteratorsNoMemCpy()
{
    KoColor c(m_colorSpace);
    c.fromQColor(QColor(250,120,0));
    KisPaintDevice dab(m_colorSpace);
    dab.fill(0,0,TEST_IMAGE_WIDTH,TEST_IMAGE_HEIGHT, c.data());
    
    KisRectIteratorPixel writeIterator = m_device->createRectIterator(0, 0, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
    KisRectConstIteratorPixel constReadIterator = dab.createRectConstIterator(0, 0, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);

    QBENCHMARK
    {
        while (!constReadIterator.isDone()) {
            ++constReadIterator;
            ++writeIterator;
        }
    }
}


QTEST_KDEMAIN(KisRectIteratorBenchmark, GUI)
#include "kis_rect_iterator_benchmark.moc"
