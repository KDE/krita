/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "kis_dynamic_sensor.h"
#include <QDomElement>

#include "kis_algebra_2d.h"

#include "sensors/kis_dynamic_sensors.h"
#include "sensors/kis_dynamic_sensor_distance.h"
#include "sensors/kis_dynamic_sensor_drawing_angle.h"
#include "sensors/kis_dynamic_sensor_time.h"
#include "sensors/kis_dynamic_sensor_fade.h"
#include "sensors/kis_dynamic_sensor_fuzzy.h"

KisDynamicSensor::KisDynamicSensor(DynamicSensorType type)
    : m_length(-1)
    , m_type(type)
    , m_customCurve(false)
    , m_active(false)
{
}

KisDynamicSensor::~KisDynamicSensor()
{
}

QWidget* KisDynamicSensor::createConfigurationWidget(QWidget* parent, QWidget*)
{
    Q_UNUSED(parent);
    return 0;
}

void KisDynamicSensor::reset()
{
}

KisDynamicSensorSP KisDynamicSensor::id2Sensor(const KoID& id, const QString &parentOptionName)
{
    if (id.id() == PressureId.id()) {
        return new KisDynamicSensorPressure();
    }
    else if (id.id() == PressureInId.id()) {
        return new KisDynamicSensorPressureIn();
    }
    else if (id.id() == XTiltId.id()) {
        return new KisDynamicSensorXTilt();
    }
    else if (id.id() == YTiltId.id()) {
        return new KisDynamicSensorYTilt();
    }
    else if (id.id() == TiltDirectionId.id()) {
        return new KisDynamicSensorTiltDirection();
    }
    else if (id.id() == TiltElevationId.id()) {
        return new KisDynamicSensorTiltElevation();
    }
    else if (id.id() == SpeedId.id()) {
        return new KisDynamicSensorSpeed();
    }
    else if (id.id() == DrawingAngleId.id()) {
        return new KisDynamicSensorDrawingAngle();
    }
    else if (id.id() == RotationId.id()) {
        return new KisDynamicSensorRotation();
    }
    else if (id.id() == DistanceId.id()) {
        return new KisDynamicSensorDistance();
    }
    else if (id.id() == TimeId.id()) {
        return new KisDynamicSensorTime();
    }
    else if (id.id() == FuzzyPerDabId.id()) {
        return new KisDynamicSensorFuzzy(false, parentOptionName);
    }
    else if (id.id() == FuzzyPerStrokeId.id()) {
        return new KisDynamicSensorFuzzy(true, parentOptionName);
    }
    else if (id.id() == FadeId.id()) {
        return new KisDynamicSensorFade();
    }
    else if (id.id() == PerspectiveId.id()) {
        return new KisDynamicSensorPerspective();
    }
    else if (id.id() == TangentialPressureId.id()) {
        return new KisDynamicSensorTangentialPressure();
    }
    dbgPlugins << "Unknown transform parameter :" << id.id();
    return 0;
}

DynamicSensorType KisDynamicSensor::id2Type(const KoID &id)
{
    if (id.id() == PressureId.id()) {
        return PRESSURE;
    }
    else if (id.id() == PressureInId.id()) {
        return PRESSURE_IN;
    }
    else if (id.id() == XTiltId.id()) {
        return XTILT;
    }
    else if (id.id() == YTiltId.id()) {
        return YTILT;
    }
    else if (id.id() == TiltDirectionId.id()) {
        return TILT_DIRECTION;
    }
    else if (id.id() == TiltElevationId.id()) {
        return TILT_ELEVATATION;
    }
    else if (id.id() == SpeedId.id()) {
        return SPEED;
    }
    else if (id.id() == DrawingAngleId.id()) {
        return ANGLE;
    }
    else if (id.id() == RotationId.id()) {
        return ROTATION;
    }
    else if (id.id() == DistanceId.id()) {
        return DISTANCE;
    }
    else if (id.id() == TimeId.id()) {
        return TIME;
    }
    else if (id.id() == FuzzyPerDabId.id()) {
        return FUZZY_PER_DAB;
    }
    else if (id.id() == FuzzyPerStrokeId.id()) {
        return FUZZY_PER_STROKE;
    }
    else if (id.id() == FadeId.id()) {
        return FADE;
    }
    else if (id.id() == PerspectiveId.id()) {
        return PERSPECTIVE;
    }
    else if (id.id() == TangentialPressureId.id()) {
        return TANGENTIAL_PRESSURE;
    }
    return UNKNOWN;
}

