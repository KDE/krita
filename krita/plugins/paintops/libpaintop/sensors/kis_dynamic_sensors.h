/*
 *  Copyright (c) 2006-2007,2010 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
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

#ifndef _KIS_DYNAMIC_SENSORS_H_
#define _KIS_DYNAMIC_SENSORS_H_

#include "../kis_dynamic_sensor.h"

#include <kis_paintop.h>
#include <KoID.h>

#include "kis_paint_information.h"

class KisDynamicSensorFuzzy : public KisDynamicSensor
{
public:
    KisDynamicSensorFuzzy();
    virtual ~KisDynamicSensorFuzzy() { }
    virtual qreal parameter(const KisPaintInformation&) {
        return rand() / (qreal)RAND_MAX;
    }
};

class KisDynamicSensorSpeed : public KisDynamicSensor
{
public:
    KisDynamicSensorSpeed();
    virtual ~KisDynamicSensorSpeed() { }
    virtual qreal parameter(const KisPaintInformation& info);
    void reset() {
        m_lastTime = 0;
        m_lastMove = 0.0;
        m_speed = 0.0;
    }
private:
    double m_speed;
    double m_lastMove;
    int m_lastTime;
};

class KisDynamicSensorDrawingAngle : public KisDynamicSensor
{
public:
    KisDynamicSensorDrawingAngle();
    virtual ~KisDynamicSensorDrawingAngle() { }
    virtual qreal parameter(const KisPaintInformation& info);
};

class KisDynamicSensorRotation : public KisDynamicSensor
{
public:
    KisDynamicSensorRotation();
    virtual ~KisDynamicSensorRotation() { }
    virtual qreal parameter(const KisPaintInformation& info) {
        return info.rotation() / 28 + 0.5; // it appears that rotation is between -14 and +14
    }
};

class KisDynamicSensorPressure : public KisDynamicSensor
{
public:
    KisDynamicSensorPressure();
    virtual ~KisDynamicSensorPressure() { }
    virtual qreal parameter(const KisPaintInformation& info) {
        return info.pressure();
    }
};

class KisDynamicSensorXTilt : public KisDynamicSensor
{
public:
    KisDynamicSensorXTilt();
    virtual ~KisDynamicSensorXTilt() { }
    virtual qreal parameter(const KisPaintInformation& info) {
        return 1.0 - fabs(info.xTilt()) / 60.0;
    }
};

class KisDynamicSensorYTilt : public KisDynamicSensor
{
public:
    KisDynamicSensorYTilt();
    virtual ~KisDynamicSensorYTilt() { }
    virtual qreal parameter(const KisPaintInformation& info) {
        return 1.0 - fabs(info.yTilt()) / 60.0;
    }
};

class KisDynamicSensorPerspective : public KisDynamicSensor
{
public:
    KisDynamicSensorPerspective();
    virtual ~KisDynamicSensorPerspective() { }
    virtual qreal parameter(const KisPaintInformation& info) {
        return info.perspective();
    }
};

#endif
