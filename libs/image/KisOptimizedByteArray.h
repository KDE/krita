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

#ifndef KISOPTIMIZEDBYTEARRAY_H
#define KISOPTIMIZEDBYTEARRAY_H

#include <utility>
#include <QSharedDataPointer>
#include <QSharedPointer>
#include <QMutex>
#include <QVector>

#include "kritaimage_export.h"

#include "KisRollingMeanAccumulatorWrapper.h"


class KRITAIMAGE_EXPORT KisOptimizedByteArray
{
public:
    typedef std::pair<quint8*, int> MemoryChunk;

    struct KRITAIMAGE_EXPORT MemoryAllocator {
        virtual ~MemoryAllocator() = default;
        virtual MemoryChunk alloc(int size) = 0;
        virtual void free(MemoryChunk chunk) = 0;
    };

    typedef QSharedPointer<MemoryAllocator> MemoryAllocatorSP;

    struct KRITAIMAGE_EXPORT PooledMemoryAllocator : public MemoryAllocator {
        PooledMemoryAllocator();
        ~PooledMemoryAllocator();

        MemoryChunk alloc(int size) override;
        void free(MemoryChunk chunk) override;

    private:
        QMutex m_mutex;
        QVector<MemoryChunk> m_chunks;
        KisRollingMeanAccumulatorWrapper m_meanSize;
    };

public:
    KisOptimizedByteArray(MemoryAllocatorSP allocator = MemoryAllocatorSP());
    KisOptimizedByteArray(const KisOptimizedByteArray &rhs);
    KisOptimizedByteArray& operator=(const KisOptimizedByteArray &rhs);

    ~KisOptimizedByteArray();

     quint8* data();
     const quint8* constData() const;

     void resize(int size);
     void fill(quint8 value, int size = -1);

     int size() const;

     bool isEmpty() const;

     MemoryAllocatorSP customMemoryAllocator() const;

private:
     struct Private;
     QSharedDataPointer<Private> m_d;
};

#endif // KISOPTIMIZEDBYTEARRAY_H
