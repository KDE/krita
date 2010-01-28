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

#include "kis_random_iterator_benchmark.h"
#include "kis_benchmark_values.h"

#include "kis_iterators_pixel.h"
#include "kis_paint_device.h"

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColor.h>

#include <qtest_kde.h>
#include <kis_random_accessor.h>


void KisRandomIteratorBenchmark::initTestCase()
{
    m_colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    m_device = new KisPaintDevice(m_colorSpace);
    m_color = new KoColor(m_colorSpace);
    // some random color
    m_color->fromQColor(QColor(0,120,250));
    m_device->fill(0,0,TEST_IMAGE_WIDTH,TEST_IMAGE_HEIGHT,m_color->data());
}

void KisRandomIteratorBenchmark::cleanupTestCase()
{
    delete m_color;
    delete m_device;
}


void KisRandomIteratorBenchmark::benchmarkCreation()
{
    QBENCHMARK{
        KisRandomAccessor it = m_device->createRandomAccessor(0,0);
    }
}

void KisRandomIteratorBenchmark::benchmarkWriteBytes()
{
    KisRandomAccessor it = m_device->createRandomAccessor(0,0);
    
    QBENCHMARK{
        for (int i = 0; i < TEST_IMAGE_HEIGHT; i++){
            for (int j = 0; j < TEST_IMAGE_WIDTH; j++) {
                it.moveTo(j,i);
                memcpy(it.rawData(), m_color->data(), m_colorSpace->pixelSize());
            }
        }
    }
}
    

void KisRandomIteratorBenchmark::benchmarkReadBytes()
{
    KisRandomAccessor it = m_device->createRandomAccessor(0,0);

    QBENCHMARK{
        for (int i = 0; i < TEST_IMAGE_HEIGHT; i++){
            for (int j = 0; j < TEST_IMAGE_WIDTH; j++) {
                it.moveTo(j,i);
                memcpy(it.rawData(), m_color->data(), m_colorSpace->pixelSize());
            }
        }
    }
}


void KisRandomIteratorBenchmark::benchmarkConstReadBytes()
{
    KisRandomConstAccessor it = m_device->createRandomConstAccessor(0,0);

    QBENCHMARK{
        for (int i = 0; i < TEST_IMAGE_HEIGHT; i++){
            for (int j = 0; j < TEST_IMAGE_WIDTH; j++) {
                it.moveTo(j,i);
                memcpy(m_color->data(),it.rawData(), m_colorSpace->pixelSize());
            }
        }
    }
}

void KisRandomIteratorBenchmark::benchmarkReadWriteBytes(){
    KoColor c(m_colorSpace);
    c.fromQColor(QColor(250,120,0));
    KisPaintDevice dab(m_colorSpace);
    dab.fill(0,0,TEST_IMAGE_WIDTH,TEST_IMAGE_HEIGHT, c.data());
    
    KisRandomAccessor writeIterator = m_device->createRandomAccessor(0,0);
    KisRandomConstAccessor constReadIterator = dab.createRandomConstAccessor(0,0);

    QBENCHMARK{
        for (int i = 0; i < TEST_IMAGE_HEIGHT; i++){
            for (int j = 0; j < TEST_IMAGE_WIDTH; j++) {
                writeIterator.moveTo(j,i);
                constReadIterator.moveTo(j,i);
                memcpy(writeIterator.rawData(), constReadIterator.rawData(), m_colorSpace->pixelSize());
            }
        }
    }
    
}


void KisRandomIteratorBenchmark::benchmarkTotalRandom()
{
    KisRandomAccessor it = m_device->createRandomAccessor(0,0);
    // set the seed so that we always go in the same permutation over the device
    srand(123456);
   
    QBENCHMARK{
        for (int i = 0; i < TEST_IMAGE_HEIGHT; i++){
            for (int j = 0; j < TEST_IMAGE_WIDTH; j++) {
                it.moveTo( rand() % TEST_IMAGE_WIDTH, 
                           rand() % TEST_IMAGE_HEIGHT );
                memcpy(it.rawData(), m_color->data(), m_colorSpace->pixelSize());
            }
        }
    }
}

