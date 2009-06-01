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


#define lazyCopying() (m_tileData->m_usersCount>1)
#define activateLazyCopying() do { m_tileData=m_defaultTileData; \
                                   globalTileDataStore.acquireTileData(m_tileData);} while(0)

void KisTile::init(qint32 col, qint32 row, KisTileData *defaultTileData)
{
    m_col=col;
    m_row=row;

    m_nextTile=0;

    globalTileDataStore.acquireTileData(defaultTileData);
    m_defaultTileData = defaultTileData;

    activateLazyCopying();
}

KisTile::KisTile(qint32 col, qint32 row, KisTileData *defaultTileData)
{
    init(col, row, defaultTileData);
}

KisTile::KisTile(qint32 col, qint32 row, const KisTile& rhs)
    : KisShared(rhs)
{
    init(col, row, rhs.tileData());
}

KisTile::KisTile(const KisTile& rhs)
    : KisShared(rhs)
{
    init(rhs.col(), rhs.row(), rhs.tileData());
}

KisTile::~KisTile()
{
    globalTileDataStore.releaseTileData(m_tileData);
    globalTileDataStore.releaseTileData(m_defaultTileData);
}


void KisTile::lockForRead() const
{
    m_tileData->m_RWLock.lockForRead();
    globalTileDataStore.ensureTileDataLoaded(m_tileData);
}

void KisTile::lockForWrite()
{
    /* We are doing COW here */
    if(lazyCopying()) {
        globalTileDataStore.releaseTileData(m_tileData);
        m_tileData = globalTileDataStore.duplicateTileData(m_defaultTileData);
        globalTileDataStore.acquireTileData(m_tileData);
    }
    m_tileData->m_RWLock.lockForWrite();
    globalTileDataStore.ensureTileDataLoaded(m_tileData);
}

void KisTile::unlock() const
{
    m_tileData->m_RWLock.unlock();
}


#include <stdio.h>
void KisTile::debugPrintInfo()
{
    printf("------\n");
    printf("Tile:\t\t\t0x%X\n", (quintptr) this);
    printf("   data:\t0x%X\n", (quintptr) m_tileData);
    printf("   def. data:\t0x%X\n", (quintptr) m_defaultTileData);
    printf("   next:\t0x%X\n", (quintptr) m_nextTile.data());
}

void KisTile::debugDumpTile()
{
    lockForRead();
    quint8 *data = this->data();

    for(int i=0; i<KisTileData::HEIGHT; i++) {
        for(int j=0; j<KisTileData::WIDTH; j++) {
            printf("%4d ", data[ (i*KisTileData::WIDTH+j)*pixelSize() ]);
        }
        printf("\n");
     }
     unlock();
}
