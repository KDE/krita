/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "kis_dynamic_sensor.h"

const KoID FuzzyId("fuzzy", i18n("Fuzzy"));
const KoID SpeedId("speed", i18n("Speed"));
const KoID TimeId("time", i18n("Time"));
const KoID DrawingAngleId("drawingangle", i18n("Drawing angle"));
const KoID PressureId("pressure", i18n("Pressure"));
const KoID XTiltId ("xtilt", i18n("X-Tilt"));
const KoID YTiltId ("ytilt", i18n("Y-Tilt"));

KisDynamicSensor::KisDynamicSensor(const KoID& id) : m_id(id)
{
}

KisDynamicSensor* KisDynamicSensor::id2Sensor(const KoID& id)
{
    if( id.id() == PressureId.id())
    {
        return new KisDynamicSensorPressure();
    } else if( id.id() == XTiltId.id())
    {
        return new KisDynamicSensorXTilt();
    } else if( id.id() == YTiltId.id())
    {
        return new KisDynamicSensorYTilt();
    } else if( id.id() == SpeedId.id())
    {
        return new KisDynamicSensorSpeed();
    } else if( id.id() == DrawingAngleId.id())
    {
        return new KisDynamicSensorDrawingAngle();
    } else if( id.id() == TimeId.id())
    {
        return new KisDynamicSensorTime();
    } else if( id.id() == FuzzyId.id())
    {
        return new KisDynamicSensorFuzzy();
    }
    
    kDebug() << "Unknown transform parameter : " << id.id() << endl;
    return 0;
}

QList<KoID> KisDynamicSensor::sensorsIds()
{
    QList<KoID> ids;
    ids << PressureId << XTiltId << YTiltId << SpeedId << DrawingAngleId << TimeId << FuzzyId;
    return ids;
}

KisDynamicSensorFuzzy::KisDynamicSensorFuzzy() : KisDynamicSensor(FuzzyId)
{
    
}

KisDynamicSensorSpeed::KisDynamicSensorSpeed() : KisDynamicSensor(SpeedId)
{
    
}

KisDynamicSensorTime::KisDynamicSensorTime() : KisDynamicSensor(TimeId), m_time(0.0)
{
    
}

KisDynamicSensorDrawingAngle::KisDynamicSensorDrawingAngle() : KisDynamicSensor(SpeedId), m_angle(0.0)
{
    
}

double KisDynamicSensorDrawingAngle::parameter(const KisPaintInformation& info)
{
    double angle = atan2(info.movement.y() , info.movement.x());
    double v = modulo(m_angle - angle + M_PI, 2.0 * M_PI) - M_PI;
    if(v < 0)
    {
        m_angle += 0.01;
    } else if( v > 0)
    {
        m_angle -= 0.01;
    }
    m_angle = modulo(m_angle, 2.0 * M_PI);
    return m_angle / (2.0 * M_PI);
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

