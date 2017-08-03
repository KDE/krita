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

KisBaseSplatsPlane::KisBaseSplatsPlane(const KoColorSpace *colorSpace)
{
    m_cachedPD = new KisPaintDevice(colorSpace);
}

void KisBaseSplatsPlane::add(KisSplat *splat)
{
    KisPainter *painter = new KisPainter(m_cachedPD);
    splat->doPaint(painter);
}

void KisBaseSplatsPlane::remove(KisSplat *splat)
{
    m_cachedPD->clear(splat->boundingRect().toRect());
}

void KisBaseSplatsPlane::paint(KisPainter *gc, QRect rect)
{
    gc->bitBlt(rect.topLeft(),
               m_cachedPD,
               rect);
}
