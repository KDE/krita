/*
 *  copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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
#include <QList>

#include <kis_shared.h>

#include <kis_tileddatamanager.h>

class KisTile;

class KisTiledRandomAccessor : public KisShared
{

    struct KisTileInfo {
        KisTile* tile;
        KisTile* oldtile;
        quint8* data;
        const quint8* oldData;
        qint32 area_x1, area_y1, area_x2, area_y2;
    };

public:

    KisTiledRandomAccessor(KisTiledDataManager *ktm, qint32 x, qint32 y, bool writable);
    KisTiledRandomAccessor(const KisTiledRandomAccessor& lhs);
    ~KisTiledRandomAccessor();


private:
    inline quint32 xToCol(quint32 x) const {
        if (m_ktm) return m_ktm->xToCol(x); else return 0;
    }
    inline quint32 yToRow(quint32 y) const {
        if (m_ktm) return m_ktm->yToRow(y); else return 0;
    }
    KisTileInfo* fetchTileData(qint32 col, qint32 row);

public:
    /// Move to a given x,y position, fetch tiles and data
    void moveTo(qint32 x, qint32 y);
    quint8* rawData() const;
    const quint8* oldRawData() const;

private:
    KisTiledDataManager *m_ktm;
    KisTileInfo** m_tilesCache;
    quint32 m_tilesCacheSize;
    qint32 m_pixelSize;
    quint8* m_data;
    const quint8* m_oldData;
    bool m_writable;
    int m_lastX, m_lastY;
    static const quint32 CACHESIZE; // Define the number of tiles we keep in cache

};

#endif
