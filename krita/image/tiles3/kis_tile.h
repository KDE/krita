/*
 *  Copyright (c) 2004 Casper Boemann <cbr@boemann.dk>
 *            (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
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
#ifndef KIS_TILE_H_
#define KIS_TILE_H_

#include <QReadWriteLock>
#include <QRect>

#include "kis_tile_data.h"
#include "kis_tile_data_store.h"

//class KisTiledDataManager;
//class KisTiledIterator;



/**
 * Provides abstraction to a tile.  
 * + A tile contains a part of a PaintDevice, 
 *   but only the individual pixels are accesable 
 *   and that only via iterators.
 * + Actual tile data is stored in KisTileData that can be 
 *   shared between many tiles
 */
class KisTile
{
public:
    KisTile(qint32 col, qint32 row, KisTileData *defaultTileData);
    KisTile(qint32 col, qint32 row, const KisTile& rhs);
    KisTile(const KisTile& rhs);
    ~KisTile();


public:
    
    void debugPrintInfo();

    void lockForRead() const;
    void lockForWrite();
    void unlock() const;

    /* this allows us work directly on tile's data */
    inline qint8 *data() const {
        return m_tileData->data();
    }
    inline void setData(const qint8 *data) {
        m_tileData->setData(data);
    }

    inline qint32 row() const {
        return m_row;
    }
    inline qint32 col() const {
        return m_col;
    }

    inline QRect extent() const {
        return QRect(m_col * KisTileData::WIDTH, m_row * KisTileData::HEIGHT,
                     KisTileData::WIDTH, KisTileData::HEIGHT);
    }

    inline KisTile *next() const {
        return m_nextTile;
    }
    
    void setNext(KisTile *next) {
        m_nextTile=next;
    }

    inline qint32 pixelSize() const {
        /* don't lock here as pixelSize is constant */
        return m_tileData->pixelSize();
    }

    inline KisTileData*  defaultTileData() const {
        return m_defaultTileData;
    }

    inline KisTileData*  tileData() const {
        return m_tileData;
    }

private:

/*    friend class KisTiledIterator;
    friend class KisTiledDataManager;
    friend class KisMemento;
    friend class KisTileManager;
    friend class KisTileCompressor;
    friend class KisTiledRandomAccessor;
*/
    /*Why it was present before? */
/*  KisTile& operator=(const KisTile&);*/
    void init(qint32 col, qint32 row, KisTileData *defaultTileData);

private:
    KisTileData *m_tileData;
    KisTileData *m_defaultTileData;

    qint32 m_col;
    qint32 m_row;


    /**
     * For KisTiledDataManager's hash table
     */
    KisTile *m_nextTile;
};

#endif // KIS_TILE_H_

