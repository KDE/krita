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
#ifndef KIS_TILE_DATA_H_
#define KIS_TILE_DATA_H_

#include <string.h>
#include <QReadWriteLock>
#include <QAtomicInt>
#include <QList>

class KisTileData;
class KisTileDataStore;

typedef QList<KisTileData*> KisTileDataCache;

/**
 * Stores actual tile's data
 */
class KisTileData
{
public:
    KisTileData(qint32 pixelSize, const quint8 *defPixel, KisTileDataStore *store);
private:
    KisTileData(const KisTileData& rhs);

public:
    ~KisTileData();

    enum EnumTileDataState {
        NORMAL = 0,
        COMPRESSED,
        SWAPPED
    };

    inline quint8* data() const {
        Q_ASSERT(m_data);
        return m_data;
    }

    void setData(const quint8 *data) {
        Q_ASSERT(m_data);
        memcpy(m_data, data, m_pixelSize*WIDTH*HEIGHT);
    }


    inline quint32 pixelSize() const {
        return m_pixelSize;
    }


private:
    void fillWithPixel(const quint8 *defPixel);

private:
    friend class KisTileDataPooler;
    /**
     * A list of pre-duplicated tiledatas.
     * To make a COW faster, KisTileDataPooler thread duplicates
     * a tile beforehand and stores clones here, in this list
     */
    KisTileDataCache m_clonesList;

private:
    friend class KisTile;
    friend class KisTileDataStore;

    /**
     * The state of the tile.
     * Filled in by tileDataStore and
     * checked in KisTile::acquireFor*
     * see also: comment for @m_data
     */
    mutable EnumTileDataState m_state;

    /**
     * Next and previous tiledata elements in the linked list
     * of parental datastore
     */
    KisTileData *m_nextTD;
    KisTileData *m_prevTD;

private:

    /**
     * FIXME: We should be able to work in const environment
     * even when actual data is swapped out to disk
     */
    mutable quint8* m_data;

    /**
     * How many tiles/mementoes use
     * this tiledata through COW?
     */
    mutable QAtomicInt m_usersCount;

    /**
     * Shared pointer counter
     */
    mutable QAtomicInt m_refCount;


    qint32 m_pixelSize;
    //qint32 m_timeStamp;

    KisTileDataStore *m_store;

public:
    static const qint32 WIDTH;
    static const qint32 HEIGHT;
};

#endif /* KIS_TILE_DATA_H_ */

