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

/**
 * you need to inherit this class
 */
class KisCurveFunction {
    public:
        virtual KisPoint valueAt(int i) =0;
};

class KisCurveIteratorPixel{
    public:
        KisCurveIteratorPixel(KisPaintDeviceSP device, KisCurveFunction* curveFunction, int start, int end);
        ~KisCurveIteratorPixel();
        /**
         * Copy the sampled old value to destination
         */
        void sampledOldRawData(Q_UINT8* dst);
        inline KisCurveIteratorPixel& operator++() { m_position++; m_currentPoint = m_curveFunction->valueAt(m_position); return *this;  };
        inline bool isDone() { return m_position >= m_end; }
        inline void setPosition(int p) { m_position = p; }
    private:
        KisPaintDeviceSP m_device;
        KisCurveFunction* m_curveFunction;
        int m_position, m_end;
        KisPoint m_currentPoint;
        KisRandomAccessorPixel m_randomAccessor;
};

#endif
