/*
 *  Copyright (c) 2018 Andrey Kamakin <a.kamakin@icloud.com>
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

#ifndef KIS_TILEHASHTABLE_2_H
#define KIS_TILEHASHTABLE_2_H

#include "kis_shared.h"
#include "kis_shared_ptr.h"
#include "3rdparty/lock_free_map/concurrent_map.h"
#include "kis_tile.h"
#include "kis_debug.h"

#define SANITY_CHECK

/**
 * This is a  template for a hash table that stores  tiles (or some other
 * objects  resembling tiles).   Actually, this  object should  only have
 * col()/row() methods and be able to answer notifyDetachedFromDataManager() requests to
 * be   stored   here.    It   is   used   in   KisTiledDataManager   and
 * KisMementoManager.
 *
 * How to use:
 *   1) each hash must be unique, otherwise tiles would rewrite each-other
 *   2) 0 key is reserved, so can't be used
 *   3) col and row must be less than 0x7FFF to guarantee uniqueness of hash for each pair
 */

template <class T>
class KisTileHashTableIteratorTraits2;

template <class T>
class KisTileHashTableTraits2
{
    static constexpr bool isInherited = std::is_convertible<T*, KisShared*>::value;
    Q_STATIC_ASSERT_X(isInherited, "Template must inherit KisShared");

public:
    typedef T TileType;
    typedef KisSharedPtr<T> TileTypeSP;
    typedef KisWeakSharedPtr<T> TileTypeWSP;

    KisTileHashTableTraits2(KisMementoManager *mm);
    KisTileHashTableTraits2(const KisTileHashTableTraits2<T> &ht, KisMementoManager *mm);
    ~KisTileHashTableTraits2();

    bool isEmpty()
    {
        return !m_numTiles.load();
    }

    bool tileExists(qint32 col, qint32 row);

    /**
     * Returns a tile in position (col,row). If no tile exists,
     * returns null.
     * \param col column of the tile
     * \param row row of the tile
     */
    TileTypeSP getExistingTile(qint32 col, qint32 row);

    /**
     * Returns a tile in position (col,row). If no tile exists,
     * creates a new one, attaches it to the list and returns.
     * \param col column of the tile
     * \param row row of the tile
     * \param newTile out-parameter, returns true if a new tile
     *                was created
     */
    TileTypeSP getTileLazy(qint32 col, qint32 row, bool& newTile);

    /**
     * Returns a tile in position (col,row). If no tile exists,
     * creates nothing, but returns shared default tile object
     * of the table. Be careful, this object has column and row
     * parameters set to (qint32_MIN, qint32_MIN).
     * \param col column of the tile
     * \param row row of the tile
     * \param existingTile returns true if the tile actually exists in the table
     *                     and it is not a lazily created default wrapper tile
     */
    TileTypeSP getReadOnlyTileLazy(qint32 col, qint32 row, bool &existingTile);
    void addTile(TileTypeSP tile);
    bool deleteTile(TileTypeSP tile);
    bool deleteTile(qint32 col, qint32 row);

    void clear();

    void setDefaultTileData(KisTileData *defaultTileData);
    KisTileData* defaultTileData();

    qint32 numTiles()
    {
        return m_numTiles.load();
    }

    void debugPrintInfo();
    void debugMaxListLength(qint32 &min, qint32 &max);

    friend class KisTileHashTableIteratorTraits2<T>;

private:
    struct MemoryReclaimer {
        MemoryReclaimer(TileType *data) : d(data) {}

        void destroy()
        {
            TileTypeSP::deref(reinterpret_cast<TileTypeSP*>(this), d);
            delete this;
        }

    private:
        TileType *d;
    };

    inline quint32 calculateHash(qint32 col, qint32 row)
    {
#ifdef SANITY_CHECK
        KIS_ASSERT_RECOVER_NOOP(row < 0x7FFF && col < 0x7FFF)
#endif // SANITY_CHECK

        if (col == 0 && row == 0) {
            col = 0x7FFF;
            row = 0x7FFF;
        }

        return ((static_cast<quint32>(row) << 16) | (static_cast<quint32>(col) & 0xFFFF));
    }

