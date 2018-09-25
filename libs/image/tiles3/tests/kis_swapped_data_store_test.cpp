/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_swapped_data_store_test.h"
#include <QTest>

#include "kis_debug.h"

#include "kis_image_config.h"

#include "tiles3/kis_tile_data.h"
#include "tiles_test_utils.h"

#include "tiles3/kis_tile_data_store.h"


#define COLUMN2COLOR(col) (col%255)

void KisSwappedDataStoreTest::testRoundTrip()
{
    const qint32 pixelSize = 1;
    const quint8 defaultPixel = 128;
    const qint32 NUM_TILES = 10000;

    KisImageConfig config(false);
    config.setMaxSwapSize(4);
    config.setSwapSlabSize(1);
    config.setSwapWindowSize(1);


    KisSwappedDataStore store;

    QList<KisTileData*> tileDataList;
    for(qint32 i = 0; i < NUM_TILES; i++)
        tileDataList.append(new KisTileData(pixelSize, &defaultPixel, KisTileDataStore::instance()));

    for(qint32 i = 0; i < NUM_TILES; i++) {
        KisTileData *td = tileDataList[i];
        QVERIFY(memoryIsFilled(defaultPixel, td->data(), TILESIZE));

        memset(td->data(), COLUMN2COLOR(i), TILESIZE);
        QVERIFY(memoryIsFilled(COLUMN2COLOR(i), td->data(), TILESIZE));

        // FIXME: take a lock of the tile data
        QVERIFY(store.trySwapOutTileData(td));
    }

    store.debugStatistics();

    for(qint32 i = 0; i < NUM_TILES; i++) {
        KisTileData *td = tileDataList[i];
        QVERIFY(!td->data());
        // TODO: check num clones

        // FIXME: take a lock of the tile data
        store.swapInTileData(td);
        QVERIFY(memoryIsFilled(COLUMN2COLOR(i), td->data(), TILESIZE));
    }

    store.debugStatistics();

    for(qint32 i = 0; i < NUM_TILES; i++)
        delete tileDataList[i];
}

void KisSwappedDataStoreTest::processTileData(qint32 column, KisTileData *td, KisSwappedDataStore &store)
{
    if(td->data()) {
        memset(td->data(), COLUMN2COLOR(column), TILESIZE);
        QVERIFY(memoryIsFilled(COLUMN2COLOR(column), td->data(), TILESIZE));

        // FIXME: take a lock of the tile data
        QVERIFY(store.trySwapOutTileData(td));
    }
    else {
        // TODO: check num clones
        // FIXME: take a lock of the tile data
        store.swapInTileData(td);
        QVERIFY(memoryIsFilled(COLUMN2COLOR(column), td->data(), TILESIZE));
    }
}

void KisSwappedDataStoreTest::testRandomAccess()
{
    qsrand(10);
    const qint32 pixelSize = 1;
    const quint8 defaultPixel = 128;
    const qint32 NUM_CYCLES = 50000;
    const qint32 NUM_TILES = 10000;

    KisImageConfig config(false);
    config.setMaxSwapSize(40);
    config.setSwapSlabSize(1);
    config.setSwapWindowSize(1);


    KisSwappedDataStore store;

    QList<KisTileData*> tileDataList;
    for(qint32 i = 0; i < NUM_TILES; i++)
        tileDataList.append(new KisTileData(pixelSize, &defaultPixel, KisTileDataStore::instance()));

    for(qint32 i = 0; i < NUM_CYCLES; i++) {
        if(!(i%5000))
            dbgKrita << i << "of" << NUM_CYCLES;

        qint32 col = qrand() % NUM_TILES;

        KisTileData *td = tileDataList[col];
        processTileData(col, td, store);
    }

    store.debugStatistics();

    for(qint32 i = 0; i < NUM_TILES; i++)
        delete tileDataList[i];
}

QTEST_MAIN(KisSwappedDataStoreTest)

