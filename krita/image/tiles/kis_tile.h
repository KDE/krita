/*
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
#ifndef KIS_TILE_H_
#define KIS_TILE_H_

#include <qglobal.h>
#include <QRect>

class KisTiledDataManager;
class KisTiledIterator;

/**
 * Provides abstraction to a tile.  A tile contains
 * a part of a PaintDevice, but only the individual pixels
 * are accesable and that only via iterators.
 */
class KisTile  {
public:
    KisTile(qint32 pixelSize, qint32 col, qint32 row, const quint8 *defPixel);
    KisTile(const KisTile& rhs, qint32 col, qint32 row);
    KisTile(const KisTile& rhs);
    ~KisTile();

public:
    void release();
    void allocate();

    quint8 *data(qint32 xoff, qint32 yoff) const;
    quint8 *data() const { return m_data; }

    void setData(const quint8 *pixel);

    qint32 refCount() const;
    void ref();

    qint32 getRow() const { return m_row; }
    qint32 getCol() const { return m_col; }

    QRect extent() const { return QRect(m_col * WIDTH, m_row * HEIGHT, WIDTH, HEIGHT); }

    void setNext(KisTile *);
    KisTile *getNext() const { return m_nextTile; }

    /// Functions that are needed for locking the tiles into memory for caching
    void addReader();
    void removeReader();
    qint32 readers() { return m_nReadlock; }

    friend class KisTiledIterator;
    friend class KisTiledDataManager;
    friend class KisMemento;
    friend class KisTileManager;
private:
    KisTile& operator=(const KisTile&);

private:
    quint8 *m_data;
    qint32 m_nReadlock;
    qint32 m_row;
    qint32 m_col;
    qint32 m_pixelSize;
    KisTile *m_nextTile;

public:
    static const qint32 WIDTH;
    static const qint32 HEIGHT;
};

#endif // KIS_TILE_H_

