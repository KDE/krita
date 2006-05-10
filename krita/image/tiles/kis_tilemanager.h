/*
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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
#ifndef KIS_TILEMANAGER_H_
#define KIS_TILEMANAGER_H_

#include <qglobal.h>
#include <QMap>
#include <q3valuelist.h>
#include <qmutex.h>

#include <ktempfile.h>
#include <krita_export.h>

class KisTile;
class KisTiledDataManager;

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
 */
class KRITAIMAGE_EXPORT KisTileManager  {
public:
    ~KisTileManager();
    static KisTileManager* instance();

public: // Tile management
    void registerTile(KisTile* tile);
    void deregisterTile(KisTile* tile);
    void ensureTileLoaded(KisTile* tile);
    void maySwapTile(KisTile* tile);

public: // Pool management
    quint8* requestTileData(qint32 pixelSize);
    void dontNeedTileData(quint8* data, qint32 pixelSize);

public: // Configuration
    void configChanged();

private:
    KisTileManager();
    KisTileManager(KisTileManager&) {}
    KisTileManager operator=(const KisTileManager&);

private:
    static KisTileManager *m_singleton;
    KTempFile m_tempFile;
    int m_fileSize;
    // For use when any swap-allocating function failed; the risk of swap allocating failing
    // again is too big, and we'd clutter the logs with kWarnings otherwise
    bool m_swapForbidden;

    struct TileInfo { KisTile *tile; bool inMem; int filePos; int size; int fsize;
        bool validNode; Q3ValueList<TileInfo*>::iterator node; };
    typedef struct { quint8 *pointer; int filePos; int size; } FreeInfo;
    typedef QMap<KisTile*, TileInfo*> TileMap;
    typedef Q3ValueList<TileInfo*> TileList;
    typedef Q3ValueList<FreeInfo*> FreeList;
    typedef Q3ValueVector<FreeList> FreeListList;
    typedef Q3ValueList<quint8*> PoolFreeList;

    TileMap m_tileMap;
    TileList m_swappableList;
    FreeListList m_freeLists;
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
    QMutex * m_poolMutex;
    QMutex * m_swapMutex;

    // debug
    int counter;

private:
    void fromSwap(TileInfo* info);
    void toSwap(TileInfo* info);
    void doSwapping();
    void printInfo();
    quint8* findTileFor(qint32 pixelSize);
    bool isPoolTile(quint8* data, qint32 pixelSize);
    void reclaimTileToPool(quint8* data, qint32 pixelSize);

};

#endif // KIS_TILEMANAGER_H_
