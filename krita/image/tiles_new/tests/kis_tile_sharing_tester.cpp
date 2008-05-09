/*
 *  Copyright (c) 2007 Bart Coppens <kde@bartcoppens.be>
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

#include "kis_tile_sharing_tester.h"
#include <qtest_kde.h>

#include <sys/mman.h>
#include <string.h>
#include <map>

#include "kis_global.h"

// ### HACK so that we can easily test the internals
#define private public
#define protected public

#include "tiles_new/kis_tilestorememory.h"
#include "tiles_new/kis_tileswapper.h"
#include "tiles_new/kis_sharedtiledata.h"

#include "kis_datamanager.h" // ### Check that, indeed, this is a TILED dm?
#include "kis_iterator.h"
#include "kis_tilediterator.h"

#undef private
#undef protected

static quint8 defPixel = 145;

// This file does NOT yet check multithreading issues. In fact, most (all?) of the threads run the swap code not in a swap-thread!

// A generic swap check: swap in & out, verify
void KisTileSharingTester::tileSharingTest()
{
    KisSharedPtr<KisTileStoreMemory> memoryStore = new KisTileStoreMemory;
    KisTile tile1(memoryStore, sizeof(defPixel), 0, 0, &defPixel);

    tile1.addReader();
    tile1.m_tileData->data[0] = 125;
    tile1.m_tileData->data[64*64*1 - 1] = 45;
    tile1.removeReader();

    KisTile tile2(tile1); // Implicitly shared
    KisTile tile3(tile2); // Implicitly shared

    // Check they actually share the same data:
    QVERIFY(tile1.m_tileData->data == tile2.m_tileData->data);
    QVERIFY(tile1.m_tileData->data == tile3.m_tileData->data);

    // Detatch tile 2
    tile2.detachShared();

    QVERIFY(tile1.m_tileData->data == tile3.m_tileData->data);
    QVERIFY(tile1.m_tileData->data != tile2.m_tileData->data);

    // Check if the data is still OK:
    tile1.addReader();
    QVERIFY(tile1.m_tileData->data != 0);
    QVERIFY(tile1.m_tileData->data[0] == 125);
    QVERIFY(tile1.m_tileData->data[64*64*1 - 1] == 45);
    tile1.removeReader();

    tile2.addReader();
    QVERIFY(tile2.m_tileData->data != 0);
    QVERIFY(tile2.m_tileData->data[0] == 125);
    QVERIFY(tile2.m_tileData->data[64*64*1 - 1] == 45);
    tile2.removeReader();

    tile3.addReader();
    QVERIFY(tile3.m_tileData->data != 0);
    QVERIFY(tile3.m_tileData->data[0] == 125);
    QVERIFY(tile3.m_tileData->data[64*64*1 - 1] == 45);
    tile3.removeReader();

    // Maybe check swapping too? ###
}

// Check how well the implicit tilesharing works with swapping
void KisTileSharingTester::swappingSharedTilesTest()
{
    KisSharedPtr<KisTileStoreMemory> memoryStore = new KisTileStoreMemory;
    KisTile tile1(memoryStore, sizeof(defPixel), 0, 0, &defPixel);
    KisTileSwapper* swapper = KisTileSwapper::instance();
    KisTileStoreMemory::SharedDataMemoryInfo* memInfo1 = dynamic_cast<KisTileStoreMemory::SharedDataMemoryInfo*>(tile1.m_tileData->storeData);

    KisTile tile2(tile1); // Implicitly shared
    KisTileStoreMemory::SharedDataMemoryInfo* memInfo2 = dynamic_cast<KisTileStoreMemory::SharedDataMemoryInfo*>(tile2.m_tileData->storeData);

    Q_ASSERT(tile1.m_tileData->data);
    // Now, artificially age the first tile, and try to swap
    tile1.m_tileData->lastUse = QTime();
    {
        QMutexLocker dataLock(&(tile1.m_tileData->lock)); // ### deadlocky
        if (tile1.m_tileData->data) {
            // Not yet swapped by the swap-thread ...
            swapper->fromSwappableList(tile1.m_tileData);
            swapper->swapTileData(tile1.m_tileData);
        }
    }

    QVERIFY(memInfo1->inMem == false);
    QVERIFY(memInfo1->onFile == true);

    // Tile2 should also have been swapped
    QVERIFY(memInfo2->inMem == false);
    QVERIFY(memInfo2->onFile == true);

    QVERIFY(tile1.m_tileData->data == 0);
    QVERIFY(tile2.m_tileData->data == 0);

    // Swap in tile 2, should swap in tile 1:
    swapper->fromSwap(tile2.m_tileData);

    QVERIFY(memInfo1->inMem == true);
    QVERIFY(memInfo1->onFile == true);
    QVERIFY(memInfo2->inMem == true);
    QVERIFY(memInfo2->onFile == true);

    QVERIFY(tile1.m_tileData->data == tile2.m_tileData->data);

    // Swap tile2 again
    tile2.m_tileData->lastUse = QTime();
    memoryStore->maySwapTile(tile2.m_tileData);

    swapper->swapTileData(tile1.m_tileData);
    //swapper->swapTileData(tile2.m_tileData);

    QVERIFY(memInfo2->inMem == false);
    QVERIFY(memInfo1->inMem == false);


    // Detatch tile 2 while swapped
    tile2.detachShared();
    memInfo2 = dynamic_cast<KisTileStoreMemory::SharedDataMemoryInfo*>(tile2.m_tileData->storeData); // memInfo is invalidated

    QVERIFY(memInfo2 != memInfo1);


    // QVERIFY(memInfo1->inMem == false); ### This came out of swap, because we needed to copy the tile data!
    QVERIFY(memInfo1->onFile == true);
    QVERIFY(memInfo2->inMem == true);
    QVERIFY(memInfo2->onFile == false);

    // QVERIFY(tile1.m_tileData->data == 0); ### Same as above
    QVERIFY(tile2.m_tileData->data != 0);

    // Unswap tile 1
    swapper->fromSwap(tile1.m_tileData);

    QVERIFY(memInfo1->inMem == true);
    QVERIFY(memInfo1->onFile == true);
    QVERIFY(memInfo2->inMem == true);
    QVERIFY(memInfo2->onFile == false);

    QVERIFY(tile1.m_tileData->data != tile2.m_tileData->data);
}

// Check that detaching unshared tiles works
void KisTileSharingTester::detachUnshared()
{
    KisSharedPtr<KisTileStoreMemory> memoryStore = new KisTileStoreMemory;
    KisTile tile(memoryStore, sizeof(defPixel), 0, 0, &defPixel);

    quint8* oldData = tile.data();
    tile.detachShared();

    QVERIFY(oldData == tile.data());
}

// ### TODO: Check KisMemento interaction
// ### TODO: Check default tile sharing! (this is being done, but how good is it?)
// Check if implicitly sharing the datamanager works (using iterators)
void KisTileSharingTester::dataManagerTileSharingTest() {
    KisDataManager dm1(sizeof(defPixel), &defPixel);
    // First, initialize this device with a pixel. Otherwise, we just look at the shared default tile!
    {
        KisHLineConstIterator dm2Iter(&dm1, 0, 0, 128, true);
    }

    KisDataManager dm2(dm1);

    {
        KisHLineConstIterator dm1ConstIter(&dm1, 0, 0, 128, false /* writable */);
        KisHLineConstIterator dm2ConstIter(&dm2, 0, 0, 128, false /* writable */);

        QVERIFY(dm1ConstIter.rawData() == dm2ConstIter.rawData());
        QVERIFY(dm1ConstIter.m_iter->m_tile->m_tileData->references == 2);
        QVERIFY(dm1ConstIter.m_iter->m_tile != dm2ConstIter.m_iter->m_tile);
        QVERIFY(dm1ConstIter.m_iter->m_tile->m_tileData == dm2ConstIter.m_iter->m_tile->m_tileData);
    }

    {
        // TODO: Ask Cyrille why you can have writable const iterators...
        KisHLineConstIterator dm1ConstIter(&dm1, 0, 0, 128, false /* writable */);
        KisHLineConstIterator dm2Iter(&dm2, 0, 0, 128, true /* writable */);

        QVERIFY(dm1ConstIter.rawData() != dm2Iter.rawData());

        QVERIFY(dm1ConstIter.m_iter->m_tile->m_tileData != dm2Iter.m_iter->m_tile->m_tileData);

        //QVERIFY(dm1ConstIter.m_iter->m_tile->m_tileData->timesLockedInMemory == 1); ### Check should include swapper
        QVERIFY(dm1ConstIter.m_iter->m_tile->m_tileData->references == 1);

        // People should not use this 'feature'
        //KisHLineConstIterator dm1Iter(&dm1, 0, 0, 128, true);
        //QVERIFY(dm1ConstIter.rawData() == dm1Iter.rawData()); <- Should only work in case of an unshared tile. But we Q_ASSERT, so people will notice it when coding!
    }
}

