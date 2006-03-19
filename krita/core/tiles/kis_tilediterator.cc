/*
 * This file is part of the Krita
 *
 * Copyright (c) 2004 Casper Boemann <cbr@boemann.dk>
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
#include <kdebug.h>

#include "kis_tile_global.h"
#include "kis_tilediterator.h"

KisTiledIterator::KisTiledIterator( KisTiledDataManager *ndevice)
{
    Q_ASSERT(ndevice != 0);
    m_ktm = ndevice;
    m_x = 0;
    m_y = 0;
    m_row = 0;
    m_col = 0;
    m_pixelSize = m_ktm->pixelSize();
    m_tile = 0;
}

KisTiledIterator::~KisTiledIterator( )
{
    if (m_tile)
        m_tile->removeReader();
}

KisTiledIterator::KisTiledIterator(const KisTiledIterator& rhs)
    : KShared()
{
    if (this != &rhs) {
        m_ktm = rhs.m_ktm;
        m_pixelSize = rhs.m_pixelSize;
        m_x = rhs.m_x;
        m_y = rhs.m_y;
        m_row = rhs.m_row;
        m_col = rhs.m_col;
        m_data = rhs.m_data;
        m_oldData = rhs.m_oldData;
        m_offset = rhs.m_offset;
        m_tile = rhs.m_tile;
        m_writable = rhs.m_writable;
        if (m_tile)
            m_tile->addReader();
    }
}

KisTiledIterator& KisTiledIterator::operator=(const KisTiledIterator& rhs)
{
    if (this != &rhs) {
        if (m_tile)
            m_tile->removeReader();
        m_ktm = rhs.m_ktm;
        m_pixelSize = rhs.m_pixelSize;
        m_x = rhs.m_x;
        m_y = rhs.m_y;
        m_row = rhs.m_row;
        m_col = rhs.m_col;
        m_data = rhs.m_data;
        m_oldData = rhs.m_oldData;
        m_offset = rhs.m_offset;
        m_tile = rhs.m_tile;
        m_writable = rhs.m_writable;
        if (m_tile)
            m_tile->addReader();
    }
    return *this;
}

Q_UINT8 * KisTiledIterator::rawData() const
{
    return m_data + m_offset;
}


const Q_UINT8 * KisTiledIterator::oldRawData() const
{
#ifdef DEBUG
    // Warn if we're misusing oldRawData(). If there's no memento, oldRawData is the same
    // as rawData().
    kdWarning(!m_ktm->hasCurrentMemento(), DBG_AREA_TILES) << "Accessing oldRawData() when no transaction is in progress.\n";
#endif
    return m_oldData + m_offset;
}

void KisTiledIterator::fetchTileData(Q_INT32 col, Q_INT32 row)
{
    if (m_tile)
        m_tile->removeReader();

    m_tile = m_ktm->getTile(col, row, m_writable);
    
    if (m_tile == 0) return;
    //Q_ASSERT(m_tile != 0);
    m_tile->addReader();

    m_data = m_tile->data();
    if (m_data == 0) return;

    //Q_ASSERT(m_data != 0);

    // set old data but default to current value
    m_oldData = m_ktm->getOldTile(col, row, m_tile)->data();
}
