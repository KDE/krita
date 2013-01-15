/*
 *  copyright (c) 2006,2010 Cyrille Berger <cberger@cberger.net>
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

#include "kis_random_accessor.h"


#include <kis_debug.h>


const quint32 KisRandomAccessor2::CACHESIZE = 4; // Define the number of tiles we keep in cache

KisRandomAccessor2::KisRandomAccessor2(KisTiledDataManager *ktm, qint32 x, qint32 y, qint32 offsetX, qint32 offsetY, bool writable) :
        m_ktm(ktm),
        m_tilesCache(new KisTileInfo*[CACHESIZE]),
        m_tilesCacheSize(0),
        m_pixelSize(m_ktm->pixelSize()),
        m_writable(writable),
        m_offsetX(offsetX),
        m_offsetY(offsetY)
{
    Q_ASSERT(ktm != 0);
    moveTo(x, y);
}

KisRandomAccessor2::~KisRandomAccessor2()
{
    for (uint i = 0; i < m_tilesCacheSize; i++) {
        unlockTile(m_tilesCache[i]->tile);
        unlockTile(m_tilesCache[i]->oldtile);
        delete m_tilesCache[i];
    }
    delete [] m_tilesCache;
}

void KisRandomAccessor2::moveTo(qint32 x, qint32 y)
{
    x -= m_offsetX;
    y -= m_offsetY;
    m_lastX = x;
    m_lastY = y;
    // Look in the cache if the tile if the data is available
    for (uint i = 0; i < m_tilesCacheSize; i++) {
        if (x >= m_tilesCache[i]->area_x1 && x <= m_tilesCache[i]->area_x2 &&
                y >= m_tilesCache[i]->area_y1 && y <= m_tilesCache[i]->area_y2) {
            KisTileInfo* kti = m_tilesCache[i];
            quint32 offset = x - kti->area_x1 + (y - kti->area_y1) * KisTileData::WIDTH;
            offset *= m_pixelSize;
            m_data = kti->data + offset;
            m_oldData = kti->oldData + offset;
            if (i > 0) {
                memmove(m_tilesCache + 1, m_tilesCache, i * sizeof(KisTileInfo*));
                m_tilesCache[0] = kti;
            }
            return;
        }
    }
    // The tile wasn't in cache
    if (m_tilesCacheSize == KisRandomAccessor2::CACHESIZE) { // Remove last element of cache
        unlockTile(m_tilesCache[CACHESIZE-1]->tile);
        unlockTile(m_tilesCache[CACHESIZE-1]->oldtile);
        delete m_tilesCache[CACHESIZE-1];
    } else {
        m_tilesCacheSize++;
    }
    quint32 col = xToCol(x);
    quint32 row = yToRow(y);
    KisTileInfo* kti = fetchTileData(col, row);
    quint32 offset = x - kti->area_x1 + (y - kti->area_y1) * KisTileData::WIDTH;
    offset *= m_pixelSize;
    m_data = kti->data + offset;
    m_oldData = kti->oldData + offset;
    memmove(m_tilesCache + 1, m_tilesCache, (KisRandomAccessor2::CACHESIZE - 1) * sizeof(KisTileInfo*));
    m_tilesCache[0] = kti;
}


quint8* KisRandomAccessor2::rawData()
{
    return m_data;
}


const quint8* KisRandomAccessor2::oldRawData() const
{
#ifdef DEBUG
    kWarning(!m_ktm->hasCurrentMemento(), 41004) << "Accessing oldRawData() when no transaction is in progress.";
#endif
    return m_oldData;
}

const quint8* KisRandomAccessor2::rawDataConst() const
{
    return m_data;
}

KisRandomAccessor2::KisTileInfo* KisRandomAccessor2::fetchTileData(qint32 col, qint32 row)
{
    KisTileInfo* kti = new KisTileInfo;
    kti->tile = m_ktm->getTile(col, row, m_writable);
    lockTile(kti->tile);

    kti->data = kti->tile->data();

    kti->area_x1 = col * KisTileData::HEIGHT;
    kti->area_y1 = row * KisTileData::WIDTH;
    kti->area_x2 = kti->area_x1 + KisTileData::HEIGHT - 1;
    kti->area_y2 = kti->area_y1 + KisTileData::WIDTH - 1;

    // set old data
    kti->oldtile = m_ktm->getOldTile(col, row);
    lockOldTile(kti->oldtile);
    kti->oldData = kti->oldtile->data();
    return kti;
}

qint32 KisRandomAccessor2::nConseqPixels() const
{
    qFatal("Not implemented.");
    return 0;
}


qint32 KisRandomAccessor2::x() const
{
    return m_lastX;
}

qint32 KisRandomAccessor2::y() const
{
    return m_lastY;
}
