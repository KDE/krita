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

#include "kis_tile_data_pooler.h"
#include "swap/kis_swapped_data_store.h"


class KisTestingTileDataStoreAccessor;

/**
 * Stores tileData objects. When needed compresses them and swaps.
 */
class KisTileDataStore
{
public:
    KisTileDataStore();
    ~KisTileDataStore();

    void debugPrintList();

    KisTileData *duplicateTileData(KisTileData *rhs);


    /**
     * Increments usersCount of a TD and refs shared pointer counter
     * Used by KisTile for COW
     */
    inline quint32 acquireTileData(const KisTileData *td) const {
        qint32 ref = refTileData(td);
        td->m_usersCount.ref();
        return ref;
    }

    /**
     * Decrements usersCount of a TD and derefs shared pointer counter
     * Used by KisTile for COW
     */
    inline quint32 releaseTileData(KisTileData *td) {
        td->m_usersCount.deref();
        qint32 ref = derefTileData(td);
        return ref;
    }

    /**
     * Only refs shared pointer counter.
     * Used only by KisMementoManager without
     * consideration of COW.
     */
    inline quint32 refTileData(const KisTileData *td) const {
        return td->m_refCount.ref();
    }

    /**
     * Only refs shared pointer counter.
     * Used only by KisMementoManager without
     * consideration of COW.
     */
    inline quint32 derefTileData(KisTileData *td) {
        if (!(td->m_refCount.deref())) {
            freeTileData(td);
            return 0;
        }
        return td->m_refCount;
    }

    inline KisTileData* createDefaultTileData(qint32 pixelSize, const quint8 *defPixel) {
        return allocTileData(pixelSize, defPixel);
    }

    // Called by The Memento Manager after every commit
    inline void kickPooler() {
        m_pooler.kick();
    }

    inline void blockSwapping(KisTileData *td) {
        td->m_swapLock.lockForRead();
        if(!td->data()) {
            td->m_swapLock.unlock();
            ensureTileDataLoaded(td);
        }
    }

    inline void unblockSwapping(KisTileData *td) {
        td->m_swapLock.unlock();
    }

    /**
     * Try swap out the tile data.
     * It may fail in case the tile is being accessed
     * at the same moment of time.
     */
    bool trySwapTileData(KisTileData *td);

private:
    /**
     * Ensures that the tile data is totally present in memory
     * and it's swapping is blocked by holding td->m_swapLock
     * in a read mode.
     * PRECONDITIONS: td->m_swapLock is *unlocked*
     * POSTCONDITIONS: td->m_data is in memory and
     *                 td->m_swapLock is locked
     */
    void ensureTileDataLoaded(KisTileData *td);

private:
    KisTileData *allocTileData(qint32 pixelSize, const quint8 *defPixel);
    void freeTileData(KisTileData *td);

    void tileListAppend(KisTileData *td);
    void tileListDetach(KisTileData *td);
    void tileListClear();

    void debugSwapAll();
private:
    friend class KisTileDataPooler;
    KisTileDataPooler m_pooler;

    friend class KisTestingTileDataStoreAccessor;
    KisSwappedDataStore m_swappedStore;

    QReadWriteLock m_listRWLock;
    KisTileData *m_tileDataListHead;
};

extern KisTileDataStore globalTileDataStore;

#endif /* KIS_TILE_DATA_STORE_H_ */

