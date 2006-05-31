/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
#include <assert.h>
#include <kdebug.h>

#include "kis_tile_global.h"
#include "kis_tile.h"
#include "kis_tileddatamanager.h"
#include "kis_tilemanager.h"

const Q_INT32 KisTile::WIDTH = 64;
const Q_INT32 KisTile::HEIGHT = 64;


KisTile::KisTile(Q_INT32 pixelSize, Q_INT32 col, Q_INT32 row, const Q_UINT8 *defPixel)
{
    m_pixelSize = pixelSize;
    m_data = 0;
    m_nextTile = 0;
    m_col = col;
    m_row = row;
    m_nReadlock = 0;

    allocate();

    KisTileManager::instance()->registerTile(this);

    setData(defPixel);
}

KisTile::KisTile(const KisTile& rhs, Q_INT32 col, Q_INT32 row)
{
    if (this != &rhs) {
        m_pixelSize = rhs.m_pixelSize;
        m_data = 0;
        m_nextTile = 0;
        m_nReadlock = 0;

        allocate();

        // Assure we have data to copy
        rhs.addReader();
        memcpy(m_data, rhs.m_data, WIDTH * HEIGHT * m_pixelSize * sizeof(Q_UINT8));
        rhs.removeReader();

        m_col = col;
        m_row = row;

        KisTileManager::instance()->registerTile(this);
    }
}

KisTile::KisTile(const KisTile& rhs)
{
    if (this != &rhs) {
        m_pixelSize = rhs.m_pixelSize;
        m_col = rhs.m_col;
        m_row = rhs.m_row;
        m_data = 0;
        m_nextTile = 0;
        m_nReadlock = 0;

        allocate();

        rhs.addReader();
        memcpy(m_data, rhs.m_data, WIDTH * HEIGHT * m_pixelSize * sizeof(Q_UINT8));
        rhs.removeReader();

        KisTileManager::instance()->registerTile(this);
    }
}

KisTile::~KisTile()
{
    KisTileManager::instance()->deregisterTile(this); // goes before the deleting of m_data!

    if (m_data) {
//        delete[] m_data;
        KisTileManager::instance()->dontNeedTileData(m_data, m_pixelSize);
        m_data = 0;
    }
    assert( !readers() );
}

void KisTile::allocate()
{
    if (m_data == 0) {
        assert (!readers());
        m_data = KisTileManager::instance()->requestTileData(m_pixelSize);
        Q_CHECK_PTR(m_data);
    }
}

void KisTile::setNext(KisTile *n)
{
    m_nextTile = n;
}

Q_UINT8 *KisTile::data(Q_INT32 x, Q_INT32 y ) const
{
    addReader();
    removeReader();

    Q_ASSERT(m_data != 0);
    if (m_data == 0) return 0;

    return m_data + m_pixelSize * ( y * WIDTH + x );
}

void KisTile::setData(const Q_UINT8 *pixel)
{
    addReader();
    Q_UINT8 *dst = m_data;
    for(int i=0; i <WIDTH * HEIGHT;i++)
    {
        memcpy(dst, pixel, m_pixelSize);
        dst+=m_pixelSize;
    }
    removeReader();
}

void KisTile::addReader() const
{
    if (m_nReadlock++ == 0)
        KisTileManager::instance()->ensureTileLoaded(this);
    else if (m_nReadlock < 0) {
        kdDebug(41000) << m_nReadlock << endl;
        assert(0);
    }
    assert(m_data);
}

void KisTile::removeReader() const
{
    if (--m_nReadlock == 0)
        KisTileManager::instance()->maySwapTile(this);
}
