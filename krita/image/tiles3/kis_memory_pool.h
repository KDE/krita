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

#ifndef __KIS_MEMORY_POOL_H
#define __KIS_MEMORY_POOL_H

#include "stdlib.h"

//#define DEBUG_HIT_MISS

#ifdef DEBUG_HIT_MISS

#include "kis_debug.h"
#define DECLARE_STATUS_VAR() qint64 m_hit; qint64 m_miss; qint64 m_averIndex; qint32 m_maxIndex
#define INIT_STATUS_VAR() m_hit=0; m_miss=0; m_averIndex=0; m_maxIndex=0
#define POOL_MISS() m_miss++
#define POOL_HIT(idx) m_hit++; m_averIndex+=idx; m_maxIndex=qMax(m_maxIndex, idx)
#define REPORT_STATUS() qDebug() << ppVar(m_hit) << ppVar(m_miss) << "Hit rate:" << double(m_hit)/(m_hit+m_miss) << "Max index:" << m_maxIndex <<"Av. index:" << double(m_averIndex)/(m_hit)

#else

#define DECLARE_STATUS_VAR()
#define INIT_STATUS_VAR()
#define POOL_MISS()
#define POOL_HIT(idx)
#define REPORT_STATUS()

#endif

/**
 * Memory pool object for fast allocation of big objects.
 * Please, don't even try to use it for small objects -
 * it's pointless. glibc will be faster anyway.
 *
 * What is small and what is big object? On my machine
 * (2GiB of RAM) glibc's memory pool seemed to be about
 * 128-256KiB. So if the total size of constantly
 * allocated/deallocated objects grows higher 128KiB,
 * you need to use this pool.
 *
 * In tests, there is no big difference between pools of
 * size 32 and of size 64.
 *
 * How does it work? Very simple. The pool has an array
 * of atomic pointers to memory chunks. When memory request
 * comes, it searches in the array  for the first non-zero
 * pointer and rerurns it. When the pointer is returned back -
 * the pool writes it to the first free slot.
 */

template<class T, int SIZE>
class KisMemoryPool
{
public:
    KisMemoryPool() {
        INIT_STATUS_VAR();
    }

    ~KisMemoryPool() {
        for(qint32 i = 0; i < SIZE; i++) {
            free(m_array[i]);
        }
        REPORT_STATUS();
    }

    inline void push(void *ptr) {
        if(m_allocated < SIZE) {
            for(qint32 i = 0; i < SIZE; i++) {
                if(m_array[i].testAndSetOrdered(0, ptr)) {
                    m_allocated.ref();
                    return;
                }
            }
        }
        free(ptr);
    }

    inline void* pop() {
        if(m_allocated > 0) {
            void *ptr;
            for(qint32 i = 0; i < SIZE; i++) {
                ptr = m_array[i].fetchAndStoreOrdered(0);
                if (ptr) {
                    m_allocated.deref();
                    POOL_HIT(i);
                    return ptr;
                }
            }
        }
        POOL_MISS();
        return malloc(sizeof(T));
    }

private:
    QAtomicPointer<void> m_array[SIZE];
    QAtomicInt m_allocated;
    DECLARE_STATUS_VAR()
};


#define POOL_OPERATORS(T)                                               \
    void* operator new(size_t size) {                                   \
        return size == sizeof(T) ? __m_pool.pop() : malloc(size);       \
    }                                                                   \
    void operator delete(void *ptr, size_t size) {                      \
        if(size == sizeof(T)) __m_pool.push(ptr);                       \
        else free(ptr);                                                 \
    }

#define DECLARE_POOL(T,SIZE)                    \
    static KisMemoryPool<T,SIZE> __m_pool

#define DEFINE_POOL(T,SIZE)                     \
    KisMemoryPool<T,SIZE> T::__m_pool




#endif /* __KIS_MEMORY_POOL_H */

