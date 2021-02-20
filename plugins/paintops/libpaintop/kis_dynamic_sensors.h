/*
 *  SPDX-FileCopyrightText: 2006-2007, 2010 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _KIS_DYNAMIC_SENSORS_H_
#define _KIS_DYNAMIC_SENSORS_H_

#include "../kis_dynamic_sensor.h"


#include "kis_paint_information.h"

class KisDynamicSensorSpeed : public KisDynamicSensor
{
public:
    KisDynamicSensorSpeed();
    virtual ~KisDynamicSensorSpeed() { }
    virtual qreal value(const KisPaintInformation& info);
    void reset() {
        m_speed = -1.0;
    }
private:
    double m_speed;
};

class KisDynamicSensorRotation : public KisDynamicSensor
{
public:
    KisDynamicSensorRotation();
    virtual ~KisDynamicSensorRotation() { }
    virtual qreal value(const KisPaintInformation& info) {
        return info.rotation() / 360.0;
    }
};

class KisDynamicSensorPressure : public KisDynamicSensor
{
public:
    KisDynamicSensorPressure();
    virtual ~KisDynamicSensorPressure() { }
    virtual qreal value(const KisPaintInformation& info) {
        return info.pressure();
    }
};

class KisDynamicSensorPressureIn : public KisDynamicSensor
{
public:
    KisDynamicSensorPressureIn();
    virtual ~KisDynamicSensorPressureIn() { }
    virtual qreal value(const KisPaintInformation& info) {
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
    virtual ~KisDynamicSensorXTilt() { }
    virtual qreal value(const KisPaintInformation& info) {
        return 1.0 - fabs(info.xTilt()) / 60.0;
    }
};

class KisDynamicSensorYTilt : public KisDynamicSensor
{
public:
    KisDynamicSensorYTilt();
    virtual ~KisDynamicSensorYTilt() { }
    virtual qreal value(const KisPaintInformation& info) {
        return 1.0 - fabs(info.yTilt()) / 60.0;
    }
};

class KisDynamicSensorTiltDirection : public KisDynamicSensor
{
public:
    KisDynamicSensorTiltDirection();
    virtual ~KisDynamicSensorTiltDirection() {}
    virtual qreal value(const KisPaintInformation& info) {
        return KisPaintInformation::tiltDirection(info, true);
    }
};

class KisDynamicSensorTiltElevation : public KisDynamicSensor
{
public:
    KisDynamicSensorTiltElevation();
    virtual ~KisDynamicSensorTiltElevation() {}
    virtual qreal value(const KisPaintInformation& info) {
        return KisPaintInformation::tiltElevation(info, 60.0, 60.0, true);
    }
};

class KisDynamicSensorPerspective : public KisDynamicSensor
{
public:
    KisDynamicSensorPerspective();
    virtual ~KisDynamicSensorPerspective() { }
    virtual qreal value(const KisPaintInformation& info) {
        return info.perspective();
    }
};

class KisDynamicSensorTangentialPressure : public KisDynamicSensor
{
public:
    KisDynamicSensorTangentialPressure();
    virtual ~KisDynamicSensorTangentialPressure() { }
    virtual qreal value(const KisPaintInformation& info) {
        return info.tangentialPressure();
    }
};

#endif
