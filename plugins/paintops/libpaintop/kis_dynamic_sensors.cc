/*
 *  SPDX-FileCopyrightText: 2007, 2010 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#include "kis_dynamic_sensors.h"


KisDynamicSensorSpeed::KisDynamicSensorSpeed()
    : KisDynamicSensor(SPEED)
{
}

qreal KisDynamicSensorSpeed::value(const KisPaintInformation& info)
{
    /**
     * The value of maximum speed was measured empirically. This is
     * the speed that is quite easy to get with an A6 tablet and quite
     * a big image. If you need smaller speeds, just change the curve.
     */
    const qreal maxSpeed = 30.0; // px / ms
    const qreal blendExponent = 0.05;

    qreal currentSpeed = info.drawingSpeed() / maxSpeed;

    if (m_speed >= 0.0) {
        m_speed = qMin(1.0, (m_speed * (1 - blendExponent) +
                             currentSpeed * blendExponent));
    }
    else {
        m_speed = currentSpeed;
    }

    return m_speed;
}

KisDynamicSensorRotation::KisDynamicSensorRotation()
    : KisDynamicSensor(ROTATION)
{
}

KisDynamicSensorPressure::KisDynamicSensorPressure()
    : KisDynamicSensor(PRESSURE)
{
}

KisDynamicSensorPressureIn::KisDynamicSensorPressureIn() 
    : KisDynamicSensor(PRESSURE_IN)
    , lastPressure(0.0)
{
}

KisDynamicSensorXTilt::KisDynamicSensorXTilt() 
    : KisDynamicSensor(XTILT)
{
}

KisDynamicSensorYTilt::KisDynamicSensorYTilt()
    : KisDynamicSensor(YTILT)
{
}

KisDynamicSensorTiltDirection::KisDynamicSensorTiltDirection() :
    KisDynamicSensor(TILT_DIRECTION)
{
}

KisDynamicSensorTiltElevation::KisDynamicSensorTiltElevation()
    : KisDynamicSensor(TILT_ELEVATATION)
{
}


KisDynamicSensorPerspective::KisDynamicSensorPerspective()
    : KisDynamicSensor(PERSPECTIVE)
{
}

KisDynamicSensorTangentialPressure::KisDynamicSensorTangentialPressure()
    : KisDynamicSensor(TANGENTIAL_PRESSURE)
{
}

