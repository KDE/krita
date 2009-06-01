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

#ifndef KIS_TILEDDATAMANAGER_P_H_
#define KIS_TILEDDATAMANAGER_P_H_

#include "kis_tile.h"


class KisTileHashTable
{
public:
    KisTileHashTable();
    KisTileHashTable(const KisTileHashTable &ht);

    /* virtual? */
    ~KisTileHashTable();
    
    KisTileSP getTile(qint32 col, qint32 row);
    bool tileExists(qint32 col, qint32 row);

    void linkTile(KisTileSP tile);
    KisTileSP unlinkTile(qint32 col, qint32 row);
    
    KisTileSP getTileLazy(qint32 col, qint32 row);
    void deleteTile(KisTileSP tile);
    void deleteTile(qint32 col, qint32 row);

    void clear();

    void setDefaultTileData(KisTileData *defaultTileData);
    KisTileData* defaultTileData();

    void debugPrintInfo();
    void debugMaxListLength(qint32 &min, qint32 &max);
private:
    
    static inline quint32 calculateHash(qint32 col, qint32 row);

    inline qint32 debugChainLen(qint32 idx);
    void debugListLengthDistibution();
private:
//    Q_DISABLE_COPY(KisTileHashTable);
    
    static const qint32 TABLE_SIZE = 1024;
    KisTileSP *m_hashTable;
    qint32 m_numTiles;
    
    KisTileData *m_defaultTileData;

    QReadWriteLock m_lock;
};


#endif /* KIS_TILEDDATAMANAGER_P_H_ */
