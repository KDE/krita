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

KisTiledVLineIterator::KisTiledVLineIterator( KisTiledDataManager *ndevice,  Q_INT32 x,  Q_INT32 y, Q_INT32 h, bool writable) :
    KisTiledIterator(ndevice),
    m_bottom(y + h - 1)
{
    m_writable = writable;
    m_top = y;
    m_x = x;
    m_y = y;

    // Find tile row,col matching x,y
    m_col = xToCol(m_x);
    m_topRow = yToRow(m_y);
    m_bottomRow = yToRow(m_bottom);
    m_row = m_topRow;

    // calc limits within the tile
    m_xInTile = m_x - m_col * KisTile::WIDTH;
    m_topInTile = m_y - m_topRow * KisTile::HEIGHT;

    if(m_row == m_bottomRow)
        m_bottomInTile = m_bottom - m_bottomRow * KisTile::HEIGHT;
    else
        m_bottomInTile = KisTile::HEIGHT - 1;

    m_yInTile = m_topInTile;

    fetchTileData(m_col, m_row);
    m_offset = m_pixelSize * (m_yInTile * KisTile::WIDTH + m_xInTile);
}

KisTiledVLineIterator::KisTiledVLineIterator(const KisTiledVLineIterator& rhs)
    : KisTiledIterator(rhs)
{
    if (this != &rhs) {
        m_top = rhs.m_top;
        m_bottom = rhs.m_bottom;
        m_topRow = rhs.m_topRow;
        m_bottomRow = rhs.m_bottomRow;
        m_xInTile = rhs.m_xInTile;
        m_yInTile = rhs.m_yInTile;
        m_topInTile = rhs.m_topInTile;
        m_bottomInTile = rhs.m_bottomInTile;
    }
}

KisTiledVLineIterator& KisTiledVLineIterator::operator=(const KisTiledVLineIterator& rhs)
{
    if (this != &rhs) {
        KisTiledIterator::operator=(rhs);

        m_top = rhs.m_top;
        m_bottom = rhs.m_bottom;
        m_topRow = rhs.m_topRow;
        m_bottomRow = rhs.m_bottomRow;
        m_xInTile = rhs.m_xInTile;
        m_yInTile = rhs.m_yInTile;
        m_topInTile = rhs.m_topInTile;
        m_bottomInTile = rhs.m_bottomInTile;
    }
    return *this;
}

KisTiledVLineIterator::~KisTiledVLineIterator( )
{
}

KisTiledVLineIterator & KisTiledVLineIterator::operator ++ ()
{
    if(m_yInTile >= m_bottomInTile)
    {
        nextTile();
        fetchTileData(m_col, m_row);
        m_yInTile =m_topInTile;
        m_offset = m_pixelSize * (m_yInTile * KisTile::WIDTH + m_xInTile);
    }
    else
    {
        m_yInTile++;
        m_offset += m_pixelSize * KisTile::WIDTH;
    }
    m_y++;

    return *this;
}

void KisTiledVLineIterator::nextTile()
{
    if(m_row < m_bottomRow)
    {
        m_row++;
        m_topInTile = 0;

        if(m_row == m_bottomRow)
            m_bottomInTile = m_bottom - m_bottomRow * KisTile::HEIGHT;
        else
            m_bottomInTile = KisTile::HEIGHT - 1;
    }
}

void KisTiledVLineIterator::nextCol()
{
    m_x++;
    m_xInTile++;
    m_y = m_top;
    m_topInTile = m_y - m_topRow * KisTile::HEIGHT;
    m_yInTile = m_topInTile;
    if( m_xInTile >= KisTile::WIDTH )
    { // Need a new row
        m_xInTile = 0;
        m_col++;
        m_row = m_topRow;
        fetchTileData(m_col, m_row);
    } else if( m_topRow != m_row ) {
        m_row = m_topRow;
        fetchTileData(m_col, m_row);
    }
    if(m_row == m_bottomRow)
        m_bottomInTile = m_bottom - m_bottomRow * KisTile::HEIGHT;
    else
        m_bottomInTile = KisTile::HEIGHT - 1;

    m_offset = m_pixelSize * (m_yInTile * KisTile::WIDTH + m_xInTile);
}

/*
KisTiledVLineIterator & KisTiledVLineIterator::operator -- ()
{
    return *this;
}
*/
