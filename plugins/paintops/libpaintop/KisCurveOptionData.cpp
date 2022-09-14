/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisCurveOptionData.h"

#include <QDomDocument>
#include <QDomElement>

KisSensorData::KisSensorData(const KoID &sensorId)
    : id(sensorId),
      curve(DEFAULT_CURVE_STRING)
{
}

KisSensorData::~KisSensorData()
{
}

void KisSensorData::write(QDomDocument& doc, QDomElement &e) const
{
    e.setAttribute("id", id.id());
    if (curve != DEFAULT_CURVE_STRING) {
        QDomElement curve_elt = doc.createElement("curve");
        QDomText text = doc.createTextNode(curve);
        curve_elt.appendChild(text);
        e.appendChild(curve_elt);
    }
}

void KisSensorData::read(const QDomElement& e)
{
    KIS_ASSERT(e.attribute("id", "") == id.id());
    QDomElement curve_elt = e.firstChildElement("curve");
    if (!curve_elt.isNull()) {
        curve = curve_elt.text();
    } else {
        curve = DEFAULT_CURVE_STRING;
    }
}

void KisSensorData::reset()
{
    *this = KisSensorData(id);
}

KisSensorDataWithLength::KisSensorDataWithLength(const KoID &sensorId)
    : KisSensorData(sensorId)
{
    if (sensorId == FadeId) {
        isPeriodic = false;
        length = 1000;
    } else if (sensorId == DistanceId) {
        isPeriodic = false;
        length = 30;
    } else if (sensorId == TimeId) {
        isPeriodic = false;
        length = 30;
    } else {
        qFatal("This sensor type \"%s\" has no length associated!", sensorId.id().toLatin1().data());
    }
}

void KisSensorDataWithLength::write(QDomDocument &doc, QDomElement &e) const
{
    KisSensorData::write(doc, e);
    e.setAttribute("periodic", isPeriodic);
    e.setAttribute("length", length);
}

void KisSensorDataWithLength::read(const QDomElement &e)
{
    reset();
    KisSensorData::read(e);

    if (e.hasAttribute("periodic")) {
        isPeriodic = e.attribute("periodic").toInt();
    }

    if (e.hasAttribute("length")) {
        isPeriodic = e.attribute("length").toInt();
    }
}

void KisSensorDataWithLength::reset()
{
    *this = KisSensorDataWithLength(id);
}

KisDrawingAngleSensorData::KisDrawingAngleSensorData()
    : KisSensorData(DrawingAngleId)
{
}

void KisDrawingAngleSensorData::write(QDomDocument &doc, QDomElement &e) const
{
    KisSensorData::write(doc, e);
    e.setAttribute("fanCornersEnabled", lockedAngleMode);
    e.setAttribute("fanCornersStep", fanCornersStep);
    e.setAttribute("angleOffset", angleOffset);
    e.setAttribute("lockedAngleMode", lockedAngleMode);
}

void KisDrawingAngleSensorData::read(const QDomElement &e)
{
    reset();
    KisSensorData::read(e);

    if (e.hasAttribute("fanCornersEnabled")) {
        fanCornersEnabled = e.attribute("fanCornersEnabled").toInt();
    }
    if (e.hasAttribute("fanCornersStep")) {
        fanCornersStep = e.attribute("fanCornersStep").toInt();
    }
    if (e.hasAttribute("angleOffset")) {
        angleOffset = e.attribute("angleOffset").toInt();
    }
    if (e.hasAttribute("lockedAngleMode")) {
        lockedAngleMode = e.attribute("lockedAngleMode").toInt();
    }
}

void KisDrawingAngleSensorData::reset()
{
    *this = KisDrawingAngleSensorData();
}

KisCurveOptionData::KisCurveOptionData(const KoID _id, KisPaintOpOption::PaintopCategory _category, bool _isCheckable, bool _isChecked, bool _separateCurveValue, qreal _minValue, qreal _maxValue)
    : id(_id),
      category(_category),
      isCheckable(_isCheckable),
      separateCurveValue(_separateCurveValue),
      minValue(_minValue),
      maxValue(_maxValue),
      isChecked(_isChecked),
      value(_maxValue),
      sensorPressure(PressureId),
      sensorPressureIn(PressureInId),
      sensorXTilt(XTiltId),
      sensorYTilt(YTiltId),
      sensorTiltDirection(TiltDirectionId),
      sensorTiltElevation(TiltElevationId),
      sensorSpeed(SpeedId),
      sensorDrawingAngle(),
      sensorRotation(RotationId),
      sensorDistance(DistanceId),
      sensorTime(TimeId),
      sensorFuzzyPerDab(FuzzyPerDabId),
      sensorFuzzyPerStroke(FuzzyPerStrokeId),
      sensorFade(FadeId),
      sensorPerspective(PerspectiveId),
      sensorTangentialPressure(TangentialPressureId)
{
    sensorPressure.isActive = true;
}

