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

#include <QtGlobal>
#include "kis_debug.h"
#include "kis_global.h"

//#define SHARED_TILES_SANITY_CHECK


template<class T>
KisTileHashTableTraits<T>::KisTileHashTableTraits(KisMementoManager *mm)
        : m_lock(QReadWriteLock::NonRecursive)
{
    m_hashTable = new TileTypeSP [TABLE_SIZE];
    Q_CHECK_PTR(m_hashTable);

    m_numTiles = 0;
    m_defaultTileData = 0;
    m_mementoManager = mm;
}

template<class T>
KisTileHashTableTraits<T>::KisTileHashTableTraits(const KisTileHashTableTraits<T> &ht,
        KisMementoManager *mm)
        : m_lock(QReadWriteLock::NonRecursive)
{
    QReadLocker locker(&ht.m_lock);

    m_mementoManager = mm;
    m_defaultTileData = 0;
    setDefaultTileDataImp(ht.m_defaultTileData);

    m_hashTable = new TileTypeSP [TABLE_SIZE];
    Q_CHECK_PTR(m_hashTable);


    TileTypeSP foreignTile;
    TileTypeSP nativeTile;
    TileTypeSP nativeTileHead;
    for (qint32 i = 0; i < TABLE_SIZE; i++) {
        nativeTileHead = 0;

        foreignTile = ht.m_hashTable[i];
        while (foreignTile) {
            nativeTile = TileTypeSP(new TileType(*foreignTile, m_mementoManager));
            nativeTile->setNext(nativeTileHead);
            nativeTileHead = nativeTile;

            foreignTile = foreignTile->next();
        }

        m_hashTable[i] = nativeTileHead;
    }
    m_numTiles = ht.m_numTiles;
}

template<class T>
KisTileHashTableTraits<T>::~KisTileHashTableTraits()
{
    clear();
    delete[] m_hashTable;
    setDefaultTileDataImp(0);
}

template<class T>
quint32 KisTileHashTableTraits<T>::calculateHash(qint32 col, qint32 row)
{
    return ((row << 5) + (col & 0x1F)) & 0x3FF;
}

template<class T>
typename KisTileHashTableTraits<T>::TileTypeSP
KisTileHashTableTraits<T>::getTileMinefieldWalk(qint32 col, qint32 row, qint32 idx)
{
    // WARNING: this function is here only for educational purposes! Don't
    //          use it! It causes race condition in a shared pointer copy-ctor
    //          when accessing m_hashTable!

    /**
     * This is a special method for dangerous and unsafe access to
     * the tiles table. Thanks to the fact that our shared pointers
     * are thread safe, we can iterate through the linked list without
     * having any locks help. In the worst case, we will miss the needed
     * tile. In that case, the higher level code will do the proper
     * locking and do the second try with all the needed locks held.
     */

    TileTypeSP headTile = m_hashTable[idx];
    TileTypeSP tile = headTile;

    for (; tile; tile = tile->next()) {
        if (tile->col() == col &&
            tile->row() == row) {

            if (m_hashTable[idx] != headTile) {
                tile.clear();
            }

            break;
        }
    }

    return tile;
}

template<class T>
typename KisTileHashTableTraits<T>::TileTypeSP
KisTileHashTableTraits<T>::getTile(qint32 col, qint32 row, qint32 idx)
{
    TileTypeSP tile = m_hashTable[idx];

    for (; tile; tile = tile->next()) {
        if (tile->col() == col &&
                tile->row() == row) {

            return tile;
        }
    }

    return TileTypeSP();
}

template<class T>
void KisTileHashTableTraits<T>::linkTile(TileTypeSP tile, qint32 idx)
{
    TileTypeSP firstTile = m_hashTable[idx];

#ifdef SHARED_TILES_SANITY_CHECK
    Q_ASSERT_X(!tile->next(), "KisTileHashTableTraits<T>::linkTile",
               "A tile can't be shared by several hash tables, sorry.");
#endif

    tile->setNext(firstTile);
    m_hashTable[idx] = tile;
    m_numTiles++;
}