KisDynamicSensorSP KisDynamicSensor::type2Sensor(DynamicSensorType sensorType, const QString &parentOptionName)
{
    switch (sensorType) {
    case FUZZY_PER_DAB:
        return new KisDynamicSensorFuzzy(false, parentOptionName);
    case FUZZY_PER_STROKE:
        return new KisDynamicSensorFuzzy(true, parentOptionName);
    case SPEED:
        return new KisDynamicSensorSpeed();
    case FADE:
        return new KisDynamicSensorFade();
    case DISTANCE:
        return new KisDynamicSensorDistance();
    case TIME:
        return new KisDynamicSensorTime();
    case ANGLE:
        return new KisDynamicSensorDrawingAngle();
    case ROTATION:
        return new KisDynamicSensorRotation();
    case PRESSURE:
        return new KisDynamicSensorPressure();
    case XTILT:
        return new KisDynamicSensorXTilt();
    case YTILT:
        return new KisDynamicSensorYTilt();
    case TILT_DIRECTION:
        return new KisDynamicSensorTiltDirection();
    case TILT_ELEVATATION:
        return new KisDynamicSensorTiltElevation();
    case PERSPECTIVE:
        return new KisDynamicSensorPerspective();
    case TANGENTIAL_PRESSURE:
        return new KisDynamicSensorTangentialPressure();
    case PRESSURE_IN:
        return new KisDynamicSensorPressureIn();
    default:
        return 0;
    }
}

QString KisDynamicSensor::minimumLabel(DynamicSensorType sensorType)
{
    switch (sensorType) {
    case FUZZY_PER_DAB:
    case FUZZY_PER_STROKE:
        return QString();
    case FADE:
        return i18n("0");
    case DISTANCE:
        return i18n("0 px");
    case TIME:
        return i18n("0 s");
    case ANGLE:
        return i18n("0°");
    case SPEED:
        return i18n("Slow");
    case ROTATION:
        return i18n("0°");
    case PRESSURE:
        return i18n("Low");
    case XTILT:
        return i18n("-30°");
    case YTILT:
        return i18n("-30°");
    case TILT_DIRECTION:
        return i18n("0°");
    case TILT_ELEVATATION:
        return i18n("90°");
    case PERSPECTIVE:
        return i18n("Far");
    case TANGENTIAL_PRESSURE:
    case PRESSURE_IN:
        return i18n("Low");
    default:
        return i18n("0.0");
    }
}

QString KisDynamicSensor::maximumLabel(DynamicSensorType sensorType, int max)
{
    switch (sensorType) {
    case FUZZY_PER_DAB:
    case FUZZY_PER_STROKE:
        return QString();
    case FADE:
        if (max < 0)
            return i18n("1000");
        else
            return i18n("%1", max);
    case DISTANCE:
        if (max < 0)
            return i18n("30 px");
        else
            return i18n("%1 px", max);
    case TIME:
        if (max < 0)
           return i18n("3 s");
        else
            return i18n("%1 s", max / 1000);
    case ANGLE:
        return i18n("360°");
    case SPEED:
        return i18n("Fast");
    case ROTATION:
        return i18n("360°");
    case PRESSURE:
        return i18n("High");
    case XTILT:
        return i18n("30°");
    case YTILT:
        return i18n("30°");
    case TILT_DIRECTION:
        return i18n("360°");
    case TILT_ELEVATATION:
        return i18n("0°");
    case PERSPECTIVE:
        return i18n("Near");
    case TANGENTIAL_PRESSURE:
    case PRESSURE_IN:
        return i18n("High");
    default:
        return i18n("1.0");
    };
}

int KisDynamicSensor::minimumValue(DynamicSensorType sensorType)
{
    switch (sensorType) {
    case FUZZY_PER_DAB:
    case FUZZY_PER_STROKE:
    case FADE:
    case DISTANCE:
    case TIME:
    case ANGLE:
    case SPEED:
    case ROTATION:
    case PRESSURE:
    case TILT_DIRECTION:
    case PERSPECTIVE:
    case PRESSURE_IN:
        return 0;
    case XTILT:
    case YTILT:
        return -30;
    case TILT_ELEVATATION:
        return 90;
    case TANGENTIAL_PRESSURE:
    default:
        return 0;
    }

}

int KisDynamicSensor::maximumValue(DynamicSensorType sensorType, int max)
{
    switch (sensorType) {
    case FUZZY_PER_DAB:
    case FUZZY_PER_STROKE:
    case SPEED:
    case PERSPECTIVE:
    case TANGENTIAL_PRESSURE:
    case PRESSURE_IN:
    case PRESSURE:
        return 100;
    case FADE:
        if (max < 0) {
            return 1000;
	} else {
            return  max;
	}
    case DISTANCE:
        if (max < 0) {
            return 30;
	} else {
            return max;
	}
    case TIME:
        if (max < 0) {
           return 3000;
	} else {
            return max;
	}
    case ANGLE:
    case ROTATION:
    case TILT_DIRECTION:
        return 360;
    case XTILT:
    case YTILT:
        return 30;
    case TILT_ELEVATATION:
        return 0;
    default:
        return 100;
    };
}

