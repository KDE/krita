/*
 *  Copyright (c) 2004 Casper Boemann <cbr@boemann.dk>
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

    KisTileHashTableTraits(KisMementoManager *mm);
    KisTileHashTableTraits(const KisTileHashTableTraits<T> &ht,
                           KisMementoManager *mm);

    /* virtual? */
    ~KisTileHashTableTraits();

    bool tileExists(qint32 col, qint32 row);
    TileTypeSP getTileLazy(qint32 col, qint32 row, bool& newTile);
    TileTypeSP getExistedTile(qint32 col, qint32 row);
    void addTile(TileTypeSP tile);
    void deleteTile(TileTypeSP tile);
    void deleteTile(qint32 col, qint32 row);

    void clear();

    void setDefaultTileData(KisTileData *defaultTileData);
    KisTileData* defaultTileData();

    qint32 numTiles() {
        return m_numTiles;
    }

    void debugPrintInfo();
    void debugMaxListLength(qint32 &min, qint32 &max);
private:

    TileTypeSP getTile(qint32 col, qint32 row);
    void linkTile(TileTypeSP tile);
    TileTypeSP unlinkTile(qint32 col, qint32 row);

    static inline quint32 calculateHash(qint32 col, qint32 row);

    inline qint32 debugChainLen(qint32 idx);
    void debugListLengthDistibution();
private:
    template<class U> friend class KisTileHashTableIteratorTraits;

    static const qint32 TABLE_SIZE = 1024;
    TileTypeSP *m_hashTable;
    qint32 m_numTiles;

    KisTileData *m_defaultTileData;
    KisMementoManager *m_mementoManager;

    QReadWriteLock m_lock;
};

#include "kis_tile_hash_table_p.h"



/**
 * Walks through all tiles inside hash table
 * Note: You can't work with your hash table in a regular way
 *       during iterating with this iterator, because HT is locked.
 *       The only thing you can do is to delete current tile.
 */
template<class T>
class KisTileHashTableIteratorTraits
{
public:
    typedef T               TileType;
    typedef KisSharedPtr<T> TileTypeSP;

    KisTileHashTableIteratorTraits(KisTileHashTableTraits<T> *ht) {
        m_hashTable = ht;
        m_index = nextNonEmptyList(0);
        if (m_index < KisTileHashTableTraits<T>::TABLE_SIZE)
            m_tile = m_hashTable->m_hashTable[m_index];

        m_hashTable->m_lock.lockForWrite();
    }

    ~KisTileHashTableIteratorTraits<T>() {
        if (m_index != -1)
            m_hashTable->m_lock.unlock();
    }

    KisTileHashTableIteratorTraits<T>& operator++() {
        next();
        return *this;
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
                    destroy();
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

    void deleteCurrent() {
        TileTypeSP tile = m_tile;
        next();
        m_hashTable->unlinkTile(tile->col(), tile->row());
    }

    void destroy() {
        m_index = -1;
        m_hashTable->m_lock.unlock();
    }
protected:
    TileTypeSP m_tile;
    qint32 m_index;
    KisTileHashTableTraits<T> *m_hashTable;

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
    Q_DISABLE_COPY(KisTileHashTableIteratorTraits<T>);
};


typedef KisTileHashTableTraits<KisTile> KisTileHashTable;
typedef KisTileHashTableIteratorTraits<KisTile> KisTileHashTableIterator;

#endif /* KIS_TILEHASHTABLE_H_ */
