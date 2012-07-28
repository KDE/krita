/*
 *  Copyright (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
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


#include <kglobal.h>

// to disable assert when the leak tracker is active
#include "config-memory-leak-tracker.h"

#include "kis_tile_data_store.h"
#include "kis_tile_data.h"
#include "kis_debug.h"

#include "kis_tile_data_store_iterators.h"

//#define DEBUG_PRECLONE

#ifdef DEBUG_PRECLONE
#include <stdio.h>
#define DEBUG_PRECLONE_ACTION(action, oldTD, newTD) \
    printf("!!! %s:\t\t\t  0x%X -> 0x%X    \t\t!!!\n",  \
           action, (quintptr)oldTD, (quintptr) newTD)
#define DEBUG_FREE_ACTION(td)                   \
    printf("Tile data free'd \t(0x%X)\n", td)
#else
#define DEBUG_PRECLONE_ACTION(action, oldTD, newTD)
#define DEBUG_FREE_ACTION(td)
#endif

#ifdef DEBUG_HIT_MISS
qint64 __preclone_miss = 0;
qint64 __preclone_hit = 0;

qint64 __preclone_miss_user_count = 0;
qint64 __preclone_miss_age = 0;

#define DEBUG_COUNT_PRECLONE_HIT(td) __preclone_hit++
#define DEBUG_COUNT_PRECLONE_MISS(td) __preclone_miss++; __preclone_miss_user_count+=td->numUsers(); __preclone_miss_age+=td->age()
#define DEBUG_REPORT_PRECLONE_EFFICIENCY()                      \
    qDebug() << "Hits:" << __preclone_hit                       \
             << "of" << __preclone_hit + __preclone_miss        \
             << "("                                             \
             << qreal(__preclone_hit) / (__preclone_hit + __preclone_miss)       \
             << ")"                                             \
             << "miss users" << qreal(__preclone_miss_user_count) / __preclone_miss \
             << "miss age" << qreal(__preclone_miss_age) / __preclone_miss
#else
#define DEBUG_COUNT_PRECLONE_HIT(td)
#define DEBUG_COUNT_PRECLONE_MISS(td)
#define DEBUG_REPORT_PRECLONE_EFFICIENCY()
#endif

KisTileDataStore::KisTileDataStore()
    : m_pooler(this),
      m_swapper(this),
      m_numTiles(0),
      m_memoryMetric(0)
{
    m_clockIterator = m_tileDataList.end();
    m_pooler.start();
    m_swapper.start();
}

KisTileDataStore::~KisTileDataStore()
{
    m_pooler.terminatePooler();
    m_swapper.terminateSwapper();

    if(numTiles() > 0) {
        qCritical() << "CRITICAL: According to statistics of the KisTileDataStore"
                    << "some tiles have leaked from the Krita control!";
        qCritical() << "CRITICAL: Tiles in memory:" << numTilesInMemory()
                    << "Total tiles:" << numTiles();

#ifdef HAVE_MEMORY_LEAK_TRACKER
        Q_ASSERT_X(0, "KisTileDataStore::~KisTileDataStore",
                   "Let's crash to be on the safe side! ;)");
#endif /* HAVE_MEMORY_LEAK_TRACKER */

    }
}

KisTileDataStore* KisTileDataStore::instance()
{
    K_GLOBAL_STATIC(KisTileDataStore, s_instance);
    return s_instance;
}

inline void KisTileDataStore::registerTileDataImp(KisTileData *td)
{
    td->m_listIterator = m_tileDataList.insert(m_tileDataList.end(), td);
    m_numTiles++;
    m_memoryMetric += td->pixelSize();
}

void KisTileDataStore::registerTileData(KisTileData *td)
{
    QMutexLocker lock(&m_listLock);
    registerTileDataImp(td);
}

inline void KisTileDataStore::unregisterTileDataImp(KisTileData *td)
{
    KisTileDataListIterator tempIterator = td->m_listIterator;

    if(m_clockIterator == tempIterator) {
        m_clockIterator = tempIterator + 1;
    }

    td->m_listIterator = m_tileDataList.end();
    m_tileDataList.erase(tempIterator);
    m_numTiles--;
    m_memoryMetric -= td->pixelSize();
}

void KisTileDataStore::unregisterTileData(KisTileData *td)
{
    QMutexLocker lock(&m_listLock);
    unregisterTileDataImp(td);
}

KisTileData *KisTileDataStore::allocTileData(qint32 pixelSize, const quint8 *defPixel)
{
    KisTileData *td = new KisTileData(pixelSize, defPixel, this);
    registerTileData(td);
    return td;
}

KisTileData *KisTileDataStore::duplicateTileData(KisTileData *rhs)
{
    KisTileData *td = 0;

    if (rhs->m_clonesStack.pop(td)) {
        DEBUG_PRECLONE_ACTION("+ Pre-clone HIT", rhs, td);
        DEBUG_COUNT_PRECLONE_HIT(rhs);
    } else {
        rhs->blockSwapping();
        td = new KisTileData(*rhs);
        rhs->unblockSwapping();
        DEBUG_PRECLONE_ACTION("- Pre-clone #MISS#", rhs, td);
        DEBUG_COUNT_PRECLONE_MISS(rhs);
    }

    registerTileData(td);
    return td;
}

