/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý lukast.dev @gmail.com
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_random_iterator_benchmark.h"
#include "kis_benchmark_values.h"

#include "kis_paint_device.h"

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColor.h>

#include <simpletest.h>
#include <kis_random_accessor_ng.h>


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
        KisRandomAccessorSP it = m_device->createRandomAccessorNG();
    }
}

void KisRandomIteratorBenchmark::benchmarkWriteBytes()
{
    KisRandomAccessorSP it = m_device->createRandomAccessorNG();
    
    QBENCHMARK{
        for (int i = 0; i < TEST_IMAGE_HEIGHT; i++){
            for (int j = 0; j < TEST_IMAGE_WIDTH; j++) {
                it->moveTo(j,i);
                memcpy(it->rawData(), m_color->data(), m_colorSpace->pixelSize());
            }
        }
    }
}
    

void KisRandomIteratorBenchmark::benchmarkReadBytes()
{
    KisRandomAccessorSP it = m_device->createRandomAccessorNG();

    QBENCHMARK{
        for (int i = 0; i < TEST_IMAGE_HEIGHT; i++){
            for (int j = 0; j < TEST_IMAGE_WIDTH; j++) {
                it->moveTo(j,i);
                memcpy(m_color->data(), it->rawData(), m_colorSpace->pixelSize());
            }
        }
    }
}


void KisRandomIteratorBenchmark::benchmarkConstReadBytes()
{
    KisRandomConstAccessorSP it = m_device->createRandomConstAccessorNG();

    QBENCHMARK{
        for (int i = 0; i < TEST_IMAGE_HEIGHT; i++){
            for (int j = 0; j < TEST_IMAGE_WIDTH; j++) {
                it->moveTo(j,i);
                memcpy(m_color->data(), it->oldRawData(), m_colorSpace->pixelSize());
            }
        }
    }
}

void KisRandomIteratorBenchmark::benchmarkReadWriteBytes(){
    KoColor c(m_colorSpace);
    c.fromQColor(QColor(250,120,0));
    KisPaintDevice dab(m_colorSpace);
    dab.fill(0,0,TEST_IMAGE_WIDTH,TEST_IMAGE_HEIGHT, c.data());
    
    KisRandomAccessorSP writeIterator = m_device->createRandomAccessorNG();
    KisRandomConstAccessorSP constReadIterator = dab.createRandomConstAccessorNG();

    QBENCHMARK{
        for (int i = 0; i < TEST_IMAGE_HEIGHT; i++){
            for (int j = 0; j < TEST_IMAGE_WIDTH; j++) {
                writeIterator->moveTo(j,i);
                constReadIterator->moveTo(j,i);
                memcpy(writeIterator->rawData(), constReadIterator->oldRawData(), m_colorSpace->pixelSize());
            }
        }
    }
    
}


void KisRandomIteratorBenchmark::benchmarkTotalRandom()
{
    KisRandomAccessorSP it = m_device->createRandomAccessorNG();
    // set the seed so that we always go in the same permutation over the device
    srand(123456);
   
    QBENCHMARK{
        for (int i = 0; i < TEST_IMAGE_HEIGHT; i++){
            for (int j = 0; j < TEST_IMAGE_WIDTH; j++) {
                it->moveTo( rand() % TEST_IMAGE_WIDTH, 
                           rand() % TEST_IMAGE_HEIGHT );
                memcpy(it->rawData(), m_color->data(), m_colorSpace->pixelSize());
            }
        }
    }
}

void KisRandomIteratorBenchmark::benchmarkTotalRandomConst()
{
    KisRandomConstAccessorSP it = m_device->createRandomConstAccessorNG();
    // set the seed so that we always go in the same permutation over the device
    srand(123456);
   
    QBENCHMARK{
        for (int i = 0; i < TEST_IMAGE_HEIGHT; i++){
            for (int j = 0; j < TEST_IMAGE_WIDTH; j++) {
                it->moveTo( rand() % TEST_IMAGE_WIDTH, 
                           rand() % TEST_IMAGE_HEIGHT );
                memcpy(m_color->data(), it->oldRawData(), m_colorSpace->pixelSize());
            }
        }
    }
}



void KisRandomIteratorBenchmark::benchmarkNoMemCpy()
{
    KisRandomAccessorSP it = m_device->createRandomAccessorNG();
    
    QBENCHMARK{
        for (int i = 0; i < TEST_IMAGE_HEIGHT; i++){
            for (int j = 0; j < TEST_IMAGE_WIDTH; j++) {
                it->moveTo(j,i);
            }
        }
    }
}
    

void KisRandomIteratorBenchmark::benchmarkConstNoMemCpy()
{
    KisRandomConstAccessorSP it = m_device->createRandomConstAccessorNG();

    QBENCHMARK{
        for (int i = 0; i < TEST_IMAGE_HEIGHT; i++){
            for (int j = 0; j < TEST_IMAGE_WIDTH; j++) {
                it->moveTo(j,i);
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
        dbgKrita << "There will be some unprocessed pixels! Test area differs from the image size";
    }
    
    KisRandomAccessorSP it = m_device->createRandomAccessorNG();
    QBENCHMARK{
        for (int yTile = 0; yTile < yTiles; yTile++){
            for (int xTile = 0; xTile < xTiles; xTile++){
                int x = xTile * TEST_AREA_WIDTH;
                int y = yTile * TEST_AREA_HEIGHT;
                for (int j = y; j< y+TEST_AREA_HEIGHT; j++ ){
                    for (int i = x; i < x+TEST_AREA_WIDTH ; i++){
                        it->moveTo(i,j);
                        memcpy(it->rawData(), m_color->data(), m_colorSpace->pixelSize());
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
    
    KisRandomAccessorSP writeIterator = m_device->createRandomAccessorNG();
    KisRandomConstAccessorSP constReadIterator = dab.createRandomConstAccessorNG();

    QBENCHMARK{
        for (int i = 0; i < TEST_IMAGE_HEIGHT; i++){
            for (int j = 0; j < TEST_IMAGE_WIDTH; j++) {
                writeIterator->moveTo(j,i);
                constReadIterator->moveTo(j,i);
            }
        }
    }
}


SIMPLE_TEST_MAIN(KisRandomIteratorBenchmark)
