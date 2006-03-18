/*
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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
#include "kis_global.h"
#include "kis_memento.h"
#include "kis_tile.h"
#include "kis_tile_global.h"

KisMemento::KisMemento(Q_UINT32 pixelSize) : KShared()
{
    m_hashTable = new KisTile * [1024];
    Q_CHECK_PTR(m_hashTable);

    m_redoHashTable = new KisTile * [1024];
    Q_CHECK_PTR(m_redoHashTable);

    for(int i = 0; i < 1024; i++)
    {
        m_hashTable [i] = 0;
        m_redoHashTable [i] = 0;
    }
    m_numTiles = 0;
    m_defPixel = new Q_UINT8[pixelSize];
    m_redoDefPixel = new Q_UINT8[pixelSize];
    m_valid = true;
}

KisMemento::~KisMemento()
{
    // Deep delete every tile
    for(int i = 0; i < 1024; i++)
    {
        deleteAll(m_hashTable[i]);
        deleteAll(m_redoHashTable[i]);
    }
    delete [] m_hashTable;
    delete [] m_redoHashTable;

    // Delete defPixel arrays;
    delete [] m_defPixel;
    delete [] m_redoDefPixel;
}

KisMemento::DeletedTileList::~DeletedTileList()
{
    clear();
}

void KisMemento::DeletedTileList::clear()
{
    // They are not tiles just references. The actual tiles have already been deleted,
    // so just delete the references.

    const DeletedTile *deletedTile = m_firstDeletedTile;

    while (deletedTile)
    {
        const DeletedTile *d = deletedTile;
        deletedTile = deletedTile->next();
        delete d;
    }

    m_firstDeletedTile = 0;
}

void KisMemento::deleteAll(KisTile *tile)
{
    while(tile)
    {
        KisTile *deltile = tile;
        tile = tile->getNext();
        delete deltile;
    }
}

void KisMemento::extent(Q_INT32 &x, Q_INT32 &y, Q_INT32 &w, Q_INT32 &h) const
{
    Q_INT32 maxX = Q_INT32_MIN;
    Q_INT32 maxY = Q_INT32_MIN;
    x = Q_INT32_MAX;
    y = Q_INT32_MAX;

    for(int i = 0; i < 1024; i++)
    {
        KisTile *tile = m_hashTable[i];

        while(tile)
        {
            if(x > tile->getCol() * KisTile::WIDTH)
                x = tile->getCol() * KisTile::WIDTH;
            if(maxX < (tile->getCol() + 1) * KisTile::WIDTH - 1)
                maxX = (tile->getCol() + 1) * KisTile::WIDTH - 1;
            if(y > tile->getRow() * KisTile::HEIGHT)
                y = tile->getRow() * KisTile::HEIGHT;
            if(maxY < (tile->getRow() +1) * KisTile::HEIGHT - 1)
                maxY = (tile->getRow() +1) * KisTile::HEIGHT - 1;

            tile = tile->getNext();
        }
    }

    if(maxX < x)
        w = 0;
    else
        w = maxX - x +1;

    if(maxY < y)
        h = 0;
    else
        h = maxY - y +1;
}

QRect KisMemento::extent() const
{
    Q_INT32 x;
    Q_INT32 y;
    Q_INT32 w;
    Q_INT32 h;

    extent(x, y, w, h);

    return QRect(x, y, w, h);
}

bool KisMemento::containsTile(Q_INT32 col, Q_INT32 row, Q_UINT32 tileHash) const
{
    const KisTile *tile = m_hashTable[tileHash];

    while (tile != 0)
    {
        if (tile->getRow() == row && tile->getCol() == col) {
            return true;
        }

        tile = tile->getNext();
    }

    return false;
}

