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
#include "kis_tilesourcefile.h"
#include "kis_tile.h"


KisTileSourceFile::KisTileSourceFile()
        : m_decodingCache(2*KisTile::HEIGHT) /* ### make configurable? */
{
    ;
}

KisTileSourceFile::~KisTileSourceFile()
{
    ;
}

const quint8* KisTileSourceFile::getCacheLine(qint32 col)
{
    CacheLine* line = m_decodingCache.object(col);
    if (line)
        return line->data;

    // Data was not in the cache, (re)get it and add it to the cache
    const quint8* data = getColumnData(col);
    m_decodingCache.insert(col, new CacheLine(data));
    return data;
}

KisTile* KisTileSourceFile::getTileDataAt(qint32 col, qint32 row, bool write, KisTile* defaultTile)
{
    if (col < 0 || row < 0)
        return 0;

    int iw = width();
    int ih = height();

    // ### Cleaner:
    int maxw = iw;
    int maxh = ih;
    if (maxw % KisTile::WIDTH != 0)
        maxw = ((iw + KisTile::WIDTH) / KisTile::WIDTH) * KisTile::WIDTH;
    if (maxh % KisTile::HEIGHT != 0)
        maxh = ((ih + KisTile::HEIGHT) / KisTile::HEIGHT) * KisTile::HEIGHT;

    int fromX = col * KisTile::WIDTH;
    int fromY = row * KisTile::HEIGHT;
    int toX = fromX + KisTile::WIDTH;
    int toY = fromY + KisTile::HEIGHT;
    int w = KisTile::WIDTH;
    int h = KisTile::HEIGHT;

    // Sanity:
    if (w <= 0 || h <= 0)
        return 0;

    // Rule out tiles that lie completely outside our area
    if (toX > maxw || toY > maxh) {
        return 0;
    }

    bool partial = false;
    KisTile* tile = 0;
    quint8* dstData = 0;
    int copyAmount = 0;
    int skipAmount = 0;

    if (toX > iw || toY > ih) {
        // Partially filled tile: create a blank tile, and fill the good bits with the read data
        partial = true;
        tile = new KisTile(*defaultTile, col, row); // Also means it shared defaultTile's store
        tile->detachShared();
        tile->addReader();

        dstData = tile->data();

        if (toX > iw) { // incomplete tile width
            copyAmount = (iw % w) * pixelSize();
        } else {
            copyAmount = w * pixelSize();
        }

        if (toY > ih) // incomplete tile height
            toY = ih;

        skipAmount = w * pixelSize();
    } else {
        // Tile completely occupied by image data
        KisSharedTileData* data = new KisSharedTileData(this /*store*/, pixelSize() * KisTile::WIDTH * KisTile::HEIGHT, pixelSize());
        dstData = data->data;
        copyAmount = w * pixelSize();
        skipAmount = copyAmount;

        tile = new KisTile(pixelSize(), col, row, data);
    }

    // Fill the tile with data:
    m_cacheMutex.lock();
    for (int y = fromY; y < toY; y++) {
        const quint8* line = getCacheLine(y);
        memcpy(dstData, &line[fromX * pixelSize()], copyAmount);
        dstData = &dstData[skipAmount];
    }
    m_cacheMutex.unlock();

    if (partial)
        tile->removeReader();

    return tile;
}


KisTileStoreData* KisTileSourceFile::registerTileData(const KisSharedTileData* tile)
{
    // This could add info about the origin of the tile, for speading up (re)reading, I guess?
    return 0;
}
void KisTileSourceFile::deregisterTileData(const KisSharedTileData* tile)
{
    ; // don't recursively call delete here (~KisSharedTileData calls us again)
}

KisSharedTileData* KisTileSourceFile::degradedTileDataForSharing(KisSharedTileData* tileData)
{
    return new KisSharedTileData(defaultTileStore(), tileData->tileSize, tileData->pixelSize);
}

void KisTileSourceFile::ensureTileLoaded(KisSharedTileData* tile) {}
void KisTileSourceFile::maySwapTile(KisSharedTileData* tile) {}

void KisTileSourceFile::requestTileData(KisSharedTileData* tileData)
{
    // Gets called from KisSharedTileData constructor
    Q_ASSERT(!tileData->data);
    tileData->data = new quint8[pixelSize() * KisTile::WIDTH * KisTile::HEIGHT];
}

void KisTileSourceFile::dontNeedTileData(KisSharedTileData* tileData)
{
    delete[] tileData->data;
    tileData->data = 0;
}
