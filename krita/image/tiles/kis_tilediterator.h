/* This file is part of the KDE project
 *   Copyright (c) 2004 Casper Boemann <cbr@boemann.dkt>
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

#ifndef KIS_TILED_ITERATOR_H_
#define KIS_TILED_ITERATOR_H_

#include <qglobal.h>

#include <ksharedptr.h>

#include "kis_tile.h"
#include "kis_tileddatamanager.h"
#include "krita_export.h"
/**
 * The KisIterator class iterates through the pixels of a KisPaintDevice hiding the tile structure
 */
class KRITAIMAGE_EXPORT KisTiledIterator : public KShared {

protected:
    KisTiledDataManager *m_ktm;
    qint32 m_pixelSize;        // bytes per pixel
    qint32 m_x;        // current x position
    qint32 m_y;        // cirrent y position
    qint32 m_row;    // row in tilemgr
    qint32 m_col;    // col in tilemgr
    quint8 *m_data;
    quint8 *m_oldData;
    qint32 m_offset;
    KisTile *m_tile;
    bool m_writable;

protected:
    inline quint32 xToCol(quint32 x) const { if (m_ktm) return m_ktm->xToCol(x); else return 0; };
    inline quint32 yToRow(quint32 y) const { if (m_ktm) return m_ktm->yToRow(y); else return 0; };
    void fetchTileData(qint32 col, qint32 row);

public:
    KisTiledIterator( KisTiledDataManager *ktm);
    KisTiledIterator(const KisTiledIterator&);
    KisTiledIterator& operator=(const KisTiledIterator&);
    ~KisTiledIterator();

public:
    // current x position
    qint32 x() const { return m_x; };

    // cirrent y position
    qint32 y() const { return m_y; };

    /// Returns a pointer to the pixel data. Do NOT interpret the data - leave that to a colorstrategy
    quint8 *rawData() const;

    /// Returns a pointer to the pixel data as it was at the moment tof he last memento creation.
    const quint8 * oldRawData() const;
};

/**
 * The KisRectIterator class iterates through the pixels of a rect in a KisPaintDevice hiding the
 * tile structure
 */
class KRITAIMAGE_EXPORT KisTiledRectIterator : public KisTiledIterator
{

public:
    /// do not call constructor directly use factory method in KisDataManager instead.
    KisTiledRectIterator( KisTiledDataManager *dm, qint32  x, qint32  y, qint32  w, qint32  h, bool writable);
    KisTiledRectIterator(const KisTiledRectIterator&);
    KisTiledRectIterator& operator=(const KisTiledRectIterator&);
    ~KisTiledRectIterator();

public:
    qint32 nConseqPixels() const;

    /// Advances a number of pixels until it reaches the end of the rect
    KisTiledRectIterator & operator+=(int n);

    /// Advances one pixel. Going to the beginning of the next line when it reaches the end of a line
    KisTiledRectIterator & operator++();

    /// Goes back one pixel. Going to the end of the line above when it reaches the beginning of a line
    //KisTiledRectIterator & operator--();

    /// returns true when the iterator has reached the end
    inline bool isDone() const { return m_beyondEnd; }


protected:
     qint32 m_left;
     qint32 m_top;
     qint32 m_w;
     qint32 m_h;
     qint32 m_topRow;
     qint32 m_bottomRow;
     qint32 m_leftCol;
     qint32 m_rightCol;
     qint32 m_xInTile;
     qint32 m_yInTile;
     qint32 m_leftInTile;
     qint32 m_rightInTile;
     qint32 m_topInTile;
     qint32 m_bottomInTile;
     bool m_beyondEnd;

private:
     void nextTile();
};

/**
 * The KisHLineIterator class iterates through the pixels of a horizontal line in a KisPaintDevice hiding the
 * tile structure
 */
class KRITAIMAGE_EXPORT KisTiledHLineIterator : public KisTiledIterator
{

public:
    /// do not call constructor directly use factory method in KisDataManager instead.
    KisTiledHLineIterator( KisTiledDataManager *dm, qint32  x, qint32  y, qint32 w, bool writable);
    KisTiledHLineIterator(const KisTiledHLineIterator&);
    KisTiledHLineIterator& operator=(const KisTiledHLineIterator&);
    ~KisTiledHLineIterator();

public:
    /// Advances one pixel. Going to the beginning of the next line when it reaches the end of a line
    KisTiledHLineIterator & operator++();

    /// Returns the number of consequtive horizontal pixels that we point at
    /// This is useful for optimizing
    qint32 nConseqHPixels() const;

    /// Advances a number of pixels until it reaches the end of the line
    KisTiledHLineIterator & operator+=(int);

    /// Goes back one pixel. Going to the end of the line above when it reaches the beginning of a line
    KisTiledHLineIterator & operator--();

    /// returns true when the iterator has reached the end
    bool isDone() const { return m_x > m_right; }

    /// increment to the next row and rewind to the begining
    void nextRow();

protected:
     qint32 m_right;
     qint32 m_left;
     qint32 m_leftCol;
     qint32 m_rightCol;
     qint32 m_xInTile;
     qint32 m_yInTile;
     qint32 m_leftInTile;
     qint32 m_rightInTile;

private:
     void nextTile();
     void prevTile();
};

/**
 * The KisVLineIterator class iterates through the pixels of a vertical line in a KisPaintDevice hiding the
 * tile structure
 */
class KRITAIMAGE_EXPORT KisTiledVLineIterator : public KisTiledIterator
{

public:
    /// do not call constructor directly use factory method in KisDataManager instead.
    KisTiledVLineIterator( KisTiledDataManager *dm, qint32  x, qint32 y, qint32 h, bool writable);
    KisTiledVLineIterator(const KisTiledVLineIterator&);
    KisTiledVLineIterator& operator=(const KisTiledVLineIterator&);
    ~KisTiledVLineIterator();

public:
    /// Advances one pixel. Going to the beginning of the next line when it reaches the end of a line
    KisTiledVLineIterator & operator++();

    /// Goes back one pixel. Going to the end of the line above when it reaches the beginning of a line
    //KisTiledVLineIterator & operator--();

    /// returns true when the iterator has reached the end
    bool isDone() const { return m_y > m_bottom; }

    /// increment to the next column and rewind to the begining
    void nextCol();

protected:
    qint32 m_top;
    qint32 m_bottom;
    qint32 m_topRow;
    qint32 m_bottomRow;
    qint32 m_xInTile;
    qint32 m_yInTile;
    qint32 m_topInTile;
    qint32 m_bottomInTile;

private:
     void nextTile();
};

#endif // KIS_TILED_ITERATOR_H_
