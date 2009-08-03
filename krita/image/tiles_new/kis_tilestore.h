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
#ifndef KIS_TILESTORE_H_
#define KIS_TILESTORE_H_

#include <sys/types.h>

#include <qglobal.h>
#include <QHash>
#include <QList>
#include <QVector>
#include <QMutex>

#include "krita_export.h"

#include "kis_shared.h"

class KisSharedTileData;

struct KRITAIMAGE_EXPORT KisTileStoreData { // used to be struct KisTileStore::SharedDataInfo
    virtual ~KisTileStoreData() {};
};

/**
 * This class keeps has the intention to make certain tile-related operations faster or more
 * efficient. It does this by keeping lots of info on KisTiles, and manages the way they are
 * created, used, etc.
 * It mainly does the following more visible things
 *  * provide a way to store tiles on disk to a swap file, to reduce memory usage
 *  * keep a list of previously swapped (but now unused) tiles, to reuse these when we want
 *    to swap new tiles.
 *  * tries to preallocate and recycle some tiles to make future allocations faster
 *    (not done yet)
 *
 *    XXX: Use QReadWriteLock, QReadLocker and QWriteLocker to make sure everyone can
 *    read any tile, as long as nobody is writing to it. (bsar)
 *    See: http://doc.trolltech.com/qq/qq14-threading.html
 */
class KRITAIMAGE_EXPORT KisTileStore : public virtual KisShared
{
public:
    virtual ~KisTileStore() {}

public: // Tile management
    // ### Perhaps add sth like hasCapability(TileDataSharing), to check if the backend can support sharing tiles?
    virtual KisTileStoreData* registerTileData(const KisSharedTileData* tileData);
    virtual void deregisterTileData(const KisSharedTileData* tile);

    virtual KisSharedTileData* degradedTileDataForSharing(KisSharedTileData* tileData); // Might change m_store ! locked!

    // these can change the tile indirectly, though, through the actual swapping!
    virtual void ensureTileLoaded(KisSharedTileData* tile);
    virtual void maySwapTile(KisSharedTileData* tile);

public: // Pool management
    virtual void requestTileData(KisSharedTileData* tileData);
    virtual void dontNeedTileData(KisSharedTileData* tileData);

public: // Configuration
    virtual void configChanged();

public:
    virtual void lock() {
        m_lock.lock();
    }
    virtual void unlock() {
        m_lock.unlock();
    }

protected:
    QMutex m_lock;

    // Mmap wrapper that prints warnings on error. The result is stored in the *& result
    // the return value is true on succes, false on failure. Other args as in man mmap
    bool kritaMmap(quint8*& result, void *start, size_t length,
                   int prot, int flags, int fd, off_t offset);

    KisTileStore(); // Protected, since we need to be able to construct it from subclasses!
private:
    KisTileStore(KisTileStore&);
    KisTileStore operator=(const KisTileStore&);

    friend class KisTile;
};

typedef KisSharedPtr<KisTileStore> KisTileStoreSP;

KisTileStoreSP defaultTileStore(); // ### Do this in a better way!

//typedef QHash<const KisTile*, KisTileStore::TileInfo*> TileMap;
//typedef QList<KisTileStore::TileInfo*> TileList;

#endif // KIS_TILESTORE_H_
