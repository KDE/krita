/*
 *  This file is part of the KDE project
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

#include "kis_random_sub_accessor.h"

#include "kis_paint_device.h"

KisRandomSubAccessorPixel::KisRandomSubAccessorPixel(KisPaintDeviceSP device) :
    m_device(device), m_currentPoint( 0, 0 ), m_randomAccessor(device->createRandomAccessor(0,0, false))
{
}


KisRandomSubAccessorPixel::~KisRandomSubAccessorPixel()
{
}


void KisRandomSubAccessorPixel::sampledOldRawData(Q_UINT8* dst)
{
    const Q_UINT8* pixels[4];
    Q_UINT8 weights[4];
    int x = (int)floor(m_currentPoint.x());
    int y = (int)floor(m_currentPoint.y());
    double hsub = m_currentPoint.x() - x;
    if(hsub < 0.0 ) hsub = 1.0 + hsub;
    double vsub = m_currentPoint.y() - y;
    if(vsub < 0.0 ) vsub = 1.0 + vsub;
    weights[0] = (int)round( ( 1.0 - hsub) * ( 1.0 - vsub) * 255 );
    m_randomAccessor.moveTo(x, y);
    pixels[0] = m_randomAccessor.oldRawData();
    weights[1] = (int)round( ( 1.0 - vsub) * hsub * 255 );
    m_randomAccessor.moveTo(x+1, y);
    pixels[1] = m_randomAccessor.oldRawData();
    weights[2] = (int)round( vsub * ( 1.0 - hsub) * 255 );
    m_randomAccessor.moveTo(x, y+1);
    pixels[2] = m_randomAccessor.oldRawData();
    weights[3] = (int)round( hsub * vsub * 255 );
    m_randomAccessor.moveTo(x+1, y+1);
    pixels[3] = m_randomAccessor.oldRawData();
    m_device->colorSpace()->mixColors(pixels, weights, 4, dst);
}

void KisRandomSubAccessorPixel::sampledRawData(Q_UINT8* dst)
{
    const Q_UINT8* pixels[4];
    Q_UINT8 weights[4];
    int x = (int)floor(m_currentPoint.x());
    int y = (int)floor(m_currentPoint.y());
    double hsub = m_currentPoint.x() - x;
    if(hsub < 0.0 ) hsub = 1.0 + hsub;
    double vsub = m_currentPoint.y() - y;
    if(vsub < 0.0 ) vsub = 1.0 + vsub;
    weights[0] = (int)round( ( 1.0 - hsub) * ( 1.0 - vsub) * 255 );
    m_randomAccessor.moveTo(x, y);
    pixels[0] = m_randomAccessor.rawData();
    weights[1] = (int)round( ( 1.0 - vsub) * hsub * 255 );
    m_randomAccessor.moveTo(x+1, y);
    pixels[1] = m_randomAccessor.rawData();
    weights[2] = (int)round( vsub * ( 1.0 - hsub) * 255 );
    m_randomAccessor.moveTo(x, y+1);
    pixels[2] = m_randomAccessor.rawData();
    weights[3] = (int)round( hsub * vsub * 255 );
    m_randomAccessor.moveTo(x+1, y+1);
    pixels[3] = m_randomAccessor.rawData();
    m_device->colorSpace()->mixColors(pixels, weights, 4, dst);
}

