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

const qint32 KisTile::WIDTH = 64;
const qint32 KisTile::HEIGHT = 64;


KisTile::KisTile(qint32 pixelSize, qint32 col, qint32 row, const quint8 *defPixel)
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

KisTile::KisTile(const KisTile& rhs, qint32 col, qint32 row)
{
    if (this != &rhs) {
        m_pixelSize = rhs.m_pixelSize;
        m_data = 0;
        m_nextTile = 0;
        m_nReadlock = 0;

        allocate();

        // If we ensure it's loaded, we won't need to modify the tiles with addReader and so,
        // which modify the constness much more explicitly than this one, that only does it
        // implicitly (that is, it has a non-const copy of the pointer tile somewhere)
        KisTileManager::instance()->ensureTileLoaded(&rhs);
        assert(rhs.m_data);
        if (rhs.m_data) {
            memcpy(m_data, rhs.m_data, WIDTH * HEIGHT * m_pixelSize * sizeof(quint8));
        }

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

        // Ditto as above
        KisTileManager::instance()->ensureTileLoaded(&rhs);
        assert(rhs.m_data);
        if (rhs.m_data) {
            memcpy(m_data, rhs.m_data, WIDTH * HEIGHT * m_pixelSize * sizeof(quint8));
        }

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

    //assert( !readers() );
    if( readers() ) kWarning() << "KisTile::~KisTile() readers is non zero! (" << readers() << ")\n";
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

quint8 *KisTile::data(qint32 x, qint32 y ) const
{
    Q_ASSERT(m_data != 0);
    //addReader(); [!] const!
    if (m_data == 0) return 0;
    //removeReader(); [!]

    return m_data + m_pixelSize * ( y * WIDTH + x );
}

void KisTile::setData(const quint8 *pixel)
{
    addReader();
    quint8 *dst = m_data;
    for(int i=0; i <WIDTH * HEIGHT;i++)
    {
        memcpy(dst, pixel, m_pixelSize);
        dst+=m_pixelSize;
    }
    removeReader();
}

void KisTile::addReader()
{
    if (m_nReadlock++ == 0)
        KisTileManager::instance()->ensureTileLoaded(this);
    else if (m_nReadlock < 0) {
        kDebug(41000) << m_nReadlock << endl;
        assert(0);
    }
    assert(m_data);
}

void KisTile::removeReader()
{
    if (--m_nReadlock == 0)
        KisTileManager::instance()->maySwapTile(this);
}
