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

//#include <qglobal.h>

#include <kis_shared.h>

#include "kis_tile.h"
#include "kis_tiled_data_manager.h"
#include "krita_export.h"

/**
 * The KisIterator class iterates through the pixels of a
 * KisPaintDevice hiding the tile structure
 */
class KisTiledIterator : public KisShared
{

protected:

    KisTiledDataManager *m_dataManager;
    qint32 m_pixelSize;        // bytes per pixel
    qint32 m_x;        // current x position
    qint32 m_y;        // current y position
    qint32 m_row;    // current row in tilemgr
    qint32 m_col;    // current col in tilemgr
    quint8 *m_data;
    quint8 *m_oldData;
    qint32 m_offset;
    KisTileSP m_tile;
    KisTileSP m_oldTile;
    bool m_writable;

protected:
    inline void lockTile(KisTileSP &tile) {
        if (m_writable)
            tile->lockForWrite();
        else
            tile->lockForRead();
    }
    inline void lockOldTile(KisTileSP &tile) {
        // Doesn't depend on current access type
        tile->lockForRead();
    }
    inline void unlockTile(KisTileSP &tile) {
        tile->unlock();
    }

    inline quint32 xToCol(quint32 x) const {
        return m_dataManager ? m_dataManager->xToCol(x) : 0;
    }
    inline quint32 yToRow(quint32 y) const {
        return m_dataManager ? m_dataManager->yToRow(y) : 0;
    }

    inline qint32 calcOffset(qint32 x, qint32 y) const {
        return m_pixelSize *(y * KisTileData::WIDTH + x);
    }

    inline qint32 calcXInTile(qint32 x, qint32 col) const {
        return x - col * KisTileData::WIDTH;
    }

    inline qint32 calcYInTile(qint32 y, qint32 row) const {
        return y - row * KisTileData::HEIGHT;
    }


    void fetchTileData(qint32 col, qint32 row);

public:
    KisTiledIterator(KisTiledDataManager *dataManager);
    KisTiledIterator(const KisTiledIterator&);
    KisTiledIterator& operator=(const KisTiledIterator&);
    ~KisTiledIterator();

public:
    // current x position
    qint32 x() const {
        return m_x;
    }

    // cirrent y position
    qint32 y() const {
        return m_y;
    }

    /// Returns a pointer to the pixel data. Do NOT interpret the data - leave that to a colorspace
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
    KisTiledRectIterator(KisTiledDataManager *dataManager, qint32  x, qint32  y, qint32  w, qint32  h, bool writable);
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
    inline bool isDone() const {
        return m_beyondEnd;
    }


protected:
    qint32 m_left;
    qint32 m_top;
    qint32 m_width;
    qint32 m_height;

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
    KisTiledHLineIterator(KisTiledDataManager *dm, qint32  x, qint32  y, qint32 w, bool writable);
    KisTiledHLineIterator(const KisTiledHLineIterator&);
    KisTiledHLineIterator& operator=(const KisTiledHLineIterator&);
    ~KisTiledHLineIterator();

public:
    /// Advances one pixel. Going to the beginning of the next line when it reaches the end of a line
    KisTiledHLineIterator & operator++();

    /// Returns the number of consecutive horizontal pixels that we point at
    /// This is useful for optimizing
    qint32 nConseqHPixels() const;

    /// Advances a number of pixels until it reaches the end of the line
    KisTiledHLineIterator & operator+=(int);

    /// Goes back one pixel. Going to the end of the line above when it reaches the beginning of a line
    KisTiledHLineIterator & operator--();

    /// returns true when the iterator has reached the end
    bool isDone() const {
        return m_isDoneFlag;
    }

    /// increment to the next row and rewind to the beginning
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

    bool m_isDoneFlag;

private:
    inline qint32 calcLeftInTile(qint32 col) const {
        return (col > m_leftCol) ? 0
               : m_left - m_leftCol * KisTileData::WIDTH;
    }

    inline qint32 calcRightInTile(qint32 col) const {
        return (col < m_rightCol)
               ? KisTileData::WIDTH - 1
               : m_right - m_rightCol * KisTileData::WIDTH;
    }

    void switchToTile(qint32 col, qint32 xInTile);
};

/**
 * The KisVLineIterator class iterates through the pixels of a vertical line in a KisPaintDevice hiding the
 * tile structure
 */
class KRITAIMAGE_EXPORT KisTiledVLineIterator : public KisTiledIterator
{

public:
    /// do not call constructor directly use factory method in KisDataManager instead.
    KisTiledVLineIterator(KisTiledDataManager *dm, qint32  x, qint32 y, qint32 h, bool writable);
    KisTiledVLineIterator(const KisTiledVLineIterator&);
    KisTiledVLineIterator& operator=(const KisTiledVLineIterator&);
    ~KisTiledVLineIterator();

public:
    /// Advances one pixel. Going to the beginning of the next line when it reaches the end of a line
    KisTiledVLineIterator & operator++();

    /// Goes back one pixel. Going to the end of the line above when it reaches the beginning of a line
    //KisTiledVLineIterator & operator--();

    /// returns true when the iterator has reached the end
    bool isDone() const {
        return m_isDoneFlag;
    }

    /// increment to the next column and rewind to the beginning
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
    bool m_isDoneFlag;
    qint32 m_lineStride;

private:
    inline qint32 calcTopInTile(qint32 row) const {
        return (row > m_topRow) ? 0
               : m_top - m_topRow * KisTileData::HEIGHT;
    }

    inline qint32 calcBottomInTile(qint32 row) const {
        return (row < m_bottomRow)
               ? KisTileData::HEIGHT - 1
               : m_bottom - m_bottomRow * KisTileData::HEIGHT;
    }

    void switchToTile(qint32 col, qint32 xInTile);
};

#endif // KIS_TILED_ITERATOR_H_
