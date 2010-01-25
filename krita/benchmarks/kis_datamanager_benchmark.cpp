/*
 *  Copyright (c) 2007 Boudewijn Rempt boud@valdyas.org
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

#include "kis_datamanager_benchmark.h"
#include "kis_benchmark_values.h"

#include <qtest_kde.h>
#include <kis_datamanager.h>

void KisDatamanagerBenchmark::initTestCase()
{
    // To make sure all the first-time startup costs are done
    quint8 * p = new quint8[3];
    memset(p, 0, 3);
    KisDataManager dm(3, p);
}

void KisDatamanagerBenchmark::benchmarkCreation()
{
    // tests the cost of creating a new datamanager

    QBENCHMARK {
        quint8 * p = new quint8[3];
        memset(p, 0, 3);
        KisDataManager dm(3, p);
    }
}

void KisDatamanagerBenchmark::benchmarkWriteBytes()
{
    quint8 *p = new quint8[3];
    memset(p, 0, 3);
    KisDataManager dm(3, p);

    quint8 *bytes = new quint8[3 * TEST_IMAGE_WIDTH * TEST_IMAGE_HEIGHT];
    memset(bytes, 0, 3 * TEST_IMAGE_WIDTH * TEST_IMAGE_HEIGHT);

    QBENCHMARK {
        for(int i = 0; i < 100 * 1024; i += 1024) {
            dm.writeBytes(bytes, i, 0, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
        }
    }

    delete[] bytes;
}

void KisDatamanagerBenchmark::benchmarkReadBytes()
{
    quint8 *p = new quint8[3];
    memset(p, 0, 3);
    KisDataManager dm(3, p);

    quint8 *bytes = new quint8[3 * TEST_IMAGE_WIDTH * TEST_IMAGE_HEIGHT];
    memset(bytes, 0, 3 * TEST_IMAGE_WIDTH * TEST_IMAGE_HEIGHT);

    QBENCHMARK {
        for(int i = 0; i < 100 * TEST_IMAGE_WIDTH; i += TEST_IMAGE_WIDTH) {
            dm.readBytes(bytes, i, 0, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
        }
    }

    delete[] bytes;
}


void KisDatamanagerBenchmark::benchmarkReadWriteBytes()
{
    quint8 *p = new quint8[3];
    memset(p, 0, 3);
    KisDataManager dm(3, p);

    quint8 *bytes = new quint8[3 * TEST_IMAGE_WIDTH * TEST_IMAGE_HEIGHT];
    memset(bytes, 0, 3 * TEST_IMAGE_WIDTH * TEST_IMAGE_HEIGHT);

    QBENCHMARK {
        for(int i = 0; i < 100 * TEST_IMAGE_WIDTH; i += TEST_IMAGE_WIDTH) {
            dm.writeBytes(bytes, i, 0, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
        }
        for(int i = 0; i < 100 * TEST_IMAGE_WIDTH; i += TEST_IMAGE_WIDTH) {
            dm.readBytes(bytes, i, 0, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
        }
    }

    delete[] bytes;
}

void KisDatamanagerBenchmark::benchmarkExtent()
{
    quint8 *p = new quint8[3];
    memset(p, 0, 3);
    KisDataManager dm(3, p);
    quint8 *bytes = new quint8[3 * NO_TILE_EXACT_BOUNDARY_WIDTH * NO_TILE_EXACT_BOUNDARY_HEIGHT];
    memset(bytes, 0, 3 * NO_TILE_EXACT_BOUNDARY_WIDTH * NO_TILE_EXACT_BOUNDARY_HEIGHT);
    dm.writeBytes(bytes, 0, 0, NO_TILE_EXACT_BOUNDARY_WIDTH, NO_TILE_EXACT_BOUNDARY_HEIGHT);
    QBENCHMARK {
        QRect extent = dm.extent();
    }
}

void KisDatamanagerBenchmark::benchmarkClear()
{
    quint8 *p = new quint8[3];
    memset(p, 128, 3);
    KisDataManager dm(3, p);
    quint8 *bytes = new quint8[3 * NO_TILE_EXACT_BOUNDARY_WIDTH * NO_TILE_EXACT_BOUNDARY_HEIGHT];
    
    memset(bytes, 0, 3 * NO_TILE_EXACT_BOUNDARY_WIDTH * NO_TILE_EXACT_BOUNDARY_HEIGHT);
    dm.writeBytes(bytes, 0, 0, NO_TILE_EXACT_BOUNDARY_WIDTH, NO_TILE_EXACT_BOUNDARY_HEIGHT);

    // 80% of the image will be cleared
    quint32 clearWidth = 0.8 * NO_TILE_EXACT_BOUNDARY_WIDTH;
    quint32 clearHeight = 0.8 * NO_TILE_EXACT_BOUNDARY_HEIGHT;

    QBENCHMARK {
        dm.clear(0, 0, clearWidth, clearHeight, p);
    }

}

QTEST_KDEMAIN(KisDatamanagerBenchmark, GUI)
#include "kis_datamanager_benchmark.moc"
