/*
 *  SPDX-FileCopyrightText: 2007, 2010 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_dynamic_sensors.h"
#include "kis_config.h"


KisDynamicSensorSpeed::KisDynamicSensorSpeed()
    : KisDynamicSensor(SPEED)
{
    KisConfig cfg(true);
    m_maxAllowedSpeedValue = cfg.readEntry("maxAllowedSpeedValue", 30);
}

qreal KisDynamicSensorSpeed::value(const KisPaintInformation& info)
{
    const qreal currentSpeed = qMin(1.0, info.drawingSpeed() / m_maxAllowedSpeedValue);
    return currentSpeed;
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

