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
#ifndef KIS_TILE_DATA_STORE_H_
#define KIS_TILE_DATA_STORE_H_


#include <QReadWriteLock>
#include "kis_tile_data.h"


/**
 * Stores tileData objects. When needed compresses them and swaps.
 */
class KisTileDataStore
{
public:
    KisTileDataStore();
    ~KisTileDataStore();

    void debugPrintList();

    void ensureTileDataLoaded(const KisTileData *td);
    KisTileData *duplicateTileData(KisTileData *rhs);


    inline quint32 acquireTileData(const KisTileData *td) const {
        return td->m_refCount.ref();
    }

    inline quint32 releaseTileData(KisTileData *td) {
        if( !(td->m_refCount.deref())) {
            freeTileData(td);
            return 0;
        }
        return td->m_refCount;
    }

    inline KisTileData *createDefaultTileData(qint32 pixelSize, const qint8 *defPixel) {
        return allocTileData(pixelSize, defPixel);
    }

private:
    KisTileData *allocTileData(qint32 pixelSize, const qint8 *defPixel);
    void freeTileData(KisTileData *td);

    void tileListAppend(KisTileData *td);
    void tileListDetach(KisTileData *td);
    void tileListClear();

private:
    QReadWriteLock m_listRWLock;
    KisTileData *m_tileDataListHead;
};


extern KisTileDataStore globalTileDataStore;

#endif /* KIS_TILE_DATA_STORE_H_ */

