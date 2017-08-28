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
    : KisBaseSplatsPlane(true, driedPlane, colorSpace),
      m_splatsTree(4, 2)
{
}

QRect KisFixedSplatsPlane::update(KisWetMap *wetMap)
{
    for (auto it = m_splats.begin(); it != m_splats.end();) {
        KisSplat *splat = *it;

        if (splat->update(wetMap) == KisSplat::Dried) {
            m_lowLvlPlane->add(splat);
            {
                // move to protected call to parent class
                it = m_splats.erase(it);
                if (m_useCaching) {
                    m_cachedPD->clear(splat->boundingRect().toAlignedRect());
                }
                setDirty(splat);
            }
        } else {
            ++it;
        }
    }

    return QRect();
}

void KisFixedSplatsPlane::rewet(KisWetMap *wetMap, QPointF pos, qreal rad, KisBaseSplatsPlane *flowingPlane)
{
    QRectF rect(pos.x() - rad, pos.y() - rad,
                rad * 2, rad * 2);

    QList<KisSplat *> reweted = m_splatsTree.intersects(rect);\
    Q_FOREACH(KisSplat *splat, reweted) {
        splat->rewet(wetMap, pos, rad);
        remove(splat);
        m_splatsTree.remove(splat);
        flowingPlane->add(splat);
    }
}
