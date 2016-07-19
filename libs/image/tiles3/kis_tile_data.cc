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


#include "kis_tile_data.h"
#include "kis_tile_data_store.h"

#include <kis_debug.h>

#include <boost/pool/singleton_pool.hpp>

// BPP == bytes per pixel
#define TILE_SIZE_4BPP (4 * __TILE_DATA_WIDTH * __TILE_DATA_HEIGHT)
#define TILE_SIZE_8BPP (8 * __TILE_DATA_WIDTH * __TILE_DATA_HEIGHT)

typedef boost::singleton_pool<KisTileData, TILE_SIZE_4BPP, boost::default_user_allocator_new_delete, boost::details::pool::default_mutex, 256, 4096> BoostPool4BPP;
typedef boost::singleton_pool<KisTileData, TILE_SIZE_8BPP, boost::default_user_allocator_new_delete, boost::details::pool::default_mutex, 128, 2048> BoostPool8BPP;

const qint32 KisTileData::WIDTH = __TILE_DATA_WIDTH;
const qint32 KisTileData::HEIGHT = __TILE_DATA_HEIGHT;


KisTileData::KisTileData(qint32 pixelSize, const quint8 *defPixel, KisTileDataStore *store)
    : m_state(NORMAL),
      m_mementoFlag(0),
      m_age(0),
      m_usersCount(0),
      m_refCount(0),
      m_pixelSize(pixelSize),
      m_store(store)
{
    m_store->checkFreeMemory();
    m_data = allocateData(m_pixelSize);

    fillWithPixel(defPixel);
}


/**
 * Duplicating tiledata
 * + new object loaded in memory
 * + it's unlocked and has refCount==0
 *
 * NOTE: the memory allocated by the pooler for clones is not counted
 * by the store in memoryHardLimit. The pooler has it's own slice of
 * memory and keeps track of the its size itself. So we should be able
 * to disable the memory check with checkFreeMemory, otherwise, there
 * is a deadlock.
 */
KisTileData::KisTileData(const KisTileData& rhs, bool checkFreeMemory)
    : m_state(NORMAL),
      m_mementoFlag(0),
      m_age(0),
      m_usersCount(0),
      m_refCount(0),
      m_pixelSize(rhs.m_pixelSize),
      m_store(rhs.m_store)
{
    if(checkFreeMemory) {
        m_store->checkFreeMemory();
    }
    m_data = allocateData(m_pixelSize);

    memcpy(m_data, rhs.data(), m_pixelSize * WIDTH * HEIGHT);
}


KisTileData::~KisTileData()
{
    releaseMemory();
}

void KisTileData::fillWithPixel(const quint8 *defPixel)
{
    quint8 *it = m_data;

    for (int i = 0; i < WIDTH*HEIGHT; i++, it += m_pixelSize) {
        memcpy(it, defPixel, m_pixelSize);
    }
}

void KisTileData::releaseMemory()
{
    if (m_data) {
        freeData(m_data, m_pixelSize);
        m_data = 0;
    }

    KisTileData *clone = 0;
    while(m_clonesStack.pop(clone)) {
        delete clone;
    }

    Q_ASSERT(m_clonesStack.isEmpty());
}

void KisTileData::allocateMemory()
{
    Q_ASSERT(!m_data);
    m_data = allocateData(m_pixelSize);
}

quint8* KisTileData::allocateData(const qint32 pixelSize)
{
    quint8 *ptr = 0;

    switch(pixelSize) {
    case 4:
        ptr = (quint8*)BoostPool4BPP::malloc();
        break;
    case 8:
        ptr = (quint8*)BoostPool8BPP::malloc();
        break;
    default:
        ptr = (quint8*) malloc(pixelSize * WIDTH * HEIGHT);
    }

    return ptr;
}

void KisTileData::freeData(quint8* ptr, const qint32 pixelSize)
{
    switch(pixelSize) {
    case 4:
        BoostPool4BPP::free(ptr);
        break;
    case 8:
        BoostPool8BPP::free(ptr);
        break;
    default:
        free(ptr);
    }
}

//#define DEBUG_POOL_RELEASE

#ifdef DEBUG_POOL_RELEASE
#include <unistd.h>
#endif /* DEBUG_POOL_RELEASE */

void KisTileData::releaseInternalPools()
{
    if (!KisTileDataStore::instance()->numTiles() &&
        !KisTileDataStore::instance()->numTilesInMemory()) {

        BoostPool4BPP::purge_memory();
        BoostPool8BPP::purge_memory();

#ifdef DEBUG_POOL_RELEASE
        dbgKrita << "After purging unused memory:";

        char command[256];
        sprintf(command, "cat /proc/%d/status | grep -i vm", (int)getpid());
        printf("--- %s ---\n", command);
        (void)system(command);
#endif /* DEBUG_POOL_RELEASE */

    }
//    else {
//        warnKrita << "WARNING: trying to purge pool memory while there are used tiles present!";
//        warnKrita << "         The memory will *NOT* be returned to the system, though it will";
//        warnKrita << "         be reused by Krita internally. Please report to developers!";
//    }
}