template<class T>
bool KisTileHashTableTraits<T>::unlinkTile(qint32 col, qint32 row, qint32 idx)
{
    TileTypeSP tile = m_hashTable[idx];
    TileTypeSP prevTile;

    for (; tile; tile = tile->next()) {
        if (tile->col() == col &&
                tile->row() == row) {

            if (prevTile)
                prevTile->setNext(tile->next());
            else
                /* optimize here*/
                m_hashTable[idx] = tile->next();

            /**
             * The shared pointer may still be accessed by someone, so
             * we need to disconnects the tile from memento manager
             * explicitly
             */
            tile->setNext(TileTypeSP());
            tile->notifyDetachedFromDataManager();
            tile.clear();

            m_numTiles--;
            return true;
        }
        prevTile = tile;
    }

    return false;
}

template<class T>
inline void KisTileHashTableTraits<T>::setDefaultTileDataImp(KisTileData *defaultTileData)
{
    if (m_defaultTileData) {
        m_defaultTileData->release();
        m_defaultTileData = 0;
    }

    if (defaultTileData) {
        defaultTileData->acquire();
        m_defaultTileData = defaultTileData;
    }
}

template<class T>
inline KisTileData* KisTileHashTableTraits<T>::defaultTileDataImp() const
{
    return m_defaultTileData;
}


template<class T>
bool KisTileHashTableTraits<T>::tileExists(qint32 col, qint32 row)
{
    return this->getExisitngTile(col, row);
}

template<class T>
typename KisTileHashTableTraits<T>::TileTypeSP
KisTileHashTableTraits<T>::getExistingTile(qint32 col, qint32 row)
{
    const qint32 idx = calculateHash(col, row);

    // NOTE: minefield walk is disabled due to supposed unsafety,
    //       see bug 391270

    QReadLocker locker(&m_lock);
    return getTile(col, row, idx);
}

template<class T>
typename KisTileHashTableTraits<T>::TileTypeSP
KisTileHashTableTraits<T>::getTileLazy(qint32 col, qint32 row,
                                       bool& newTile)
{
    const qint32 idx = calculateHash(col, row);

    // NOTE: minefield walk is disabled due to supposed unsafety,
    //       see bug 391270

    newTile = false;
    TileTypeSP tile;

    {
        QReadLocker locker(&m_lock);
        tile = getTile(col, row, idx);
    }

    if (!tile) {
        QWriteLocker locker(&m_lock);
        tile = new TileType(col, row, m_defaultTileData, m_mementoManager);
        linkTile(tile, idx);
        newTile = true;
    }

    return tile;
}

template<class T>
typename KisTileHashTableTraits<T>::TileTypeSP
KisTileHashTableTraits<T>::getReadOnlyTileLazy(qint32 col, qint32 row, bool &existingTile)
{
    const qint32 idx = calculateHash(col, row);

    // NOTE: minefield walk is disabled due to supposed unsafety,
    //       see bug 391270

    QReadLocker locker(&m_lock);

    TileTypeSP tile = getTile(col, row, idx);
    existingTile = tile;

    if (!existingTile) {
        tile = new TileType(col, row, m_defaultTileData, 0);
    }

    return tile;
}

template<class T>
void KisTileHashTableTraits<T>::addTile(TileTypeSP tile)
{
    const qint32 idx = calculateHash(tile->col(), tile->row());

    QWriteLocker locker(&m_lock);
    linkTile(tile, idx);
}

template<class T>
bool KisTileHashTableTraits<T>::deleteTile(qint32 col, qint32 row)
{
    const qint32 idx = calculateHash(col, row);

    QWriteLocker locker(&m_lock);
    return unlinkTile(col, row, idx);
}

template<class T>
bool KisTileHashTableTraits<T>::deleteTile(TileTypeSP tile)
{
    return deleteTile(tile->col(), tile->row());
}

