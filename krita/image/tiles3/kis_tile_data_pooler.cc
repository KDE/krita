/*
 *  Copyright (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
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


#include <stdio.h>
#include "kis_tile_data.h"
#include "kis_tile_data_store.h"

#include "kis_tile_data_pooler.h"

const qint32 KisTileDataPooler::MAX_NUM_CLONES = 16;
const qint32 KisTileDataPooler::MAX_TIMEOUT = 60000; // 01m00s
const qint32 KisTileDataPooler::MIN_TIMEOUT = 100; // 00m00.100s
const qint32 KisTileDataPooler::TIMEOUT_FACTOR = 2;

//#define DEBUG_POOLER

#ifdef DEBUG_POOLER
#define DEBUG_CLONE_ACTION(td, numClones)                               \
    printf("Cloned (%d):\t\t\t\t0x%X (clones: %d, users: %d, refs: %d)\n", \
           numClones, td, td->m_clonesList.size(),                      \
           (int)td->m_usersCount, (int)td->m_refCount)
#define DEBUG_SIMPLE_ACTION(action)     \
    printf("pooler: %s\n", action)


#else
#define DEBUG_CLONE_ACTION(td, numClones)
#define DEBUG_SIMPLE_ACTION(action)
#endif

KisTileDataPooler::KisTileDataPooler(KisTileDataStore *store)
        : QThread()
{
    m_shouldExitFlag = 0;
    m_store = store;
    m_timeout = MIN_TIMEOUT;
    m_lastCycleHadWork = false;
}

KisTileDataPooler::~KisTileDataPooler()
{
}

void KisTileDataPooler::kick()
{
    m_semaphore.release();
}

void KisTileDataPooler::terminatePooler()
{
    m_shouldExitFlag = 1;
    kick();
    wait();
}

qint32 KisTileDataPooler::numClonesNeeded(KisTileData *td) const
{
    qint32 numUsers = td->m_usersCount;
    qint32 numPresentClones = td->m_clonesList.size();
    qint32 totalClones = qMin(numUsers - 1, MAX_NUM_CLONES);

    return totalClones - numPresentClones;
}

void KisTileDataPooler::cloneTileData(KisTileData *td, qint32 numClones) const
{
    if (numClones > 0) {
        for (qint32 i = 0; i < numClones; i++)
            td->m_clonesList.prepend(new KisTileData(*td));
    } else {
        qint32 numUnnededClones = qAbs(numClones);
        for (qint32 i = 0; i < numUnnededClones; i++)
            td->m_clonesList.removeFirst();
    }

    DEBUG_CLONE_ACTION(td, numClones);
}

#define tileListForEach(iter, first, last) for(iter=first; iter; iter=(iter==last ? 0 : iter->m_nextTD))
#define tileListForEachReverse(iter, last, first) for(iter=last; iter; iter=(iter==first ? 0 : iter->m_prevTD))
#define tileListHead() (m_store->m_tileDataListHead)
#define tileListTail() (m_store->m_tileDataListHead->m_prevTD)
#define tileListEmpty() (!m_store->m_tileDataListHead)

void KisTileDataPooler::waitForWork()
{
    bool success;

    if (m_lastCycleHadWork)
        success = m_semaphore.tryAcquire(1, m_timeout);
    else {
        m_semaphore.acquire();
        success = true;
    }

    m_lastCycleHadWork = false;
    if (success) {
        m_timeout = MIN_TIMEOUT;
    } else {
        m_timeout *= TIMEOUT_FACTOR;
        m_timeout = qMin(m_timeout, MAX_TIMEOUT);
    }
}

inline bool KisTileDataPooler::interestingTileData(KisTileData* td)
{
    /**
     * We have to look after all clones we created.
     * That is why we recheck all tiles with non-empty clones lists
     */

    return td->m_state == KisTileData::NORMAL &&
           (td->m_usersCount > 1 || !td->m_clonesList.isEmpty());
}

void KisTileDataPooler::run()
{
    while (1) {
        DEBUG_SIMPLE_ACTION("went to bed... Zzz...");

        waitForWork();

        if (m_shouldExitFlag)
            return;

        QThread::msleep(0);
        DEBUG_SIMPLE_ACTION("cycle started");

        m_store->m_listRWLock.lockForRead();
        KisTileData *iter;

        if (!tileListEmpty()) {
            tileListForEachReverse(iter, tileListTail(), tileListHead()) {
                if (interestingTileData(iter)) {

                    qint32 clonesNeeded = numClonesNeeded(iter);
                    if (clonesNeeded) {
                        m_lastCycleHadWork = true;
                        cloneTileData(iter, clonesNeeded);
                    }
                }
            }
        }
        m_store->m_listRWLock.unlock();

        DEBUG_SIMPLE_ACTION("cycle finished");
    }
}
