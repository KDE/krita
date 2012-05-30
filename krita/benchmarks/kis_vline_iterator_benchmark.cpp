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

#include "kis_vline_iterator_benchmark.h"
#include "kis_benchmark_values.h"

#include "kis_paint_device.h"

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColor.h>
#include <kis_iterator_ng.h>
#include <qtest_kde.h>


void KisVLineIteratorBenchmark::initTestCase()
{
    m_colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    m_device = new KisPaintDevice(m_colorSpace);
    m_color = new KoColor(m_colorSpace);
    // some random color
    m_color->fromQColor(QColor(0,120,250));
    m_device->fill(0,0,TEST_IMAGE_WIDTH,TEST_IMAGE_HEIGHT,m_color->data());
}

void KisVLineIteratorBenchmark::cleanupTestCase()
{
    delete m_color;
    delete m_device;
}


void KisVLineIteratorBenchmark::benchmarkCreation()
{
    QBENCHMARK{
        KisVLineIteratorSP it = m_device->createVLineIteratorNG(0, 0, TEST_IMAGE_HEIGHT);
    }
}

void KisVLineIteratorBenchmark::benchmarkWriteBytes()
{
    KisVLineIteratorSP it = m_device->createVLineIteratorNG(0, 0, TEST_IMAGE_HEIGHT);

    QBENCHMARK{
        for (int j = 0; j < TEST_IMAGE_WIDTH; j++) {
            do {
                memcpy(it->rawData(), m_color->data(), m_colorSpace->pixelSize());
            } while (it->nextPixel());
            it->nextColumn();
        }
    }
}
    

void KisVLineIteratorBenchmark::benchmarkReadBytes()
{
    KisVLineIteratorSP it = m_device->createVLineIteratorNG(0, 0, TEST_IMAGE_HEIGHT);

    QBENCHMARK{
        for (int j = 0; j < TEST_IMAGE_WIDTH; j++) {
            do {
                memcpy(m_color->data(), it->rawData(), m_colorSpace->pixelSize());
            } while (it->nextPixel());
            it->nextColumn();
        }
    }
}


void KisVLineIteratorBenchmark::benchmarkConstReadBytes()
{
    KisVLineConstIteratorSP it = m_device->createVLineConstIteratorNG(0, 0, TEST_IMAGE_HEIGHT);

    QBENCHMARK{
        for (int j = 0; j < TEST_IMAGE_WIDTH; j++) {
            do {
                memcpy(m_color->data(), it->oldRawData(), m_colorSpace->pixelSize());
            } while (it->nextPixel());
            it->nextColumn();
        }
    }
}

void KisVLineIteratorBenchmark::benchmarkReadWriteBytes(){
    KoColor c(m_colorSpace);
    c.fromQColor(QColor(250,120,0));
    KisPaintDevice dab(m_colorSpace);
    dab.fill(0,0,TEST_IMAGE_WIDTH,TEST_IMAGE_HEIGHT, c.data());
    
    KisVLineIteratorSP writeIterator = m_device->createVLineIteratorNG(0, 0, TEST_IMAGE_HEIGHT);
    KisVLineConstIteratorSP constReadIterator = dab.createVLineConstIteratorNG(0, 0, TEST_IMAGE_HEIGHT);

    QBENCHMARK{
        for (int j = 0; j < TEST_IMAGE_WIDTH; j++) {
            do {
                memcpy(writeIterator->rawData(), constReadIterator->oldRawData(), m_colorSpace->pixelSize());
            } while (constReadIterator->nextPixel() && writeIterator->nextPixel());
            constReadIterator->nextColumn();
            writeIterator->nextColumn();
        }
    }
    
}

void KisVLineIteratorBenchmark::benchmarkNoMemCpy()
{
    KisVLineIteratorSP it = m_device->createVLineIteratorNG(0, 0, TEST_IMAGE_HEIGHT);

    QBENCHMARK{
        for (int j = 0; j < TEST_IMAGE_WIDTH; j++) {
            do {
            } while (it->nextPixel());
            it->nextColumn();
        }
    }
}
    



void KisVLineIteratorBenchmark::benchmarkConstNoMemCpy()
{
    KisVLineConstIteratorSP it = m_device->createVLineConstIteratorNG(0, 0, TEST_IMAGE_HEIGHT);

    QBENCHMARK{
        for (int j = 0; j < TEST_IMAGE_WIDTH; j++) {
            do {} while (it->nextPixel());
            it->nextColumn();
        }
    }
}


void KisVLineIteratorBenchmark::benchmarkTwoIteratorsNoMemCpy()
{
    KoColor c(m_colorSpace);
    c.fromQColor(QColor(250,120,0));
    KisPaintDevice dab(m_colorSpace);
    dab.fill(0,0,TEST_IMAGE_WIDTH,TEST_IMAGE_HEIGHT, c.data());
    
    KisVLineIteratorSP writeIterator = m_device->createVLineIteratorNG(0, 0, TEST_IMAGE_HEIGHT);
    KisVLineConstIteratorSP constReadIterator = dab.createVLineConstIteratorNG(0, 0, TEST_IMAGE_HEIGHT);

    QBENCHMARK{
        for (int j = 0; j < TEST_IMAGE_WIDTH; j++) {
            do {} while (constReadIterator->nextPixel() && writeIterator->nextPixel());
            constReadIterator->nextColumn();
            writeIterator->nextColumn();
        }
    }
}


QTEST_KDEMAIN(KisVLineIteratorBenchmark, GUI)
#include "kis_vline_iterator_benchmark.moc"
