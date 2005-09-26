/*
 * This file is part of Krita
 *
 * Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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

#include <kis_paint_device_impl.h>
#include <kis_iterators_pixel.h>

#include "kis_cachedhistogram.h"

void KisCachedHistogramObserver::regionUpdated(KisPaintDeviceImplSP dev) {
    m_producer -> clear();
    KisRectIteratorPixel it = dev -> createRectIterator(m_x, m_y, m_w, m_h, false);
    m_producer -> addRegionToBin(it, dev -> colorSpace());
    kdDebug() << "added region " << m_x << " , " << m_y << endl;
}
