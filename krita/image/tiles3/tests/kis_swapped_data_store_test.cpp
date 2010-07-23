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
#include <qtest_kde.h>

#include "kis_debug.h"

#include "kis_image_config.h"

#include "tiles3/kis_tiled_data_manager.h"
#include "tiles_test_utils.h"

#include "tiles3/swap/kis_swapped_data_store.h"

#define COLUMN2COLOR(col) (col%255)

void KisSwappedDataStoreTest::testRoundTrip()
{
    const qint32 pixelSize = 1;
    quint8 defaultPixel = 128;

    /**
     * A small hack to acquire a standalone tile data.
     * globalTileDataStore is not exported out of kritaimage.so,
     * so we get it from the data manager
     */
    KisTiledDataManager dm(pixelSize, &defaultPixel);

    KisImageConfig config;
    config.setMaxSwapSize(4);
    config.setSwapSlabSize(1);
    config.setSwapWindowSize(1);

    KisSwappedDataStore store;


    for(qint32 col = 0; col < 10000; col++) {
        KisTileSP tile = dm.getTile(col, 0, true);
        tile->lockForWrite();
        tile->unlock();

        KisTileData *td = tile->tileData();
        QVERIFY(memoryIsFilled(defaultPixel, td->data(), TILESIZE));

        memset(td->data(), COLUMN2COLOR(col), TILESIZE);
        QVERIFY(memoryIsFilled(COLUMN2COLOR(col), td->data(), TILESIZE));

        // FIXME: take a lock of the tile data
        store.swapOutTileData(td);
    }

    store.debugStatistics();

    for(qint32 col = 0; col < 10000; col++) {
        KisTileSP tile = dm.getTile(col, 0, true);

        KisTileData *td = tile->tileData();
        QVERIFY(!td->data());
        // TODO: check num clones

        // FIXME: take a lock of the tile data
        store.swapInTileData(td);
        QVERIFY(memoryIsFilled(COLUMN2COLOR(col), td->data(), TILESIZE));
    }

    store.debugStatistics();
}

void KisSwappedDataStoreTest::processTileData(qint32 column, KisTileData *td, KisSwappedDataStore &store)
{
    if(td->data()) {
        memset(td->data(), COLUMN2COLOR(column), TILESIZE);
        QVERIFY(memoryIsFilled(COLUMN2COLOR(column), td->data(), TILESIZE));

        // FIXME: take a lock of the tile data
        store.swapOutTileData(td);
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
    const qint32 pixelSize = 1;
    quint8 defaultPixel = 128;
    KisTiledDataManager dm(pixelSize, &defaultPixel);

    KisImageConfig config;
    config.setMaxSwapSize(40);
    config.setSwapSlabSize(1);
    config.setSwapWindowSize(1);

    KisSwappedDataStore store;

    const qint32 NUM_CYCLES = 50000;
    const qint32 NUM_TILES = 10000;

    qsrand(10);

    for(qint32 i = 0; i < NUM_CYCLES; i++) {
        if(!(i%5000))
            qDebug() << i << "of" << NUM_CYCLES;

        qint32 col = qrand() % NUM_TILES;
        KisTileSP tile = dm.getTile(col, 0, true);
        tile->lockForWrite();
        tile->unlock();

        KisTileData *td = tile->tileData();
        processTileData(col, td, store);
    }

    store.debugStatistics();
}

QTEST_KDEMAIN(KisSwappedDataStoreTest, NoGUI)
#include "kis_swapped_data_store_test.moc"

