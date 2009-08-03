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
#ifndef KIS_TILESTOREMEMORY_H_
#define KIS_TILESTOREMEMORY_H_

#include <sys/types.h>

#include <qglobal.h>
#include <QHash>
#include <QLinkedList>
#include <QVector>
#include <QMutex>

#include <ktemporaryfile.h>
#include <krita_export.h>

#include "kis_tilestore.h"

//#define DEBUG_OWN_TILES

#include "kis_tile.h"

class KisSharedTileData;

//#define TILESTOREDEBUGGING

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
class KRITAIMAGE_EXPORT KisTileStoreMemory : public KisTileStore
{
public:
    virtual ~KisTileStoreMemory();

public: // Tile management

    KisTileStoreData* registerTileData(const KisSharedTileData* tile);

    void deregisterTileData(const KisSharedTileData* tile); // Deletes its TileInfo*

    // these can change the tile indirectly, though, through the actual swapping!
    void ensureTileLoaded(KisSharedTileData* tile);

    void maySwapTile(KisSharedTileData* tile);

public: // Pool management

    void requestTileData(KisSharedTileData* tileData);
    void dontNeedTileData(KisSharedTileData* tileData);

public: // Configuration

    void configChanged();

private:
    KisTileStoreMemory();
    KisTileStoreMemory(KisTileStoreMemory&); // : KisTileStore(rhs) {}
    KisTileStoreMemory operator=(const KisTileStoreMemory&);

private:
    // ### Rewrite this explanation (or scrap it)
    // validNode says if you can swap it (true) or not (false) mmapped, if this tile
    // currently is memory mapped. If it is false, but onFile, it is on disk,
    // but not mmapped, and should be mapped!
    // filePos is the position inside the file; size is the actual size, fsize is the size
    // being used in the swap for this tile (may be larger!)
    // The file points to 0 if it is not swapped, and to the relevant TempFile otherwise
    struct SharedDataMemoryInfo : public KisTileStoreData {
        // No mutex, it should be locked from the SharedData already!

        // Swap info
        KTemporaryFile* file;
        off_t filePos;

        QLinkedList<KisSharedTileData*>::iterator node; // For swappablelist (not const, since we have to erase)

        // Data state:

        bool isSwappable;
        bool isInSwappableList; // Meh! Looks ugly

        bool inMem;            // Tile is currently loaded in memory
        bool onFile;           // Tile has a corresponding on-disk data structure
        bool compressedOnFile; // Tile's structure on-disk is compressed
    };

    typedef QList<quint8*> PoolFreeList;

    qint32 m_maxInMem;
    qint32 m_currentInMem;
    quint32 m_swappiness;
    qint32 m_tileSize; // size of a tile if it used 1 byte per pixel
    unsigned long m_bytesInMem;
    unsigned long m_bytesTotal;

    quint8 **m_pools;
    qint32 *m_poolPixelSizes;
    qint32 m_tilesPerPool;
    PoolFreeList *m_poolFreeList;

    // debug
    int counter;

private:
    void printInfo();
    quint8* findTileFor(qint32 pixelSize);
    bool isPoolTile(quint8* data, qint32 pixelSize);
    void reclaimTileToPool(quint8* data, qint32 pixelSize);

    friend class KisTileSwapper;
    friend KisTileStoreSP defaultTileStore();

#ifdef TILESTOREDEBUGGING
    static int instances;
#endif
};

#endif // KIS_TILEMANAGER_H_
