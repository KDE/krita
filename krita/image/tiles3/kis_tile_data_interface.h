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
#ifndef KIS_TILE_DATA_INTERFACE_H_
#define KIS_TILE_DATA_INTERFACE_H_

#include <QReadWriteLock>
#include <QAtomicInt>

#include "kis_lockless_stack.h"
#include "kis_memory_pool.h"
#include "swap/kis_chunk_allocator.h"

class KisTileData;
class KisTileDataStore;

/**
 * WARNING: Those definitions for internal use only!
 * Please use KisTileData::WIDTH/HEIGHT instead
 */
#define __TILE_DATA_WIDTH 64
#define __TILE_DATA_HEIGHT 64

#define TILE_DATA_POOL_SIZE 32


// BPP == bytes per pixel
#define TILE_SIZE_4BPP (4 * __TILE_DATA_WIDTH * __TILE_DATA_HEIGHT)
#define TILE_SIZE_8BPP (8 * __TILE_DATA_WIDTH * __TILE_DATA_HEIGHT)


typedef KisMemoryPool<quint8[TILE_SIZE_4BPP],TILE_DATA_POOL_SIZE> KisTileMemoryPool4BPP;
typedef KisMemoryPool<quint8[TILE_SIZE_8BPP],TILE_DATA_POOL_SIZE> KisTileMemoryPool8BPP;
typedef KisLocklessStack<KisTileData*> KisTileDataCache;


typedef QLinkedList<KisTileData*> KisTileDataList;
typedef KisTileDataList::iterator KisTileDataListIterator;
typedef KisTileDataList::const_iterator KisTileDataListConstIterator;


/**
 * Stores actual tile's data
 */
class KisTileData
{
public:
    KisTileData(qint32 pixelSize, const quint8 *defPixel, KisTileDataStore *store);

private:
    KisTileData(const KisTileData& rhs, bool checkFreeMemory = true);

public:
    ~KisTileData();

    enum EnumTileDataState {
        NORMAL = 0,
        COMPRESSED,
        SWAPPED
    };

    /**
     * Information about data stored
     */
    inline quint8* data() const;
    inline void setData(const quint8 *data);
    inline quint32 pixelSize() const;

    /**
     * Increments usersCount of a TD and refs shared pointer counter
     * Used by KisTile for COW
     */
    inline bool acquire();

    /**
     * Decrements usersCount of a TD and derefs shared pointer counter
     * Used by KisTile for COW
     */
    inline bool release();

    /**
     * Only refs shared pointer counter.
     * Used only by KisMementoManager without
     * consideration of COW.
     */
    inline bool ref() const;

    /**
     * Only refs shared pointer counter.
     * Used only by KisMementoManager without
     * consideration of COW.
     */
    inline bool deref();

    /**
     * Creates a clone of the tile data safely.
     * It will try to use the cached clones.
     */
    inline KisTileData* clone();

    /**
     * Control the access of swapper to the tile data
     */
    inline void blockSwapping();
    inline void unblockSwapping();

    /**
     * The position of the tile data in a swap file
     */
    inline KisChunk swapChunk() const;
    inline void setSwapChunk(KisChunk chunk);

    /**
     * Show whether a tile data is a part of history
     */
    inline bool mementoed() const;
    inline void setMementoed(bool value);

    /**
     * Controlling methods for setting 'age' marks
     */
    inline int age() const;
    inline void resetAge();
    inline void markOld();

    /**
     * Returns number of tiles (or memento items),
     * referencing the tile data.
     */
    inline qint32 numUsers() const;


    /**
     * Used for swapping purposes only.
     * Frees the memory occupied by the tile data.
     * (the caller must save the data beforehand)
     */
    void releaseMemory();

    /**
     * Used for swapping purposes only.
     * Allocates memory for the tile data after
     * it has been freed in releaseMemory().
     * NOTE: the new data can be not-initialized
     *       and you must fill it yourself!
     *
     * \see releaseMemory()
     */
    void allocateMemory();

private:
    void fillWithPixel(const quint8 *defPixel);

    static quint8* allocateData(const qint32 pixelSize);
    static void freeData(quint8 *ptr, const qint32 pixelSize);
private:
    friend class KisTileDataPooler;
    friend class KisTileDataPoolerTest;
    /**
     * A list of pre-duplicated tiledatas.
     * To make a COW faster, KisTileDataPooler thread duplicates
     * a tile beforehand and stores clones here, in this stack
     */
    KisTileDataCache m_clonesStack;

private:
    friend class KisTile;
    friend class KisTileDataStore;

    friend class KisTileDataStoreIterator;
    friend class KisTileDataStoreReverseIterator;
    friend class KisTileDataStoreClockIterator;

    /**
     * The state of the tile.
     * Filled in by tileDataStore and
     * checked in KisTile::acquireFor*
     * see also: comment for @m_data
     */
    mutable EnumTileDataState m_state;

    /**
     * Iterator that points to a position in the list
     * where the tile data is stored
     */
    KisTileDataListIterator m_listIterator;

private:
    /**
     * The chunk of the swap file, that corresponds
     * to this tile data. Used by KisSwappedDataStore.
     */
    KisChunk m_swapChunk;


    /**
     * The flag is set by KisMementoItem to show this
     * tile data is going down in history.
     *
     * (m_mementoFlag && m_usersCount == 1) means that
     * the only user of tile data is a memento manager.
     */
    qint32 m_mementoFlag;

    /**
     * Counts up time after last access to the tile data.
     * 0 - recently accessed
     * 1+ - not recently accessed
     */
    //FIXME: make memory aligned
    int m_age;


    /**
     * The primitive for controlling swapping of the tile.
     * lockForRead() - used by regular threads to ensure swapper
     *                 won't touch this tile data.
     * tryLockForWrite() - used by swapper to check no-one reads
     *                     this tile data
     */
    QReadWriteLock m_swapLock;

private:
    friend class KisLowMemoryTests;

    /**
     * FIXME: We should be able to work in const environment
     * even when actual data is swapped out to disk
     */
    mutable quint8* m_data;

    /**
     * How many tiles/mementoes use
     * this tiledata through COW?
     */
    mutable QAtomicInt m_usersCount;

    /**
     * Shared pointer counter
     */
    mutable QAtomicInt m_refCount;


    qint32 m_pixelSize;
    //qint32 m_timeStamp;

    KisTileDataStore *m_store;

    static KisTileMemoryPool4BPP m_pool4BPP;
    static KisTileMemoryPool8BPP m_pool8BPP;
public:
    static const qint32 WIDTH;
    static const qint32 HEIGHT;
};

#endif /* KIS_TILE_DATA_INTERFACE_H_ */
