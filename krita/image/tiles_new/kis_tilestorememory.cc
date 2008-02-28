/*
 *  Copyright (c) 2005-2007 Bart Coppens <kde@bartcoppens.be>
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

#include "kis_tilestorememory.h"

#include <kis_debug.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <cassert>

#include <QMutex>
#include <QThread>
#include <qfile.h>

#include <k3staticdeleter.h>
#include <kglobal.h>
#include <ksharedconfig.h>

#include "kis_tileddatamanager.h"
#include "kis_tile.h"

#include "kis_tileswapper.h"

#define DO_NOT_PRINT_INFO

#define USE_SWAPPING

// Note: the cache file doesn't get deleted when we crash and so :(

// ### ### TODO When reviewing the locking policy, take care of unlocking, dyncast meminfo, lock meminfo, since the meminfo COULD be deleted?

KisTileStoreMemory::KisTileStoreMemory()
{
    m_bytesInMem = 0;
    m_bytesTotal = 0;

    // Hardcoded (at the moment only?): 4 pools of 1000 tiles each
    m_tilesPerPool = 100;

    m_pools = new quint8*[4];
    m_poolPixelSizes = new qint32[4];
    m_poolFreeList = new PoolFreeList[4];
    for (int i = 0; i < 4; i++) {
        m_pools[i] = 0;
        m_poolPixelSizes[i] = 0;
        m_poolFreeList[i] = PoolFreeList();
    }
    m_currentInMem = 0;

    //KConfigGroup cfg = KGlobal::config()->group("");
    //m_maxInMem = cfg.readEntry("maxtilesinmem",  4000);
    //m_swappiness = cfg.readEntry("swappiness", 100);
    m_maxInMem = 4000;
    m_swappiness = 100;

    m_tileSize = KisTile::WIDTH * KisTile::HEIGHT;
    /*for (int i = 0; i < 8; i++) {
        m_freeLists.push_back(FreeList());
    }*/

    counter = 0;

}

KisTileStoreMemory::~KisTileStoreMemory() {
    /*if (!m_freeLists.empty()) { // See if there are any nonempty freelists
        FreeListList::iterator listsIt = m_freeLists.begin();
        FreeListList::iterator listsEnd = m_freeLists.end();

        while(listsIt != listsEnd) {
            if ( ! (*listsIt).empty() ) {
                FreeList::iterator it = (*listsIt).begin();
                FreeList::iterator end = (*listsIt).end();

                while (it != end) {
                    delete *it;
                    ++it;
                }
                (*listsIt).clear();
            }
            ++listsIt;
        }
        m_freeLists.clear();
    }*/

    /*for (FileList::iterator it = m_files.begin(); it != m_files.end(); ++it) {
        (*it).tempFile->close();
        (*it).tempFile->setAutoRemove(true);
        delete (*it).tempFile;
    }*/

    //delete [] m_poolPixelSizes;
    //delete [] m_pools;
    // Where did this go to? delete [] m_poolFreeList;
}

KisTileStoreData* KisTileStoreMemory::registerTileData(const KisTile::SharedTileData* tileData)
{
    m_lock.lock(); // ### This is locked too long? (Should we lock at all, except for debug info?)

    SharedDataMemoryInfo* data;

    data = new SharedDataMemoryInfo();

    data->file = 0;
    data->filePos = 0;

    data->isSwappable = true; // Not yet shared, so is swappable at this time
    data->isInSwappableList = false; // Will be done afterwards!

    // Tile state:
    data->inMem = true;
    data->onFile = false;
    data->compressedOnFile = false;

    // data->node == uninited at this point !

    // The Tile should add itself to a swappablelist if needed

    //m_currentInMem++;
    //m_bytesTotal += info->size;
    //m_bytesInMem += info->size;

#ifndef DO_NOT_PRINT_INFO
    if (++counter % 50 == 0)
        printInfo();
#endif

    m_lock.unlock();

    return data;
}

void KisTileStoreMemory::deregisterTileData(const KisTile::SharedTileData* tile) {
    // Does nothing atm...
}

void KisTileStoreMemory::ensureTileLoaded(KisTile::SharedTileData* tileData)
{
    Q_ASSERT(tileData);

    QMutexLocker dataLock(&(tileData->lock));

    KisTileStoreMemory::SharedDataMemoryInfo* memInfo = dynamic_cast<KisTileStoreMemory::SharedDataMemoryInfo*>(tileData->storeData);

    KisTileSwapper* swapper = KisTileSwapper::instance();

    if (memInfo->isInSwappableList) {
        swapper->fromSwappableList(tileData);
    }

    if (memInfo->isSwappable) {
        memInfo->isSwappable = false;
    }

    if (!memInfo->inMem) {
        swapper->fromSwap(tileData);
    }
}

