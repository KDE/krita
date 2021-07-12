/*
 *  SPDX-FileCopyrightText: 2009 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2018 Andrey Kamakin <a.kamakin@icloud.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TILE_DATA_STORE_H_
#define KIS_TILE_DATA_STORE_H_

#include "kritaimage_export.h"

#include <QReadWriteLock>
#include "kis_tile_data_interface.h"

#include "kis_tile_data_pooler.h"
#include "swap/kis_tile_data_swapper.h"
#include "swap/kis_swapped_data_store.h"
#include "3rdparty/lock_free_map/concurrent_map.h"

class KisTileDataStoreIterator;
class KisTileDataStoreReverseIterator;
class KisTileDataStoreClockIterator;

/**
 * Stores tileData objects. When needed compresses them and swaps.
 */
class KRITAIMAGE_EXPORT KisTileDataStore
{
public:
    KisTileDataStore();
    ~KisTileDataStore();
    static KisTileDataStore* instance();

    void debugPrintList();

    struct MemoryStatistics {
        qint64 totalMemorySize;
        qint64 realMemorySize;
        qint64 historicalMemorySize;

        qint64 poolSize;

        qint64 swapSize;
    };

    MemoryStatistics memoryStatistics();
    void tryForceUpdateMemoryStatisticsWhileIdle();

    /**
     * Returns total number of tiles present: in memory
     * or in a swap file
     */
    inline qint32 numTiles() const
    {
        return m_numTiles.loadAcquire() + m_swappedStore.numTiles();
    }

    /**
     * Returns the number of tiles present in memory only
     */
    inline qint32 numTilesInMemory() const
    {
        return m_numTiles.loadAcquire();
    }

    inline void checkFreeMemory()
    {
        m_swapper.checkFreeMemory();
    }

    /**
     * \see m_memoryMetric
     */
    inline qint64 memoryMetric() const
    {
        return m_memoryMetric.loadAcquire();
    }

    KisTileDataStoreIterator* beginIteration();
    void endIteration(KisTileDataStoreIterator* iterator);

    KisTileDataStoreReverseIterator* beginReverseIteration();
    void endIteration(KisTileDataStoreReverseIterator* iterator);

    KisTileDataStoreClockIterator* beginClockIteration();
    void endIteration(KisTileDataStoreClockIterator* iterator);

    inline KisTileData* createDefaultTileData(qint32 pixelSize, const quint8 *defPixel)
    {
        return allocTileData(pixelSize, defPixel);
    }

    // Called by The Memento Manager after every commit
    inline void kickPooler()
    {
        m_pooler.kick();

        //FIXME: maybe, rename a function?
        m_swapper.kick();
    }

    /**
     * Try swap out the tile data.
     * It may fail in case the tile is being accessed
     * at the same moment of time.
     */
    bool trySwapTileData(KisTileData *td);


    /**
     * WARN: The following three method are only for usage
     * in KisTileData. Do not call them directly!
     */

    KisTileData *duplicateTileData(KisTileData *rhs);

    void freeTileData(KisTileData *td);

    /**
     * Ensures that the tile data is totally present in memory
     * and it's swapping is blocked by holding td->m_swapLock
     * in a read mode.
     * PRECONDITIONS: td->m_swapLock is *unlocked*
     *                m_listRWLock is *unlocked*
     * POSTCONDITIONS: td->m_data is in memory and
     *                 td->m_swapLock is locked
     *                 m_listRWLock is unlocked
     */
    void ensureTileDataLoaded(KisTileData *td);

    void registerTileData(KisTileData *td);
    void unregisterTileData(KisTileData *td);

private:
    KisTileData *allocTileData(qint32 pixelSize, const quint8 *defPixel);

    inline void registerTileDataImp(KisTileData *td);
    inline void unregisterTileDataImp(KisTileData *td);
    void freeRegisteredTiles();

    friend class DeadlockyThread;
    friend class KisLowMemoryTests;
    void debugSwapAll();
    void debugClear();

    friend class KisTiledDataManagerTest;
    void testingSuspendPooler();
    void testingResumePooler();

    friend class KisLowMemoryBenchmark;
    void testingRereadConfig();
private:
    KisTileDataPooler m_pooler;
    KisTileDataSwapper m_swapper;

    friend class KisTileDataStoreTest;
    friend class KisTileDataPoolerTest;
    KisSwappedDataStore m_swappedStore;

    /**
     * This metric is used for computing the volume
     * of memory occupied by tile data objects.
     * metric = num_bytes / (KisTileData::WIDTH * KisTileData::HEIGHT)
     */
    QAtomicInt m_numTiles;
    QAtomicInt m_memoryMetric;
    QAtomicInt m_counter;
    QAtomicInt m_clockIndex;
    ConcurrentMap<int, KisTileData*> m_tileDataMap;
    QReadWriteLock m_iteratorLock;
};

template<typename T>
inline T MiB_TO_METRIC(T value)
{
    unsigned long long __MiB = 1ULL << 20;
    return value * (__MiB / (KisTileData::WIDTH * KisTileData::HEIGHT));
}

#endif /* KIS_TILE_DATA_STORE_H_ */

