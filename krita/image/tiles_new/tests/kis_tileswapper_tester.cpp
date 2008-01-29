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

#include <qtest_kde.h>

#include <unistd.h> // sleep(3), usleep(3)

#include "kis_tileswapper_tester.h"
#include "kis_global.h"

// ### HACK so that we can easily test the internals
#define private public
#define protected public

#include "tiles_new/kis_tilestorememory.h"
#include "tiles_new/kis_tileswapper.h"

#undef private
#undef protected

static quint8 defPixel = 145;

// This file does NOT yet check multithreading issues. In fact, most (all?) of the threads run the swap code not in a swap-thread!

// A generic swap check: swap in & out, verify
void KisTileSwapperTester::checkAgeTest()
{
    KisTileStoreMemory memoryStore;
    KisTile tile(&memoryStore, sizeof(defPixel), 0, 0, &defPixel);
    KisTileSwapper* swapper = KisTileSwapper::instance();
    KisTileStoreMemory::SharedDataMemoryInfo* memInfo
            = dynamic_cast<KisTileStoreMemory::SharedDataMemoryInfo*>(tile.m_tileData->storeData);

    tile.addReader();
    tile.m_tileData->data[0] = 125;
    tile.m_tileData->data[1] = 0;
    tile.m_tileData->data[64*64*1 - 2] = 0;
    tile.m_tileData->data[64*64*1 - 1] = 45;
    tile.removeReader();

    // Artificially age the tile, and swap (if this wouldn't happen, it's checked by checkAgeTest)
    tile.m_tileData->lastUse = QTime();
    //KisTileSwapper::instance()->swapTilesInStore(&memoryStore);
    swapper->swapTileData(tile.m_tileData);

    QVERIFY(memInfo->inMem == false);
    QVERIFY(memInfo->onFile == true);
    QVERIFY(tile.m_tileData->data == 0);

    // Unswap
    KisTileSwapper::instance()->fromSwap(tile.m_tileData);

    QVERIFY(memInfo->inMem == true);
    QVERIFY(memInfo->onFile == true);

    tile.addReader();
    QVERIFY(tile.m_tileData->data != 0);
    QVERIFY(tile.m_tileData->data[0] == 125);
    QVERIFY(tile.m_tileData->data[1] == 0);
    QVERIFY(tile.m_tileData->data[64*64*1 - 2] == 0);
    QVERIFY(tile.m_tileData->data[64*64*1 - 1] == 45);
    tile.removeReader();
}


// Does the memorystore swapping check for the tile last use time?
void KisTileSwapperTester::tileSwapTest()
{
    KisTileStoreMemory memoryStore;
    KisTile tile(&memoryStore, sizeof(defPixel), 0, 0, &defPixel);
    KisTileSwapper* swapper = KisTileSwapper::instance();
    KisTileStoreMemory::SharedDataMemoryInfo* memInfo =
            dynamic_cast<KisTileStoreMemory::SharedDataMemoryInfo*>(tile.m_tileData->storeData);


    // By default, tile's last use time is now(). So now it should not yet swap ... (unless sickly bad luck, assume not :-)
    // Not longer true...
    //swapper->swapTileData(tile.m_tileData);
    //QVERIFY(memInfo->inMem == true);
    //QVERIFY(memInfo->onFile == false);

    // Now, artificially age the tile, and try to swap again
    tile.m_tileData->lastUse = QTime();
    swapper->swapTileData(tile.m_tileData);
    QVERIFY(memInfo->inMem == false);
    QVERIFY(memInfo->onFile == true);
}

void KisTileSwapperTester::interactionWithTileReadersTest() {
    KisTileStoreMemory memoryStore;
    KisTile tile(&memoryStore, sizeof(defPixel), 0, 0, &defPixel);
    KisTileSwapper* swapper = KisTileSwapper::instance();
    KisTileStoreMemory::SharedDataMemoryInfo* memInfo =
            dynamic_cast<KisTileStoreMemory::SharedDataMemoryInfo*>(tile.m_tileData->storeData);

    // Artificially age the tile, and swap (if this wouldn't happen, it's checked by checkAgeTest)
    tile.m_tileData->lastUse = QTime();
    swapper->swapTileData(tile.m_tileData);

    QVERIFY(memInfo->inMem == false);
    QVERIFY(memInfo->onFile == true);

    tile.addReader();
    // Does it swap in again?
    QVERIFY(memInfo->inMem == true);
    tile.removeReader();
}

void KisTileSwapperTester::idleSwappingTest() {
    KisTileStoreMemory memoryStore;
    KisTile tile(&memoryStore, sizeof(defPixel), 0, 0, &defPixel);
    KisTileSwapper* swapper = KisTileSwapper::instance();
    KisTileStoreMemory::SharedDataMemoryInfo* memInfo =
            dynamic_cast<KisTileStoreMemory::SharedDataMemoryInfo*>(tile.m_tileData->storeData);

    // Prevent the swapper from swapping, you never know... (### Do this elsewhere too?)
    tile.m_tileData->lock.lock();

    QVERIFY(memInfo->inMem == true);
    QVERIFY(memInfo->onFile == false);
    QVERIFY(memInfo->isInSwappableList == true);

    // Artificially age the tile
    tile.m_tileData->lastUse = QTime();
    tile.m_tileData->lock.unlock();

    // The constructor of the store should have added the store to the to-watch list of stores automatically (### QVERIFY this)
    //swapper->slotTrySwapping(); // ### Note that we don't actually do the test of the timer mechanism!!!
    //usleep(1200); // ### Agressive swapping for now... (!### But the old 'dirty age' is still 500 ms, so shorter wait time won't work) ### NOT TRUE TOO SLOW
    ::sleep(1);

    QVERIFY(memInfo->inMem == false);
    QVERIFY(memInfo->onFile == true);

    tile.addReader();
    // Does it swap in again?
    QVERIFY(memInfo->inMem == true);
    tile.removeReader();

    // Now, for the big trick: very small test of the idle timer mechanism (does not test the mechanism at the fullest)
    // the removeReader should have automatically updated the lastUse, and called notify().
    // So just sleep for > idleThreshold, and see what happened!
    //QThread::msleep(2*swapper->idleThreshold());
    //::sleep((10*swapper->idleThreshold())/1000);

    //usleep(1200); // ### Agressive swapping for now... ### NOT TRUE TOO SLOW
    ::sleep(1);

    QVERIFY(memInfo->inMem == false);
}

QTEST_KDEMAIN(KisTileSwapperTester, NoGUI)

#include "kis_tileswapper_tester.moc"
