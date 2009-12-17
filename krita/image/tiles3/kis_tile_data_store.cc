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
#include "kis_tile_data_store.h"
#include "kis_debug.h"

/**
 * TODO:
 * 1) Swapper thread
 * 2) [Done] Pool of prealloc'ed tiledata objects
 * 3) Compression before swapping
 */


KisTileDataStore globalTileDataStore;


#define tileListHead() (m_tileDataListHead)
#define tileListTail() (m_tileDataListHead->m_prevTD)
#define tileListEmpty() (!m_tileDataListHead)
#define tileListForEach(iter, first, last) for(iter=first; iter; iter=(iter==last ? 0 : iter->m_nextTD))

/* TODO: _|_ */
/*#define tileListForEachSafe(iter, first, last)        \
  for(iter=first; iter!=last ;iter=iter->nextTD;)
*/

void KisTileDataStore::tileListAppend(KisTileData *td)
{
    QWriteLocker lock(&m_listRWLock);
    if (!tileListEmpty()) {
        td->m_prevTD = tileListTail();
        td->m_nextTD = tileListHead();

        tileListTail()->m_nextTD = td;
        tileListHead()->m_prevTD = td;
    } else {
        td->m_prevTD = td->m_nextTD = td;
        tileListHead() = td;
    }
}

void KisTileDataStore::tileListDetach(KisTileData *td)
{
    QWriteLocker lock(&m_listRWLock);
    if (td != td->m_nextTD) {
        td->m_prevTD->m_nextTD = td->m_nextTD;
        td->m_nextTD->m_prevTD = td->m_prevTD;
        if (td == tileListHead())
            tileListHead() = td->m_nextTD;
    } else {
        /* List has the only element */
        tileListHead() = 0;
    }
}

void KisTileDataStore::tileListClear()
{
    QWriteLocker lock(&m_listRWLock);
    if (!tileListEmpty()) {
        KisTileData *tmp;
        while (!tileListEmpty()) {
            tmp = tileListHead();
            tileListDetach(tmp);
            delete tmp;
        }
    }
}

KisTileDataStore::KisTileDataStore()
        : m_pooler(this),
        m_listRWLock(QReadWriteLock::Recursive),
        m_tileDataListHead(0)
{
    m_pooler.start();
}

KisTileDataStore::~KisTileDataStore()
{
    m_pooler.terminatePooler();
    tileListClear();
}

KisTileData *KisTileDataStore::allocTileData(qint32 pixelSize, const quint8 *defPixel)
{
    KisTileData *td = new KisTileData(pixelSize, defPixel, this);

    tileListAppend(td);
    return td;
}


//#define DEBUG_PRECLONE

#ifdef DEBUG_PRECLONE
#define DEBUG_PRECLONE_ACTION(action, oldTD, newTD) \
    printf("!!! %s:\t\t\t  0x%X -> 0x%X    \t\t!!!\n",  \
           action, (quintptr)oldTD, (quintptr) newTD)
#define DEBUG_FREE_ACTION(td)                   \
    printf("Tile data free'd \t(0x%X)\n", td)
#else
#define DEBUG_PRECLONE_ACTION(action, oldTD, newTD)
#define DEBUG_FREE_ACTION(td)
#endif

KisTileData *KisTileDataStore::duplicateTileData(KisTileData *rhs)
{
    KisTileData *td;

    /* FIXME: race condition in QList */
    if (!rhs->m_clonesList.isEmpty()) {
        td = rhs->m_clonesList.takeFirst();
        DEBUG_PRECLONE_ACTION("+ Pre-clone HIT", rhs, td);
    } else {
        td = new KisTileData(*rhs);
        DEBUG_PRECLONE_ACTION("- Pre-clone #MISS#", rhs, td);
    }

    tileListAppend(td);
    return td;
}

void KisTileDataStore::freeTileData(KisTileData *td)
{
    Q_ASSERT(td->m_store == this);

    DEBUG_FREE_ACTION(td);

    tileListDetach(td);
    delete td;
}

void KisTileDataStore::ensureTileDataLoaded(const KisTileData *td)
{
    Q_UNUSED(td);
    /* Swapping isn't implemented yet */
    Q_ASSERT(td->m_state == KisTileData::NORMAL);
}


void KisTileDataStore::debugPrintList()
{
    KisTileData *iter;
    tileListForEach(iter, tileListHead(), tileListTail()) {
        dbgTiles << "-------------------------\n"
                 << "TileData:\t\t\t" << iter
                 << "\n  refCount:\t" << iter->m_refCount
                 << "\n  next:    \t" << iter->m_nextTD
                 << "\n  prev:    \t" << iter->m_prevTD;
    }
}