    inline void insert(quint32 idx, TileTypeSP item)
    {
        TileTypeSP::ref(&item, item.data());
        TileType *tile = 0;

        {
            QReadLocker locker(&m_iteratorLock);
            m_map.getGC().lockRawPointerAccess();
            tile = m_map.assign(idx, item.data());
        }

        if (tile) {
            tile->notifyDeadWithoutDetaching();
            m_map.getGC().enqueue(&MemoryReclaimer::destroy, new MemoryReclaimer(tile));
        } else {
            m_numTiles.fetchAndAddRelaxed(1);
        }

        m_map.getGC().unlockRawPointerAccess();

        m_map.getGC().update(m_map.migrationInProcess());
    }

    inline bool erase(quint32 idx)
    {
        m_map.getGC().lockRawPointerAccess();

        bool wasDeleted = false;
        TileType *tile = m_map.erase(idx);

        if (tile) {
            tile->notifyDetachedFromDataManager();

            wasDeleted = true;
            m_numTiles.fetchAndSubRelaxed(1);
            m_map.getGC().enqueue(&MemoryReclaimer::destroy, new MemoryReclaimer(tile));
        }

        m_map.getGC().unlockRawPointerAccess();

        m_map.getGC().update(m_map.migrationInProcess());
        return wasDeleted;
    }

private:
    mutable ConcurrentMap<quint32, TileType*> m_map;

    /**
     * We still need something to guard changes in m_defaultTileData,
     * otherwise there will be concurrent read/writes, resulting in broken memory.
     */
    QReadWriteLock m_defaultPixelDataLock;
    mutable QReadWriteLock m_iteratorLock;

    QAtomicInt m_numTiles;
    KisTileData *m_defaultTileData;
    KisMementoManager *m_mementoManager;
};

template <class T>
class KisTileHashTableIteratorTraits2
{
public:
    typedef T TileType;
    typedef KisSharedPtr<T> TileTypeSP;
    typedef typename ConcurrentMap<quint32, TileType*>::Iterator Iterator;

    KisTileHashTableIteratorTraits2(KisTileHashTableTraits2<T> *ht) : m_ht(ht)
    {
        m_ht->m_iteratorLock.lockForWrite();
        m_iter.setMap(m_ht->m_map);
    }

    ~KisTileHashTableIteratorTraits2()
    {
        m_ht->m_iteratorLock.unlock();
    }

    void next()
    {
        m_iter.next();
    }

    TileTypeSP tile() const
    {
        return TileTypeSP(m_iter.getValue());
    }

    bool isDone() const
    {
        return !m_iter.isValid();
    }

    void deleteCurrent()
    {
        m_ht->erase(m_iter.getKey());
        next();
    }

    void moveCurrentToHashTable(KisTileHashTableTraits2<T> *newHashTable)
    {
        TileTypeSP tile = m_iter.getValue();
        next();

        quint32 idx = m_ht->calculateHash(tile->col(), tile->row());
        m_ht->erase(idx);
        newHashTable->insert(idx, tile);
    }

private:
    KisTileHashTableTraits2<T> *m_ht;
    Iterator m_iter;
};

template <class T>
KisTileHashTableTraits2<T>::KisTileHashTableTraits2(KisMementoManager *mm)
    : m_numTiles(0), m_defaultTileData(0), m_mementoManager(mm)
{
}

template <class T>
KisTileHashTableTraits2<T>::KisTileHashTableTraits2(const KisTileHashTableTraits2<T> &ht, KisMementoManager *mm)
    : KisTileHashTableTraits2(mm)
{
    setDefaultTileData(ht.m_defaultTileData);

    QWriteLocker locker(&ht.m_iteratorLock);
    typename ConcurrentMap<quint32, TileType*>::Iterator iter(ht.m_map);

    while (iter.isValid()) {
        TileTypeSP tile = new TileType(*iter.getValue(), m_mementoManager);
        insert(iter.getKey(), tile);
        iter.next();
    }
}

template <class T>
KisTileHashTableTraits2<T>::~KisTileHashTableTraits2()
{
    clear();
    setDefaultTileData(0);
}

template<class T>
bool KisTileHashTableTraits2<T>::tileExists(qint32 col, qint32 row)
{
    return getExistingTile(col, row);
}

template <class T>
typename KisTileHashTableTraits2<T>::TileTypeSP KisTileHashTableTraits2<T>::getExistingTile(qint32 col, qint32 row)
{
    quint32 idx = calculateHash(col, row);

    m_map.getGC().lockRawPointerAccess();
    TileTypeSP tile = m_map.get(idx);
    m_map.getGC().unlockRawPointerAccess();

    m_map.getGC().update(m_map.migrationInProcess());
    return tile;
}

