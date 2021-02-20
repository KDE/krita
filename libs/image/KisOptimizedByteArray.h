/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
