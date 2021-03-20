/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý lukast.dev @gmail.com
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_vline_iterator_benchmark.h"
#include "kis_benchmark_values.h"

#include "kis_paint_device.h"

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColor.h>
#include <kis_iterator_ng.h>
#include <simpletest.h>


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


SIMPLE_TEST_MAIN(KisVLineIteratorBenchmark)
