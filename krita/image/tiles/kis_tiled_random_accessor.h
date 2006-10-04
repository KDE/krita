/*
 *  copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
 */
#ifndef KIS_TILED_RANDOM_ACCESSOR_H
#define KIS_TILED_RANDOM_ACCESSOR_H

#include <QRect>
#include <q3valuelist.h>

#include <ksharedptr.h>

#include <kis_tileddatamanager.h>

class KisTile;

class KisTiledRandomAccessor : public KShared {
    struct KisTileInfo {
        KisTile* tile;
        KisTile* oldtile;
        quint8* data;
        const quint8* oldData;
        qint32 area_x1, area_y1, area_x2, area_y2;
    };
    public:
        KisTiledRandomAccessor(KisTiledDataManager *ktm, qint32 x, qint32 y, bool writable);
        ~KisTiledRandomAccessor();


    private:
        inline quint32 xToCol(quint32 x) const { if (m_ktm) return m_ktm->xToCol(x); else return 0; };
        inline quint32 yToRow(quint32 y) const { if (m_ktm) return m_ktm->yToRow(y); else return 0; };
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
        static const quint32 CACHESIZE; // Define the number of tiles we keep in cache

};

#endif
