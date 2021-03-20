/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_datamanager_benchmark.h"
#include "kis_benchmark_values.h"

#include <simpletest.h>
#include <kis_datamanager.h>

// RGBA
#define PIXEL_SIZE 4
//#define CYCLES 100

void KisDatamanagerBenchmark::initTestCase()
{
    // To make sure all the first-time startup costs are done
    quint8 * p = new quint8[PIXEL_SIZE];
    memset(p, 0, PIXEL_SIZE);
    KisDataManager dm(PIXEL_SIZE, p);
}

void KisDatamanagerBenchmark::benchmarkCreation()
{
    // tests the cost of creating a new datamanager

    QBENCHMARK {
        quint8 * p = new quint8[PIXEL_SIZE];
        memset(p, 255, PIXEL_SIZE);
        KisDataManager dm(PIXEL_SIZE, p);
    }
}

void KisDatamanagerBenchmark::benchmarkWriteBytes()
{
    quint8 *p = new quint8[PIXEL_SIZE];
    memset(p, 0, PIXEL_SIZE);
    KisDataManager dm(PIXEL_SIZE, p);

    quint8 *bytes = new quint8[PIXEL_SIZE * TEST_IMAGE_WIDTH * TEST_IMAGE_HEIGHT];
    memset(bytes, 128, PIXEL_SIZE * TEST_IMAGE_WIDTH * TEST_IMAGE_HEIGHT);

    QBENCHMARK {
#ifdef CYCLES
        for (int i = 0; i < CYCLES; i++){
            dm.writeBytes(bytes, 0, 0, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
        }
#else
        dm.writeBytes(bytes, 0, 0, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
#endif
    }

    delete[] bytes;
}

void KisDatamanagerBenchmark::benchmarkReadBytes()
{
    quint8 *p = new quint8[PIXEL_SIZE];
    memset(p, 0, PIXEL_SIZE);
    KisDataManager dm(PIXEL_SIZE, p);

    quint8 *bytes = new quint8[PIXEL_SIZE * TEST_IMAGE_WIDTH * TEST_IMAGE_HEIGHT];
    memset(bytes, 128, PIXEL_SIZE * TEST_IMAGE_WIDTH * TEST_IMAGE_HEIGHT);

    QBENCHMARK {
        dm.readBytes(bytes, 0, 0, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
    }

    delete[] bytes;
}


void KisDatamanagerBenchmark::benchmarkReadWriteBytes()
{
    quint8 *p = new quint8[PIXEL_SIZE];
    memset(p, 0, PIXEL_SIZE);
    KisDataManager dm(PIXEL_SIZE, p);

    quint8 *bytes = new quint8[PIXEL_SIZE * TEST_IMAGE_WIDTH * TEST_IMAGE_HEIGHT];
    memset(bytes, 120, PIXEL_SIZE * TEST_IMAGE_WIDTH * TEST_IMAGE_HEIGHT);

    dm.writeBytes(bytes, 0, 0, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);

    QBENCHMARK {
#ifdef CYCLES
        for (int i = 0; i < 100; i++){
            dm.readBytes(bytes, 0, 0, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
        }
#else
            dm.readBytes(bytes, 0, 0, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);    
#endif
    }
    delete[] bytes;
}

void KisDatamanagerBenchmark::benchmarkReadWriteBytes2()
{
    quint8 *p = new quint8[PIXEL_SIZE];
    memset(p, 0, PIXEL_SIZE);
    KisDataManager dm(PIXEL_SIZE, p);
    
    KisDataManager dab(PIXEL_SIZE, p);

    {
        quint8 *bytes = new quint8[PIXEL_SIZE * TEST_IMAGE_WIDTH * TEST_IMAGE_HEIGHT];
        memset(bytes, 120, PIXEL_SIZE * TEST_IMAGE_WIDTH * TEST_IMAGE_HEIGHT);

        dm.writeBytes(bytes, 0, 0, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
        dab.writeBytes(bytes, 0, 0, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
        delete[] bytes;
    }

    QBENCHMARK {
        int size = TEST_IMAGE_WIDTH * TEST_IMAGE_HEIGHT;
        quint8 *bytes = new quint8[PIXEL_SIZE * TEST_IMAGE_WIDTH * TEST_IMAGE_HEIGHT];
        quint8 *bytes2 = new quint8[PIXEL_SIZE * TEST_IMAGE_WIDTH * TEST_IMAGE_HEIGHT];
        dm.readBytes(bytes, 0, 0, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
        dab.readBytes(bytes2, 0, 0, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
        quint8 *bytes_it = bytes;
        quint8 *bytes2_it = bytes2;
        for (int i = 0; i < size; i += PIXEL_SIZE, bytes_it += PIXEL_SIZE, bytes2_it += PIXEL_SIZE)
        {
            memcpy(bytes_it, bytes2_it, PIXEL_SIZE);
        }
        dm.writeBytes(bytes, 0, 0, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
        delete[] bytes;
        delete[] bytes2;
    }
}

void KisDatamanagerBenchmark::benchmarkExtent()
{
    quint8 *p = new quint8[PIXEL_SIZE];
    memset(p, 0, PIXEL_SIZE);
    KisDataManager dm(PIXEL_SIZE, p);
    quint8 *bytes = new quint8[PIXEL_SIZE * NO_TILE_EXACT_BOUNDARY_WIDTH * NO_TILE_EXACT_BOUNDARY_HEIGHT];
    memset(bytes, 0, PIXEL_SIZE * NO_TILE_EXACT_BOUNDARY_WIDTH * NO_TILE_EXACT_BOUNDARY_HEIGHT);
    dm.writeBytes(bytes, 0, 0, NO_TILE_EXACT_BOUNDARY_WIDTH, NO_TILE_EXACT_BOUNDARY_HEIGHT);
    QBENCHMARK {
        QRect extent = dm.extent();
	Q_UNUSED(extent);
    }
}

void KisDatamanagerBenchmark::benchmarkClear()
{
    quint8 *p = new quint8[PIXEL_SIZE];
    memset(p, 128, PIXEL_SIZE);
    KisDataManager dm(PIXEL_SIZE, p);
    quint8 *bytes = new quint8[PIXEL_SIZE * NO_TILE_EXACT_BOUNDARY_WIDTH * NO_TILE_EXACT_BOUNDARY_HEIGHT];
    
    memset(bytes, 0, PIXEL_SIZE * NO_TILE_EXACT_BOUNDARY_WIDTH * NO_TILE_EXACT_BOUNDARY_HEIGHT);
    dm.writeBytes(bytes, 0, 0, NO_TILE_EXACT_BOUNDARY_WIDTH, NO_TILE_EXACT_BOUNDARY_HEIGHT);

    // 80% of the image will be cleared
    quint32 clearWidth = 0.8 * NO_TILE_EXACT_BOUNDARY_WIDTH;
    quint32 clearHeight = 0.8 * NO_TILE_EXACT_BOUNDARY_HEIGHT;

    QBENCHMARK {
        dm.clear(0, 0, clearWidth, clearHeight, p);
    }

}


void KisDatamanagerBenchmark::benchmarkMemCpy()
{
    quint64 imgSize = PIXEL_SIZE * TEST_IMAGE_WIDTH * TEST_IMAGE_HEIGHT;
    quint8 * src = new quint8[imgSize];
    quint8 * dst = new quint8[imgSize];
    memset(src,128, imgSize);
    memset(dst,0, imgSize);
    QBENCHMARK{
#ifdef CYCLES
        for (int i = 0; i < 100; i++){
            memcpy(dst, src , imgSize);
        }
#else
            memcpy(dst, src , imgSize);
#endif
    }

    delete[] src;
    delete[] dst;
}


SIMPLE_TEST_MAIN(KisDatamanagerBenchmark)
