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

#include "kis_tilediterator.h"

#include "kis_debug.h"


KisTiledIterator::KisTiledIterator(KisTiledDataManager *dataManager)
{
    Q_ASSERT(dataManager != 0);
    m_dataManager = dataManager;

    m_pixelSize = m_dataManager->pixelSize();
    m_x = 0;
    m_y = 0;
    m_row = 0;
    m_col = 0;
    m_tile = 0;
    m_oldTile = 0;
}

KisTiledIterator::~KisTiledIterator()
{
    if (m_tile)
        unlockTile(m_tile);
    if (m_oldTile)
        unlockTile(m_oldTile);
}

KisTiledIterator::KisTiledIterator(const KisTiledIterator& rhs)
        : KisShared()
{
    if (this != &rhs) {
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
    }
}

KisTiledIterator& KisTiledIterator::operator=(const KisTiledIterator & rhs)
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
    }
    return *this;
}

quint8 * KisTiledIterator::rawData() const
{
    return m_data + m_offset;
}


const quint8 * KisTiledIterator::oldRawData() const
{
#ifdef DEBUG
    // Warn if we're misusing oldRawData(). If there's no memento, oldRawData is the same
    // as rawData().
//    kWarning(!m_ktm->hasCurrentMemento(), 41004) << "Accessing oldRawData() when no transaction is in progress.";
#endif
    return m_oldData + m_offset;
}

void KisTiledIterator::fetchTileData(qint32 col, qint32 row)
{
    if (m_tile)
        unlockTile(m_tile);
    if (m_oldTile)
        unlockTile(m_oldTile);

    m_tile = m_dataManager->getTile(col, row);
    lockTile(m_tile);

    m_oldTile = m_dataManager->getOldTile(col, row);
    lockOldTile(m_oldTile);

    m_data = m_tile->data();
    m_oldData = m_oldTile->data();
}
