/*
 * This file is part of the Krita project
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
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

KisRandomConstAccessor::KisRandomConstAccessor(KisTiledDataManager *ktm, qint32 x, qint32 y, qint32 offsetx, qint32 offsety, bool writable) : m_offsetx(offsetx), m_offsety(offsety)
{
    m_accessor = new KisTiledRandomAccessor(ktm, x, y, writable);
}

KisRandomConstAccessor::KisRandomConstAccessor(KisTiledDataManager *ktm, qint32 x, qint32 y, qint32 offsetx, qint32 offsety) : m_offsetx(offsetx), m_offsety(offsety)
{
    m_accessor = new KisTiledRandomAccessor(ktm, x, y, false);
}

KisRandomConstAccessor::KisRandomConstAccessor(const KisRandomConstAccessor& rhs) {
    m_accessor = rhs.m_accessor;
}

KisRandomConstAccessor::~KisRandomConstAccessor()
{
    
}

void KisRandomConstAccessor::moveTo(qint32 x, qint32 y)
{
    m_accessor->moveTo(x - m_offsetx, y  - m_offsety);
}

const quint8* KisRandomConstAccessor::rawData() const
{
    return m_accessor->rawData();
}

quint8* KisRandomAccessor::rawData() const
{
    return m_accessor->rawData();
}

const quint8* KisRandomConstAccessor::oldRawData() const
{
    return m_accessor->oldRawData();
}