QString KisDynamicSensor::valueSuffix(DynamicSensorType sensorType)
{
    switch (sensorType) {
    case FUZZY_PER_DAB:
    case FUZZY_PER_STROKE:
    case SPEED:
    case PRESSURE:
    case PERSPECTIVE:
    case TANGENTIAL_PRESSURE:
    case PRESSURE_IN:
        return i18n("%");
    case FADE:
        return QString();
    case DISTANCE:
        return i18n(" px");
    case TIME:
        return i18n(" ms");
    case ANGLE:
    case ROTATION:
    case XTILT:
    case YTILT:
    case TILT_DIRECTION:
    case TILT_ELEVATATION:
        return i18n("°");
    default:
        return i18n("%");
    };
}

KisDynamicSensorSP KisDynamicSensor::createFromXML(const QString& s, const QString &parentOptionName)
{
    QDomDocument doc;
    doc.setContent(s);
    QDomElement e = doc.documentElement();
    return createFromXML(e, parentOptionName);
}

KisDynamicSensorSP KisDynamicSensor::createFromXML(const QDomElement& e, const QString &parentOptionName)
{
    QString id = e.attribute("id", "");
    KisDynamicSensorSP sensor = id2Sensor(id, parentOptionName);
    if (sensor) {
        sensor->fromXML(e);
    }
    return sensor;
}

QList<KoID> KisDynamicSensor::sensorsIds()
{
    QList<KoID> ids;

    ids << PressureId
        << PressureInId
        << XTiltId
        << YTiltId
        << TiltDirectionId
        << TiltElevationId
        << SpeedId
        << DrawingAngleId
        << RotationId
        << DistanceId
        << TimeId
        << FuzzyPerDabId
        << FuzzyPerStrokeId
        << FadeId
        << PerspectiveId
        << TangentialPressureId;

    return ids;
}

QList<DynamicSensorType> KisDynamicSensor::sensorsTypes()
{
    QList<DynamicSensorType> sensorTypes;
    sensorTypes
            << PRESSURE
            << PRESSURE_IN
            << XTILT
            << YTILT
            << TILT_DIRECTION
            << TILT_ELEVATATION
            << SPEED
            << ANGLE
            << ROTATION
            << DISTANCE
            << TIME
            << FUZZY_PER_DAB
            << FUZZY_PER_STROKE
            << FADE
            << PERSPECTIVE
            << TANGENTIAL_PRESSURE;
    return sensorTypes;
}

QString KisDynamicSensor::id(DynamicSensorType sensorType)
{
    switch (sensorType) {
    case FUZZY_PER_DAB:
        return "fuzzy";
    case FUZZY_PER_STROKE:
        return "fuzzystroke";
    case FADE:
        return "fade";
    case DISTANCE:
        return "distance";
    case TIME:
        return "time";
    case ANGLE:
        return "drawingangle";
    case SPEED:
        return "speed";
    case ROTATION:
        return "rotation";
    case PRESSURE:
        return "pressure";
    case XTILT:
        return "xtilt";
    case YTILT:
        return "ytilt";
    case TILT_DIRECTION:
        return "ascension";
    case TILT_ELEVATATION:
        return "declination";
    case PERSPECTIVE:
        return "perspective";
    case TANGENTIAL_PRESSURE:
        return "tangentialpressure";
    case PRESSURE_IN:
        return "pressurein";
    case SENSORS_LIST:
        return "sensorslist";
    default:
        return QString();
    };
}


void KisDynamicSensor::toXML(QDomDocument& doc, QDomElement& elt) const
{
    elt.setAttribute("id", id(sensorType()));
    if (m_customCurve) {
        QDomElement curve_elt = doc.createElement("curve");
        QDomText text = doc.createTextNode(m_curve.toString());
        curve_elt.appendChild(text);
        elt.appendChild(curve_elt);
    }
}

void KisDynamicSensor::fromXML(const QDomElement& e)
{
    Q_ASSERT(e.attribute("id", "") == id(sensorType()));
    m_customCurve = false;
    QDomElement curve_elt = e.firstChildElement("curve");
    if (!curve_elt.isNull()) {
        m_customCurve = true;
        m_curve.fromString(curve_elt.text());
    }
}

qreal KisDynamicSensor::parameter(const KisPaintInformation& info)
{
    const qreal val = value(info);
    if (m_customCurve) {
        qreal scaledVal = isAdditive() ? additiveToScaling(val) : val;

        const QVector<qreal> transfer = m_curve.floatTransfer(256);
        scaledVal = KisCubicCurve::interpolateLinear(scaledVal, transfer);

        return isAdditive() ? scalingToAdditive(scaledVal) : scaledVal;
    }
    else {
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

bool KisDynamicSensor::isAdditive() const
{
    return false;
}

bool KisDynamicSensor::isAbsoluteRotation() const
{
    return false;
}

void KisDynamicSensor::setActive(bool active)
{
    m_active = active;
}

bool KisDynamicSensor::isActive() const
{
    return m_active;
}
