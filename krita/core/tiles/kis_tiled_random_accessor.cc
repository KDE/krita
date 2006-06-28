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

KisTiledRandomAccessor::KisTiledRandomAccessor(KisTiledDataManager *ktm, Q_INT32 x, Q_INT32 y, bool writable) : m_ktm(ktm), m_pixelSize (m_ktm->pixelSize()), m_writable(writable)
{
    Q_ASSERT(ktm != 0);
    moveTo(x, y);
}

KisTiledRandomAccessor::~KisTiledRandomAccessor()
{
    for( KisListTileInfo::iterator it = m_tilesCache.begin(); it != m_tilesCache.end(); it++)
    {
        (*it).tile->removeReader();
        (*it).oldtile->removeReader();
    }
}

void KisTiledRandomAccessor::moveTo(Q_INT32 x, Q_INT32 y)
{
    // Look in the cache if the tile if the data is available
    for( KisListTileInfo::iterator it = m_tilesCache.begin(); it != m_tilesCache.end(); it++)
    {
        if( (*it).area.contains(QPoint(x,y)))
        {
            KisTileInfo kti = *it;
            Q_UINT32 offset = x - kti.area.x() + (y -kti.area.y()) * KisTile::WIDTH;
            offset *= m_pixelSize;
            m_data = kti.data + offset;
            m_oldData = kti.oldData + offset;
            m_tilesCache.remove(it);
            m_tilesCache.prepend(kti);
            return;
        }
    }
    // The tile wasn't in cache
    if(m_tilesCache.size() == KisTiledRandomAccessor::CACHESIZE )
    { // Remove last element of cache
        KisTileInfo& it = m_tilesCache.last();
        it.tile->removeReader();
        it.oldtile->removeReader();
        m_tilesCache.pop_back();
    }
    Q_UINT32 col = xToCol( x );
    Q_UINT32 row = yToRow( y );
    KisTileInfo kti = fetchTileData(col, row);
    Q_UINT32 offset = x - kti.area.x() + (y - kti.area.y()) * KisTile::WIDTH;
    offset *= m_pixelSize;
    m_data = kti.data + offset;
    m_oldData = kti.oldData + offset;
    m_tilesCache.prepend(kti);
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

KisTiledRandomAccessor::KisTileInfo KisTiledRandomAccessor::fetchTileData(Q_INT32 col, Q_INT32 row)
{
    KisTileInfo kti;
    kti.tile = m_ktm->getTile(col, row, m_writable);
    
    kti.tile->addReader();

    kti.data = kti.tile->data();

    kti.area = QRect( col * KisTile::HEIGHT, row * KisTile::WIDTH, KisTile::WIDTH - 1, KisTile::HEIGHT - 1 );

    // set old data
    kti.oldtile = m_ktm->getOldTile(col, row, kti.tile);
    kti.oldtile->addReader();
    kti.oldData = kti.oldtile->data();
    return kti;
}
