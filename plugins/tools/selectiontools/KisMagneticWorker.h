/*
 *  Copyright (c) 2019 Kuntal Majumder <hellozee@disroot.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KISMAGNETICWORKER_H
#define KISMAGNETICWORKER_H

#include <kis_paint_device.h>
#include <kritaselectiontools_export.h>

class KisMagneticLazyTiles{
private:
    QVector<QRect> m_tiles;
    QVector<qreal> m_radiusRecord;
    KisPaintDeviceSP m_dev;
    QSize m_tileSize;
    int m_tilesPerRow;

public:
    KisMagneticLazyTiles(KisPaintDeviceSP dev);
    void filter(qreal radius, QRect &rect);
    inline KisPaintDeviceSP device() { return m_dev; }
};

class KRITASELECTIONTOOLS_EXPORT KisMagneticWorker {
public:
    KisMagneticWorker(const KisPaintDeviceSP &dev);

    QVector<QPointF> computeEdge(int bounds, QPoint start, QPoint end, qreal radius);
    void saveTheImage(vQPointF points);

private:
    KisMagneticLazyTiles m_lazyTileFilter;
};

#endif // ifndef KISMAGNETICWORKER_H
