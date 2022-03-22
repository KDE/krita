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
        quint8* data {nullptr};
        quint8* oldData {nullptr};
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
    qint32 m_offsetX {0};
    qint32 m_offsetY {0};

    qint32 m_x {0};        // current x position
    qint32 m_y {0};        // current y position
    qint32 m_row {0};    // current row in tilemgr
    quint32 m_index {0};    // current col in tilemgr
    quint32 m_tileWidth {0};
    quint8 *m_data {nullptr};
    quint8 *m_oldData {nullptr};
    bool m_havePixels {false};
    
    qint32 m_right {0};
    qint32 m_left {0};
    qint32 m_top {0};
    qint32 m_leftCol {0};
    qint32 m_rightCol {0};

    qint32 m_rightmostInTile {0}; // limited by the current tile border only

    qint32 m_leftInLeftmostTile {0};
    qint32 m_yInTile {0};

    QVector<KisTileInfo> m_tilesCache;
    quint32 m_tilesCacheSize {0};
    
private:

    void switchToTile(qint32 xInTile);
    void fetchTileDataForCache(KisTileInfo& kti, qint32 col, qint32 row);
    void preallocateTiles();
};
#endif
