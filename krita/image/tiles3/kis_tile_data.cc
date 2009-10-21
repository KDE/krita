/*
 *  Copyright (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
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


#include "kis_tile_data.h"
#include "kis_tile_data_store.h"


const qint32 KisTileData::WIDTH = 64;
const qint32 KisTileData::HEIGHT = 64;


KisTileData::KisTileData(qint32 pixelSize, const quint8 *defPixel, KisTileDataStore *store)
        : m_state(NORMAL),
        m_usersCount(0),
        m_refCount(0),
        m_pixelSize(pixelSize),
        m_store(store)
{
    m_nextTD = m_prevTD = this;

    m_data = new quint8[m_pixelSize*WIDTH*HEIGHT];

    fillWithPixel(defPixel);
}


/**
 * Duplicating tiledata
 * + new object loaded in memory
 * + it's unlocked and has refCount==0
 */
KisTileData::KisTileData(const KisTileData& rhs)
        : m_state(NORMAL),
        m_usersCount(0),
        m_refCount(0),
        m_pixelSize(rhs.m_pixelSize),
        m_store(rhs.m_store)
{
    m_nextTD = m_prevTD = this;

    const quint32 tileDataSize = m_pixelSize * WIDTH * HEIGHT;
    m_data = new quint8[tileDataSize];

    rhs.m_store->ensureTileDataLoaded(&rhs);
    memcpy(m_data, rhs.data(), tileDataSize);

}


KisTileData::~KisTileData()
{
    /* FIXME: this _|_ */
    m_store->ensureTileDataLoaded(this);

    if (m_data)
        delete[] m_data;

    /* Free clones list */
    KisTileData *td;
    foreach(td, m_clonesList) {
        delete td;
    }
}

void KisTileData::fillWithPixel(const quint8 *defPixel)
{
    quint8 *it = m_data;

    for (int i = 0; i < WIDTH*HEIGHT; i++, it += m_pixelSize) {
        memcpy(it, defPixel, m_pixelSize);
    }
}



