/*
 *  SPDX-FileCopyrightText: 2019 Kuntal Majumder <hellozee@disroot.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef KISMAGNETICWORKER_H
#define KISMAGNETICWORKER_H

#include <kis_paint_device.h>
#include <kritaselectiontools_export.h>

struct KisMagneticGraph;

class KisMagneticLazyTiles {
private:
    QVector<QRect> m_tiles;
    QVector<qreal> m_radiusRecord;
    KisPaintDeviceSP m_dev;
    QSize m_tileSize;
    int m_tilesPerRow;

public:
    KisMagneticLazyTiles(KisPaintDeviceSP dev);
    void filter(qreal radius, QRect &rect);
    inline KisPaintDeviceSP device(){ return m_dev; }
    inline QVector<QRect> tiles(){ return m_tiles; }
};

class KRITASELECTIONTOOLS_EXPORT KisMagneticWorker {
public:
    KisMagneticWorker(const KisPaintDeviceSP &dev);

    QVector<QPointF> computeEdge(int bounds, QPoint start, QPoint end, qreal radius);
    void saveTheImage(vQPointF points);
    qreal intensity(QPoint pt);

private:
    KisMagneticLazyTiles m_lazyTileFilter;
    KisMagneticGraph *m_graph;
};

#endif // ifndef KISMAGNETICWORKER_H
