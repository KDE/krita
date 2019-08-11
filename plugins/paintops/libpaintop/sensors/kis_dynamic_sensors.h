/*
 *  Copyright (c) 2006-2007,2010 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_DYNAMIC_SENSORS_H_
#define _KIS_DYNAMIC_SENSORS_H_

#include "../kis_dynamic_sensor.h"


#include <brushengine/kis_paint_information.h>

class KisDynamicSensorSpeed : public KisDynamicSensor
{
public:
    KisDynamicSensorSpeed();
    ~KisDynamicSensorSpeed() override { }
    qreal value(const KisPaintInformation& info) override;
    void reset() override {
        m_speed = -1.0;
    }
private:
    double m_speed;
};

class KisDynamicSensorRotation : public KisDynamicSensor
{
public:
    KisDynamicSensorRotation();
    ~KisDynamicSensorRotation() override { }
    qreal value(const KisPaintInformation& info) override {
        return info.rotation() / 360.0;
    }
};

class KisDynamicSensorPressure : public KisDynamicSensor
{
public:
    KisDynamicSensorPressure();
    ~KisDynamicSensorPressure() override { }
    qreal value(const KisPaintInformation& info) override {
        return info.pressure();
    }
};

class KisDynamicSensorPressureIn : public KisDynamicSensor
{
public:
    KisDynamicSensorPressureIn();
    ~KisDynamicSensorPressureIn() override { }
    qreal value(const KisPaintInformation& info) override {
        if(!info.isHoveringMode()) {
            if(info.pressure() > lastPressure) {
                lastPressure = info.pressure();
            }
            return lastPressure;
        }

        lastPressure = 0.0;
        return 0.0;
    }
private:
    qreal lastPressure;
};

class KisDynamicSensorXTilt : public KisDynamicSensor
{
public:
    KisDynamicSensorXTilt();
    ~KisDynamicSensorXTilt() override { }
    qreal value(const KisPaintInformation& info) override {
        return 1.0 - fabs(info.xTilt()) / 60.0;
    }
};

class KisDynamicSensorYTilt : public KisDynamicSensor
{
public:
    KisDynamicSensorYTilt();
    ~KisDynamicSensorYTilt() override { }
    qreal value(const KisPaintInformation& info) override {
        return 1.0 - fabs(info.yTilt()) / 60.0;
    }
};

class KisDynamicSensorTiltDirection : public KisDynamicSensor
{
public:
    KisDynamicSensorTiltDirection();
    ~KisDynamicSensorTiltDirection() override {}
    qreal value(const KisPaintInformation& info) override {
        return KisPaintInformation::tiltDirection(info, true);
    }
};

class KisDynamicSensorTiltElevation : public KisDynamicSensor
{
public:
    KisDynamicSensorTiltElevation();
    ~KisDynamicSensorTiltElevation() override {}
    qreal value(const KisPaintInformation& info) override {
        return KisPaintInformation::tiltElevation(info, 60.0, 60.0, true);
    }
};

class KisDynamicSensorPerspective : public KisDynamicSensor
{
public:
    KisDynamicSensorPerspective();
    ~KisDynamicSensorPerspective() override { }
    qreal value(const KisPaintInformation& info) override {
        return info.perspective();
    }
};

class KisDynamicSensorTangentialPressure : public KisDynamicSensor
{
public:
    KisDynamicSensorTangentialPressure();
    ~KisDynamicSensorTangentialPressure() override { }
    qreal value(const KisPaintInformation& info) override {
        return info.tangentialPressure();
    }
};

#endif
