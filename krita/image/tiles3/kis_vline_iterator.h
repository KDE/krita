/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef _KIS_VLINE_ITERATOR_H_
#define _KIS_VLINE_ITERATOR_H_

#include "kis_base_iterator.h"
#include "krita_export.h"
#include "kis_iterator_ng.h"

class KRITAIMAGE_EXPORT KisVLineIterator2 : public KisVLineIteratorNG, KisBaseIterator {
    KisVLineIterator2(const KisVLineIterator2&);
    KisVLineIterator2& operator=(const KisVLineIterator2&);

public:
    struct KisTileInfo {
        KisTileSP tile;
        KisTileSP oldtile;
        quint8* data;
        quint8* oldData;
    };


public:
    KisVLineIterator2(KisDataManager *dataManager, qint32 x, qint32 y, qint32 h, qint32 offsetX, qint32 offsetY, bool writable);
    ~KisVLineIterator2();

    virtual bool nextPixel();
    virtual void nextColumn();
    virtual const quint8* oldRawData() const;
    virtual quint8* rawData();
    virtual qint32 nConseqPixels() const;
    virtual bool nextPixels(qint32 n);
    virtual qint32 x() const;
    virtual qint32 y() const;

private:
    qint32 m_x;        // current x position
    qint32 m_y;        // current y position
    qint32 m_column;    // current column in tilemgr
    qint32 m_index;    // current row in tilemgr
    qint32 m_tileSize;
    quint8 *m_data;
    quint8 *m_dataBottom;
    quint8 *m_oldData;
    bool m_havePixels;

    qint32 m_top;
    qint32 m_bottom;
    qint32 m_topRow;
    qint32 m_bottomRow;

    qint32 m_topInTopmostTile;
    qint32 m_xInTile;
    qint32 m_lineStride;

    QVector<KisTileInfo> m_tilesCache;
    qint32 m_tilesCacheSize;

private:

    void switchToTile(qint32 xInTile);
    void fetchTileDataForCache(KisTileInfo& kti, qint32 col, qint32 row);
    void preallocateTiles();
};
#endif
