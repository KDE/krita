/*
 *  Copyright (c) 2007,2010 Cyrille Berger <cberger@cberger.net>
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

