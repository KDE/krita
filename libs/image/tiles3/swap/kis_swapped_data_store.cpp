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

//#include "kis_debug.h"
#include "kis_swapped_data_store.h"
#include "kis_memory_window.h"
#include "kis_image_config.h"

#include "kis_tile_compressor_2.h"

//#define COMPRESSOR_VERSION 2

KisSwappedDataStore::KisSwappedDataStore()
    : m_memoryMetric(0)
{
    KisImageConfig config(true);
    const quint64 maxSwapSize = config.maxSwapSize() * MiB;
    const quint64 swapSlabSize = config.swapSlabSize() * MiB;
    const quint64 swapWindowSize = config.swapWindowSize() * MiB;

    m_allocator = new KisChunkAllocator(swapSlabSize, maxSwapSize);
    m_swapSpace = new KisMemoryWindow(config.swapDir(), swapWindowSize);

    // FIXME: use a factory after the patch is committed
    m_compressor = new KisTileCompressor2();
}

KisSwappedDataStore::~KisSwappedDataStore()
{
    delete m_compressor;
    delete m_swapSpace;
    delete m_allocator;
}

quint64 KisSwappedDataStore::numTiles() const
{
    // We are not acquiring the lock here...
    // Hope QLinkedList will ensure atomic access to it's size...

    return m_allocator->numChunks();
}

bool KisSwappedDataStore::trySwapOutTileData(KisTileData *td)
{
    Q_ASSERT(td->data());
    QMutexLocker locker(&m_lock);

    /**
     * We are expecting that the lock of KisTileData
     * has already been taken by the caller for us.
     * So we can modify the tile data freely.
     */

    const qint32 expectedBufferSize = m_compressor->tileDataBufferSize(td);
    if(m_buffer.size() < expectedBufferSize)
        m_buffer.resize(expectedBufferSize);

    qint32 bytesWritten;
    m_compressor->compressTileData(td, (quint8*) m_buffer.data(), m_buffer.size(), bytesWritten);

    KisChunk chunk = m_allocator->getChunk(bytesWritten);
    quint8 *ptr = m_swapSpace->getWriteChunkPtr(chunk);
    if (!ptr) {
        qWarning() << "swap out of tile failed";
        return false;
    }
    memcpy(ptr, m_buffer.data(), bytesWritten);

    td->releaseMemory();
    td->setSwapChunk(chunk);

    m_memoryMetric += td->pixelSize();

    return true;
}

void KisSwappedDataStore::swapInTileData(KisTileData *td)
{
    Q_ASSERT(!td->data());
    QMutexLocker locker(&m_lock);

    // see comment in swapOutTileData()

    KisChunk chunk = td->swapChunk();

    td->allocateMemory();
    td->setSwapChunk(KisChunk());

    quint8 *ptr = m_swapSpace->getReadChunkPtr(chunk);
    Q_ASSERT(ptr);
    m_compressor->decompressTileData(ptr, chunk.size(), td);
    m_allocator->freeChunk(chunk);

    m_memoryMetric -= td->pixelSize();
}

void KisSwappedDataStore::forgetTileData(KisTileData *td)
{
    QMutexLocker locker(&m_lock);

    m_allocator->freeChunk(td->swapChunk());
    td->setSwapChunk(KisChunk());

    m_memoryMetric -= td->pixelSize();
}

qint64 KisSwappedDataStore::totalMemoryMetric() const
{
    return m_memoryMetric;
}

void KisSwappedDataStore::debugStatistics()
{
    m_allocator->sanityCheck();
    m_allocator->debugFragmentation();
}
