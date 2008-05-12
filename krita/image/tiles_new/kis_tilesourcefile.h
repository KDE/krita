/*
 *  Copyright (c) 2008 Bart Coppens <kde@bartcoppens.be>
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
#ifndef KIS_TILESOURCEFILE_H_
#define KIS_TILESOURCEFILE_H_

#include <QCache>
#include <QMutex>

#include "kis_datamanagerproxy.h"
#include "kis_tilestorefromfile.h"

/**
 * A class that provides an easy interface having a paint device's tiles come directly from some tile. Note that
 * this class fills the tile WIHTOUT ANY REGARD TO THE COLORSPACE!
 */
class KRITAIMAGE_EXPORT KisTileSourceFile : public virtual KisTileStoreFromFile, public virtual KisDataManagerProxy {
protected: // Subclasses should implement these:
    virtual const quint8* getColumnData(qint32 col) = 0; /// the callee takes ownership of the returned, it should be delete[]able
    virtual qint32 width() = 0;
    virtual qint32 height() = 0;
    virtual qint32 pixelSize() const = 0;

public:
    KisTileSourceFile();
    virtual ~KisTileSourceFile();
public: // Tile store
  public: // Tile management
    KisTileStoreData* registerTileData(const KisSharedTileData* tile);
    void deregisterTileData(const KisSharedTileData* tile); // Deletes its TileInfo*

    void ensureTileLoaded(KisSharedTileData* tile);
    void maySwapTile(KisSharedTileData* tile);

    KisSharedTileData* degradedTileDataForSharing(KisSharedTileData* tileData);

  public: // Pool management
    void requestTileData(KisSharedTileData* tileData);
    void dontNeedTileData(KisSharedTileData* tileData);
public: // Proxy
    virtual KisTile* getTileDataAt(qint32 col, qint32 row, bool write, KisTile* defaultTile);

private:
    // ! Assumes locked cache for the duration of the use of the returned value (and before calling the function)
    const quint8* getCacheLine(qint32 col);
    // What's best: Cache columns, tiles, combination? ###
    // TODO: make the data a shared ptr (to avoid too much locking?)
    QMutex m_cacheMutex;
    struct CacheLine {
        CacheLine(const quint8* d) : data(d) {}
        ~CacheLine() { delete[] data; }
        const quint8* data;
    };
    QCache<qint32, CacheLine> m_decodingCache; // column -> data (takes ownership)
};

#endif // KIS_TILESOURCEFILE_H_
