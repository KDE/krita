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


template<class T>
KisTileHashTableTraits<T>::KisTileHashTableTraits(KisMementoManager *mm)
        : m_lock(QReadWriteLock::NonRecursive)
{
    m_hashTable = new TileTypeSP [TABLE_SIZE];
    Q_CHECK_PTR(m_hashTable);

    for (int i = 0; i < TABLE_SIZE; i++)
        m_hashTable[i] = 0;

    m_numTiles = 0;
    m_defaultTileData = 0;
    m_mementoManager = mm;
}

template<class T>
KisTileHashTableTraits<T>::KisTileHashTableTraits(const KisTileHashTableTraits<T> &ht,
        KisMementoManager *mm)
        : m_lock(QReadWriteLock::NonRecursive)
{
    m_mementoManager = mm;

    m_defaultTileData = 0;
    setDefaultTileData(ht.m_defaultTileData);

    m_hashTable = new TileTypeSP [TABLE_SIZE];
    Q_CHECK_PTR(m_hashTable);


    TileTypeSP foreignTile;
    TileType* nativeTile;
    TileType* nativeTileHead;
    for (qint32 i = 0; i < TABLE_SIZE; i++) {
        nativeTileHead = 0;

        foreignTile = ht.m_hashTable[i];
        while (foreignTile) {
            nativeTile = new TileType(*foreignTile, m_mementoManager);
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
    setDefaultTileData(0);
}

template<class T>
quint32 KisTileHashTableTraits<T>::calculateHash(qint32 col, qint32 row)
{
    return ((row << 5) + (col & 0x1F)) & 0x3FF;
}

template<class T>
typename KisTileHashTableTraits<T>::TileTypeSP
KisTileHashTableTraits<T>::getTile(qint32 col, qint32 row)
{
    qint32 idx = calculateHash(col, row);
    TileTypeSP tile = m_hashTable[idx];

    for (; tile; tile = tile->next()) {
        if (tile->col() == col &&
                tile->row() == row) {

            return tile;
        }
    }

    return 0;
}

template<class T>
void KisTileHashTableTraits<T>::linkTile(TileTypeSP tile)
{
    qint32 idx = calculateHash(tile->col(), tile->row());
    TileTypeSP firstTile = m_hashTable[idx];

    tile->setNext(firstTile);
    m_hashTable[idx] = tile;
    m_numTiles++;
}

template<class T>
typename KisTileHashTableTraits<T>::TileTypeSP
KisTileHashTableTraits<T>::unlinkTile(qint32 col, qint32 row)
{
    qint32 idx = calculateHash(col, row);
    TileTypeSP tile = m_hashTable[idx];
    TileTypeSP prevTile = 0;

    for (; tile; tile = tile->next()) {
        if (tile->col() == col &&
                tile->row() == row) {

            if (prevTile)
                prevTile->setNext(tile->next());
            else
                /* optimize here*/
                m_hashTable[idx] = tile->next();

            m_numTiles--;
            return tile;
        }
        prevTile = tile;
    }

    return 0;
}

template<class T>
bool KisTileHashTableTraits<T>::tileExists(qint32 col, qint32 row)
{
    QReadLocker locker(&m_lock);
    return getTile(col, row);
}

template<class T>
typename KisTileHashTableTraits<T>::TileTypeSP
KisTileHashTableTraits<T>::getExistedTile(qint32 col, qint32 row)
{
    QReadLocker locker(&m_lock);
    return getTile(col, row);
}

template<class T>
typename KisTileHashTableTraits<T>::TileTypeSP
KisTileHashTableTraits<T>::getTileLazy(qint32 col, qint32 row,
                                       bool& newTile)
{
    /**
     * FIXME: Read access is better
     */
    QWriteLocker locker(&m_lock);

    newTile = false;
    TileTypeSP tile = getTile(col, row);
    if (!tile) {
        tile = new TileType(col, row, m_defaultTileData, m_mementoManager);
        linkTile(tile);
        newTile = true;
    }

    return tile;
}

template<class T>
void KisTileHashTableTraits<T>::addTile(TileTypeSP tile)
{
    QWriteLocker locker(&m_lock);
    linkTile(tile);
}

template<class T>
void KisTileHashTableTraits<T>::deleteTile(qint32 col, qint32 row)
{
    QWriteLocker locker(&m_lock);

    TileTypeSP tile = unlinkTile(col, row);

    /* Done by KisSharedPtr */
    //if(tile)
    //    delete tile;

}

template<class T>
void KisTileHashTableTraits<T>::deleteTile(TileTypeSP tile)
{
    QWriteLocker locker(&m_lock);

    deleteTile(tile->col(), tile->row());
}

template<class T>
void KisTileHashTableTraits<T>::clear()
{
    QWriteLocker locker(&m_lock);
    TileTypeSP tile = 0;
//    KisTile* tmp;
    qint32 i;

    for (i = 0; i < TABLE_SIZE; i++) {
        tile = m_hashTable[i];

        while (tile) {
            //tmp = tile;
            tile = tile->next();
            /* done by KisShared */
            //delete tmp;
            m_numTiles--;
        }
        m_hashTable[i] = 0;
    }

    Q_ASSERT(!m_numTiles);
}


template<class T>
void KisTileHashTableTraits<T>::setDefaultTileData(KisTileData *defaultTileData)
{
    if (m_defaultTileData) {
        globalTileDataStore.releaseTileData(m_defaultTileData);
        m_defaultTileData = 0;
    }

    if (defaultTileData) {
        globalTileDataStore.acquireTileData(defaultTileData);
        m_defaultTileData = defaultTileData;
    }
}

template<class T>
KisTileData* KisTileHashTableTraits<T>::defaultTileData()
{
    return m_defaultTileData;
}


/*************** Debugging stuff ***************/

template<class T>
void KisTileHashTableTraits<T>::debugPrintInfo()
{
    printf("==========================\n");
    printf("TileHashTable:\n");
    printf("   def. data:\t\t0x%X\n", (quintptr) m_defaultTileData);
    printf("   numTiles:\t\t%d\n", (int) m_numTiles);
    debugListLengthDistibution();
    printf("==========================\n");
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

    printf("   minChain:\t\t%d\n", min);
    printf("   maxChain:\t\t%d\n", max);

    printf("   Chain size distribution:\n");
    for (qint32 i = 0; i < arraySize; i++)
        printf("      %3d:\t%4d\n", i + min, array[i]);

    delete[] array;
}
