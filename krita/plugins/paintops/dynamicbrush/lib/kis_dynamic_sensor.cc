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

const KoID PressureId("pressure", i18n("Pressure"));
const KoID XTiltId ("xtilt", i18n("X-Tilt"));
const KoID YTiltId ("ytilt", i18n("Y-Tilt"));

static KisDynamicSensor* KisDynamicSensor::id2TransformParameter(const KoID& id)
{
    if( id == "pressure")
    {
        return new KisDynamicSensorPressure();
    } else if( id == "xtilt")
    {
        return new KisDynamicSensorXTilt();
    } else if( id == "ytilt")
    {
        return new KisDynamicSensorYTilt();
    }
    kDebug() << "Unknown transform parameter : " << id << endl;
    return 0;
}

static QList<KoID> KisDynamicSensor::transformParameterIds()
{
    QList<KoID> ids << PressureId << XTiltId << YTiltId;
    return ids;
}
