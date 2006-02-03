/*
 * This file is part of the KDE project
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

KisTiledHLineIterator::KisTiledHLineIterator( KisTiledDataManager *ndevice,  Q_INT32 x, Q_INT32 y, Q_INT32 w, bool writable) :
    KisTiledIterator(ndevice),
    m_right(x+w-1), m_left(x)
{
    Q_ASSERT(ndevice != 0);

    m_writable = writable;
    m_x = x;
    m_y = y;

    // Find tile row,col matching x,y
    m_row = yToRow(m_y);
    m_leftCol = xToCol(m_x);
    m_rightCol = xToCol(m_right);
    m_col = m_leftCol;

    // calc limits within the tile
    m_yInTile = m_y - m_row * KisTile::HEIGHT;
    m_leftInTile = m_x - m_leftCol * KisTile::WIDTH;

    if(m_col == m_rightCol)
        m_rightInTile = m_right - m_rightCol * KisTile::WIDTH;
    else
        m_rightInTile = KisTile::WIDTH - 1;

    m_xInTile = m_leftInTile;

    fetchTileData(m_col, m_row);
    m_offset = m_pixelSize * (m_yInTile * KisTile::WIDTH + m_xInTile);
}

KisTiledHLineIterator::KisTiledHLineIterator(const KisTiledHLineIterator& rhs)
    : KisTiledIterator(rhs)
{
    if (this != &rhs) {
        m_right = rhs.m_right;
        m_left = rhs.m_left;
        m_leftCol = rhs.m_leftCol;
        m_rightCol = rhs.m_rightCol;
        m_xInTile = rhs.m_xInTile;
        m_yInTile = rhs.m_yInTile;
        m_leftInTile = rhs.m_leftInTile;
        m_rightInTile = rhs.m_rightInTile;
    }
}

KisTiledHLineIterator& KisTiledHLineIterator::operator=(const KisTiledHLineIterator& rhs)
{
    if (this != &rhs) {
        KisTiledIterator::operator=(rhs);
        m_right = rhs.m_right;
        m_left = rhs.m_left;
        m_leftCol = rhs.m_leftCol;
        m_rightCol = rhs.m_rightCol;
        m_xInTile = rhs.m_xInTile;
        m_yInTile = rhs.m_yInTile;
        m_leftInTile = rhs.m_leftInTile;
        m_rightInTile = rhs.m_rightInTile;
    }
    return *this;
}

KisTiledHLineIterator::~KisTiledHLineIterator( )
{
}

KisTiledHLineIterator & KisTiledHLineIterator::operator ++ ()
{
    if(m_xInTile >= m_rightInTile)
    {
        nextTile();
        fetchTileData(m_col, m_row);
        m_xInTile =m_leftInTile;
        m_offset = m_pixelSize * (m_yInTile * KisTile::WIDTH + m_xInTile);
    }
    else
    {
        m_xInTile++;
        m_offset += m_pixelSize;
    }
    m_x++;

    return *this;
}

void KisTiledHLineIterator::nextTile()
{
    if(m_col < m_rightCol)
    {
        m_col++;
        m_leftInTile = 0;

        if(m_col == m_rightCol)
            m_rightInTile = m_right - m_rightCol * KisTile::WIDTH;
        else
            m_rightInTile = KisTile::WIDTH - 1;
    }
}

void KisTiledHLineIterator::prevTile()
{
    if(m_col > m_leftCol)
    {
        m_col--;

        if(m_col == m_leftCol) {
            m_leftInTile = m_left - m_leftCol * KisTile::WIDTH;
        } else {
            m_leftInTile = 0;
        }
        // the only place this doesn't apply, is if we're in rightCol, and we can't go there
        m_rightInTile = KisTile::WIDTH - 1;
    }
}

Q_INT32 KisTiledHLineIterator::nConseqHPixels() const
{
    return m_rightInTile - m_xInTile + 1;
}

KisTiledHLineIterator & KisTiledHLineIterator::operator+=(int n)
{
    // XXX what if outside the valid range of this iterator?
    if(m_xInTile + n > m_rightInTile)
    {
        m_x += n;
        m_col = xToCol(m_x);
        m_xInTile = m_x - m_col * KisTile::WIDTH;
        m_leftInTile = 0;

        if(m_col == m_rightCol)
            m_rightInTile = m_right - m_rightCol * KisTile::WIDTH;
        else
            m_rightInTile = KisTile::WIDTH - 1;

        fetchTileData(m_col, m_row);
    }
    else
    {
        m_xInTile += n;
        m_x += n;
    }
    m_offset = m_pixelSize * (m_yInTile * KisTile::WIDTH + m_xInTile);

    return *this;
}

KisTiledHLineIterator & KisTiledHLineIterator::operator -- ()
{
    if(m_xInTile <= 0)
    {
        prevTile();
        fetchTileData(m_col, m_row);
        m_xInTile = KisTile::WIDTH - 1;
        m_offset = m_pixelSize * (m_yInTile * KisTile::WIDTH + m_xInTile);
    }
    else
    {
        m_xInTile--;
        m_offset -= m_pixelSize;
    }
    m_x--;

    return *this;
}

void KisTiledHLineIterator::nextRow()
{
    m_y++;
    m_yInTile++;
    m_x = m_left;
    m_leftInTile = m_x - m_leftCol * KisTile::WIDTH;
    m_xInTile = m_leftInTile;
    if( m_yInTile >= KisTile::HEIGHT )
    { // Need a new row
        m_yInTile = 0;
        m_row++;
        m_col = m_leftCol;
        fetchTileData(m_col, m_row);
    } else if( m_leftCol != m_col ) {
        m_col = m_leftCol;
        fetchTileData(m_col, m_row);
    }
    if(m_col == m_rightCol)
        m_rightInTile = m_right - m_rightCol * KisTile::WIDTH;
    else
        m_rightInTile = KisTile::WIDTH - 1;
    m_offset = m_pixelSize * (m_yInTile * KisTile::WIDTH + m_xInTile);
}
