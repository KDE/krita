/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_low_memory_tests.h"

#include <QTest>

#include <QThreadPool>

#include "kis_image_config.h"
#include "tiles_test_utils.h"
#include "tiles3/kis_tiled_data_manager.h"
#include "tiles3/kis_tile_data_store.h"
#include <kis_debug.h>
#include "config-limit-long-tests.h"

void KisLowMemoryTests::initTestCase()
{
    // hard limit of 1MiB, no undo in memory, no clones
    KisImageConfig config(false);
    config.setMemoryHardLimitPercent(1.1 * 100.0 / KisImageConfig::totalRAM());
    config.setMemorySoftLimitPercent(0);
    config.setMemoryPoolLimitPercent(0);
}


class DeadlockyThread : public QRunnable
{
public:
    enum Type {
        PRODUCER,
        CONSUMER_SRC,
        CONSUMER_DST
    };

    DeadlockyThread(Type type,
                    KisTiledDataManager &srcDM,
                    KisTiledDataManager &dstDM,
                    int numTiles,
                    int numCycles)
        : m_type(type),
          m_srcDM(srcDM),
          m_dstDM(dstDM),
          m_numTiles(numTiles),
          m_numCycles(numCycles)
    {
    }

    void run() override {
        switch(m_type) {
        case PRODUCER:
            for (int j = 0; j < m_numCycles; j++) {
                for (int i = 0; i < m_numTiles; i++) {
                    KisTileSP voidTile = m_srcDM.getTile(i, 0, true);
                    voidTile->lockForWrite();
                    QTest::qSleep(1);
                    voidTile->unlockForWrite();
                }

                QRect cloneRect(0, 0, m_numTiles * 64, 64);
                m_dstDM.bitBltRough(&m_srcDM, cloneRect);

                if(j % 50 == 0) dbgKrita << "Producer:" << j << "of" << m_numCycles;

                KisTileDataStore::instance()->debugSwapAll();
            }
            break;
        case CONSUMER_SRC:
            for (int j = 0; j < m_numCycles; j++) {
                for (int i = 0; i < m_numTiles; i++) {
                    KisTileSP voidTile = m_srcDM.getTile(i, 0, false);
                    voidTile->lockForRead();
                    char temp = *voidTile->data();
                    Q_UNUSED(temp);
                    QTest::qSleep(1);
                    voidTile->unlockForRead();
                }

                if(j % 50 == 0) dbgKrita << "Consumer_src:" << j << "of" << m_numCycles;

                KisTileDataStore::instance()->debugSwapAll();
            }
            break;
        case CONSUMER_DST:
            for (int j = 0; j < m_numCycles; j++) {
                for (int i = 0; i < m_numTiles; i++) {
                    KisTileSP voidTile = m_dstDM.getTile(i, 0, false);
                    voidTile->lockForRead();
                    char temp = *voidTile->data();
                    Q_UNUSED(temp);
                    QTest::qSleep(1);
                    voidTile->unlockForRead();
                }

                if(j % 50 == 0) dbgKrita << "Consumer_dst:" << j << "of" << m_numCycles;

                KisTileDataStore::instance()->debugSwapAll();
            }

        }
    }

private:
    Type m_type;
    KisTiledDataManager &m_srcDM;
    KisTiledDataManager &m_dstDM;
    int m_numTiles;
    int m_numCycles;
};

void KisLowMemoryTests::readWriteOnSharedTiles()
{
    quint8 defaultPixel = 0;
    KisTiledDataManager srcDM(1, &defaultPixel);
    KisTiledDataManager dstDM(1, &defaultPixel);

    const int NUM_TILES = 10;

#ifdef LIMIT_LONG_TESTS
    const int NUM_CYCLES = 800;
#else
    const int NUM_CYCLES = 10000;
#endif

    QThreadPool pool;
    pool.setMaxThreadCount(10);

    pool.start(new DeadlockyThread(DeadlockyThread::PRODUCER,
                                   srcDM, dstDM, NUM_TILES, NUM_CYCLES));

    for (int i = 0; i < 4; i++) {
        pool.start(new DeadlockyThread(DeadlockyThread::CONSUMER_SRC,
                                       srcDM, dstDM, NUM_TILES, NUM_CYCLES));
        pool.start(new DeadlockyThread(DeadlockyThread::CONSUMER_DST,
                                       srcDM, dstDM, NUM_TILES, NUM_CYCLES));
    }

    pool.waitForDone();
}

void KisLowMemoryTests::hangingTilesTest()
{
    quint8 defaultPixel = 0;
    KisTiledDataManager srcDM(1, &defaultPixel);

    KisTileSP srcTile = srcDM.getTile(0, 0, true);

    srcTile->lockForWrite();
    srcTile->lockForRead();


    KisTiledDataManager dstDM(1, &defaultPixel);
    dstDM.bitBlt(&srcDM, QRect(0,0,64,64));

    KisTileSP dstTile = dstDM.getTile(0, 0, true);

    dstTile->lockForRead();
    KisTileData *weirdTileData = dstTile->tileData();
    quint8 *weirdData = dstTile->data();

    QCOMPARE(weirdTileData, srcTile->tileData());
    QCOMPARE(weirdData, srcTile->data());

    KisTileDataStore::instance()->debugSwapAll();
    QCOMPARE(srcTile->tileData(), weirdTileData);
    QCOMPARE(dstTile->tileData(), weirdTileData);
    QCOMPARE(srcTile->data(), weirdData);
    QCOMPARE(dstTile->data(), weirdData);

    dstTile->lockForWrite();
    KisTileData *cowedTileData = dstTile->tileData();
    quint8 *cowedData = dstTile->data();

    QVERIFY(cowedTileData != weirdTileData);

    KisTileDataStore::instance()->debugSwapAll();
    QCOMPARE(srcTile->tileData(), weirdTileData);
    QCOMPARE(dstTile->tileData(), cowedTileData);
    QCOMPARE(srcTile->data(), weirdData);
    QCOMPARE(dstTile->data(), cowedData);

    QCOMPARE((int)weirdTileData->m_usersCount, 2);

    srcTile->unlockForWrite();
    srcTile->unlockForRead();
    srcTile = 0;

    srcDM.clear();

    KisTileDataStore::instance()->debugSwapAll();
    QCOMPARE(dstTile->tileData(), cowedTileData);
    QCOMPARE(dstTile->data(), cowedData);

    // two crash tests
    QCOMPARE(weirdTileData->data(), weirdData);
    quint8 testPixel = *weirdData;
    QCOMPARE(testPixel, defaultPixel);

    QCOMPARE((int)weirdTileData->m_usersCount, 1);

    dstTile->unlockForWrite();
    dstTile->unlockForRead();
    dstTile = 0;
}

QTEST_MAIN(KisLowMemoryTests)
