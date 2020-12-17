/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SWAPPED_DATA_STORE_H
#define __KIS_SWAPPED_DATA_STORE_H

#include "kritaimage_export.h"

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
    bool trySwapOutTileData(KisTileData *td);

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
     * Retorns the metric of the total memory stored in the swap
     * in *uncompressed* form!
     */
    qint64 totalMemoryMetric() const;

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

    qint64 m_memoryMetric;
};

#endif /* __KIS_SWAPPED_DATA_STORE_H */

