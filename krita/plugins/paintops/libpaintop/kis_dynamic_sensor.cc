/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include "sensors/kis_dynamic_sensor_distance.h"
#include "sensors/kis_dynamic_sensor_time.h"
#include "sensors/kis_dynamic_sensor_fade.h"
#include "sensors/kis_dynamic_sensor_list.h"

KisDynamicSensor::KisDynamicSensor(const KoID& id)
    : m_id(id)
    , m_customCurve(false)
{
    setMinimumLabel(i18n("0.0"));
    setMaximumLabel(i18n("1.0"));
}

KisDynamicSensor::~KisDynamicSensor() { }

KisDynamicSensor* KisDynamicSensor::clone() const
{
    return createFromXML(toXML());
}

QWidget* KisDynamicSensor::createConfigurationWidget(QWidget* parent, QWidget*)
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
    } else if (id.id() == AscensionId.id()) {
        return new KisDynamicSensorAscension();
    } else if (id.id() == DeclinationId.id()) {
        return new KisDynamicSensorDeclination();
    } else if (id.id() == SpeedId.id()) {
        return new KisDynamicSensorSpeed();
    } else if (id.id() == DrawingAngleId.id()) {
        return new KisDynamicSensorDrawingAngle();
    } else if (id.id() == RotationId.id()) {
        return new KisDynamicSensorRotation();
    } else if (id.id() == DistanceId.id()) {
        return new KisDynamicSensorDistance();
    } else if (id.id() == TimeId.id()) {
        return new KisDynamicSensorTime();
    } else if (id.id() == FuzzyId.id()) {
        return new KisDynamicSensorFuzzy();
    } else if (id.id() == FadeId.id()) {
        return new KisDynamicSensorFade();
    } else if (id.id() == PerspectiveId.id()) {
        return new KisDynamicSensorPerspective();
    } else if (id.id() == TangentialPressureId.id()) {
        return new KisDynamicSensorTangentialPressure();
    } else if (id.id() == SensorsListId.id()) {
        return new KisDynamicSensorList();
    }

    dbgPlugins << "Unknown transform parameter :" << id.id();
    return 0;
}


KisDynamicSensor* KisDynamicSensor::createFromXML(const QString& s)
{
    QDomDocument doc;
    doc.setContent(s);
    QDomElement e = doc.documentElement();
    return createFromXML(e);
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

    ids << PressureId
        << XTiltId
        << YTiltId
        << AscensionId
        << DeclinationId
        << SpeedId
        << DrawingAngleId
        << RotationId
        << DistanceId
        << TimeId
        << FuzzyId
        << FadeId
        << PerspectiveId
        << TangentialPressureId;

    return ids;
}


void KisDynamicSensor::toXML(QDomDocument& doc, QDomElement& e) const
{
    e.setAttribute("id", id());
    if(m_customCurve)
    {
        QDomElement curve_elt = doc.createElement("curve");
        QDomText text = doc.createTextNode(m_curve.toString());
        curve_elt.appendChild(text);
        e.appendChild(curve_elt);
    }
}

void KisDynamicSensor::fromXML(const QDomElement& e)
{
    Q_UNUSED(e);
    Q_ASSERT(e.attribute("id", "") == id());
    m_customCurve = false;
    QDomElement curve_elt = e.firstChildElement("curve");
    if(!curve_elt.isNull())
    {
        m_customCurve = true;
        m_curve.fromString(curve_elt.text());
    }
}

const KisCurveLabel& KisDynamicSensor::minimumLabel() const
{
    return m_minimumLabel;
}

const KisCurveLabel& KisDynamicSensor::maximumLabel() const
{
    return m_maximumLabel;
}

void KisDynamicSensor::setMinimumLabel(const KisCurveLabel& _label)
{
    m_minimumLabel = _label;
}

void KisDynamicSensor::setMaximumLabel(const KisCurveLabel& _label)
{
    m_maximumLabel = _label;
}

qreal KisDynamicSensor::parameter(const KisPaintInformation& info)
{
    qreal val = value(info);
    if (m_customCurve) {
        int offset = qRound(256.0 * val);
        return m_curve.floatTransfer(257)[qBound(0, offset, 256)];
    } else {
        return val;
    }
}

void KisDynamicSensor::setCurve(const KisCubicCurve& curve)
{
    m_customCurve = true;
    m_curve = curve;
}

const KisCubicCurve& KisDynamicSensor::curve() const
{
    return m_curve;
}

void KisDynamicSensor::removeCurve()
{
    m_customCurve = false;
}

bool KisDynamicSensor::hasCustomCurve() const
{
    return m_customCurve;
}

bool KisDynamicSensor::dependsOnCanvasRotation() const
{
    return true;
}
