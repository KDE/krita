/*
 *  Copyright (c) 2005-2007 Bart Coppens <kde@bartcoppens.be>
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

#include "kis_tilestore.h"

#include <kis_debug.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <cassert>

#include "kis_tile.h"
#include "kis_sharedtiledata.h"

// Basic implementation of all functions (a swapless memory tilestore)

KisTileStore::KisTileStore()
        : m_lock(QMutex::Recursive)
{

}

KisTileStoreData* KisTileStore::registerTileData(const KisSharedTileData*)
{
    return 0;
} // ###

void KisTileStore::deregisterTileData(const KisSharedTileData*) {}

KisSharedTileData* KisTileStore::degradedTileDataForSharing(KisSharedTileData* data)
{
    return new KisSharedTileData(data->store, data->tileSize, data->pixelSize);
}

void KisTileStore::ensureTileLoaded(KisSharedTileData* tile) {}
void KisTileStore::maySwapTile(KisSharedTileData* tile) {}

void KisTileStore::requestTileData(KisSharedTileData* tileData)
{
    QMutexLocker dataLock(&(tileData->lock));
    tileData->data = new quint8[tileData->pixelSize * KisTile::WIDTH * KisTile::HEIGHT];
}

void KisTileStore::dontNeedTileData(KisSharedTileData* tileData)
{
    delete[] tileData->data;
}

void KisTileStore::configChanged() {}

// A mmap can be handy in every implementation

