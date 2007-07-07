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

#include "sensors/kis_dynamic_sensors.h"
#include "sensors/kis_dynamic_sensor_time.h"

KisDynamicSensor::KisDynamicSensor(const KoID& id) : m_id(id)
{
}

QWidget* KisDynamicSensor::createConfigurationWidget(QWidget* parent)
{
    Q_UNUSED(parent);
    return 0;
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
    
    kDebug(41006) << "Unknown transform parameter : " << id.id() << endl;
    return 0;
}

QList<KoID> KisDynamicSensor::sensorsIds()
{
    QList<KoID> ids;
    ids << PressureId << XTiltId << YTiltId << SpeedId << DrawingAngleId << TimeId << FuzzyId;
    return ids;
}
