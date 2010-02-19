/* 
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_hline_iterator.h"


KisHLineIterator2::KisHLineIterator2(KisDataManager *dataManager, qint32 x, qint32 y, qint32 w, qint32 offsetX, qint32 offsetY)
{
    Q_ASSERT(dataManager != 0);
    m_dataManager = dataManager;
    m_pixelSize = m_dataManager->pixelSize();

    //m_writable = writable;
    qDebug() << "I'm Created!";
    
    
    m_x = x;
    m_y = y;

    m_left = x;
    m_right = x + w - 1;

    m_isDoneFlag = !w;
    if (m_left > m_right) {
        m_isDoneFlag = true;
        return;
    }

    m_leftCol = xToCol(m_left);
    m_rightCol = xToCol(m_right);
    
    
    m_row = yToRow(m_y);
    m_yInTile = calcYInTile(m_y, m_row);

    qint32 leftInLeftmostTile = calcLeftInTile(m_leftCol);

    m_tilesCacheSize = m_rightCol - m_leftCol + 1;
    m_tilesCache.resize(m_tilesCacheSize);
    
    // let's prealocate first row 
    for (int i = 0; i < m_tilesCacheSize; i++){
        m_tilesCache[i] = fetchTileDataForCache(m_leftCol + i, m_row);
    }
    switchToTile(m_leftCol, leftInLeftmostTile);
}


KisHLineIterator2& KisHLineIterator2::operator=(const KisHLineIterator2 & rhs)
{
    if (this != &rhs) {

        if (m_tile)
            unlockTile(m_tile);
        if (m_oldTile)
            unlockTile(m_oldTile);

        m_dataManager = rhs.m_dataManager;
        m_pixelSize = rhs.m_pixelSize;
        m_x = rhs.m_x;
        m_y = rhs.m_y;
        m_row = rhs.m_row;
        m_col = rhs.m_col;
        m_data = rhs.m_data;
        m_oldData = rhs.m_oldData;
        m_offset = rhs.m_offset;
        m_tile = rhs.m_tile;
        m_oldTile = rhs.m_oldTile;
        m_writable = rhs.m_writable;

        if (m_tile)
            lockTile(m_tile);
        if (m_oldTile)
            lockOldTile(m_oldTile);

        m_left = rhs.m_left;
        m_right = rhs.m_right;
        m_leftCol = rhs.m_leftCol;
        m_rightCol = rhs.m_rightCol;
        m_xInTile = rhs.m_xInTile;
        m_yInTile = rhs.m_yInTile;
        m_leftInTile = rhs.m_leftInTile;
        m_rightInTile = rhs.m_rightInTile;
        m_isDoneFlag = rhs.m_isDoneFlag;
    }
    return *this;
}




bool KisHLineIterator2::nextPixel()
{
    qDebug() << "!m_isDoneFlag: " << !m_isDoneFlag;
    
    // We won't increment m_x here as integer can overflow here
    if (m_x >= m_right) {
        return !m_isDoneFlag;
    } else {
        m_x++;
        if (++m_xInTile <= m_rightInTile)
            m_offset += m_pixelSize;
        else
            // Switching to the beginning of the next tile
            switchToTile(m_col + 1, 0);
    }
    return m_isDoneFlag;
}


void KisHLineIterator2::nextRow()
{
    qint32 leftInLeftmostTile = calcLeftInTile(m_leftCol);

    m_x = m_left;
    m_y++;

    if (++m_yInTile < KisTileData::HEIGHT) {
        /* do nothing, usual case */
    } else {
        m_row++;
        m_yInTile = 0;
        preallocateTiles(m_row);
    }
    switchToTile(m_leftCol, leftInLeftmostTile);

    m_isDoneFlag = false;
}



KisHLineIterator2::~KisHLineIterator2()
{
    for (uint i = 0; i < m_tilesCacheSize; i++) {
        unlockTile(m_tilesCache[i].tile);
        unlockTile(m_tilesCache[i].oldtile);
    }
}


quint8 * KisHLineIterator2::rawData()
{
    return m_data + m_offset;
}


const quint8 * KisHLineIterator2::oldRawData() const
{
    return m_oldData + m_offset;
}

void KisHLineIterator2::fetchTileData(qint32 col, qint32 row){
    // check if we have the cached column and row
    int index = col - m_leftCol;
    
    // setup correct data
    m_data = m_tilesCache[index].data;
    m_oldData = m_tilesCache[index].oldData;
}


void KisHLineIterator2::switchToTile(qint32 col, qint32 xInTile)
{
    // The caller must ensure that we are not out of bounds
    Q_ASSERT(col <= m_rightCol);
    Q_ASSERT(col >= m_leftCol);

    m_rightInTile = calcRightInTile(col);
    m_leftInTile = calcLeftInTile(col);

    m_col = col;
    m_xInTile = xInTile;
    m_offset = calcOffset(m_xInTile, m_yInTile);

    fetchTileData(col, m_row);
}


KisHLineIterator2::KisTileInfo KisHLineIterator2::fetchTileDataForCache(qint32 col, qint32 row)
{
    KisTileInfo kti;
    kti.tile = m_dataManager->getTile(col, row);
    lockTile(kti.tile);
    kti.data = kti.tile->data();

    // set old data
    kti.oldtile = m_dataManager->getOldTile(col, row);
    lockOldTile(kti.oldtile);
    kti.oldData = kti.oldtile->data();
    return kti;
}

void KisHLineIterator2::preallocateTiles(qint32 row)
{
    for (int i = 0; i < m_tilesCacheSize; i++){
        unlockTile(m_tilesCache[i].tile);
        unlockTile(m_tilesCache[i].oldtile);
        m_tilesCache[i] = fetchTileDataForCache(m_leftCol + i, row);
    }
}
