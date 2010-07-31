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

#include "kis_tile_data_store_test.h"
#include <qtest_kde.h>

#include "kis_debug.h"

//#include "kis_image_config.h"

#include "tiles3/kis_tiled_data_manager.h"
#include "tiles_test_utils.h"

#include "tiles3/kis_tile_data_store.h"
#include "tiles3/kis_tile_data_store_iterators.h"
#include "tiles3/kis_testing_tile_data_store_accessor.h"


void KisTileDataStoreTest::testClockIterator()
{
    const qint32 pixelSize = 1;
    quint8 defaultPixel = 128;

    QList<KisTileData*> tileDataList;

    tileDataList.append(KisTestingTileDataStoreAccessor::allocTileData(pixelSize, &defaultPixel));
    tileDataList.append(KisTestingTileDataStoreAccessor::allocTileData(pixelSize, &defaultPixel));
    tileDataList.append(KisTestingTileDataStoreAccessor::allocTileData(pixelSize, &defaultPixel));


    /// First, full cycle!
    KisTileDataStoreClockIterator *iter = KisTestingTileDataStoreAccessor::beginClockIteration();
    KisTileData *item;

    QVERIFY(iter->hasNext());
    item = iter->next();
    QCOMPARE(item, tileDataList[0]);

    QVERIFY(iter->hasNext());
    item = iter->next();
    QCOMPARE(item, tileDataList[1]);

    QVERIFY(iter->hasNext());
    item = iter->next();
    QCOMPARE(item, tileDataList[2]);

    QVERIFY(!iter->hasNext());

    KisTestingTileDataStoreAccessor::endIteration(iter);


    /// Second, iterate until the second item!
    iter = KisTestingTileDataStoreAccessor::beginClockIteration();

    QVERIFY(iter->hasNext());
    item = iter->next();
    QCOMPARE(item, tileDataList[0]);

    KisTestingTileDataStoreAccessor::endIteration(iter);


    /// Third, check the position restored!
    iter = KisTestingTileDataStoreAccessor::beginClockIteration();

    QVERIFY(iter->hasNext());
    item = iter->next();
    QCOMPARE(item, tileDataList[1]);

    QVERIFY(iter->hasNext());
    item = iter->next();
    QCOMPARE(item, tileDataList[2]);

    QVERIFY(iter->hasNext());
    item = iter->next();
    QCOMPARE(item, tileDataList[0]);

    QVERIFY(!iter->hasNext());

    KisTestingTileDataStoreAccessor::endIteration(iter);


    /// By this moment KisTileDataStore::m_clockIterator has been set
    /// onto the second (tileDataList[1]) item.
    /// Let's try remove it and see what will happen...

    KisTestingTileDataStoreAccessor::freeTileData(tileDataList[1]);

    iter = KisTestingTileDataStoreAccessor::beginClockIteration();

    QVERIFY(iter->hasNext());
    item = iter->next();
    QCOMPARE(item, tileDataList[2]);

    QVERIFY(iter->hasNext());
    item = iter->next();
    QCOMPARE(item, tileDataList[0]);

    QVERIFY(!iter->hasNext());

    KisTestingTileDataStoreAccessor::endIteration(iter);

}

void KisTileDataStoreTest::testLeaks()
{
    KisTestingTileDataStoreAccessor::clear();

    const qint32 pixelSize = 1;
    quint8 defaultPixel = 128;
    KisTiledDataManager *dm = new KisTiledDataManager(pixelSize, &defaultPixel);

    KisTileSP tile = dm->getTile(0, 0, true);
    tile->lockForWrite();
    tile->unlock();

    tile = 0;

    delete dm;

    QCOMPARE(KisTestingTileDataStoreAccessor::numTiles(), 0);
}

#define COLUMN2COLOR(col) (col%255)

void KisTileDataStoreTest::testSwapping()
{
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

        tile->unlock();
    }

    KisTestingTileDataStoreAccessor::swapAll();

    for(qint32 col = 0; col < 1000; col++) {
        KisTileSP tile = dm.getTile(col, 0, true);
        tile->lockForRead();

        KisTileData *td = tile->tileData();
        QVERIFY(memoryIsFilled(COLUMN2COLOR(col), td->data(), TILESIZE));
        tile->unlock();
    }
}

QTEST_KDEMAIN(KisTileDataStoreTest, NoGUI)
#include "kis_tile_data_store_test.moc"

