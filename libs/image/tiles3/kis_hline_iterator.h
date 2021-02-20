/* 
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_HLINE_ITERATOR_H_
#define _KIS_HLINE_ITERATOR_H_

#include "kis_base_iterator.h"
#include "kritaimage_export.h"
#include "kis_iterator_ng.h"

class KRITAIMAGE_EXPORT KisHLineIterator2 : public KisHLineIteratorNG, public KisBaseIterator {
    KisHLineIterator2(const KisHLineIterator2&);
    KisHLineIterator2& operator=(const KisHLineIterator2&);

public:
    struct KisTileInfo {
        KisTileSP tile;
        KisTileSP oldtile;
        quint8* data;
        quint8* oldData;
    };


public:    
    KisHLineIterator2(KisDataManager *dataManager, qint32 x, qint32 y, qint32 w, qint32 offsetX, qint32 offsetY, bool writable, KisIteratorCompleteListener *listener);
    ~KisHLineIterator2() override;
    
    bool nextPixel() override;
    void nextRow() override;
    const quint8* oldRawData() const override;
    const quint8* rawDataConst() const override;
    quint8* rawData() override;
    qint32 nConseqPixels() const override;
    bool nextPixels(qint32 n) override;
    qint32 x() const override;
    qint32 y() const override;

    void resetPixelPos() override;
    void resetRowPos() override;

private:
    qint32 m_offsetX;
    qint32 m_offsetY;

    qint32 m_x;        // current x position
    qint32 m_y;        // current y position
    qint32 m_row;    // current row in tilemgr
    quint32 m_index;    // current col in tilemgr
    quint32 m_tileWidth;
    quint8 *m_data;
    quint8 *m_oldData;
    bool m_havePixels;
    
    qint32 m_right;
    qint32 m_left;
    qint32 m_top;
    qint32 m_leftCol;
    qint32 m_rightCol;

    qint32 m_rightmostInTile; // limited by the current tile border only

    qint32 m_leftInLeftmostTile;
    qint32 m_yInTile;

    QVector<KisTileInfo> m_tilesCache;
    quint32 m_tilesCacheSize;
    
private:

    void switchToTile(qint32 xInTile);
    void fetchTileDataForCache(KisTileInfo& kti, qint32 col, qint32 row);
    void preallocateTiles();
};
#endif
