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

#include "kis_debug.h"
#include "kis_chunk_allocator.h"


#define GAP_SIZE(low, high) ((high) - (low) > 0 ? (high) - (low) - 1 : 0)

#define HAS_NEXT(list,iter) ((iter)!=(list).end())
#define HAS_PREVIOUS(list,iter) ((iter)!=(list).begin())

#define PEEK_NEXT(iter) (*(iter))
#define PEEK_PREVIOUS(iter) (*((iter)-1))
#define WRAP_PREVIOUS_CHUNK_DATA(iter) (KisChunk((iter)-1))


KisChunkAllocator::KisChunkAllocator(quint64 slabSize, quint64 storeSize)
{
    m_storeMaxSize = storeSize;
    m_storeSlabSize = slabSize;

    m_iterator = m_list.begin();
    m_storeSize = m_storeSlabSize;
    INIT_FAIL_COUNTER();
}

KisChunkAllocator::~KisChunkAllocator()
{
}

KisChunk KisChunkAllocator::getChunk(quint64 size)
{
    KisChunkDataListIterator startPosition = m_iterator;
    START_COUNTING();

    forever {
        if(tryInsertChunk(m_list, m_iterator, size))
            return WRAP_PREVIOUS_CHUNK_DATA(m_iterator);

        if(m_iterator == m_list.end())
            break;

        m_iterator++;
        REGISTER_STEP();
    }

    REGISTER_FAIL();
    m_iterator = m_list.begin();

    forever {
        if(tryInsertChunk(m_list, m_iterator, size))
            return WRAP_PREVIOUS_CHUNK_DATA(m_iterator);

        if(m_iterator == m_list.end() || m_iterator == startPosition)
            break;

        m_iterator++;
        REGISTER_STEP();
    }

    REGISTER_FAIL();
    m_iterator = m_list.end();

    while ((m_storeSize += m_storeSlabSize) <= m_storeMaxSize) {
        if(tryInsertChunk(m_list, m_iterator, size))
            return WRAP_PREVIOUS_CHUNK_DATA(m_iterator);
    }

    qFatal("KisChunkAllocator: out of swap space");

    // just let gcc be happy! :)
    return KisChunk(m_list.end());
}

bool KisChunkAllocator::tryInsertChunk(KisChunkDataList &list,
                                       KisChunkDataListIterator &iterator,
                                       quint64 size)
{
    bool result = false;
    quint64 highBound = m_storeSize;
    quint64 lowBound = 0;
    quint64 shift = 0;

    if(HAS_NEXT(list, iterator))
        highBound = PEEK_NEXT(iterator).m_begin;

    if(HAS_PREVIOUS(list, iterator)) {
        lowBound = PEEK_PREVIOUS(iterator).m_end;
        shift = 1;
    }

    if(GAP_SIZE(lowBound, highBound) >= size) {
        list.insert(iterator, KisChunkData(lowBound + shift, size));
        result = true;
    }

    return result;
}

void KisChunkAllocator::freeChunk(KisChunk chunk)
{
    if(m_iterator != m_list.end() && m_iterator == chunk.position()) {
        m_iterator = m_list.erase(m_iterator);
        return;
    }

    Q_ASSERT(chunk.position()->m_begin == chunk.begin());
    m_list.erase(chunk.position());
}



/**************************************************************/
/*******             Debugging features                ********/
/**************************************************************/


void KisChunkAllocator::debugChunks()
{
    quint64 idx = 0;
    KisChunkDataListIterator i;

    for(i = m_list.begin(); i != m_list.end(); ++i) {
        qInfo("chunk #%lld: [%lld %lld]", idx++, i->m_begin, i->m_end);
    }
}

bool KisChunkAllocator::sanityCheck(bool pleaseCrash)
{
    bool failed = false;
    KisChunkDataListIterator i;

    for(i = m_list.begin(); i != m_list.end(); ++i) {
        if(HAS_PREVIOUS(m_list, i)) {
            if(PEEK_PREVIOUS(i).m_end >= i->m_begin) {
                qWarning("Chunks overlapped: [%lld %lld], [%lld %lld]", PEEK_PREVIOUS(i).m_begin, PEEK_PREVIOUS(i).m_end, i->m_begin, i->m_end);
                failed = true;
                break;
            }
        }
    }

    i = m_list.end();
    if(HAS_PREVIOUS(m_list, i)) {
        if(PEEK_PREVIOUS(i).m_end >= m_storeSize) {
            warnKrita << "Last chunk exceeds the store size!";
            failed = true;
        }
    }

    if(failed && pleaseCrash)
        qFatal("KisChunkAllocator: sanity check failed!");

    return !failed;
}

qreal KisChunkAllocator::debugFragmentation(bool toStderr)
{
    KisChunkDataListIterator i;

    quint64 totalSize = 0;
    quint64 allocated = 0;
    quint64 free = 0;
    qreal fragmentation = 0;

    for(i = m_list.begin(); i != m_list.end(); ++i) {
        allocated += i->m_end - i->m_begin + 1;

        if(HAS_PREVIOUS(m_list, i))
            free += GAP_SIZE(PEEK_PREVIOUS(i).m_end, i->m_begin);
        else
            free += i->m_begin;
    }

    i = m_list.end();
    if(HAS_PREVIOUS(m_list, i))
        totalSize = PEEK_PREVIOUS(i).m_end + 1;

    if(totalSize)
        fragmentation = qreal(free) / totalSize;

    if(toStderr) {
        qInfo() << "Hard store limit:\t" << m_storeMaxSize;
        qInfo() << "Slab size:\t\t" << m_storeSlabSize;
        qInfo() << "Num slabs:\t\t" << m_storeSize / m_storeSlabSize;
        qInfo() << "Store size:\t\t" << m_storeSize;
        qInfo() << "Total used:\t\t" << totalSize;
        qInfo() << "Allocated:\t\t" << allocated;
        qInfo() << "Free:\t\t\t" << free;
        qInfo() << "Fragmentation:\t\t" << fragmentation;
        DEBUG_FAIL_COUNTER();
    }

    Q_ASSERT(totalSize == allocated + free);

    return fragmentation;
}
