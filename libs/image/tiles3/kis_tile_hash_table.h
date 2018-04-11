/*
 *  Copyright (c) 2004 C. Boemann <cbo@boemann.dk>
 *            (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KIS_TILEHASHTABLE_H_
#define KIS_TILEHASHTABLE_H_

#include "kis_tile.h"



/**
 * This is a  template for a hash table that stores  tiles (or some other
 * objects  resembling tiles).   Actually, this  object should  only have
 * col()/row() methods and be able to answer setNext()/next() requests to
 * be   stored   here.    It   is   used   in   KisTiledDataManager   and
 * KisMementoManager.
 */

template<class T>
class KisTileHashTableTraits
{
public:
    typedef T               TileType;
    typedef KisSharedPtr<T> TileTypeSP;
    typedef KisWeakSharedPtr<T> TileTypeWSP;

    KisTileHashTableTraits(KisMementoManager *mm);
    KisTileHashTableTraits(const KisTileHashTableTraits<T> &ht,
                           KisMementoManager *mm);

    /* virtual? */
    ~KisTileHashTableTraits();

    bool isEmpty() {
        return !m_numTiles;
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
    KisTileData* defaultTileData() const;

    qint32 numTiles() {
        return m_numTiles;
    }

    void debugPrintInfo();
    void debugMaxListLength(qint32 &min, qint32 &max);

private:

    TileTypeSP getTileMinefieldWalk(qint32 col, qint32 row, qint32 idx);
    TileTypeSP getTile(qint32 col, qint32 row, qint32 idx);
    void linkTile(TileTypeSP tile, qint32 idx);
    bool unlinkTile(qint32 col, qint32 row, qint32 idx);

    inline void setDefaultTileDataImp(KisTileData *defaultTileData);
    inline KisTileData* defaultTileDataImp() const;

    static inline quint32 calculateHash(qint32 col, qint32 row);

    inline qint32 debugChainLen(qint32 idx);
    void debugListLengthDistibution();
    void sanityChecksumCheck();
private:
    template<class U, class LockerType> friend class KisTileHashTableIteratorTraits;

    static const qint32 TABLE_SIZE = 1024;
    TileTypeSP *m_hashTable;
    qint32 m_numTiles;

    KisTileData *m_defaultTileData;
    KisMementoManager *m_mementoManager;

    mutable QReadWriteLock m_lock;
};

#include "kis_tile_hash_table_p.h"



/**
 * Walks through all tiles inside hash table
 * Note: You can't work with your hash table in a regular way
 *       during iterating with this iterator, because HT is locked.
 *       The only thing you can do is to delete current tile.
 *
 * LockerType defines if the iterator is constant or mutable. One should
 * pass either QReadLocker or QWriteLocker as a parameter.
 */
template<class T, class LockerType>
class KisTileHashTableIteratorTraits
{
public:
    typedef T               TileType;
    typedef KisSharedPtr<T> TileTypeSP;

    KisTileHashTableIteratorTraits(KisTileHashTableTraits<T> *ht)
        : m_locker(&ht->m_lock)
    {
        m_hashTable = ht;
        m_index = nextNonEmptyList(0);
        if (m_index < KisTileHashTableTraits<T>::TABLE_SIZE)
            m_tile = m_hashTable->m_hashTable[m_index];
    }

    ~KisTileHashTableIteratorTraits() {
    }

    void next() {
        if (m_tile) {
            m_tile = m_tile->next();
            if (!m_tile) {
                qint32 idx = nextNonEmptyList(m_index + 1);
                if (idx < KisTileHashTableTraits<T>::TABLE_SIZE) {
                    m_index = idx;
                    m_tile = m_hashTable->m_hashTable[idx];
                } else {
                    //EOList reached
                    m_index = -1;
                    // m_tile.clear(); // already null
                }
            }
        }
    }

    TileTypeSP tile() const {
        return m_tile;
    }
    bool isDone() const {
        return !m_tile;
    }

    // disable the method if we didn't lock for writing
    template <class Helper = LockerType>
    typename std::enable_if<std::is_same<Helper, QWriteLocker>::value, void>::type
    deleteCurrent() {
        TileTypeSP tile = m_tile;
        next();

        const qint32 idx = m_hashTable->calculateHash(tile->col(), tile->row());
        m_hashTable->unlinkTile(tile->col(), tile->row(), idx);
    }

    // disable the method if we didn't lock for writing
    template <class Helper = LockerType>
    typename std::enable_if<std::is_same<Helper, QWriteLocker>::value, void>::type
    moveCurrentToHashTable(KisTileHashTableTraits<T> *newHashTable) {
        TileTypeSP tile = m_tile;
        next();

        const qint32 idx = m_hashTable->calculateHash(tile->col(), tile->row());
        m_hashTable->unlinkTile(tile->col(), tile->row(), idx);

        newHashTable->addTile(tile);
    }

protected:
    TileTypeSP m_tile;
    qint32 m_index;
    KisTileHashTableTraits<T> *m_hashTable;
    LockerType m_locker;

protected:
    qint32 nextNonEmptyList(qint32 startIdx) {
        qint32 idx = startIdx;

        while (idx < KisTileHashTableTraits<T>::TABLE_SIZE &&
                !m_hashTable->m_hashTable[idx]) {
            idx++;
        }

        return idx;
    }
private:
    Q_DISABLE_COPY(KisTileHashTableIteratorTraits)
};


typedef KisTileHashTableTraits<KisTile> KisTileHashTable;
typedef KisTileHashTableIteratorTraits<KisTile, QWriteLocker> KisTileHashTableIterator;
typedef KisTileHashTableIteratorTraits<KisTile, QReadLocker> KisTileHashTableConstIterator;

#endif /* KIS_TILEHASHTABLE_H_ */
