/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_LOD_TRANSFORM_H
#define __KIS_LOD_TRANSFORM_H

#include <QTransform>
#include <kis_paint_information.h>

class KisLodTransform {
public:
    KisLodTransform(qreal lodScale) {
        m_transform = QTransform::fromScale(lodScale, lodScale);
    }

    KisLodTransform(int levelOfDetail) {
        qreal scale = lodToScale(levelOfDetail);
        m_transform = QTransform::fromScale(scale, scale);
    }

    KisLodTransform(KisPaintDeviceSP device) {
        qreal scale = lodToScale(device->defaultBounds()->currentLevelOfDetail());
        m_transform = QTransform::fromScale(scale, scale);
    }

    static qreal lodToScale(int levelOfDetail) {
        return 1.0 / (1 << qMax(0, levelOfDetail));
    }

    static qreal lodToScale(KisPaintDeviceSP device) {
        return lodToScale(device->defaultBounds()->currentLevelOfDetail());
    }

    QRectF map(const QRectF &rc) const {
        return m_transform.mapRect(rc);
    }

    QRect map(const QRect &rc) const {
        return m_transform.mapRect(rc);
    }

    KisPaintInformation map(KisPaintInformation pi) const {
        QPointF pos = pi.pos();
        pi.setPos(m_transform.map(pos));
        return pi;
    }

    vQPointF map(vQPointF v) const {
        vQPointF::iterator it = v.begin();
        vQPointF::iterator end = v.end();

        while (it != end) {
            *it = m_transform.map(*it);
        }

        return v;
    }

    template <class T>
    T map(const T &object) const {
        return m_transform.map(object);
    }

private:
    QTransform m_transform;
};

#endif /* __KIS_LOD_TRANSFORM_H */
