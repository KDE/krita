/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_VLINE_ITERATOR_H_
#define _KIS_VLINE_ITERATOR_H_

#include "kis_base_iterator.h"
#include "kritaimage_export.h"
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
    KisVLineIterator2(KisDataManager *dataManager, qint32 x, qint32 y, qint32 h, qint32 offsetX, qint32 offsetY, bool writable, KisIteratorCompleteListener *completeListener);
    ~KisVLineIterator2() override;

    void resetPixelPos() override;
    void resetColumnPos() override;

    bool nextPixel() override;
    void nextColumn() override;
    const quint8* rawDataConst() const override;
    const quint8* oldRawData() const override;
    quint8* rawData() override;
    qint32 nConseqPixels() const override;
    bool nextPixels(qint32 n) override;
    qint32 x() const override;
    qint32 y() const override;

private:
    qint32 m_offsetX;
    qint32 m_offsetY;

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
    qint32 m_left;
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
