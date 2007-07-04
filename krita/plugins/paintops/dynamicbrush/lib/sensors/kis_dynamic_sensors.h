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

#ifndef _KIS_DYNAMICS_SENSORS_H_
#define _KIS_DYNAMICS_SENSORS_H_

#include "kis_dynamic_sensor.h"

#include <kis_paintop.h>
#include <KoID.h>

#include "kis_paint_information.h"

class KisDynamicSensorFuzzy : public KisDynamicSensor {
    public:
        KisDynamicSensorFuzzy();
        virtual ~KisDynamicSensorFuzzy() { }
        virtual double parameter(const KisPaintInformation& )
        { return rand() / (double)RAND_MAX; }
};

class KisDynamicSensorSpeed : public KisDynamicSensor {
    public:
        KisDynamicSensorSpeed();
        virtual ~KisDynamicSensorSpeed() { }
        virtual double parameter(const KisPaintInformation& info)
        { return 1.0 + info.movement.length() * 0.1; }
};

class KisDynamicSensorDrawingAngle : public KisDynamicSensor {
    public:
        KisDynamicSensorDrawingAngle();
        virtual ~KisDynamicSensorDrawingAngle() { }
        virtual double parameter(const KisPaintInformation& info);
    private:
        inline double modulo(double x, double r)
        {
            return x-floor(x/r)*r;
        }
        double m_angle;
};

class KisDynamicSensorPressure : public KisDynamicSensor {
    public:
        KisDynamicSensorPressure();
        virtual ~KisDynamicSensorPressure() { }
        virtual double parameter(const KisPaintInformation& info)
        { return info.pressure; }
};

class KisDynamicSensorXTilt : public KisDynamicSensor {
    public:
        KisDynamicSensorXTilt();
        virtual ~KisDynamicSensorXTilt() { }
        virtual double parameter(const KisPaintInformation& info)
        { return 1.0 - fabs( info.xTilt ) / 60.0; }
};

class KisDynamicSensorYTilt : public KisDynamicSensor {
    public:
        KisDynamicSensorYTilt();
        virtual ~KisDynamicSensorYTilt() { }
        virtual double parameter(const KisPaintInformation& info)
        { return 1.0 - fabs( info.yTilt ) / 60.0; }
};

#endif