void KisTileDataStore::freeTileData(KisTileData *td)
{
    Q_ASSERT(td->m_store == this);

    DEBUG_FREE_ACTION(td);

    m_listLock.lock();
    td->m_swapLock.lockForWrite();

    if(!td->data()) {
        m_swappedStore.forgetTileData(td);
    }
    else {
        unregisterTileDataImp(td);
    }

    td->m_swapLock.unlock();
    m_listLock.unlock();

    delete td;
}

void KisTileDataStore::ensureTileDataLoaded(KisTileData *td)
{
//    qDebug() << "#### SWAP MISS! ####" << td << ppVar(td->mementoed()) << ppVar(td->age()) << ppVar(td->numUsers());
    checkFreeMemory();

    td->m_swapLock.lockForRead();

    while(!td->data()) {
        td->m_swapLock.unlock();

        /**
         * The order of this heavy locking is very important.
         * Change it only in case, you really know what you are doing.
         */
        m_listLock.lock();

        /**
         * If someone has managed to load the td from swap, then, most
         * probably, they have already taken the swap lock. This may
         * lead to a deadlock, because COW mechanism breaks lock
         * ordering rules in duplicateTileData() (it takes m_listLock
         * while the swap lock is held). In our case it is enough just
         * to check whether the other thread has already fetched the
         * data. Please notice that we do not take both of the locks
         * while checking this, because holding m_listLock is
         * enough. Nothing can happen to the tile while we hold
         * m_listLock.
         */

        if(!td->data()) {
            td->m_swapLock.lockForWrite();

            m_swappedStore.swapInTileData(td);
            registerTileDataImp(td);

            td->m_swapLock.unlock();
        }

        m_listLock.unlock();

        /**
         * <-- In theory, livelock is possible here...
         */

        td->m_swapLock.lockForRead();
    }
}

bool KisTileDataStore::trySwapTileData(KisTileData *td)
{
    /**
     * This function is called with m_listLock acquired
     */

    bool result = false;
    if(!td->m_swapLock.tryLockForWrite()) return result;

    if(td->data()) {
        unregisterTileDataImp(td);
        m_swappedStore.swapOutTileData(td);
        result = true;
    }
    td->m_swapLock.unlock();

    return result;
}

KisTileDataStoreIterator* KisTileDataStore::beginIteration()
{
    m_listLock.lock();
    return new KisTileDataStoreIterator(m_tileDataList, this);
}
void KisTileDataStore::endIteration(KisTileDataStoreIterator* iterator)
{
    delete iterator;
    m_listLock.unlock();
}

KisTileDataStoreReverseIterator* KisTileDataStore::beginReverseIteration()
{
    m_listLock.lock();
    return new KisTileDataStoreReverseIterator(m_tileDataList, this);
}
void KisTileDataStore::endIteration(KisTileDataStoreReverseIterator* iterator)
{
    delete iterator;
    m_listLock.unlock();
    DEBUG_REPORT_PRECLONE_EFFICIENCY();
}

KisTileDataStoreClockIterator* KisTileDataStore::beginClockIteration()
{
    m_listLock.lock();
    return new KisTileDataStoreClockIterator(m_clockIterator, m_tileDataList, this);
}
void KisTileDataStore::endIteration(KisTileDataStoreClockIterator* iterator)
{
    m_clockIterator = iterator->getFinalPosition();
    delete iterator;
    m_listLock.unlock();
}

void KisTileDataStore::debugPrintList()
{
    KisTileData *item;
    foreach(item, m_tileDataList) {
        dbgTiles << "-------------------------\n"
                 << "TileData:\t\t\t" << item
                 << "\n  refCount:\t" << item->m_refCount;
    }
}

void KisTileDataStore::debugSwapAll()
{
    KisTileDataStoreIterator* iter = beginIteration();
    KisTileData *item;
    while(iter->hasNext()) {
        item = iter->next();
        iter->trySwapOut(item);
    }
    endIteration(iter);

//    qDebug() << "Number of tiles:" << numTiles();
//    qDebug() << "Tiles in memory:" << numTilesInMemory();
//    m_swappedStore.debugStatistics();
}

void KisTileDataStore::debugClear()
{
    QMutexLocker lock(&m_listLock);

    foreach(KisTileData *item, m_tileDataList) {
        delete item;
    }

    m_tileDataList.clear();
    m_clockIterator = m_tileDataList.end();

    m_numTiles = 0;
    m_memoryMetric = 0;
}

void KisTileDataStore::testingSuspendPooler()
{
    m_pooler.terminatePooler();
}

void KisTileDataStore::testingResumePooler()
{
    m_pooler.start();
}
