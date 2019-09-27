/*
 *  copyright (c) 2006,2010 Cyrille Berger <cberger@cberger.net>
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
#ifndef KIS_TILED_RANDOM_ACCESSOR_H
#define KIS_TILED_RANDOM_ACCESSOR_H

#include <QRect>

#include <kis_shared.h>

#include "kis_tiled_data_manager.h"
#include "kis_random_accessor_ng.h"
#include "kis_iterator_complete_listener.h"


class KisRandomAccessor2 : public KisRandomAccessorNG
{

    struct KisTileInfo {
        KisTileSP tile;
        KisTileSP oldtile;
        quint8* data;
        const quint8* oldData;
        qint32 area_x1, area_y1, area_x2, area_y2;
    };

public:

    KisRandomAccessor2(KisTiledDataManager *ktm, qint32 x, qint32 y, qint32 offsetX, qint32 offsetY, bool writable, KisIteratorCompleteListener *completeListener);
    KisRandomAccessor2(const KisTiledRandomAccessor& lhs);
    ~KisRandomAccessor2() override;


private:
    inline void lockTile(KisTileSP &tile) {
        if (m_writable)
            tile->lockForWrite();
        else
            tile->lockForRead();
    }

    inline void lockOldTile(KisTileSP &tile) {
        // Doesn't depend on access type
        tile->lockForRead();
    }

    inline void unlockTile(KisTileSP &tile) {
        if (m_writable) {
            tile->unlockForWrite();
        } else {
            tile->unlockForRead();
        }
    }

    inline void unlockOldTile(KisTileSP &tile) {
        tile->unlockForRead();
    }

    inline quint32 xToCol(quint32 x) const {
        return m_ktm ? m_ktm->xToCol(x) : 0;
    }
    inline quint32 yToRow(quint32 y) const {
        return m_ktm ? m_ktm->yToRow(y) : 0;
    }

    KisTileInfo* fetchTileData(qint32 col, qint32 row);

public:
    /// Move to a given x,y position, fetch tiles and data
    void moveTo(qint32 x, qint32 y) override;
    quint8* rawData() override;
    const quint8* oldRawData() const override;
    const quint8* rawDataConst() const override;
    qint32 numContiguousColumns(qint32 x) const override;
    qint32 numContiguousRows(qint32 y) const override;
    qint32 rowStride(qint32 x, qint32 y) const override;
    qint32 x() const override;
    qint32 y() const override;

private:
    KisTiledDataManager *m_ktm;
    KisTileInfo** m_tilesCache;
    quint32 m_tilesCacheSize;
    qint32 m_pixelSize;
    quint8* m_data;
    const quint8* m_oldData;
    bool m_writable;
    int m_lastX, m_lastY;
    qint32 m_offsetX, m_offsetY;
    KisIteratorCompleteListener *m_completeListener;
    static const quint32 CACHESIZE; // Define the number of tiles we keep in cache

};

#endif
