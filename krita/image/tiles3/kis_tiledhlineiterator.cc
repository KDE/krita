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

#include <QtGlobal>

#include "kis_tilediterator.h"
#include "kis_debug.h"

KisTiledHLineIterator::KisTiledHLineIterator(KisTiledDataManager *dataManager,
        qint32 x, qint32 y,
        qint32 w, bool writable)
        : KisTiledIterator(dataManager)
{
    m_writable = writable;

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
    switchToTile(m_leftCol, leftInLeftmostTile);
}

KisTiledHLineIterator::KisTiledHLineIterator(const KisTiledHLineIterator& rhs)
        : KisTiledIterator(rhs)
{
    if (this != &rhs) {
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
}

KisTiledHLineIterator& KisTiledHLineIterator::operator=(const KisTiledHLineIterator & rhs)
{
    if (this != &rhs) {
        KisTiledIterator::operator=(rhs);
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

KisTiledHLineIterator::~KisTiledHLineIterator()
{
}

qint32 KisTiledHLineIterator::nConseqHPixels() const
{
    return m_rightInTile - m_xInTile + 1;
}

void KisTiledHLineIterator::switchToTile(qint32 col, qint32 xInTile)
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

KisTiledHLineIterator & KisTiledHLineIterator::operator++ ()
{
    // We won't increment m_x here as integer can overflow here
    if (m_x >= m_right) {
        m_isDoneFlag = true;
    } else {
        m_x++;
        if (++m_xInTile <= m_rightInTile)
            m_offset += m_pixelSize;
        else
            // Switching to the beginning of the next tile
            switchToTile(m_col + 1, 0);
    }

    return *this;
}

KisTiledHLineIterator & KisTiledHLineIterator::operator+=(int n)
{
    // XXX what if outside the valid range of this iterator?
    // AAA two ways: int overflow is caught in assert or
    //               borders are checked below

    Q_ASSERT_X(!(m_x > 0 && (m_x + n) < 0), "hlineIt+=", "Integer overflow");

    // We won't increment m_x here first as integer can overflow
    if (m_x >= m_right || (m_x += n) > m_right) {
        m_isDoneFlag = true;
    } else {
        qint32 col = xToCol(m_x);

        if (col == m_col) {
            /**
             * FIXME: most unlikely case. Think over unlikely(...) here
             */
            m_xInTile += n;
            m_offset += n * m_pixelSize;
        } else {
            qint32 xInTile = calcXInTile(m_x, col);
            switchToTile(col, xInTile);
        }
    }

    return *this;
}

KisTiledHLineIterator & KisTiledHLineIterator::operator-- ()
{
    if (m_x <= m_left) {
        /* do nothing */
    } else {
        m_x--;
        if (--m_xInTile >= m_leftInTile)
            m_offset -= m_pixelSize;
        else
            // Switching to the end of the previous tile
            switchToTile(m_col - 1, KisTileData::WIDTH - 1);

        m_isDoneFlag = false;
    }

    return *this;
}

void KisTiledHLineIterator::nextRow()
{
    qint32 leftInLeftmostTile = calcLeftInTile(m_leftCol);

    m_x = m_left;
    m_y++;

    if (++m_yInTile < KisTileData::HEIGHT) {
        /* do nothing, usual case */
    } else {
        m_row++;
        m_yInTile = 0;
    }

    switchToTile(m_leftCol, leftInLeftmostTile);

    m_isDoneFlag = false;
}
