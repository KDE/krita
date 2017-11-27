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

struct DefaultMemoryAllocator : KisOptimizedByteArray::MemoryAllocator
{
    KisOptimizedByteArray::MemoryChunk alloc(int size) override {
        return KisOptimizedByteArray::MemoryChunk(new quint8[size], size);
    }

    void free(KisOptimizedByteArray::MemoryChunk chunk) override {
        // chunk.first might be null
        delete[] chunk.first;
    }

    static DefaultMemoryAllocator* instance();
};

Q_GLOBAL_STATIC(DefaultMemoryAllocator, s_instance);

DefaultMemoryAllocator *DefaultMemoryAllocator::instance()
{
    return s_instance;
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
    }

    if (chunk.second < size) {
        delete[] chunk.first;
        chunk = KisOptimizedByteArray::MemoryChunk(new quint8[size], size);
    }

    return chunk;
}

void KisOptimizedByteArray::PooledMemoryAllocator::free(KisOptimizedByteArray::MemoryChunk chunk)
{
    if (chunk.first) {
        QMutexLocker l(&m_mutex);
        m_chunks.append(chunk);
    }
}

struct KisOptimizedByteArray::Private : public QSharedData
{
    Private(MemoryAllocator *allocator)
        : allocator(allocator ? allocator : DefaultMemoryAllocator::instance())
    {
    }

    Private(const Private &rhs)
        : QSharedData(rhs)
    {
        allocator = rhs.allocator;
        dataSize = rhs.dataSize;
        if (dataSize) {
            data = allocator->alloc(dataSize);
            memcpy(data.first, rhs.data.first, dataSize);
        }
    }

    ~Private() {
        allocator->free(data);
    }

    MemoryAllocator *allocator = 0;

    MemoryChunk data;
    int dataSize = 0;
};

KisOptimizedByteArray::KisOptimizedByteArray(MemoryAllocator *allocator)
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
    return m_d->dataSize;
}

KisOptimizedByteArray::MemoryAllocator* KisOptimizedByteArray::memoryAllocator() const
{
    return m_d->allocator;
}

