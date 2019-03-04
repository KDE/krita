/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisOptimizedByteArray.h"

#include <QGlobalStatic>
#include <QMutexLocker>

#include <string.h>

namespace {

/*****************************************************************/
/*         DefaultMemoryAllocator                                */
/*****************************************************************/

struct DefaultMemoryAllocator : KisOptimizedByteArray::MemoryAllocator
{
    KisOptimizedByteArray::MemoryChunk alloc(int size) override {
        return KisOptimizedByteArray::MemoryChunk(new quint8[size], size);
    }

    void free(KisOptimizedByteArray::MemoryChunk chunk) override {
        // chunk.first might be null
        delete[] chunk.first;
    }
};


/*****************************************************************/
/*         DefaultMemoryAllocatorStore                           */
/*****************************************************************/

struct DefaultMemoryAllocatorStore {
    static DefaultMemoryAllocatorStore* instance();

    DefaultMemoryAllocatorStore()
        : m_allocator(new DefaultMemoryAllocator())
    {
    }

    inline KisOptimizedByteArray::MemoryAllocatorSP allocator() const {
        return m_allocator;
    }

private:
    KisOptimizedByteArray::MemoryAllocatorSP m_allocator;
};

Q_GLOBAL_STATIC(DefaultMemoryAllocatorStore, s_instance)

DefaultMemoryAllocatorStore *DefaultMemoryAllocatorStore::instance()
{
    return s_instance;
}

} // namespace


/*****************************************************************/
/*         KisOptimizedByteArray::PooledMemoryAllocator          */
/*****************************************************************/

KisOptimizedByteArray::PooledMemoryAllocator::PooledMemoryAllocator()
    : m_meanSize(500)
{
}

KisOptimizedByteArray::PooledMemoryAllocator::~PooledMemoryAllocator()
{
    Q_FOREACH (const MemoryChunk &chunk, m_chunks) {
        delete[] chunk.first;
    }
}

KisOptimizedByteArray::MemoryChunk
KisOptimizedByteArray::PooledMemoryAllocator::alloc(int size)
{
    MemoryChunk chunk;

    {
        QMutexLocker l(&m_mutex);
        if (!m_chunks.isEmpty()) {
            chunk = m_chunks.takeLast();
        }

        m_meanSize(size);
    }

    if (chunk.second < size) {
        delete[] chunk.first;

        // we alloc a bit more memory for the dabs to let the chunks
        // be more reusable
        const int allocSize = 1.2 * size;
        chunk = KisOptimizedByteArray::MemoryChunk(new quint8[allocSize], allocSize);
    }

    return chunk;
}

void KisOptimizedByteArray::PooledMemoryAllocator::free(KisOptimizedByteArray::MemoryChunk chunk)
{
    if (chunk.first) {
        QMutexLocker l(&m_mutex);

        // keep bigger chunks for ourselves and return the
        // smaller ones to the system
        if (chunk.second > 0.8 * m_meanSize.rollingMean()) {
            m_chunks.append(chunk);
        } else {
            delete[] chunk.first;
        }
    }
}


/*****************************************************************/
/*         KisOptimizedByteArray::Private                        */
/*****************************************************************/

struct KisOptimizedByteArray::Private : public QSharedData
{
    Private(MemoryAllocatorSP _allocator)
    {
        storedAllocator =
            _allocator ? _allocator : DefaultMemoryAllocatorStore::instance()->allocator();

        allocator = storedAllocator.data();
    }

    Private(const Private &rhs)
        : QSharedData(rhs)
    {
        allocator = rhs.allocator;
        storedAllocator = rhs.storedAllocator;
        dataSize = rhs.dataSize;
        if (dataSize) {
            data = allocator->alloc(dataSize);
            memcpy(data.first, rhs.data.first, dataSize);
        }
    }

    ~Private() {
        allocator->free(data);
    }

    MemoryAllocator *allocator;

    // stored allocator shared pointer is used only for keeping
    // the lifetime of the allocator until the detach of the last
    // allocated chunk
    MemoryAllocatorSP storedAllocator;

    MemoryChunk data;
    int dataSize = 0;
};


/*****************************************************************/
/*         KisOptimizedByteArray                                 */
/*****************************************************************/

KisOptimizedByteArray::KisOptimizedByteArray(MemoryAllocatorSP allocator)
    : m_d(new Private(allocator))
{
}

KisOptimizedByteArray::KisOptimizedByteArray(const KisOptimizedByteArray &rhs)
    : m_d(rhs.m_d)
{
}

KisOptimizedByteArray &KisOptimizedByteArray::operator=(const KisOptimizedByteArray &rhs)
{
    m_d = rhs.m_d;
    return *this;
}

KisOptimizedByteArray::~KisOptimizedByteArray()
{
}

quint8 *KisOptimizedByteArray::data()
{
    return const_cast<Private*>(m_d.data())->data.first;
}

const quint8 *KisOptimizedByteArray::constData() const
{
    return const_cast<const Private*>(m_d.constData())->data.first;
}

void KisOptimizedByteArray::resize(int size)
{
    if (size == m_d->dataSize) return;

    if (size > m_d->data.second) {
        m_d->allocator->free(m_d->data);
        m_d->data = m_d->allocator->alloc(size);
    }
    m_d->dataSize = size;
}

void KisOptimizedByteArray::fill(quint8 value, int size)
{
    resize(size);
    memset(m_d->data.first, value, m_d->dataSize);
}

int KisOptimizedByteArray::size() const
{
    return m_d->dataSize;
}

bool KisOptimizedByteArray::isEmpty() const
{
    return !m_d->dataSize;
}

KisOptimizedByteArray::MemoryAllocatorSP KisOptimizedByteArray::customMemoryAllocator() const
{
    return m_d->storedAllocator;
}

