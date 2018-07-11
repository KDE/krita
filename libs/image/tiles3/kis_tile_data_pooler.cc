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


#include <stdio.h>
#include "kis_tile_data.h"
#include "kis_tile_data_store.h"
#include "kis_tile_data_store_iterators.h"
#include "kis_debug.h"
#include "kis_tile_data_pooler.h"
#include "kis_image_config.h"


const qint32 KisTileDataPooler::MAX_NUM_CLONES = 16;
const qint32 KisTileDataPooler::MAX_TIMEOUT = 60000; // 01m00s
const qint32 KisTileDataPooler::MIN_TIMEOUT = 100; // 00m00.100s
const qint32 KisTileDataPooler::TIMEOUT_FACTOR = 2;

//#define DEBUG_POOLER

#ifdef DEBUG_POOLER
#define DEBUG_CLONE_ACTION(td, numClones)                               \
    printf("Cloned (%d):\t\t\t\t0x%X (clones: %d, users: %d, refs: %d)\n", \
           numClones, td, td->m_clonesStack.size(),                      \
           (int)td->m_usersCount, (int)td->m_refCount)
#define DEBUG_SIMPLE_ACTION(action)     \
    printf("pooler: %s\n", action)

#define RUNTIME_SANITY_CHECK(td) do {                                   \
        if(td->m_usersCount < td->m_refCount) {                         \
            qInfo("**** Suspicious tiledata: 0x%X (clones: %d, users: %d, refs: %d) ****", \
                   td, td->m_clonesStack.size(),                         \
                   (int)td->m_usersCount, (int)td->m_refCount);         \
        }                                                               \
        if(td->m_usersCount <= 0) {                                     \
            qFatal("pooler: Tiledata 0x%X has zero users counter. Crashing...", td); \
        }                                                               \
    } while(0)                                                          \

#define DEBUG_TILE_STATISTICS() debugTileStatistics()

#define DEBUG_LISTS(mem, beggers, beggersMem, donors, donorsMem)        \
    do {                                                                \
    dbgKrita << "--- getLists finished ---";                            \
    dbgKrita << "  memoryOccupied:" << mem << "/" << m_memoryLimit;     \
    dbgKrita << "  donors:" << donors.size()                            \
             << "(mem:" << donorsMem << ")";                            \
    dbgKrita << "  beggers:" << beggers.size()                          \
             << "(mem:" << beggersMem << ")";                           \
    dbgKrita << "--- ----------------- ---";                            \
    } while(0)

#define DEBUG_ALLOC_CLONE(mem, totalMem)                                \
        dbgKrita << "Alloc mem for clones:" << mem                      \
                 << "\tMem usage:" << totalMem << "/" << m_memoryLimit

#define DEBUG_FREE_CLONE(freed, demanded)                               \
            dbgKrita << "Freed mem for clones:" << freed                \
                     << "/" << qAbs(demanded)

#else
#define DEBUG_CLONE_ACTION(td, numClones)
#define DEBUG_SIMPLE_ACTION(action)
#define RUNTIME_SANITY_CHECK(td)
#define DEBUG_TILE_STATISTICS()
#define DEBUG_LISTS(mem, beggers, beggersMem, donors, donorsMem)
#define DEBUG_ALLOC_CLONE(mem, totalMem)
#define DEBUG_FREE_CLONE(freed, demanded)
#endif


KisTileDataPooler::KisTileDataPooler(KisTileDataStore *store, qint32 memoryLimit)
    : QThread()
{
    m_shouldExitFlag = 0;
    m_store = store;
    m_timeout = MIN_TIMEOUT;
    m_lastCycleHadWork = false;
    m_lastPoolMemoryMetric = 0;
    m_lastRealMemoryMetric = 0;
    m_lastHistoricalMemoryMetric = 0;

    if(memoryLimit >= 0) {
        m_memoryLimit = memoryLimit;
    }
    else {
        m_memoryLimit = MiB_TO_METRIC(KisImageConfig(true).poolLimit());
    }
}

KisTileDataPooler::~KisTileDataPooler()
{
}

void KisTileDataPooler::kick()
{
    m_semaphore.release();
}

void KisTileDataPooler::terminatePooler()
{
    unsigned long exitTimeout = 100;
    do {
        m_shouldExitFlag = true;
        kick();
    } while(!wait(exitTimeout));
}

