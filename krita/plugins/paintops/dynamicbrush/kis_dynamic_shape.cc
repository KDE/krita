/*
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_dynamic_shape.h"

#include "kis_qimage_mask.h"
#include "kis_autobrush_resource.h"

quint8 KisAlphaMaskBrush::alphaAt(int x, int y)
{
    return alphaMask->alphaAt(x,y);
}

quint8 KisAutoMaskBrush::alphaAt(int x, int y)
{
    return 255 - m_shape->valueAt(x,y);
}

KisAutoMaskBrush::~KisAutoMaskBrush() { if(m_shape) delete m_shape; }

KisDabBrush::~KisDabBrush() { }
KisDabBrush::KisDabBrush() { }


void KisAlphaMaskBrush::resize(double xs, double ys)
{
    Q_UNUSED(xs);
    Q_UNUSED(ys);
    // TODO: implement it
}

void KisAutoMaskBrush::resize(double xs, double ys)
{
    autoDab.width *= 2 * xs;
    autoDab.hfade *= 2 * xs;
    autoDab.height *= 2 * ys;
    autoDab.vfade *= 2 * ys;
}