void KisTileSharingTester::shareTilesAcrossDatamanagersTest() {
    
}

// Test if the degrading of tiledata works: in this case, we have a store that mprotects the data, so we MUST degrade to write to it!
namespace {
    struct StrictReadOnlyTileStore : public KisTileStore {
        virtual ~StrictReadOnlyTileStore() {}

        virtual void requestTileData(KisSharedTileData* tileData) {
            // TODO: share this piece of code with the tileswapper?
            long pageSize;
#ifdef Q_WS_WIN
            SYSTEM_INFO systemInfo;
            GetSystemInfo(&systemInfo);
            pageSize = systemInfo.dwPageSize;
#else
            pageSize = sysconf(_SC_PAGESIZE);
#endif

            size_t len = tileData->pixelSize * KisTile::WIDTH * KisTile::HEIGHT;
            tileData->data = 0;
            int res = posix_memalign(reinterpret_cast<void**>(&tileData->data), sysconf(_SC_PAGESIZE), len);
            assert(res == 0);
        }

        void protect(KisTile& tile) {
            // Note that we don't lock, since this class will not interact with the swapper, and as a unit test, it will
            // not be used in a multithreaded environment...
            mprotect(tile.m_tileData->data, tile.m_tileData->tileSize, PROT_READ);
        }

        virtual void dontNeedTileData(KisSharedTileData* tileData) {
            free(tileData->data);
        }

        virtual KisSharedTileData* degradedTileDataForSharing(KisSharedTileData* tileData) {
            KisSharedTileData* data = new KisSharedTileData(new KisTileStoreMemory, tileData->tileSize, tileData->pixelSize);

            tileData->addLockInMemory();
            data->addLockInMemory();

            memcpy(data->data, tileData->data, tileData->tileSize);

            data->removeLockInMemory();
            tileData->removeLockInMemory();

            return data;
        }
    };
}

void KisTileSharingTester::degradeDataTest() {
    KisSharedPtr<StrictReadOnlyTileStore> store = new StrictReadOnlyTileStore;
    KisTile tile(store, sizeof(defPixel), 0, 0, &defPixel);
    store->protect(tile);
    KisTile tile2(tile);
    tile2.detachShared();
}

QTEST_KDEMAIN(KisTileSharingTester, NoGUI)

#include "kis_tile_sharing_tester.moc"
