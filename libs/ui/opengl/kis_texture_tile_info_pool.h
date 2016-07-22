/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_TEXTURE_TILE_INFO_POOL_H
#define __KIS_TEXTURE_TILE_INFO_POOL_H

#include <boost/pool/pool.hpp>
#include <QtGlobal>
#include <QVector>

#include <QMutex>
#include <QMutexLocker>
#include <QSharedPointer>

#include "kis_assert.h"
#include "kis_debug.h"
#include "kis_global.h"

const int minPoolChunk = 32; // 8 MiB (default, with tilesize 256)
const int maxPoolChunk = 128; // 32 MiB (default, with tilesize 256)
const int freeThreshold = 64; // 16 MiB (default, with tilesize 256)


/**
 * A pool for keeping the chunks of data of constant size. We have one
 * such pool per used openGL tile size. The size of the chunk
 * obviously depends on the size of the tile in pixels and the size of
 * a single pixel in bytes.
 *
 * As soon as the number of allocations drops to zero, all the memory
 * is returned back to the operating system. Please note, that there
 * is *no way* of reclaiming even unused pool memory untill *all* the
 * allocated chunks are free'd.
 */
class KisTextureTileInfoPoolSingleSize
{
public:
    KisTextureTileInfoPoolSingleSize(int tileWidth, int tileHeight, int pixelSize)
        : m_chunkSize(tileWidth * tileHeight * pixelSize),
          m_pool(m_chunkSize, minPoolChunk, maxPoolChunk),
          m_numAllocations(0),
          m_maxAllocations(0)
    {
    }

    quint8* malloc() {
        m_numAllocations++;
        m_maxAllocations = qMax(m_maxAllocations, m_numAllocations);

        return (quint8*)m_pool.malloc();
    }

    void free(quint8 *ptr) {
        m_numAllocations--;
        m_pool.free(ptr);

        KIS_ASSERT_RECOVER_NOOP(m_numAllocations >= 0);

        if (!m_numAllocations && m_maxAllocations > freeThreshold) {
            // qDebug() << "Purging memory" << ppVar(m_maxAllocations);
            m_pool.purge_memory();
            m_maxAllocations = 0;
        }
    }

    int chunkSize() const {
        return m_chunkSize;
    }

private:
    const int m_chunkSize;
    boost::pool<boost::default_user_allocator_new_delete> m_pool;
    int m_numAllocations;
    int m_maxAllocations;
};

/**
 * A universal pool for keeping the openGL tile of different pixel
 * sizes.  The underlying pools are created for each pixel size on
 * demand.
 */
class KisTextureTileInfoPool
{
public:
    KisTextureTileInfoPool(int tileWidth, int tileHeight)
        : m_tileWidth(tileWidth),
          m_tileHeight(tileHeight)
    {
    }

    ~KisTextureTileInfoPool() {
        qDeleteAll(m_pools);
    }

    /**
     * Alloc a tile with the specified pixel size
     */
    quint8* malloc(int pixelSize) {
        QMutexLocker l(&m_mutex);

        if (m_pools.size() <= pixelSize) {
            m_pools.resize(pixelSize + 1);
        }

        if (!m_pools[pixelSize]) {
            m_pools[pixelSize] =
                new KisTextureTileInfoPoolSingleSize(m_tileWidth, m_tileHeight, pixelSize);
        }

        return m_pools[pixelSize]->malloc();
    }

    /**
     * Free a tile with the specified pixel size
     */
    void free(quint8 *ptr, int pixelSize) {
        QMutexLocker l(&m_mutex);
        m_pools[pixelSize]->free(ptr);
    }

    /**
     * \return the length of the chunks stored in the pool
     */
    int chunkSize(int pixelSize) const {
        QMutexLocker l(&m_mutex);
        return m_pools[pixelSize]->chunkSize();
    }

private:
    mutable QMutex m_mutex;
    const int m_tileWidth;
    const int m_tileHeight;
    QVector<KisTextureTileInfoPoolSingleSize*> m_pools;
};

typedef QSharedPointer<KisTextureTileInfoPool> KisTextureTileInfoPoolSP;

class KisTextureTileInfoPoolRegistry
{
    typedef QWeakPointer<KisTextureTileInfoPool> KisTextureTileInfoPoolWSP;
    typedef QPair<int, int> PoolId;

public:
    KisTextureTileInfoPoolSP getPool(int tileWidth, int tileHeight) {
        QMutexLocker l(&m_mutex);

        PoolId id(tileWidth, tileHeight);

        KisTextureTileInfoPoolSP pool =
            m_storage[id].toStrongRef();

        if (!pool) {
            pool = toQShared(
                new KisTextureTileInfoPool(tileWidth, tileHeight));
            m_storage[id] = pool;
        }

        return pool;
    }

private:
    QMutex m_mutex;
    QHash<PoolId, KisTextureTileInfoPoolWSP> m_storage;
};


#endif /* __KIS_TEXTURE_TILE_INFO_POOL_H */