qint32 KisTileDataPooler::numClonesNeeded(KisTileData *td) const
{
    RUNTIME_SANITY_CHECK(td);
    qint32 numUsers = td->m_usersCount;
    qint32 numPresentClones = td->m_clonesStack.size();
    qint32 totalClones = qMin(numUsers - 1, MAX_NUM_CLONES);

    return totalClones - numPresentClones;
}

void KisTileDataPooler::cloneTileData(KisTileData *td, qint32 numClones) const
{
    if (numClones > 0) {
        td->blockSwapping();
        for (qint32 i = 0; i < numClones; i++) {
            td->m_clonesStack.push(new KisTileData(*td, false));
        }
        td->unblockSwapping();
    } else {
        qint32 numUnnededClones = qAbs(numClones);
        for (qint32 i = 0; i < numUnnededClones; i++) {
            KisTileData *clone = 0;

            bool result = td->m_clonesStack.pop(clone);
            if(!result) break;

            delete clone;
        }
    }

    DEBUG_CLONE_ACTION(td, numClones);
}

void KisTileDataPooler::waitForWork()
{
    bool success;

    if (m_lastCycleHadWork)
        success = m_semaphore.tryAcquire(1, m_timeout);
    else {
        m_semaphore.acquire();
        success = true;
    }

    m_lastCycleHadWork = false;
    if (success) {
        m_timeout = MIN_TIMEOUT;
    } else {
        m_timeout *= TIMEOUT_FACTOR;
        m_timeout = qMin(m_timeout, MAX_TIMEOUT);
    }
}

void KisTileDataPooler::run()
{
    if(!m_memoryLimit) return;

    m_shouldExitFlag = false;

    while (1) {
        DEBUG_SIMPLE_ACTION("went to bed... Zzz...");

        waitForWork();

        if (m_shouldExitFlag)
            break;

        QThread::msleep(0);
        DEBUG_SIMPLE_ACTION("cycle started");


        KisTileDataStoreReverseIterator *iter = m_store->beginReverseIteration();
        QList<KisTileData*> beggers;
        QList<KisTileData*> donors;
        qint32 memoryOccupied;

        qint32 statRealMemory;
        qint32 statHistoricalMemory;


        getLists(iter, beggers, donors,
                 memoryOccupied,
                 statRealMemory,
                 statHistoricalMemory);

        m_lastCycleHadWork =
            processLists(beggers, donors, memoryOccupied);

        m_lastPoolMemoryMetric = memoryOccupied;
        m_lastRealMemoryMetric = statRealMemory;
        m_lastHistoricalMemoryMetric = statHistoricalMemory;

        m_store->endIteration(iter);

        DEBUG_TILE_STATISTICS();
        DEBUG_SIMPLE_ACTION("cycle finished");
    }
}

void KisTileDataPooler::forceUpdateMemoryStats()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(!isRunning());

    KisTileDataStoreReverseIterator *iter = m_store->beginReverseIteration();
    QList<KisTileData*> beggers;
    QList<KisTileData*> donors;
    qint32 memoryOccupied;

    qint32 statRealMemory;
    qint32 statHistoricalMemory;


    getLists(iter, beggers, donors,
             memoryOccupied,
             statRealMemory,
             statHistoricalMemory);

    m_lastPoolMemoryMetric = memoryOccupied;
    m_lastRealMemoryMetric = statRealMemory;
    m_lastHistoricalMemoryMetric = statHistoricalMemory;

    m_store->endIteration(iter);
}

qint64 KisTileDataPooler::lastPoolMemoryMetric() const
{
    return m_lastPoolMemoryMetric;
}

qint64 KisTileDataPooler::lastRealMemoryMetric() const
{
    return m_lastRealMemoryMetric;
}

qint64 KisTileDataPooler::lastHistoricalMemoryMetric() const
{
    return m_lastHistoricalMemoryMetric;
}

inline int KisTileDataPooler::clonesMetric(KisTileData *td, int numClones) {
    return numClones * td->pixelSize();
}

inline int KisTileDataPooler::clonesMetric(KisTileData *td) {
    return td->m_clonesStack.size() * td->pixelSize();
}