template<class T>
void KisTileHashTableTraits<T>::clear()
{
    QWriteLocker locker(&m_lock);
    TileTypeSP tile = TileTypeSP();
    qint32 i;

    for (i = 0; i < TABLE_SIZE; i++) {
        tile = m_hashTable[i];

        while (tile) {
            TileTypeSP tmp = tile;
            tile = tile->next();

            /**
             * About disconnection of tiles see a comment in unlinkTile()
             */

            tmp->setNext(TileTypeSP());
            tmp->notifyDetachedFromDataManager();
            tmp = 0;

            m_numTiles--;
        }

        m_hashTable[i] = 0;
    }

    Q_ASSERT(!m_numTiles);
}

template<class T>
void KisTileHashTableTraits<T>::setDefaultTileData(KisTileData *defaultTileData)
{
    QWriteLocker locker(&m_lock);
    setDefaultTileDataImp(defaultTileData);
}

template<class T>
KisTileData* KisTileHashTableTraits<T>::defaultTileData() const
{
    QWriteLocker locker(&m_lock);
    return defaultTileDataImp();
}


/*************** Debugging stuff ***************/

template<class T>
void KisTileHashTableTraits<T>::debugPrintInfo()
{
    if (!m_numTiles) return;

    qInfo() << "==========================\n"
             << "TileHashTable:"
             << "\n   def. data:\t\t" << m_defaultTileData
             << "\n   numTiles:\t\t" << m_numTiles;
    debugListLengthDistibution();
    qInfo() << "==========================\n";
}

template<class T>
qint32 KisTileHashTableTraits<T>::debugChainLen(qint32 idx)
{
    qint32 len = 0;
    for (TileTypeSP it = m_hashTable[idx]; it; it = it->next(), len++) ;
    return len;
}

template<class T>
void KisTileHashTableTraits<T>::debugMaxListLength(qint32 &min, qint32 &max)
{
    TileTypeSP tile;
    qint32 maxLen = 0;
    qint32 minLen = m_numTiles;
    qint32 tmp = 0;

    for (qint32 i = 0; i < TABLE_SIZE; i++) {
        tmp = debugChainLen(i);
        if (tmp > maxLen)
            maxLen = tmp;
        if (tmp < minLen)
            minLen = tmp;
    }

    min = minLen;
    max = maxLen;
}

template<class T>
void KisTileHashTableTraits<T>::debugListLengthDistibution()
{
    qint32 min, max;
    qint32 arraySize;
    qint32 tmp;

    debugMaxListLength(min, max);
    arraySize = max - min + 1;

    qint32 *array = new qint32[arraySize];
    memset(array, 0, sizeof(qint32)*arraySize);

    for (qint32 i = 0; i < TABLE_SIZE; i++) {
        tmp = debugChainLen(i);
        array[tmp-min]++;
    }

    qInfo() << QString("   minChain: %1\n").arg(min);
    qInfo() << QString("   maxChain: %1\n").arg(max);

    qInfo() << "   Chain size distribution:";
    for (qint32 i = 0; i < arraySize; i++)
        qInfo() << QString("      %1: %2").arg(i + min).arg(array[i]);

    delete[] array;
}

template<class T>
void KisTileHashTableTraits<T>::sanityChecksumCheck()
{
    /**
     * We assume that the lock should have already been taken
     * by the code that was going to change the table
     */
    Q_ASSERT(!m_lock.tryLockForWrite());

    TileTypeSP tile = 0;
    qint32 exactNumTiles = 0;

    for (qint32 i = 0; i < TABLE_SIZE; i++) {
        tile = m_hashTable[i];
        while (tile) {
            exactNumTiles++;
            tile = tile->next();
        }
    }

    if (exactNumTiles != m_numTiles) {
        dbgKrita << "Sanity check failed!";
        dbgKrita << ppVar(exactNumTiles);
        dbgKrita << ppVar(m_numTiles);
        dbgKrita << "Wrong tiles checksum!";
        Q_ASSERT(0); // not fatalKrita for a backtrace support
    }
}
