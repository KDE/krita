/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *            (c) 2009 Dmitry  Kazakov <dimula73@gmail.com>
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

//#define DEAD_TILES_SANITY_CHECK

#include "kis_tile_data.h"
#include "kis_tile_data_store.h"
#include "kis_tile.h"
#include "kis_memento_manager.h"
#include "kis_debug.h"


void KisTile::init(qint32 col, qint32 row,
                   KisTileData *defaultTileData, KisMementoManager* mm)
{
    m_col = col;
    m_row = row;
    m_lockCounter = 0;

    m_extent = QRect(m_col * KisTileData::WIDTH, m_row * KisTileData::HEIGHT,
                     KisTileData::WIDTH, KisTileData::HEIGHT);

    m_tileData = defaultTileData;
    m_tileData->acquire();

    if (mm) {
        mm->registerTileChange(this);
    }
    m_mementoManager.storeRelease(mm);
}

KisTile::KisTile(qint32 col, qint32 row,
                 KisTileData *defaultTileData, KisMementoManager* mm)
{
    init(col, row, defaultTileData, mm);
}

KisTile::KisTile(const KisTile& rhs, qint32 col, qint32 row, KisMementoManager* mm)
        : KisShared()
{
    init(col, row, rhs.tileData(), mm);
}

KisTile::KisTile(const KisTile& rhs, KisMementoManager* mm)
        : KisShared()
{
    init(rhs.col(), rhs.row(), rhs.tileData(), mm);
}

KisTile::KisTile(const KisTile& rhs)
        : KisShared()
{
    init(rhs.col(), rhs.row(), rhs.tileData(), rhs.m_mementoManager);
}

KisTile::~KisTile()
{
    Q_ASSERT(!m_lockCounter);

#ifdef DEAD_TILES_SANITY_CHECK
    /**
     * We should have been disconnected from the memento manager in notifyDetachedFromDataManager().
     * otherwise, there is a bug
     */
    Q_ASSERT(!m_mementoManager);
#endif

    m_tileData->release();
}

void KisTile::notifyDetachedFromDataManager()
{
    if (m_mementoManager.loadAcquire()) {
        KisMementoManager *manager = m_mementoManager;
        m_mementoManager.storeRelease(0);
        manager->registerTileDeleted(this);
    }
}

void KisTile::notifyDeadWithoutDetaching()
{
    m_mementoManager.storeRelease(0);
}

void KisTile::notifyAttachedToDataManager(KisMementoManager *mm)
{
    if (!m_mementoManager.loadAcquire()) {
        QMutexLocker locker(&m_COWMutex);

        if (!m_mementoManager.loadAcquire()) {

            if (mm) {
                mm->registerTileChange(this);
            }
            m_mementoManager.storeRelease(mm);
        }
    }
}

//#define DEBUG_TILE_LOCKING
//#define DEBUG_TILE_COWING

#ifdef DEBUG_TILE_LOCKING
#define DEBUG_LOG_ACTION(action)                                        \
    printf("### %s \ttile:\t0x%llX (%d, %d) (0x%llX) ###\n", action, (quintptr)this, m_col, m_row, (quintptr)m_tileData)
#else
#define DEBUG_LOG_ACTION(action)
#endif

#ifdef DEBUG_TILE_COWING
#define DEBUG_COWING(newTD)                                             \
    printf("### COW done \ttile:\t0x%X (%d, %d) (0x%X -> 0x%X) [mm: 0x%X] ###\n", (quintptr)this, m_col, m_row, (quintptr)m_tileData, (quintptr)newTD, m_mementoManager);
#else
#define DEBUG_COWING(newTD)
#endif

inline void KisTile::blockSwapping() const
{
    /**
     * We need to hold a specal barrier lock here to ensure
     * m_tileData->blockSwapping() has finished executing
     * before anyone started reading the tile data. That is
     * why we can not use atomic operations here.
     */

    QMutexLocker locker(&m_swapBarrierLock);
    Q_ASSERT(m_lockCounter >= 0);

    if(!m_lockCounter++)
        m_tileData->blockSwapping();

    Q_ASSERT(data());
}

inline void KisTile::unblockSwapping() const
{
    QMutexLocker locker(&m_swapBarrierLock);
    Q_ASSERT(m_lockCounter > 0);

    if(--m_lockCounter == 0) {
        m_tileData->unblockSwapping();

        if(!m_oldTileData.isEmpty()) {
            Q_FOREACH (KisTileData *td, m_oldTileData) {
                td->unblockSwapping();
                td->release();
            }
            m_oldTileData.clear();
        }
    }
}

inline void KisTile::safeReleaseOldTileData(KisTileData *td)
{
    QMutexLocker locker(&m_swapBarrierLock);
    Q_ASSERT(m_lockCounter >= 0);

    if(m_lockCounter > 0) {
        m_oldTileData.push(td);
    }
    else {
        td->unblockSwapping();
        td->release();
    }
}

void KisTile::lockForRead() const
{
    DEBUG_LOG_ACTION("lock [R]");
    blockSwapping();
}


#define lazyCopying() (m_tileData->m_usersCount>1)

void KisTile::lockForWrite()
{
    blockSwapping();

    /* We are doing COW here */
    if (lazyCopying()) {
        m_COWMutex.lock();

        /**
         * Everything could have happened before we took
         * the mutex, so let's check again...
         */

        if (lazyCopying()) {

            KisTileData *tileData = m_tileData->clone();
            tileData->acquire();
            tileData->blockSwapping();
            KisTileData *oldTileData = m_tileData;
            m_tileData = tileData;
            safeReleaseOldTileData(oldTileData);

            DEBUG_COWING(tileData);

            KisMementoManager *mm = m_mementoManager.load();
            if (mm) {
                mm->registerTileChange(this);
            }
        }
        m_COWMutex.unlock();
    }

    DEBUG_LOG_ACTION("lock [W]");
}

void KisTile::unlock() const
{
    unblockSwapping();
    DEBUG_LOG_ACTION("unlock");
}


#include <stdio.h>
void KisTile::debugPrintInfo()
{
    dbgTiles << "------\n"
                "Tile:\t\t\t" << this
                << "\n   data:\t" << m_tileData
                << "\n   next:\t" <<  m_nextTile.data();

}

void KisTile::debugDumpTile()
{
    lockForRead();
    quint8 *data = this->data();

    for (int i = 0; i < KisTileData::HEIGHT; i++) {
        for (int j = 0; j < KisTileData::WIDTH; j++) {
            dbgTiles << data[(i*KisTileData::WIDTH+j)*pixelSize()];
        }
    }
    unlock();
}