inline void KisTileDataPooler::tryFreeOrphanedClones(KisTileData *td)
{
    qint32 extraClones = -numClonesNeeded(td);

    if(extraClones > 0) {
        cloneTileData(td, -extraClones);
    }
}

inline qint32 KisTileDataPooler::needMemory(KisTileData *td)
{
    qint32 clonesNeeded = !td->age() ? qMax(0, numClonesNeeded(td)) : 0;
    return clonesMetric(td, clonesNeeded);
}

inline qint32 KisTileDataPooler::canDonorMemory(KisTileData *td)
{
    return td->age() && clonesMetric(td);
}

template<class Iter>
void KisTileDataPooler::getLists(Iter *iter,
                                 QList<KisTileData*> &beggers,
                                 QList<KisTileData*> &donors,
                                 qint32 &memoryOccupied,
                                 qint32 &statRealMemory,
                                 qint32 &statHistoricalMemory)
{
    memoryOccupied = 0;
    statRealMemory = 0;
    statHistoricalMemory = 0;

    qint32 needMemoryTotal = 0;
    qint32 canDonorMemoryTotal = 0;

    qint32 neededMemory;
    qint32 donoredMemory;

    KisTileData *item;

    while(iter->hasNext()) {
        item = iter->next();

        tryFreeOrphanedClones(item);

        if((neededMemory = needMemory(item))) {
            needMemoryTotal += neededMemory;
            beggers.append(item);
        }
        else if((donoredMemory = canDonorMemory(item))) {
            canDonorMemoryTotal += donoredMemory;
            donors.append(item);
        }

        memoryOccupied += clonesMetric(item);

        // statistics gathering
        if (item->historical()) {
            statHistoricalMemory += item->pixelSize();
        } else {
            statRealMemory += item->pixelSize();
        }
    }

    DEBUG_LISTS(memoryOccupied,
                beggers, needMemoryTotal,
                donors, canDonorMemoryTotal);
}

qint32 KisTileDataPooler::tryGetMemory(QList<KisTileData*> &donors,
                                       qint32 memoryMetric)
{
    qint32 memoryFreed = 0;

    QMutableListIterator<KisTileData*> iter(donors);
    iter.toBack();

    while(iter.hasPrevious() && memoryFreed < memoryMetric) {
        KisTileData *item = iter.previous();

        qint32 numClones = item->m_clonesStack.size();
        cloneTileData(item, -numClones);
        memoryFreed += clonesMetric(item, numClones);

        iter.remove();
    }

    return memoryFreed;
}

bool KisTileDataPooler::processLists(QList<KisTileData*> &beggers,
                                     QList<KisTileData*> &donors,
                                     qint32 &memoryOccupied)
{
    bool hadWork = false;


    Q_FOREACH (KisTileData *item, beggers) {
        qint32 clonesNeeded = numClonesNeeded(item);
        qint32 clonesMemory = clonesMetric(item, clonesNeeded);

        qint32 memoryLeft =
            m_memoryLimit - (memoryOccupied + clonesMemory);

        if(memoryLeft < 0) {
            qint32 freedMemory = tryGetMemory(donors, -memoryLeft);
            memoryOccupied -= freedMemory;

            DEBUG_FREE_CLONE(freedMemory, memoryLeft);

            if(m_memoryLimit < memoryOccupied + clonesMemory)
                break;
        }

        cloneTileData(item, clonesNeeded);
        DEBUG_ALLOC_CLONE(clonesMemory, memoryOccupied);

        memoryOccupied += clonesMemory;
        hadWork = true;
    }

    return hadWork;
}

void KisTileDataPooler::debugTileStatistics()
{
    /**
     * Assume we are called from the inside of the loop.
     * This means m_store is already locked
     */

    qint64 preallocatedTiles=0;

    KisTileDataStoreIterator *iter = m_store->beginIteration();
    KisTileData *item;

    while(iter->hasNext()) {
        item = iter->next();
        preallocatedTiles += item->m_clonesStack.size();
    }

    m_store->endIteration(iter);

    dbgKrita << "Tiles statistics:\t total:" << m_store->numTiles() << "\t preallocated:"<< preallocatedTiles;
}

void KisTileDataPooler::testingRereadConfig()
{
    m_memoryLimit = MiB_TO_METRIC(KisImageConfig(true).poolLimit());
}
