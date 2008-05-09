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
 *  You should have received a copy of t#include "kis_tile.h"
he GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_SHARED_TILE_DATA_H_
#define KIS_SHARED_TILE_DATA_H_

#include <qglobal.h>
#include <QRect>
#include <QMutex>
#include <QTime>
#include <QVector>

#include "krita_export.h"
#include "kis_tilestore.h"

class KisTileStoreData;

class KisTiledDataManager;
class KisTiledIterator;
class KisTile;

/**
 * The data that can be shared amongst different KisTiles
 */
struct KRITAIMAGE_EXPORT KisSharedTileData {
    typedef int TimeDiffType;
    typedef QTime TimeType;

    KisSharedTileData(KisTileStoreSP store, int tileSize, int pixelSize);
    ~KisSharedTileData();

    void addLockInMemory();
    void removeLockInMemory();

    mutable QMutex lock; // Lock when changing this data (swapping, changing the *locked* status of a dependant tile, de-sharing, etc)

    int tileSize;
    int pixelSize; // Unneeded except at construction time, could be better! :(
    quint8* data;

    // Make these QAtomicInts?
    int references; // unrelated to timesLockedInMemory. Does not include swap references
    int timesLockedInMemory;
    bool deleteable; // In case references == 0, can it be deleted from the tile itself (if false, it's probably still referenced in a swapper or so

    KisTileStoreSP store;
    KisTileStoreData* storeData;

        // The last use time is *shared* amongst the different sharers!
    TimeType lastUse;
};

#endif // KIS_SHARED_TILE_DATA_H_

