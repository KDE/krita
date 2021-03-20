/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý lukast.dev @gmail.com
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_hline_iterator_benchmark.h"
#include "kis_benchmark_values.h"

#include "kis_paint_device.h"

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColor.h>

#include <simpletest.h>

#include "kis_iterator_ng.h"

void KisHLineIteratorBenchmark::initTestCase()
{
    m_colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    m_device = new KisPaintDevice(m_colorSpace);
    m_color = new KoColor(m_colorSpace);
    // some random color
    m_color->fromQColor(QColor(0,0,250));
    m_device->fill(0,0,TEST_IMAGE_WIDTH,TEST_IMAGE_HEIGHT,m_color->data());
}

void KisHLineIteratorBenchmark::cleanupTestCase()
{
    delete m_color;
    delete m_device;
}


void KisHLineIteratorBenchmark::benchmarkCreation()
{
    QBENCHMARK{
        KisHLineIteratorSP it = m_device->createHLineIteratorNG(0, 0, TEST_IMAGE_WIDTH);
    }
}

void KisHLineIteratorBenchmark::benchmarkWriteBytes()
{
    KisHLineIteratorSP it = m_device->createHLineIteratorNG(0, 0, TEST_IMAGE_WIDTH);

    QBENCHMARK{
        for (int j = 0; j < TEST_IMAGE_HEIGHT; j++) {
            do {
                memcpy(it->rawData(), m_color->data(), m_colorSpace->pixelSize());
            } while (it->nextPixel());
            it->nextRow();
        }
    }
}

void KisHLineIteratorBenchmark::benchmarkReadBytes()
{
    KisHLineIteratorSP it = m_device->createHLineIteratorNG(0, 0, TEST_IMAGE_WIDTH);

    QBENCHMARK{
        for (int j = 0; j < TEST_IMAGE_HEIGHT; j++) {
            do {
                memcpy(m_color->data(), it->rawData(), m_colorSpace->pixelSize());
            } while (it->nextPixel());
            it->nextRow();
        }
    }
}


void KisHLineIteratorBenchmark::benchmarkConstReadBytes()
{
    KisHLineConstIteratorSP cit = m_device->createHLineConstIteratorNG(0, 0, TEST_IMAGE_WIDTH);

    QBENCHMARK{
        for (int j = 0; j < TEST_IMAGE_HEIGHT; j++) {
            do {
                memcpy(m_color->data(), cit->oldRawData(), m_colorSpace->pixelSize());
            } while (cit->nextPixel());
            cit->nextRow();
        }
    }
}

void KisHLineIteratorBenchmark::benchmarkReadWriteBytes(){
    KoColor c(m_colorSpace);
    c.fromQColor(QColor(250,120,0));
    KisPaintDevice dab(m_colorSpace);
    dab.fill(0,0,TEST_IMAGE_WIDTH,TEST_IMAGE_HEIGHT, c.data());
    
    KisHLineIteratorSP writeIterator = m_device->createHLineIteratorNG(0, 0, TEST_IMAGE_WIDTH);
    KisHLineConstIteratorSP constReadIterator = dab.createHLineConstIteratorNG(0, 0, TEST_IMAGE_WIDTH);

    QBENCHMARK{
        for (int j = 0; j < TEST_IMAGE_HEIGHT; j++) {
            do {
                memcpy(writeIterator->rawData(), constReadIterator->oldRawData(), m_colorSpace->pixelSize());
            } while (constReadIterator->nextPixel() && writeIterator->nextPixel());
            constReadIterator->nextRow();
            writeIterator->nextRow();
        }
    }
}



void KisHLineIteratorBenchmark::benchmarkReadWriteBytes2()
{
    KoColor c(m_colorSpace);
    c.fromQColor(QColor(255,0,0));
    KisPaintDevice dab(m_colorSpace);
    dab.fill(0,0,TEST_IMAGE_WIDTH,TEST_IMAGE_HEIGHT, c.data());

    KisHLineIteratorSP writeIterator = m_device->createHLineIteratorNG(0,0,TEST_IMAGE_WIDTH);
    KisHLineIteratorSP readIterator = dab.createHLineIteratorNG(0,0,TEST_IMAGE_WIDTH);
    
    QBENCHMARK{
        for (int j = 0; j < TEST_IMAGE_HEIGHT; j++) {
            do {
                memcpy(writeIterator->rawData(), readIterator->rawData(), m_colorSpace->pixelSize());
                writeIterator->nextPixel();
            } while (readIterator->nextPixel());
            readIterator->nextRow();
            writeIterator->nextRow();
        }
    }

    ///QImage img = m_device->convertToQImage(m_device->colorSpace()->profile(),0,0,TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
    //img.save("write.png");
    
}


void KisHLineIteratorBenchmark::benchmarkNoMemCpy()
{
    KisHLineIteratorSP it = m_device->createHLineIteratorNG(0, 0, TEST_IMAGE_WIDTH);

    QBENCHMARK{
        for (int j = 0; j < TEST_IMAGE_HEIGHT; j++) {
            do {} while (it->nextPixel());
            it->nextRow();
        }
    }
}


void KisHLineIteratorBenchmark::benchmarkConstNoMemCpy()
{
    KisHLineConstIteratorSP cit = m_device->createHLineConstIteratorNG(0, 0, TEST_IMAGE_WIDTH);

    QBENCHMARK{
        for (int j = 0; j < TEST_IMAGE_HEIGHT; j++) {
            do {} while (cit->nextPixel());
            cit->nextRow();
        }
    }
}

void KisHLineIteratorBenchmark::benchmarkTwoIteratorsNoMemCpy(){
    KoColor c(m_colorSpace);
    c.fromQColor(QColor(250,120,0));
    KisPaintDevice dab(m_colorSpace);
    dab.fill(0,0,TEST_IMAGE_WIDTH,TEST_IMAGE_HEIGHT, c.data());
    
    KisHLineIteratorSP writeIterator = m_device->createHLineIteratorNG(0, 0, TEST_IMAGE_WIDTH);
    KisHLineConstIteratorSP constReadIterator = dab.createHLineConstIteratorNG(0, 0, TEST_IMAGE_WIDTH);

    QBENCHMARK{
        for (int j = 0; j < TEST_IMAGE_HEIGHT; j++) {
            do {} while (writeIterator->nextPixel() && constReadIterator->nextPixel());
            constReadIterator->nextRow();
            writeIterator->nextRow();
        }
    }
    
}



SIMPLE_TEST_MAIN(KisHLineIteratorBenchmark)
