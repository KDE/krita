/*
 *  Copyright (c) 2007-2008 Bart Coppens <kde@bartcoppens.be>
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

#include <assert.h>
#include <kis_debug.h>

#include "kis_tile_global.h"

#include "kis_tile.h"
#include "kis_tilestore.h"
#include "kis_sharedtiledata.h"

KisSharedTileData::KisSharedTileData(KisTileStoreSP store, int tileSize, int pixelSize)
    : lock(QMutex::Recursive), tileSize(tileSize), pixelSize(pixelSize), store(store) {
    timesLockedInMemory = 0;
    data = 0;
    references = 0;
    deleteable = true;

    store->requestTileData(this);
    Q_CHECK_PTR(data);

    storeData = store->registerTileData(this);
}

KisSharedTileData::~KisSharedTileData() {
    store->dontNeedTileData(this);
    store->deregisterTileData(this);

    delete storeData;
    storeData = 0;
    data = 0;
}

void KisSharedTileData::addLockInMemory() {
    QMutexLocker lock(&this->lock);
    if (timesLockedInMemory++ == 0) {
        store->ensureTileLoaded(this); // Needs to come after the locking of the tile in memory!
    } else if (timesLockedInMemory < 0) {
        kDebug(41000) << timesLockedInMemory;
        assert(0);
    }
    assert(data);
}

void KisSharedTileData::removeLockInMemory() {
    QMutexLocker lock(&this->lock);
    assert(timesLockedInMemory >= 0);
    if (--timesLockedInMemory == 0) {
        lastUse = QTime::currentTime();

        if (timesLockedInMemory == 0) {
            store->maySwapTile(this);
        }
    }
}
