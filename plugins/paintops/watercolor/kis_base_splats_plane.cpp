/* This file is part of the KDE project
 *
 * Copyright (C) 2017 Grigory Tantsevov <tantsevov@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_base_splats_plane.h"

KisBaseSplatsPlane::KisBaseSplatsPlane(bool useCaching, KisBaseSplatsPlane *lowLvlPlane, const KoColorSpace* colorSpace)
    : m_lowLvlPlane(lowLvlPlane),
      m_isDirty(true),
      m_useCaching(useCaching)
{
    if (useCaching)
        m_cachedPD = new KisPaintDevice(colorSpace);
}

KisBaseSplatsPlane::~KisBaseSplatsPlane()
{

}

void KisBaseSplatsPlane::add(KisSplat *splat)
{
    m_splats << splat;
    if (m_useCaching) {
        KisPainter *painter = new KisPainter(m_cachedPD);
        splat->doPaint(painter);
    }
}

void KisBaseSplatsPlane::remove(KisSplat *splat)
{
    m_splats.removeOne(splat);
    if (m_useCaching) {
        m_cachedPD->clear(splat->boundingRect().toAlignedRect());
    }
}

void KisBaseSplatsPlane::paint(KisPainter *gc, QRect rect)
{
    if (rect.isNull())
        return;

    if (m_useCaching) {
        gc->bitBlt(rect.topLeft(),
                   m_cachedPD,
                   rect);
    } else {
        Q_FOREACH (KisSplat *splat, m_splats) {
            if (rect.contains(splat->boundingRect().toAlignedRect()))
                splat->doPaint(gc);
        }
    }
}

QRect KisBaseSplatsPlane::update(KisWetMap *wetMap)
{
    QRect dirtyRect;

    for (auto it = m_splats.begin(); it != m_splats.end();) {
        KisSplat *splat = *it;

        dirtyRect |= splat->boundingRect().toAlignedRect();
        if (splat->update(wetMap) == KisSplat::Fixed) {
            m_lowLvlPlane->add(splat);
            {
                // move to protected call to parent class
                it = m_splats.erase(it);
                setDirty(splat);
                dirtyRect |= splat->boundingRect().toAlignedRect();
            }
        } else {
            setDirty(splat);
            dirtyRect |= splat->boundingRect().toAlignedRect();
            ++it;
        }
    }

    return dirtyRect;
}

void KisBaseSplatsPlane::setDirty(KisSplat *splat)
{
    m_dirtySplats << splat;
    m_isDirty = true;
}
