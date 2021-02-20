/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tile_data_store_test.h"
#include <QTest>

#include "kis_debug.h"

#include "kis_image_config.h"

#include "tiles3/kis_tiled_data_manager.h"
#include "tiles_test_utils.h"

#include "tiles3/kis_tile_data_store.h"
#include "tiles3/kis_tile_data_store_iterators.h"


void KisTileDataStoreTest::testClockIterator()
{
    KisTileDataStore *store = KisTileDataStore::instance();
    store->debugClear();

    const qint32 pixelSize = 1;
    quint8 defaultPixel = 128;

    QList<KisTileData*> tileDataList;
    KisTileData *item;

    item = new KisTileData(pixelSize, &defaultPixel, store, false);
    store->registerTileData(item);
    tileDataList.append(item);
    item = new KisTileData(pixelSize, &defaultPixel, store, false);
    store->registerTileData(item);
    tileDataList.append(item);
    item = new KisTileData(pixelSize, &defaultPixel, store, false);
    store->registerTileData(item);
    tileDataList.append(item);


    /// First, full cycle!
    KisTileDataStoreClockIterator *iter = store->beginClockIteration();

    QVERIFY(iter->hasNext());
    item = iter->next();
    QCOMPARE(item, tileDataList[0]);

    QVERIFY(iter->hasNext());
    item = iter->next();
    QCOMPARE(item, tileDataList[2]);

    QVERIFY(iter->hasNext());
    item = iter->next();
    QCOMPARE(item, tileDataList[1]);

    QVERIFY(!iter->hasNext());

    store->endIteration(iter);


    /// Second, iterate until the second item!
    iter = store->beginClockIteration();

    QVERIFY(iter->hasNext());
    item = iter->next();
    QCOMPARE(item, tileDataList[0]);

    store->endIteration(iter);


    /// Third, check the position restored!
    iter = store->beginClockIteration();

    QVERIFY(iter->hasNext());
    item = iter->next();
    QCOMPARE(item, tileDataList[2]);

    QVERIFY(iter->hasNext());
    item = iter->next();
    QCOMPARE(item, tileDataList[1]);

    QVERIFY(iter->hasNext());
    item = iter->next();
    QCOMPARE(item, tileDataList[0]);

    QVERIFY(!iter->hasNext());

    store->endIteration(iter);


    /// By this moment clock index has been set
    /// onto the last item.
    /// Let's try remove it and see what will happen...

    store->freeTileData(tileDataList[0]);

    iter = store->beginClockIteration();

    QVERIFY(iter->hasNext());
    item = iter->next();
    QCOMPARE(item, tileDataList[2]);

    QVERIFY(iter->hasNext());
    item = iter->next();
    QCOMPARE(item, tileDataList[1]);

    QVERIFY(!iter->hasNext());

    store->endIteration(iter);

    store->freeTileData(tileDataList[2]);
    store->freeTileData(tileDataList[1]);
}

void KisTileDataStoreTest::testLeaks()
{
    KisTileDataStore::instance()->debugClear();

    QCOMPARE(KisTileDataStore::instance()->numTiles(), 0);

    const qint32 pixelSize = 1;
    quint8 defaultPixel = 128;
    KisTiledDataManager *dm = new KisTiledDataManager(pixelSize, &defaultPixel);

    KisTileSP tile = dm->getTile(0, 0, true);
    tile->lockForWrite();
    tile->unlockForWrite();

    tile = 0;

    delete dm;

    QCOMPARE(KisTileDataStore::instance()->numTiles(), 0);
}

#define COLUMN2COLOR(col) (col%255)

void KisTileDataStoreTest::testSwapping()
{
    KisImageConfig config(false);
    config.setMemoryHardLimitPercent(100.0 / KisImageConfig::totalRAM());
    config.setMemorySoftLimitPercent(0);

    KisTileDataStore::instance()->debugClear();



    const qint32 pixelSize = 1;
    quint8 defaultPixel = 128;
    KisTiledDataManager dm(pixelSize, &defaultPixel);

    for(qint32 col = 0; col < 1000; col++) {
        KisTileSP tile = dm.getTile(col, 0, true);
        tile->lockForWrite();

        KisTileData *td = tile->tileData();
        QVERIFY(memoryIsFilled(defaultPixel, td->data(), TILESIZE));

        memset(td->data(), COLUMN2COLOR(col), TILESIZE);
        QVERIFY(memoryIsFilled(COLUMN2COLOR(col), td->data(), TILESIZE));

        tile->unlockForWrite();
    }

    //KisTileDataStore::instance()->debugSwapAll();

    for(qint32 col = 0; col < 1000; col++) {
        KisTileSP tile = dm.getTile(col, 0, true);
        tile->lockForRead();

        KisTileData *td = tile->tileData();
        QVERIFY(memoryIsFilled(COLUMN2COLOR(col), td->data(), TILESIZE));
        tile->unlockForWrite();
    }
}

QTEST_MAIN(KisTileDataStoreTest)

