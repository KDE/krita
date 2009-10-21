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

#include "kis_tilediterator.h"
#include "kis_debug.h"

KisTiledRectIterator::KisTiledRectIterator(KisTiledDataManager *dataManager,
        qint32 left, qint32 top,
        qint32 width, qint32 height,
        bool writable)
        : KisTiledIterator(dataManager),
        m_left(left),
        m_top(top),
        m_width(width),
        m_height(height)
{

    Q_ASSERT(dataManager != 0);

    m_writable = writable;
    m_x = left;
    m_y  = top;
    m_beyondEnd = (m_width == 0) || (m_height == 0);

    // Find tile row,col matching x,y
    m_topRow = yToRow(m_y);
    m_bottomRow = yToRow(m_y + m_height - 1);
    m_leftCol = xToCol(m_x);
    m_rightCol = xToCol(m_x + m_width - 1);
    m_row = m_topRow;
    m_col = m_leftCol;

    // calc limits within the tile
    m_topInTile = m_top - m_topRow * KisTileData::HEIGHT;

    if (m_row == m_bottomRow)
        m_bottomInTile = m_top + m_height - 1 - m_bottomRow * KisTileData::HEIGHT;
    else
        m_bottomInTile = KisTileData::HEIGHT - 1;

    m_leftInTile = m_left - m_leftCol * KisTileData::WIDTH;

    if (m_col == m_rightCol)
        m_rightInTile = m_left + m_width - 1 - m_rightCol * KisTileData::WIDTH;
    else
        m_rightInTile = KisTileData::WIDTH - 1;

    m_xInTile = m_leftInTile;
    m_yInTile = m_topInTile;

    if (! m_beyondEnd)
        fetchTileData(m_col, m_row);
    m_offset = m_pixelSize * (m_yInTile * KisTileData::WIDTH + m_xInTile);
}

KisTiledRectIterator::KisTiledRectIterator(const KisTiledRectIterator& rhs)
        : KisTiledIterator(rhs)
{
    if (this != &rhs) {
        m_left = rhs.m_left;
        m_top = rhs.m_top;
        m_width = rhs.m_width;
        m_height = rhs.m_height;
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

KisTiledRectIterator& KisTiledRectIterator::operator=(const KisTiledRectIterator & rhs)
{
    if (this != &rhs) {
        KisTiledIterator::operator=(rhs);
        m_left = rhs.m_left;
        m_top = rhs.m_top;
        m_width = rhs.m_width;
        m_height = rhs.m_height;
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

KisTiledRectIterator::~KisTiledRectIterator()
{
}

qint32 KisTiledRectIterator::nConseqPixels() const
{
    if (m_leftInTile || (m_rightInTile != KisTileData::WIDTH - 1))
        return m_rightInTile - m_xInTile + 1;
    else /* !m_leftInTile && (m_rightInTile == KisTileData::WIDTH - 1) */
        return KisTileData::WIDTH *(m_bottomInTile - m_yInTile + 1) - m_xInTile;
}

KisTiledRectIterator & KisTiledRectIterator::operator+=(int n)
{
    int remainInTile;

    remainInTile = (m_bottomInTile - m_yInTile) * (m_rightInTile - m_leftInTile + 1);
    remainInTile += m_rightInTile - m_xInTile + 1;

    // This while loop may not bet the fastest, but usually
    // it's not entered more than once.
    while (n >= remainInTile) {
        n -= remainInTile;
        nextTile();
        if (m_beyondEnd)
            return *this;
        m_yInTile = m_topInTile;
        m_xInTile = m_leftInTile;
        remainInTile = (m_bottomInTile - m_yInTile) * (m_rightInTile - m_leftInTile + 1);
        remainInTile += m_rightInTile - m_xInTile + 1;
    }

    int lWidth = m_rightInTile - m_leftInTile + 1;
    while (n >= lWidth) {
        n -= lWidth;
        m_yInTile++;
    }
    m_xInTile += n;
    m_x = m_col * KisTileData::WIDTH + m_xInTile;
    m_y = m_row * KisTileData::HEIGHT + m_yInTile;
    fetchTileData(m_col, m_row);
    m_offset = m_pixelSize * (m_yInTile * KisTileData::WIDTH + m_xInTile);

    return *this;
}


KisTiledRectIterator & KisTiledRectIterator::operator ++ ()
{
    // advance through rect completing each tile before moving on
    // as per excellent suggestion by Cyrille, avoiding excessive tile switching
    if (m_xInTile >= m_rightInTile) {
        if (m_yInTile >= m_bottomInTile) {
            nextTile();
            if (m_beyondEnd)
                return *this;
            m_yInTile = m_topInTile;
            m_x = m_col * KisTileData::WIDTH + m_leftInTile;
            m_y = m_row * KisTileData::HEIGHT + m_topInTile;
            fetchTileData(m_col, m_row);
        } else {
            m_x -= m_rightInTile - m_leftInTile;
            m_y++;
            m_yInTile++;
        }
        m_xInTile = m_leftInTile;
        m_offset = m_pixelSize * (m_yInTile * KisTileData::WIDTH + m_xInTile);
    } else {
        m_x++;
        m_xInTile++;
        m_offset += m_pixelSize;
    }
    return *this;
}

void KisTiledRectIterator::nextTile()
{
    if (m_col >= m_rightCol) {
        // needs to switch row
        if (m_row >= m_bottomRow) {
            m_beyondEnd = true;
        } else {
            m_col = m_leftCol;
            m_row++;
            // The row has now changed, so recalc vertical limits
            if (m_row == m_topRow)
                m_topInTile = m_top - m_topRow * KisTileData::HEIGHT;
            else
                m_topInTile = 0;

            if (m_row == m_bottomRow)
                m_bottomInTile = m_top + m_height - 1 - m_bottomRow * KisTileData::HEIGHT;
            else
                m_bottomInTile = KisTileData::HEIGHT - 1;
        }
    } else
        m_col++;

    // No matter what the column has now changed, so recalc horizontal limits
    if (m_col == m_leftCol)
        m_leftInTile = m_left - m_leftCol * KisTileData::WIDTH;
    else
        m_leftInTile = 0;

    if (m_col == m_rightCol)
        m_rightInTile = m_left + m_width - 1 - m_rightCol * KisTileData::WIDTH;
    else
        m_rightInTile = KisTileData::WIDTH - 1;
}

/*
  KisTiledRectIterator & KisTiledRectIterator::operator -- ()
  {
  return *this;
  }
*/
