/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "kis_dynamic_sensor.h"
#include <QDomElement>

#include "sensors/kis_dynamic_sensors.h"
#include "sensors/kis_dynamic_sensor_time.h"

KisDynamicSensor::KisDynamicSensor(const KoID& id) : m_id(id)
{
}

KisDynamicSensor::~KisDynamicSensor() { }

QWidget* KisDynamicSensor::createConfigurationWidget(QWidget* parent, KisSensorSelector*)
{
    Q_UNUSED(parent);
    return 0;
}

void KisDynamicSensor::reset()
{
}

KisDynamicSensor* KisDynamicSensor::id2Sensor(const KoID& id)
{
    if (id.id() == PressureId.id()) {
        return new KisDynamicSensorPressure();
    } else if (id.id() == XTiltId.id()) {
        return new KisDynamicSensorXTilt();
    } else if (id.id() == YTiltId.id()) {
        return new KisDynamicSensorYTilt();
    } else if (id.id() == SpeedId.id()) {
        return new KisDynamicSensorSpeed();
    } else if (id.id() == DrawingAngleId.id()) {
        return new KisDynamicSensorDrawingAngle();
    } else if (id.id() == TimeId.id()) {
        return new KisDynamicSensorTime();
    } else if (id.id() == FuzzyId.id()) {
        return new KisDynamicSensorFuzzy();
    }

    dbgPlugins << "Unknown transform parameter :" << id.id();
    return 0;
}

KisDynamicSensor* KisDynamicSensor::createFromXML(const QDomElement& e)
{
    QString id = e.attribute("id", "");
    KisDynamicSensor* sensor = id2Sensor(id);
    if (sensor) {
        sensor->fromXML(e);
    }
    return sensor;
}

QList<KoID> KisDynamicSensor::sensorsIds()
{
    QList<KoID> ids;
    ids << PressureId << XTiltId << YTiltId << SpeedId << DrawingAngleId << TimeId << FuzzyId;
    return ids;
}


void KisDynamicSensor::toXML(QDomDocument&, QDomElement& e) const
{
    e.setAttribute("id", id());
}

void KisDynamicSensor::fromXML(const QDomElement& e)
{
    Q_UNUSED(e);
    Q_ASSERT(e.attribute("id", "") == id());

}
