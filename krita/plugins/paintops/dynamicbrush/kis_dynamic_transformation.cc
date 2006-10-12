/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License.
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

#include "kis_dynamic_transformation.h"

#include "kis_alpha_mask.h"
#include "kis_autobrush_resource.h"

quint8 KisDabAlphaMaskSource::alphaAt(int x, int y)
{
    return alphaMask->alphaAt(x,y);
}

quint8 KisDabAutoSource::alphaAt(int x, int y)
{
    return 255 - m_shape->valueAt(x,y);
}

KisDabAutoSource::~KisDabAutoSource() { if(m_shape) delete m_shape; }

KisDabSource::~KisDabSource() { }
KisDabSource::KisDabSource() { }
