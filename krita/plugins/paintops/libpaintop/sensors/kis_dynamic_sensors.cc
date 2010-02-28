/*
 *  Copyright (c) 2007,2010 Cyrille Berger <cberger@cberger.net>
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

#include "kis_dynamic_sensors.h"
KisDynamicSensorFuzzy::KisDynamicSensorFuzzy() : KisDynamicSensor(FuzzyId)
{

}

KisDynamicSensorSpeed::KisDynamicSensorSpeed() : KisDynamicSensor(SpeedId)
{

}

qreal KisDynamicSensorSpeed::parameter(const KisPaintInformation& info) {
    int dt = qMax(1, info.currentTime() - m_lastTime); // make sure dt > 1
    m_lastTime = info.currentTime();
    double currentMove = info.movement().norm() / dt;
    m_speed = qMin(1.0, (m_speed * 0.9 + currentMove * 0.1)); // average it to get nicer result, at the price of being less mathematically correct, but we quicly reach a situation where dt = 1 and currentMove = 1
    return 1.0 - m_speed;
}

KisDynamicSensorDrawingAngle::KisDynamicSensorDrawingAngle() : KisDynamicSensor(DrawingAngleId)
{

}

KisDynamicSensorRotation::KisDynamicSensorRotation() : KisDynamicSensor(RotationId)
{
}

qreal KisDynamicSensorDrawingAngle::parameter(const KisPaintInformation& info)
{
    /* so that we are in 0.0..1.0 */
    return 0.5 + info.angle() / (2.0 * M_PI);
}

KisDynamicSensorPressure::KisDynamicSensorPressure() : KisDynamicSensor(PressureId)
{

}

KisDynamicSensorXTilt::KisDynamicSensorXTilt() : KisDynamicSensor(XTiltId)
{

}

KisDynamicSensorYTilt::KisDynamicSensorYTilt() : KisDynamicSensor(YTiltId)
{

}