std::vector<KisSensorData *> KisCurveOptionData::sensors()
{
    std::vector<KisSensorData *> result;

    result.reserve(16);

    result.push_back(&sensorPressure);
    result.push_back(&sensorPressureIn);
    result.push_back(&sensorXTilt);
    result.push_back(&sensorYTilt);
    result.push_back(&sensorTiltDirection);
    result.push_back(&sensorTiltElevation);
    result.push_back(&sensorSpeed);
    result.push_back(&sensorDrawingAngle);
    result.push_back(&sensorRotation);
    result.push_back(&sensorDistance);
    result.push_back(&sensorTime);
    result.push_back(&sensorFuzzyPerDab);
    result.push_back(&sensorFuzzyPerStroke);
    result.push_back(&sensorFade);
    result.push_back(&sensorPerspective);
    result.push_back(&sensorTangentialPressure);

    return result;
}

bool KisCurveOptionData::read(const KisPropertiesConfiguration *setting)
{
    if (!setting) return false;

    if (isCheckable) {
        isChecked = setting->getBool("Pressure" + id.id(), false);
    }

    std::vector<KisSensorData*> sensors = this->sensors();
    QMap<QString, KisSensorData*> sensorById;

    Q_FOREACH (KisSensorData *sensor, sensors) {
        sensorById.insert(sensor->id.id(), sensor);
    }

    QSet<KisSensorData*> sensorsToReset =
        QSet<KisSensorData*>::fromList(sensorById.values());

    const QString sensorDefinition = setting->getString(id.id() + "Sensor");
    if (!sensorDefinition.contains("sensorslist")) {
        QDomDocument doc;
        doc.setContent(sensorDefinition);
        QDomElement e = doc.documentElement();

        const QString sensorId = e.attribute("id", "");
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!sensorId.isEmpty(), false);
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(sensorById.contains(sensorId), false);

        KisSensorData *sensor = sensorById[sensorId];
        sensor->read(e);
        sensor->isActive = true;
        sensorsToReset.remove(sensor);

        commonCurve = sensor->curve;

    } else {
        QString proposedCommonCurve;

        QDomDocument doc;
        doc.setContent(sensorDefinition);
        QDomElement elt = doc.documentElement();
        QDomNode node = elt.firstChild();
        while (!node.isNull()) {
            if (node.isElement())  {
                QDomElement childelt = node.toElement();
                if (childelt.tagName() == "ChildSensor") {

                    const QString sensorId = childelt.attribute("id", "");
                    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!sensorId.isEmpty(), false);
                    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(sensorById.contains(sensorId), false);

                    KisSensorData *sensor = sensorById[sensorId];
                    sensor->read(childelt);
                    sensor->isActive = true;
                    sensorsToReset.remove(sensor);

                    /// due to legacy reasons we initialize common curve
                    /// with the "last read curve value"
                    proposedCommonCurve = sensor->curve;
                }
            }
            node = node.nextSibling();
        }

        commonCurve = !proposedCommonCurve.isEmpty() ? proposedCommonCurve : DEFAULT_CURVE_STRING;
    }

    useSameCurve = setting->getBool(id.id() + "UseSameCurve", true);

    // Only load the old curve format if the curve wasn't saved by the sensor
    // This will give every sensor the same curve.
    if (!sensorDefinition.contains("curve")) {
        if (setting->getBool("Custom" + id.id(), false)) {
            commonCurve = setting->getString("Curve" + id.id(), DEFAULT_CURVE_STRING);
            Q_FOREACH (KisSensorData *sensor, sensors) {
                sensor->curve = commonCurve;
                sensorsToReset.remove(sensor);
            }
        } else {
            commonCurve = DEFAULT_CURVE_STRING;
        }
    }

    if (useSameCurve) {
        commonCurve = setting->getString(id.id() + "commonCurve", DEFAULT_CURVE_STRING);
    }

    Q_FOREACH (KisSensorData *sensor, sensorsToReset) {
        sensor->reset();
    }

    // At least one sensor needs to be active
    if (std::find_if(sensors.begin(), sensors.end(),
                     std::mem_fn(&KisSensorData::isActive)) == sensors.end()) {

        sensorById[PressureId.id()]->isActive = true;
    }

    value = setting->getDouble(id.id() + "Value", maxValue);
    useCurve = setting->getBool(id.id() + "UseCurve", true);
    curveMode = setting->getInt(id.id() + "curveMode", 0);

    return true;
}

void KisCurveOptionData::write(KisPropertiesConfiguration *setting)
{
    if (isCheckable) {
        setting->setProperty("Pressure" + id.id(), isChecked);
    }

    QVector<KisSensorData*> activeSensors;
    Q_FOREACH(KisSensorData *sensor, sensors()) {
        if (sensor->isActive) {
            activeSensors.append(sensor);
        }
    }

    QDomDocument doc = QDomDocument("params");
    QDomElement root = doc.createElement("params");
    doc.appendChild(root);

    if (activeSensors.size() == 1) {
        activeSensors.first()->write(doc, root);
    } else {
        root.setAttribute("id", "sensorslist");
        Q_FOREACH (KisSensorData *sensor, activeSensors) {
            QDomElement childelt = doc.createElement("ChildSensor");
            sensor->write(doc, childelt);
            root.appendChild(childelt);
        }
    }
    setting->setProperty(id.id() + "Sensor", doc.toString());

    setting->setProperty(id.id() + "UseCurve", useCurve);
    setting->setProperty(id.id() + "UseSameCurve", useSameCurve);
    setting->setProperty(id.id() + "Value", value);
    setting->setProperty(id.id() + "curveMode", curveMode);
    setting->setProperty(id.id() + "commonCurve", commonCurve);
}