void KisRandomIteratorBenchmark::benchmarkTotalRandomConst()
{
    KisRandomConstAccessor it = m_device->createRandomConstAccessor(0,0);
    // set the seed so that we always go in the same permutation over the device
    srand(123456);
   
    QBENCHMARK{
        for (int i = 0; i < TEST_IMAGE_HEIGHT; i++){
            for (int j = 0; j < TEST_IMAGE_WIDTH; j++) {
                it.moveTo( rand() % TEST_IMAGE_WIDTH, 
                           rand() % TEST_IMAGE_HEIGHT );
                memcpy(m_color->data(), it.rawData(), m_colorSpace->pixelSize());
            }
        }
    }
}



void KisRandomIteratorBenchmark::benchmarkNoMemCpy()
{
    KisRandomAccessor it = m_device->createRandomAccessor(0,0);
    
    QBENCHMARK{
        for (int i = 0; i < TEST_IMAGE_HEIGHT; i++){
            for (int j = 0; j < TEST_IMAGE_WIDTH; j++) {
                it.moveTo(j,i);
            }
        }
    }
}
    

void KisRandomIteratorBenchmark::benchmarkConstNoMemCpy()
{
    KisRandomConstAccessor it = m_device->createRandomConstAccessor(0,0);

    QBENCHMARK{
        for (int i = 0; i < TEST_IMAGE_HEIGHT; i++){
            for (int j = 0; j < TEST_IMAGE_WIDTH; j++) {
                it.moveTo(j,i);
            }
        }
    }
}

#define TEST_AREA_WIDTH 256
#define TEST_AREA_HEIGHT 64
void KisRandomIteratorBenchmark::benchmarkTileByTileWrite()
{
    int xTiles = TEST_IMAGE_WIDTH / TEST_AREA_WIDTH;
    int yTiles = TEST_IMAGE_HEIGHT / TEST_AREA_HEIGHT;
    
    int xUnprocessed = int(TEST_IMAGE_WIDTH) % int(TEST_AREA_WIDTH);
    int yUnprocessed = int(TEST_IMAGE_HEIGHT) % int(TEST_AREA_HEIGHT);
    if ((xUnprocessed) != 0 || (yUnprocessed) != 0)
    {
        kWarning() << "There will be some unprocessed pixels! Test area differs from the image size";
    }
    
    KisRandomAccessor it = m_device->createRandomAccessor(0,0);
    QBENCHMARK{
        for (int yTile = 0; yTile < yTiles; yTile++){
            for (int xTile = 0; xTile < xTiles; xTile++){
                int x = xTile * TEST_AREA_WIDTH;
                int y = yTile * TEST_AREA_HEIGHT;
                for (int j = y; j< y+TEST_AREA_HEIGHT; j++ ){
                    for (int i = x; i < x+TEST_AREA_WIDTH ; i++){
                        it.moveTo(i,j);
                        memcpy(it.rawData(), m_color->data(), m_colorSpace->pixelSize());
                    }
                }
            }
        }
    }
}


void KisRandomIteratorBenchmark::benchmarkTwoIteratorsNoMemCpy()
{
    KoColor c(m_colorSpace);
    c.fromQColor(QColor(250,120,0));
    KisPaintDevice dab(m_colorSpace);
    dab.fill(0,0,TEST_IMAGE_WIDTH,TEST_IMAGE_HEIGHT, c.data());
    
    KisRandomAccessor writeIterator = m_device->createRandomAccessor(0,0);
    KisRandomConstAccessor constReadIterator = dab.createRandomConstAccessor(0,0);

    QBENCHMARK{
        for (int i = 0; i < TEST_IMAGE_HEIGHT; i++){
            for (int j = 0; j < TEST_IMAGE_WIDTH; j++) {
                writeIterator.moveTo(j,i);
                constReadIterator.moveTo(j,i);
            }
        }
    }
}


QTEST_KDEMAIN(KisRandomIteratorBenchmark, GUI)
#include "kis_random_iterator_benchmark.moc"