template <class T>
typename KisTileHashTableTraits2<T>::TileTypeSP KisTileHashTableTraits2<T>::getTileLazy(qint32 col, qint32 row, bool &newTile)
{
    newTile = false;
    quint32 idx = calculateHash(col, row);
    TileTypeSP tile = m_map.get(idx);

    if (!tile) {
        while (!(tile = m_map.get(idx))) {
            {
                QReadLocker locker(&m_defaultPixelDataLock);
                tile = new TileType(col, row, m_defaultTileData, 0);
            }

            TileTypeSP::ref(&tile, tile.data());
            TileType *item = 0;

            {
                QReadLocker locker(&m_iteratorLock);
                m_map.getGC().lockRawPointerAccess();
                item = m_map.assign(idx, tile.data());
            }

            if (item) {
                item->notifyDeadWithoutDetaching();
                m_map.getGC().enqueue(&MemoryReclaimer::destroy, new MemoryReclaimer(item));
            } else {
                newTile = true;
                m_numTiles.fetchAndAddRelaxed(1);
            }

            m_map.getGC().unlockRawPointerAccess();
        }
    }

    tile->notifyAttachedToDataManager(m_mementoManager);

    m_map.getGC().update(m_map.migrationInProcess());
    return tile;
}

template <class T>
typename KisTileHashTableTraits2<T>::TileTypeSP KisTileHashTableTraits2<T>::getReadOnlyTileLazy(qint32 col, qint32 row, bool &existingTile)
{
    quint32 idx = calculateHash(col, row);

    m_map.getGC().lockRawPointerAccess();
    TileTypeSP tile = m_map.get(idx);
    m_map.getGC().unlockRawPointerAccess();

    existingTile = tile;

    if (!existingTile) {
        QReadLocker locker(&m_defaultPixelDataLock);
        tile = new TileType(col, row, m_defaultTileData, 0);
    }

    m_map.getGC().update(m_map.migrationInProcess());
    return tile;
}

template <class T>
void KisTileHashTableTraits2<T>::addTile(TileTypeSP tile)
{
    quint32 idx = calculateHash(tile->col(), tile->row());
    insert(idx, tile);
}

template <class T>
bool KisTileHashTableTraits2<T>::deleteTile(TileTypeSP tile)
{
    return deleteTile(tile->col(), tile->row());
}

template <class T>
bool KisTileHashTableTraits2<T>::deleteTile(qint32 col, qint32 row)
{
    quint32 idx = calculateHash(col, row);
    return erase(idx);
}

template<class T>
void KisTileHashTableTraits2<T>::clear()
{
    QWriteLocker locker(&m_iteratorLock);

    typename ConcurrentMap<quint32, TileType*>::Iterator iter(m_map);
    TileType *tile = 0;

    while (iter.isValid()) {
        m_map.getGC().lockRawPointerAccess();
        tile = m_map.erase(iter.getKey());

        if (tile) {
            tile->notifyDetachedFromDataManager();
            m_map.getGC().enqueue(&MemoryReclaimer::destroy, new MemoryReclaimer(tile));
        }
        m_map.getGC().unlockRawPointerAccess();

        iter.next();
    }

    m_numTiles.store(0);
    m_map.getGC().update(false);
}

template <class T>
inline void KisTileHashTableTraits2<T>::setDefaultTileData(KisTileData *defaultTileData)
{
    QWriteLocker locker(&m_defaultPixelDataLock);

    if (m_defaultTileData) {
        m_defaultTileData->release();
        m_defaultTileData = 0;
    }

    if (defaultTileData) {
        defaultTileData->acquire();
        m_defaultTileData = defaultTileData;
    }
}

template <class T>
inline KisTileData* KisTileHashTableTraits2<T>::defaultTileData()
{
    QReadLocker locker(&m_defaultPixelDataLock);
    return m_defaultTileData;
}

template <class T>
void KisTileHashTableTraits2<T>::debugPrintInfo()
{
}

template <class T>
void KisTileHashTableTraits2<T>::debugMaxListLength(qint32 &/*min*/, qint32 &/*max*/)
{
}

typedef KisTileHashTableTraits2<KisTile> KisTileHashTable;
typedef KisTileHashTableIteratorTraits2<KisTile> KisTileHashTableIterator;
typedef KisTileHashTableIteratorTraits2<KisTile> KisTileHashTableConstIterator;

#endif // KIS_TILEHASHTABLE_2_H
