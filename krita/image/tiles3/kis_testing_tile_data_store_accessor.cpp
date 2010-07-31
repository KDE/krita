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

#include "kis_testing_tile_data_store_accessor.h"
#include "kis_tile_data_store_iterators.h"

KisTileDataStore* KisTestingTileDataStoreAccessor::getStore() {
    return &globalTileDataStore;
}

void KisTestingTileDataStoreAccessor::swapAll() {
    globalTileDataStore.debugSwapAll();
}

KisTileDataStoreIterator* KisTestingTileDataStoreAccessor::beginIteration() {
    return globalTileDataStore.beginIteration();
}

void KisTestingTileDataStoreAccessor::endIteration(KisTileDataStoreIterator* iterator) {
    return globalTileDataStore.endIteration(iterator);
}

KisTileDataStoreClockIterator* KisTestingTileDataStoreAccessor::beginClockIteration() {
    return globalTileDataStore.beginClockIteration();
}

void KisTestingTileDataStoreAccessor::endIteration(KisTileDataStoreClockIterator* iterator) {
    return globalTileDataStore.endIteration(iterator);
}

KisTileData* KisTestingTileDataStoreAccessor::allocTileData(qint32 pixelSize, const quint8 *defPixel) {
    return globalTileDataStore.allocTileData(pixelSize, defPixel);
}

void KisTestingTileDataStoreAccessor::freeTileData(KisTileData *td) {
    globalTileDataStore.freeTileData(td);
}

void KisTestingTileDataStoreAccessor::clear() {
    globalTileDataStore.debugClear();
}

qint32  KisTestingTileDataStoreAccessor::numTiles() {
    return globalTileDataStore.numTiles();
}
