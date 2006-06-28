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

#include <qrect.h>
#include <qvaluelist.h>

#include <ksharedptr.h>

#include <kis_tileddatamanager.h>

class KisTile;

class KisTiledRandomAccessor : public KShared {
    struct KisTileInfo {
        KisTile* tile;
        KisTile* oldtile;
        Q_UINT8* data;
        const Q_UINT8* oldData;
        QRect area;
    };
    typedef QValueList<KisTileInfo> KisListTileInfo;
    public:
        KisTiledRandomAccessor(KisTiledDataManager *ktm, Q_INT32 x, Q_INT32 y, bool writable);
        ~KisTiledRandomAccessor();


    private:
        inline Q_UINT32 xToCol(Q_UINT32 x) const { if (m_ktm) return m_ktm->xToCol(x); else return 0; };
        inline Q_UINT32 yToRow(Q_UINT32 y) const { if (m_ktm) return m_ktm->yToRow(y); else return 0; };
        KisTileInfo fetchTileData(Q_INT32 col, Q_INT32 row);

    public:
        /// Move to a given x,y position, fetch tiles and data
        void moveTo(Q_INT32 x, Q_INT32 y);
        Q_UINT8* rawData() const;
        const Q_UINT8* oldRawData() const;

    private:
        KisTiledDataManager *m_ktm;
        KisListTileInfo m_tilesCache;
        Q_INT32 m_pixelSize;
        Q_UINT8* m_data;
        const Q_UINT8* m_oldData;
        bool m_writable;
        static const Q_UINT32 CACHESIZE; // Define the number of tiles we keep in cache

};

#endif
