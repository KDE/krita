/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_fixed_splats_plane.h"

KisFixedSplatsPlane::KisFixedSplatsPlane(KisBaseSplatsPlane *driedPlane, const KoColorSpace *colorSpace)
    : KisBaseSplatsPlane(true, driedPlane, colorSpace)
{
}

QRect KisFixedSplatsPlane::update(KisWetMap *wetMap)
{
    QRect dirtyRect;
    for (auto it = m_splats.begin(); it != m_splats.end();) {
        KisSplat *splat = *it;

        if (splat->update(wetMap) == KisSplat::Dried) {
            m_lowLvlPlane->add(splat);
            {
                // move to protected call to parent class
                it = m_splats.erase(it);
                splatsTree()->remove(splat);
                m_cachedPD->clear(splat->boundingRect().toAlignedRect());
                dirtyRect |= splat->boundingRect().toAlignedRect();
                QList<KisSplat *> rePaint = splatsTree()->intersects(splat->boundingRect().toAlignedRect());
                KisPainter *painter = new KisPainter(m_cachedPD);
                Q_FOREACH(KisSplat *reSplat, rePaint) {
                    if (reSplat != splat)
                        reSplat->doPaint(painter, splat->boundingRect().toAlignedRect());
                }
            }
        } else {
            ++it;
        }
    }

    return dirtyRect;
}

void KisFixedSplatsPlane::rewet(KisWetMap *wetMap, QPointF pos, qreal rad, KisBaseSplatsPlane *flowingPlane)
{
    QRectF rect(pos.x() - rad, pos.y() - rad,
                rad * 2, rad * 2);

    QList<KisSplat *> reweted = splatsTree()->intersects(rect);\
    Q_FOREACH(KisSplat *splat, reweted) {
        splat->rewet(wetMap, pos, rad);
        remove(splat);
        flowingPlane->add(splat);
    }
}