void KisTileStoreMemory::maySwapTile(KisTile::SharedTileData* tileData)
{
    QMutexLocker dataLock(&(tileData->lock));

    KisTileStoreMemory::SharedDataMemoryInfo* memInfo = dynamic_cast<KisTileStoreMemory::SharedDataMemoryInfo*>(tileData->storeData);

    memInfo->isSwappable = true;

#ifdef USE_SWAPPING
    KisTileSwapper* swapper = KisTileSwapper::instance();
    swapper->enqueueForSwapping(tileData);
#endif
}

quint8* KisTileStoreMemory::requestTileData(qint32 pixelSize)
{/*
    if ( pixelSize > 10 )
        return new quint8[ m_tileSize * pixelSize ];
    else {
        m_bigKritaLock.lock();
        quint8* data = findTileFor(pixelSize);
        m_bigKritaLock.unlock();
        if ( !data ) {
            data = new quint8[m_tileSize * pixelSize];
        }
        return data;
    }*/
    return new quint8[m_tileSize * pixelSize];
}

void KisTileStoreMemory::dontNeedTileData(quint8* data, qint32 pixelSize)
{
    /*m_bigKritaLock.lock();
    if (isPoolTile(data, pixelSize)) {
        reclaimTileToPool(data, pixelSize);
    } else*/
        delete[] data;
    //m_bigKritaLock.unlock();

}

void KisTileStoreMemory::configChanged() {
    m_lock.lock();
    //KConfigGroup cfg = KGlobal::config()->group("");
    //m_maxInMem = cfg.readEntry("maxtilesinmem",  4000);
    //m_swappiness = cfg.readEntry("swappiness", 100);
    m_maxInMem = 4000;
    m_swappiness = 100;

    //doSwapping();
    m_lock.unlock();
}

// ### TODO Shared Pointer! (And check that for threadsafe)
KisTileStore* defaultTileStore() {
    return new KisTileStoreMemory();
}


// =================== PRIVATE, SO NO LOCKS NEEDED ====================

/*
void KisTileStoreMemory::printInfo()
{
#ifndef DO_NOT_PRINT_INFO
    dbgTiles << m_bytesInMem <<" out of" << m_bytesTotal <<" bytes in memory";
    dbgTiles << m_currentInMem <<" out of" << m_tileMap.size() <<" tiles in memory";
    dbgTiles << m_files.size() <<" swap files in use";
    dbgTiles << m_swappableList.size() <<" elements in the swapable list";
    dbgTiles <<"Freelists information";
    for (int i = 0; i < m_freeLists.size(); i++) {
        if ( ! m_freeLists[i].empty() ) {
            dbgTiles << m_freeLists[i].size()
                    << " elements in the freelist for pixelsize " << i << "\n";
        }
    }
    dbgTiles <<"Pool stats (" <<  m_tilesPerPool <<" tiles per pool)";
    for (int i = 0; i < 4; i++) {
        if (m_pools[i]) {
            dbgTiles <<"Pool" << i <<": Freelist count:" << m_poolFreeList[i].count()
                    << ", pixelSize: " << m_poolPixelSizes[i] << endl;
        }
    }
    if (m_swapForbidden)
        dbgTiles <<"Something was wrong with the swap, see above for details";
    dbgTiles;
#endif
}
*/

#if 0
quint8* KisTileStoreMemory::findTileFor(qint32 pixelSize)
{
    for (int i = 0; i < 4; i++) {
        if (m_poolPixelSizes[i] == pixelSize) {
            if (!m_poolFreeList[i].isEmpty()) {
                quint8* data = m_poolFreeList[i].front();
                m_poolFreeList[i].pop_front();
                return data;
            }
        }
        if (m_pools[i] == 0) {
            // allocate new pool
            m_poolPixelSizes[i] = pixelSize;
            try {
                m_pools[i] = new quint8[pixelSize * m_tileSize * m_tilesPerPool];
            }
            catch ( std::bad_alloc ) {
                kDebug() <<">>>>>>> Could not allocated memory" << pixelSize <<"" << m_tileSize <<"" << m_tilesPerPool;
                // XXX: bart! What shall we do here?
                abort();
            }

            // j = 1 because we return the first element, so no need to add it to the freelist
            for (int j = 1; j < m_tilesPerPool; j++)
                m_poolFreeList[i].append(&m_pools[i][j * pixelSize * m_tileSize]);
            return m_pools[i];
        }
    }
    return 0;
}

bool KisTileStoreMemory::isPoolTile(quint8* data, qint32 pixelSize) {

    if (data == 0) {
        return false;
    }

    for (int i = 0; i < 4; i++) {
        if (m_poolPixelSizes[i] == pixelSize) {
            bool b = data >= m_pools[i]
                     && data < m_pools[i] + pixelSize * m_tileSize * m_tilesPerPool;
            if (b) {
                return true;
            }
        }
    }
    return false;
}

void KisTileStoreMemory::reclaimTileToPool(quint8* data, qint32 pixelSize) {

    for (int i = 0; i < 4; i++) {
        if (m_poolPixelSizes[i] == pixelSize)
            if (data >= m_pools[i] && data < m_pools[i] + pixelSize * m_tileSize * m_tilesPerPool) {
                m_poolFreeList[i].append(data);
            }
    }
}
#endif