/*
 *  copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
 */
#include "kis_tiled_random_accessor.h"


const Q_UINT32 KisTiledRandomAccessor::CACHESIZE = 4; // Define the number of tiles we keep in cache

KisTiledRandomAccessor::KisTiledRandomAccessor(KisTiledDataManager *ktm, Q_INT32 x, Q_INT32 y, bool writable) : m_ktm(ktm), m_tilesCache(new KisTileInfo*[4]), m_tilesCacheSize(0), m_pixelSize (m_ktm->pixelSize()), m_writable(writable)
{
    Q_ASSERT(ktm != 0);
    moveTo(x, y);
}

KisTiledRandomAccessor::~KisTiledRandomAccessor()
{
    for( int i = 0; i < m_tilesCacheSize; i++)
    {
        m_tilesCache[i]->tile->removeReader();
        m_tilesCache[i]->oldtile->removeReader();
        delete m_tilesCache[i];
    }
    delete m_tilesCache;
}

void KisTiledRandomAccessor::moveTo(Q_INT32 x, Q_INT32 y)
{
    // Look in the cache if the tile if the data is available
    for( int i = 0; i < m_tilesCacheSize; i++)
    {
        if( m_tilesCache[i]->area.contains(QPoint(x,y)))
        {
            KisTileInfo* kti = m_tilesCache[i];
            Q_UINT32 offset = x - kti->area.x() + (y -kti->area.y()) * KisTile::WIDTH;
            offset *= m_pixelSize;
            m_data = kti->data + offset;
            m_oldData = kti->oldData + offset;
            if(i > 0)
            {
                memmove(m_tilesCache+1,m_tilesCache, i * sizeof(KisTileInfo*));
                m_tilesCache[0] = kti;
            }
            return;
        }
    }
    // The tile wasn't in cache
    if(m_tilesCacheSize == KisTiledRandomAccessor::CACHESIZE )
    { // Remove last element of cache
        m_tilesCache[CACHESIZE-1]->tile->removeReader();
        m_tilesCache[CACHESIZE-1]->oldtile->removeReader();
        delete m_tilesCache[CACHESIZE-1];
    } else {
        m_tilesCacheSize++;
    }
    Q_UINT32 col = xToCol( x );
    Q_UINT32 row = yToRow( y );
    KisTileInfo* kti = fetchTileData(col, row);
    Q_UINT32 offset = x - kti->area.x() + (y - kti->area.y()) * KisTile::WIDTH;
    offset *= m_pixelSize;
    m_data = kti->data + offset;
    m_oldData = kti->oldData + offset;
    memmove(m_tilesCache+1,m_tilesCache, (KisTiledRandomAccessor::CACHESIZE-1) * sizeof(KisTileInfo*));
    m_tilesCache[0] = kti;
}


Q_UINT8 * KisTiledRandomAccessor::rawData() const
{
    return m_data;
}


const Q_UINT8 * KisTiledRandomAccessor::oldRawData() const
{
#ifdef DEBUG
    kdWarning(!m_ktm->hasCurrentMemento(), DBG_AREA_TILES) << "Accessing oldRawData() when no transaction is in progress.\n";
#endif
    return m_oldData;
}

KisTiledRandomAccessor::KisTileInfo* KisTiledRandomAccessor::fetchTileData(Q_INT32 col, Q_INT32 row)
{
    KisTileInfo* kti = new KisTileInfo;
    kti->tile = m_ktm->getTile(col, row, m_writable);
    
    kti->tile->addReader();

    kti->data = kti->tile->data();

    kti->area = QRect( col * KisTile::HEIGHT, row * KisTile::WIDTH, KisTile::WIDTH - 1, KisTile::HEIGHT - 1 );

    // set old data
    kti->oldtile = m_ktm->getOldTile(col, row, kti->tile);
    kti->oldtile->addReader();
    kti->oldData = kti->oldtile->data();
    return kti;
}
