/* 
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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
#ifndef KIS_RECT_ITERATOR
#define KIS_RECT_ITERATOR
#include "kis_base_iterator.h"

#include "kis_iterator_ng.h"

/**
 * The KisRectIterator class iterates through the pixels of a rect in a KisPaintDevice hiding the
 * tile structure
 */
class KRITAIMAGE_EXPORT KisRectIterator2 : public KisRectIteratorNG, KisBaseIterator
{
    KisRectIterator2(const KisRectIterator2&);
    KisRectIterator2& operator=(const KisRectIterator2&);

public:
    /// do not call constructor directly use factory method in KisDataManager instead.
    KisRectIterator2(KisTiledDataManager *dataManager, qint32  x, qint32  y, qint32  w, qint32  h, qint32 offsetX, qint32 offsetY, bool writable);
    ~KisRectIterator2();

public:
    qint32 nConseqPixels() const;

    /// Advances a number of pixels until it reaches the end of the rect
    bool nextPixels(qint32 n);

    /// Advances one pixel. Going to the beginning of the next line when it reaches the end of a line
    bool nextPixel();
    const quint8* rawDataConst() const;
    const quint8* oldRawData() const;
    quint8* rawData();
    qint32 x() const;
    qint32 y() const;

protected:
    qint32 m_x;
    qint32 m_y;
    qint32 m_row;
    qint32 m_col;
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
    KisTileSP m_tile;
    KisTileSP m_oldTile;
    qint32 m_offset;
    quint8* m_data;
    quint8* m_oldData;
private:
    void nextTile();
    void fetchTileData(qint32 col, qint32 row);
};
#endif
