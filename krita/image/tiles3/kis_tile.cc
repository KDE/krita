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

#include "kis_tile_data.h"
#include "kis_tile_data_store.h"
#include "kis_tile.h"
#include "kis_memento_manager.h"



void KisTile::init(qint32 col, qint32 row,
                   KisTileData *defaultTileData, KisMementoManager* mm)
{
    m_col = col;
    m_row = row;

    m_extent = QRect(m_col * KisTileData::WIDTH, m_row * KisTileData::HEIGHT,
                     KisTileData::WIDTH, KisTileData::HEIGHT);

    m_nextTile = 0;

    m_tileData = defaultTileData;
    globalTileDataStore.acquireTileData(m_tileData);

    m_mementoManager = mm;
}

KisTile::KisTile(qint32 col, qint32 row,
                 KisTileData *defaultTileData, KisMementoManager* mm)
        : m_lock(QMutex::Recursive)
{
    init(col, row, defaultTileData, mm);
}

KisTile::KisTile(const KisTile& rhs, qint32 col, qint32 row, KisMementoManager* mm)
        : KisShared(rhs),
        m_lock(QMutex::Recursive)
{
    init(col, row, rhs.tileData(), mm);
}

KisTile::KisTile(const KisTile& rhs, KisMementoManager* mm)
        : KisShared(rhs),
        m_lock(QMutex::Recursive)
{
    init(rhs.col(), rhs.row(), rhs.tileData(), mm);
}

KisTile::KisTile(const KisTile& rhs)
        : KisShared(rhs),
        m_lock(QMutex::Recursive)
{
    init(rhs.col(), rhs.row(), rhs.tileData(), rhs.m_mementoManager);
}

KisTile::~KisTile()
{
    if (m_mementoManager)
        m_mementoManager->registerTileDeleted(this);
    globalTileDataStore.releaseTileData(m_tileData);
}

//#define DEBUG_TILE_LOCKING
//#define DEBUG_TILE_COWING

#ifdef DEBUG_TILE_LOCKING
#define DEBUG_LOG_ACTION(action)                                        \
    printf("### %s \ttile:\t0x%X (%d, %d) (0x%X) ###\n", action, (quintptr)this, m_col, m_row, (quintptr)m_tileData)
#else
#define DEBUG_LOG_ACTION(action)
#endif

#ifdef DEBUG_TILE_COWING
#define DEBUG_COWING(newTD)                                             \
    printf("### COW done \ttile:\t0x%X (%d, %d) (0x%X -> 0x%X) [mm: 0x%X] ###\n", (quintptr)this, m_col, m_row, (quintptr)m_tileData, (quintptr)newTD, m_mementoManager);
#else
#define DEBUG_COWING(newTD)
#endif



void KisTile::lockForRead() const
{
    DEBUG_LOG_ACTION("lock [R]");
//    m_lock.lock();
    globalTileDataStore.ensureTileDataLoaded(m_tileData);
}


#define lazyCopying() (m_tileData->m_usersCount>1)

void KisTile::lockForWrite()
{
    /* We are doing COW here */
    if (lazyCopying()) {
        KisTileData *tileData = globalTileDataStore.duplicateTileData(m_tileData);
        DEBUG_COWING(tileData);
        globalTileDataStore.releaseTileData(m_tileData);
        m_tileData = tileData;
        globalTileDataStore.acquireTileData(m_tileData);
        if (m_mementoManager)
            m_mementoManager->registerTileChange(this);
    }

    DEBUG_LOG_ACTION("lock [W]");
//    m_lock.lock();
    globalTileDataStore.ensureTileDataLoaded(m_tileData);
}

void KisTile::unlock() const
{
    //DEBUG_LOG_ACTION("unlock");
//    m_lock.unlock();
}


#include <stdio.h>
void KisTile::debugPrintInfo()
{
    printf("------\n");
    printf("Tile:\t\t\t0x%X\n", (quintptr) this);
    printf("   data:\t0x%X\n", (quintptr) m_tileData);
    printf("   next:\t0x%X\n", (quintptr) m_nextTile.data());
}

void KisTile::debugDumpTile()
{
    lockForRead();
    quint8 *data = this->data();

    for (int i = 0; i < KisTileData::HEIGHT; i++) {
        for (int j = 0; j < KisTileData::WIDTH; j++) {
            printf("%4d ", data[(i*KisTileData::WIDTH+j)*pixelSize()]);
        }
        printf("\n");
    }
    unlock();
}
