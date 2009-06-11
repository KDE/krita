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


class KisTileHashTable
{
public:
    KisTileHashTable();
    KisTileHashTable(const KisTileHashTable &ht);

    /* virtual? */
    ~KisTileHashTable();
    
    bool tileExists(qint32 col, qint32 row);
    KisTileSP getTileLazy(qint32 col, qint32 row, bool& newTile);
    void addTile(KisTileSP tile);
    void deleteTile(KisTileSP tile);
    void deleteTile(qint32 col, qint32 row);

    void clear();

    void setDefaultTileData(KisTileData *defaultTileData);
    KisTileData* defaultTileData();

    void debugPrintInfo();
    void debugMaxListLength(qint32 &min, qint32 &max);
private:

    KisTileSP getTile(qint32 col, qint32 row);
    void linkTile(KisTileSP tile);
    KisTileSP unlinkTile(qint32 col, qint32 row);

    
    static inline quint32 calculateHash(qint32 col, qint32 row);

    inline qint32 debugChainLen(qint32 idx);
    void debugListLengthDistibution();
private:
//    Q_DISABLE_COPY(KisTileHashTable);
    friend class KisTileHashTableIterator;
    
    static const qint32 TABLE_SIZE = 1024;
    KisTileSP *m_hashTable;
    qint32 m_numTiles;
    
    KisTileData *m_defaultTileData;

    QReadWriteLock m_lock;
};

/**
 * Walks through all tiles inside hash table 
 * Note: You can't work with your hash table in a regular way
 *       during iterating with this iterator, because HT is locked.
 *       The only thing you can is to delete current tile.
 */
class KisTileHashTableIterator 
{
public:
    KisTileHashTableIterator(KisTileHashTable *ht) {
	m_hashTable = ht;
	m_index = nextNonEmptyList(0);
	if(m_index < KisTileHashTable::TABLE_SIZE)
	    m_tile = m_hashTable->m_hashTable[m_index];

	m_hashTable->m_lock.lockForWrite();
    }

    ~KisTileHashTableIterator() {
	if(m_index!=-1)
            m_hashTable->m_lock.unlock();
    }

    KisTileHashTableIterator& operator++() {
	next();
	return *this;
    }

    void next() {
	if(m_tile) {
	    m_tile = m_tile->next();
	    if(!m_tile) {
		qint32 idx = nextNonEmptyList(m_index+1);
		if(idx < KisTileHashTable::TABLE_SIZE) {
		    m_index = idx;
		    m_tile = m_hashTable->m_hashTable[idx];
		}
		else {
		    //EOList reached
		    destroy();
		}
	    }
	}
    }

    KisTileSP tile() const {
	return m_tile;
    }
    bool isDone() const {
	return !m_tile;
    }
    
    void deleteCurrent() {
	KisTileSP tile = m_tile;
	next();
	m_hashTable->unlinkTile(tile->col(), tile->row());
    }
    
    void destroy() {
	m_index=-1;
	m_hashTable->m_lock.unlock();
    }
protected:
    KisTileSP m_tile;
    qint32 m_index;
    KisTileHashTable *m_hashTable;

protected:
    qint32 nextNonEmptyList(qint32 startIdx);
private:
    Q_DISABLE_COPY(KisTileHashTableIterator);
};




#endif /* KIS_TILEHASHTABLE_H_ */
