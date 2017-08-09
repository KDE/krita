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

KisBaseSplatsPlane::KisBaseSplatsPlane(bool useCaching, KisBaseSplatsPlane *lowLvlPlane)
    : m_lowLvlPlane(lowLvlPlane),
      m_isDirty(true),
      m_useCaching(useCaching)
{
}

KisBaseSplatsPlane::~KisBaseSplatsPlane()
{

}

void KisBaseSplatsPlane::add(KisSplat *splat)
{
    m_splats << splat;
    setDirty(splat->boundingRect().toAlignedRect());
}

void KisBaseSplatsPlane::remove(KisSplat *splat)
{
    m_splats.removeOne(splat);
    setDirty(splat->boundingRect().toAlignedRect());
}

void KisBaseSplatsPlane::paint(KisPainter *gc, QRect rect)
{
    if (m_useCaching) {

        if (!m_dirtyRect.isNull()) {
            if (!m_cachedPD) {
                m_cachedPD = new KisPaintDevice(gc->device()->colorSpace());
            } else {
                m_cachedPD->clear(m_dirtyRect);
            }

            KisPainter *painter = new KisPainter(m_cachedPD);
            Q_FOREACH (KisSplat *splat, m_splats) {
                if (m_dirtyRect.contains(splat->boundingRect().toAlignedRect()))
                    splat->doPaint(painter);
            }
//            m_isDirty = false;
        }
        gc->bitBlt(rect.topLeft(),
                   m_cachedPD,
                   rect);
    } else {
        Q_FOREACH (KisSplat *splat, m_splats) {
            splat->doPaint(gc);
        }
    }
}

QRect KisBaseSplatsPlane::update(KisWetMap *wetMap)
{
//    QRect dirtyRect;

    for (auto it = m_splats.begin(); it != m_splats.end();) {
        KisSplat *splat = *it;
        setDirty( splat->boundingRect().toAlignedRect());
        if (splat->update(wetMap) == KisSplat::Fixed) {
            m_lowLvlPlane->add(splat);
            {
                // move to protected call to parent class
                it = m_splats.erase(it);
                setDirty(splat->boundingRect().toAlignedRect());

            }
//            setDirty(splat->boundingRect().toAlignedRect());
        } else {
            setDirty(splat->boundingRect().toAlignedRect());
            ++it;
        }
    }

    return m_dirtyRect;
}

void KisBaseSplatsPlane::setDirty(const QRect &rc)
{
    m_dirtyRect |= rc;
//    m_isDirty = true;
}
