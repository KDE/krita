/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_SWAPPED_DATA_STORE_H
#define __KIS_SWAPPED_DATA_STORE_H

#include "krita_export.h"

#include <QMutex>
#include <QByteArray>


class QMutex;
class KisTileData;
class KisAbstractTileCompressor;
class KisChunkAllocator;
class KisMemoryWindow;

class KRITAIMAGE_EXPORT KisSwappedDataStore
{
public:
    KisSwappedDataStore();
    ~KisSwappedDataStore();

    /**
     * Returns number of swapped out tile data objects
     */
    quint64 numTiles() const;

    /**
     * Swap out the data stored in the \a td to the swap file
     * and free memory occupied by td->data().
     * LOCKING: the lock on the tile data should be taken
     *          by the caller before making a call.
     */
    void swapOutTileData(KisTileData *td);

    /**
     * Restore the data of a \a td basing on information
     * stored in the swap file.
     * LOCKING: the lock on the tile data should be taken
     *          by the caller before making a call.
     */
    void swapInTileData(KisTileData *td);

    /**
     * Forget all the information linked with the tile data.
     * This should be done before deleting of the tile data,
     * whose actual data is swapped-out
     */
    void forgetTileData(KisTileData *td);

    /**
     * Some debugging output
     */
    void debugStatistics();

private:
    QByteArray m_buffer;
    KisAbstractTileCompressor *m_compressor;

    KisChunkAllocator *m_allocator;
    KisMemoryWindow *m_swapSpace;

    QMutex m_lock;
};

#endif /* __KIS_SWAPPED_DATA_STORE_H */

