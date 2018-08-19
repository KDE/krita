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

#ifndef __KIS_CHUNK_LIST_H
#define __KIS_CHUNK_LIST_H

#include <QLinkedList>
#include "kritaimage_export.h"

#define MiB (1ULL << 20)

#define DEFAULT_STORE_SIZE (4096*MiB)
#define DEFAULT_SLAB_SIZE (64*MiB)


//#define DEBUG_SLAB_FAILS

#ifdef DEBUG_SLAB_FAILS

#define WINDOW_SIZE 2000
#define DECLARE_FAIL_COUNTER() quint64 __failCount
#define INIT_FAIL_COUNTER() __failCount = 0
#define START_COUNTING() quint64 __numSteps = 0
#define REGISTER_STEP() if(++__numSteps > WINDOW_SIZE) {__numSteps=0; __failCount++;}
#define REGISTER_FAIL() __failCount++
#define DEBUG_FAIL_COUNTER() qInfo() << "Slab fail count:\t" << __failCount

#else

#define DECLARE_FAIL_COUNTER()
#define INIT_FAIL_COUNTER()
#define START_COUNTING()
#define REGISTER_STEP()
#define REGISTER_FAIL()
#define DEBUG_FAIL_COUNTER()

#endif /* DEBUG_SLAB_FAILS */



class KisChunkData;

typedef QLinkedList<KisChunkData> KisChunkDataList;
typedef KisChunkDataList::iterator KisChunkDataListIterator;

class KRITAIMAGE_EXPORT KisChunkData
{
public:
    KisChunkData(quint64 begin, quint64 size)
    {
        setChunk(begin, size);
    }

    inline void setChunk(quint64 begin, quint64 size) {
        m_begin = begin;
        m_end = begin + size - 1;
    }

    inline quint64 size() const {
        return m_end - m_begin +1;
    }

    bool operator== (const KisChunkData& other) const
    {
        Q_ASSERT(m_begin!=other.m_begin || m_end==other.m_end);

        /**
         * Chunks cannot overlap, so it is enough to check
         * the beginning of the interval only
         */
        return m_begin == other.m_begin;
    }

    quint64 m_begin;
    quint64 m_end;
};

class KRITAIMAGE_EXPORT KisChunk
{
public:
    KisChunk() {}

    KisChunk(KisChunkDataListIterator iterator)
        : m_iterator(iterator)
    {
    }

    inline quint64 begin() const {
        return m_iterator->m_begin;
    }

    inline quint64 end() const {
        return m_iterator->m_end;
    }

    inline quint64 size() const {
        return m_iterator->size();
    }

    inline KisChunkDataListIterator position() {
        return m_iterator;
    }

    inline const KisChunkData& data() {
        return *m_iterator;
    }

private:
    KisChunkDataListIterator m_iterator;
};


class KRITAIMAGE_EXPORT KisChunkAllocator
{
public:
    KisChunkAllocator(quint64 slabSize = DEFAULT_SLAB_SIZE,
                      quint64 storeSize = DEFAULT_STORE_SIZE);
    ~KisChunkAllocator();

    inline quint64 numChunks() const {
        return m_list.size();
    }

    KisChunk getChunk(quint64 size);
    void freeChunk(KisChunk chunk);

    void debugChunks();
    bool sanityCheck(bool pleaseCrash = true);
    qreal debugFragmentation(bool toStderr = true);

private:
    bool tryInsertChunk(KisChunkDataList &list,
                        KisChunkDataListIterator &iterator,
                        quint64 size);

private:
    quint64 m_storeMaxSize;
    quint64 m_storeSlabSize;


    KisChunkDataList m_list;
    KisChunkDataListIterator m_iterator;
    quint64 m_storeSize;
    DECLARE_FAIL_COUNTER()
};

#endif /* __KIS_CHUNK_ALLOCATOR_H */

