/*
 * This file is part of the Krita project
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include "kis_random_accessor.h"

#include "kis_tiled_random_accessor.h"

KisRandomAccessor::KisRandomAccessor(KisTiledDataManager *ktm, Q_INT32 x, Q_INT32 y, Q_INT32 offsetx, Q_INT32 offsety, bool writable) : m_offsetx(offsetx), m_offsety(offsety)
{
    m_accessor = new KisTiledRandomAccessor(ktm, x, y, writable);
}

KisRandomAccessor::KisRandomAccessor(const KisRandomAccessor& rhs) {
    m_accessor = rhs.m_accessor;
}

KisRandomAccessor::~KisRandomAccessor()
{
    
}

void KisRandomAccessor::moveTo(Q_INT32 x, Q_INT32 y)
{
    m_accessor->moveTo(x - m_offsetx, y  - m_offsety);
}

Q_UINT8* KisRandomAccessor::rawData() const
{
    return m_accessor->rawData();
}

const Q_UINT8* KisRandomAccessor::oldRawData() const
{
    return m_accessor->oldRawData();
}

KisRandomAccessorPixel::KisRandomAccessorPixel(KisTiledDataManager *ktm, KisTiledDataManager *ktmselect, Q_INT32 x, Q_INT32 y, Q_INT32 offsetx, Q_INT32 offsety, bool writable) :
        KisRandomAccessor( ktm, x, y, offsetx, offsety, writable),
        KisRandomAccessorPixelTrait( this, (ktmselect) ? new KisRandomAccessor(ktm, x, y, offsetx, offsety, false) : 0 )
{
    
}
