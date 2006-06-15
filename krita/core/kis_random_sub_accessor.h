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

#ifndef KIS_CURVE_ITERATOR_H
#define KIS_CURVE_ITERATOR_H

#include "kis_point.h"
#include "kis_random_accessor.h"
#include "kis_types.h"

class KisRandomSubAccessorPixel{
    public:
        KisRandomSubAccessorPixel(KisPaintDeviceSP device);
        ~KisRandomSubAccessorPixel();
        /**
         * Copy the sampled old value to destination
         */
        void sampledOldRawData(Q_UINT8* dst);
        inline void moveTo(double x, double y) { m_currentPoint.setX(x); m_currentPoint.setY(y); }
        inline void moveTo(KisPoint p ) { m_currentPoint = p; }
    private:
        KisPaintDeviceSP m_device;
        int m_position, m_end;
        KisPoint m_currentPoint;
        KisRandomAccessorPixel m_randomAccessor;
};

#endif
