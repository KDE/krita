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

#include "kis_curve_iterator_pixel.h"

#include "kis_paint_device.h"

KisCurveIteratorPixel::KisCurveIteratorPixel(KisPaintDeviceSP device, KisCurveFunction* curveFunction, int start, int end) :
    m_device(device), m_curveFunction(curveFunction), m_position(start), m_end(end), m_currentPoint( m_curveFunction->valueAt(m_position) ), m_randomAccessor(device->createRandomAccessor(0,0, true))
{
}


KisCurveIteratorPixel::~KisCurveIteratorPixel()
{
    delete m_curveFunction;
}


void KisCurveIteratorPixel::sampledOldRawData(Q_UINT8* dst)
{
/*    int x = 3;//(int)m_currentPoint.x();
    int y = 3;//(int)m_currentPoint.y();
    m_randomAccessor.moveTo(x, y);
    dst[0] = m_randomAccessor.rawData()[0];
    dst[1] = m_randomAccessor.rawData()[1];
    dst[2] = m_randomAccessor.rawData()[2];
    kdDebug() << x << " " << y << endl;
    kdDebug() << (int)m_randomAccessor.rawData()[0] << " " << (int)m_randomAccessor.rawData()[1] << " " << (int)m_randomAccessor.rawData()[1] << endl;*/
    
    const Q_UINT8* pixels[4];
    Q_UINT8 weights[4];
    int x = (int)m_currentPoint.x();
    int y = (int)m_currentPoint.y();
//     kdDebug() << x << " " << y << " " << m_currentPoint.x() << " " << m_currentPoint.y() << endl;
    double hsub = m_currentPoint.x() - x;
    double vsub = m_currentPoint.y() - y;
    weights[0] = (int)round( ( 1.0 - hsub) * ( 1.0 - vsub) * 255 );
    m_randomAccessor.moveTo(x, y);
    pixels[0] = m_randomAccessor.oldRawData();
    weights[1] = (int)round( ( 1.0 - hsub) * vsub * 255 );
    m_randomAccessor.moveTo(x+1, y);
    pixels[1] = m_randomAccessor.oldRawData();
    weights[2] = (int)round( hsub * ( 1.0 - vsub) * 255 );
    m_randomAccessor.moveTo(x, y+1);
    pixels[2] = m_randomAccessor.oldRawData();
    weights[3] = (int)round( hsub * vsub * 255 );
    m_randomAccessor.moveTo(x+1, y+1);
    pixels[3] = m_randomAccessor.oldRawData();
//     kdDebug() << (int)weights[0] << " " << (int)weights[1] << " " << (int)weights[2] << " " << (int)weights[3] << endl;
    m_device->colorSpace()->mixColors(pixels, weights, 4, dst);
}
