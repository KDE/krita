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

KisFixedSplatsPlane::KisFixedSplatsPlane(KisBaseSplatsPlane *driedPlane)
    : KisBaseSplatsPlane(true, driedPlane)
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
                setDirty(splat->boundingRect().toAlignedRect());
            }
            dirtyRect |= splat->boundingRect().toAlignedRect();
        } else {
            ++it;
        }
    }

    return dirtyRect;
}

