/*
 * This file is part of the KDE project
 *
 *  Copyright (c) 2004 Casper Boemann <cbr@boemann.dk>
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

KisTiledRectIterator::KisTiledRectIterator( KisTiledDataManager *ndevice,  qint32 nleft,
                        qint32 ntop, qint32 nw, qint32 nh, bool writable) :
    KisTiledIterator(ndevice),
    m_left(nleft),
    m_top(ntop),
    m_w(nw),
    m_h(nh)
{

    Q_ASSERT(ndevice != 0);

    m_writable = writable;
    m_x = nleft;
    m_y  = ntop;
    m_beyondEnd = (m_w == 0) || (m_h == 0);

    // Find tile row,col matching x,y
    m_topRow = yToRow(m_y);
    m_bottomRow = yToRow(m_y + m_h - 1);
    m_leftCol = xToCol(m_x);
    m_rightCol = xToCol(m_x + m_w - 1);
    m_row = m_topRow;
    m_col = m_leftCol;

    // calc limits within the tile
    m_topInTile = m_top - m_topRow * KisTile::HEIGHT;

    if(m_row == m_bottomRow)
        m_bottomInTile = m_top + m_h - 1 - m_bottomRow * KisTile::HEIGHT;
    else
        m_bottomInTile = KisTile::HEIGHT - 1;

    m_leftInTile = m_left - m_leftCol * KisTile::WIDTH;

    if(m_col == m_rightCol)
        m_rightInTile = m_left + m_w - 1 - m_rightCol * KisTile::WIDTH;
    else
        m_rightInTile = KisTile::WIDTH - 1;

    m_xInTile = m_leftInTile;
    m_yInTile = m_topInTile;

    if( ! m_beyondEnd)
        fetchTileData(m_col, m_row);
    m_offset = m_pixelSize * (m_yInTile * KisTile::WIDTH + m_xInTile);
}

KisTiledRectIterator::KisTiledRectIterator(const KisTiledRectIterator& rhs)
    : KisTiledIterator(rhs)
{
    if (this != &rhs) {
        m_left = rhs.m_left;
        m_top = rhs.m_top;
        m_w = rhs.m_w;
        m_h = rhs.m_h;
        m_topRow = rhs.m_topRow;
        m_bottomRow = rhs.m_bottomRow;
        m_leftCol = rhs.m_leftCol;
        m_rightCol = rhs.m_rightCol;
        m_xInTile = rhs.m_xInTile;
        m_yInTile = rhs.m_yInTile;
        m_leftInTile = rhs.m_leftInTile;
        m_rightInTile = rhs.m_rightInTile;
        m_topInTile = rhs.m_topInTile;
        m_bottomInTile = rhs.m_bottomInTile;
        m_beyondEnd = rhs.m_beyondEnd;
    }
}

KisTiledRectIterator& KisTiledRectIterator::operator=(const KisTiledRectIterator& rhs)
{
    if (this != &rhs) {
        KisTiledIterator::operator=(rhs);
        m_left = rhs.m_left;
        m_top = rhs.m_top;
        m_w = rhs.m_w;
        m_h = rhs.m_h;
        m_topRow = rhs.m_topRow;
        m_bottomRow = rhs.m_bottomRow;
        m_leftCol = rhs.m_leftCol;
        m_rightCol = rhs.m_rightCol;
        m_xInTile = rhs.m_xInTile;
        m_yInTile = rhs.m_yInTile;
        m_leftInTile = rhs.m_leftInTile;
        m_rightInTile = rhs.m_rightInTile;
        m_topInTile = rhs.m_topInTile;
        m_bottomInTile = rhs.m_bottomInTile;
        m_beyondEnd = rhs.m_beyondEnd;
    }
    return *this;
}

KisTiledRectIterator::~KisTiledRectIterator( )
{
}

qint32 KisTiledRectIterator::nConseqPixels() const
{
    if(m_leftInTile || (m_rightInTile != KisTile::WIDTH - 1))
        return m_rightInTile - m_xInTile + 1;
    else
        return KisTile::WIDTH * (m_bottomInTile - m_yInTile + 1) - m_xInTile;
}

KisTiledRectIterator & KisTiledRectIterator::operator+=(int n)
{
    int remainInTile;

    remainInTile= (m_bottomInTile - m_yInTile) * (m_rightInTile - m_leftInTile + 1);
    remainInTile += m_rightInTile - m_xInTile + 1;

    // This while loop may not bet the fastest, but usually it's not entered more than once.
    while(n >= remainInTile)
    {
        n -= remainInTile;
        nextTile();
        if(m_beyondEnd)
            return *this;
        m_yInTile = m_topInTile;
        m_xInTile = m_leftInTile;
        remainInTile= (m_bottomInTile - m_yInTile) * (m_rightInTile - m_leftInTile + 1);
        remainInTile += m_rightInTile - m_xInTile + 1;
    }

    int lWidth = m_rightInTile - m_leftInTile + 1;
    while(n >= lWidth)
    {
        n -= lWidth;
        m_yInTile++;
    }
    m_xInTile += n;
    m_x = m_col * KisTile::WIDTH + m_xInTile;
    m_y = m_row * KisTile::HEIGHT + m_yInTile;
    fetchTileData(m_col, m_row);
    m_offset = m_pixelSize * (m_yInTile * KisTile::WIDTH + m_xInTile);

    return *this;
}


KisTiledRectIterator & KisTiledRectIterator::operator ++ ()
{
    // advance through rect completing each tile before moving on
    // as per excellent suggestion by Cyrille, avoiding excessive tile switching
    if(m_xInTile >= m_rightInTile)
    {
        if (m_yInTile >= m_bottomInTile)
        {
            nextTile();
            if(m_beyondEnd)
                return *this;
            m_yInTile = m_topInTile;
            m_x = m_col * KisTile::WIDTH + m_leftInTile;
            m_y = m_row * KisTile::HEIGHT + m_topInTile;
            fetchTileData(m_col, m_row);
        }
        else
        {
            m_x -= m_rightInTile - m_leftInTile;
            m_y++;
            m_yInTile++;
        }
        m_xInTile =m_leftInTile;
        m_offset = m_pixelSize * (m_yInTile * KisTile::WIDTH + m_xInTile);
    }
    else
    {
        m_x++;
        m_xInTile++;
        m_offset += m_pixelSize;
    }
    return *this;
}

void KisTiledRectIterator::nextTile()
{
    if(m_col >= m_rightCol)
    {
        // needs to switch row
        if(m_row >= m_bottomRow)
            m_beyondEnd = true;
        else
        {
            m_col = m_leftCol;
            m_row++;
            // The row has now changed, so recalc vertical limits
            if(m_row == m_topRow)
                m_topInTile = m_top - m_topRow * KisTile::HEIGHT;
            else
                m_topInTile = 0;

            if(m_row == m_bottomRow)
                m_bottomInTile = m_top + m_h - 1 - m_bottomRow * KisTile::HEIGHT;
            else
                m_bottomInTile = KisTile::HEIGHT - 1;
        }
    }
    else
        m_col++;

    // No matter what the column has now changed, so recalc horizontal limits
    if(m_col == m_leftCol)
        m_leftInTile = m_left - m_leftCol * KisTile::WIDTH;
    else
        m_leftInTile = 0;

    if(m_col == m_rightCol)
        m_rightInTile = m_left + m_w - 1 - m_rightCol * KisTile::WIDTH;
    else
        m_rightInTile = KisTile::WIDTH - 1;
}

/*
KisTiledRectIterator & KisTiledRectIterator::operator -- ()
{
    return *this;
}
*/
