/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisCurveOptionData.h"

#include <QDomDocument>
#include <QDomElement>

#include <KisCppQuirks.h>

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

KisSensorWithLengthData::KisSensorWithLengthData(const KoID &sensorId, const QLatin1String &lengthTag)
    : KisSensorData(sensorId)
    , m_lengthTag(lengthTag.isNull() ? QLatin1Literal("length") : lengthTag)
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

void KisSensorWithLengthData::write(QDomDocument &doc, QDomElement &e) const
{
    KisSensorData::write(doc, e);
    e.setAttribute("periodic", isPeriodic);
    e.setAttribute(m_lengthTag, length);
}

void KisSensorWithLengthData::read(const QDomElement &e)
{
    reset();
    KisSensorData::read(e);

    if (e.hasAttribute("periodic")) {
        isPeriodic = e.attribute("periodic").toInt();
    }

    if (e.hasAttribute(m_lengthTag)) {
        length = e.attribute(m_lengthTag).toInt();
    }
}

void KisSensorWithLengthData::reset()
{
    *this = KisSensorWithLengthData(id, m_lengthTag);
}

KisDrawingAngleSensorData::KisDrawingAngleSensorData()
    : KisSensorData(DrawingAngleId)
{
}

void KisDrawingAngleSensorData::write(QDomDocument &doc, QDomElement &e) const
{
    KisSensorData::write(doc, e);
    e.setAttribute("fanCornersEnabled", fanCornersEnabled);
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

KisCurveOptionData::KisCurveOptionData(const QString &_prefix, const KoID _id, bool _isCheckable, bool _isChecked, bool _separateCurveValue, qreal _minValue, qreal _maxValue)
    : id(_id),
      prefix(_prefix),
      isCheckable(_isCheckable),
      separateCurveValue(_separateCurveValue),
      strengthMinValue(_minValue),
      strengthMaxValue(_maxValue),
      isChecked(_isChecked),
      strengthValue(_maxValue),
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
      sensorTime(TimeId, QLatin1Literal("duration")),
      sensorFuzzyPerDab(FuzzyPerDabId),
      sensorFuzzyPerStroke(FuzzyPerStrokeId),
      sensorFade(FadeId),
      sensorPerspective(PerspectiveId),
      sensorTangentialPressure(TangentialPressureId)
{
    sensorPressure.isActive = true;
}

KisCurveOptionData::KisCurveOptionData(const KoID _id, bool _isCheckable, bool _isChecked, bool _separateCurveValue, qreal _minValue, qreal _maxValue)
    : KisCurveOptionData("", _id, _isCheckable, _isChecked, _separateCurveValue, _minValue, _maxValue)
{
}

namespace detail {
template <typename Data,
          typename SensorData =
              std::add_const_if_t<std::is_const_v<Data>,
                                  KisSensorData>>
std::vector<SensorData*> sensors(Data *data)
{
    std::vector<SensorData*> result;

    result.reserve(16);

    result.push_back(&data->sensorPressure);
    result.push_back(&data->sensorPressureIn);
    result.push_back(&data->sensorTangentialPressure);

    result.push_back(&data->sensorDrawingAngle);
    result.push_back(&data->sensorXTilt);
    result.push_back(&data->sensorYTilt);
    result.push_back(&data->sensorTiltDirection);
    result.push_back(&data->sensorTiltElevation);
    result.push_back(&data->sensorRotation);

    result.push_back(&data->sensorFuzzyPerDab);
    result.push_back(&data->sensorFuzzyPerStroke);

    result.push_back(&data->sensorSpeed);
    result.push_back(&data->sensorFade);
    result.push_back(&data->sensorDistance);
    result.push_back(&data->sensorTime);

    result.push_back(&data->sensorPerspective);


    return result;
}
}

std::vector<const KisSensorData*> KisCurveOptionData::sensors() const
{
    return detail::sensors(this);
}

std::vector<KisSensorData*> KisCurveOptionData::sensors()
{
    return detail::sensors(this);
}

bool KisCurveOptionData::read(const KisPropertiesConfiguration *setting)
{
    if (!setting) return false;

    if (prefix.isEmpty()) {
        return readPrefixed(setting);
    } else {
        KisPropertiesConfiguration prefixedSetting;
        setting->getPrefixedProperties(prefix, &prefixedSetting);
        return readPrefixed(&prefixedSetting);
    }
}

void KisCurveOptionData::write(KisPropertiesConfiguration *setting) const
{
    if (prefix.isEmpty()) {
        writePrefixed(setting);
    } else {
        KisPropertiesConfiguration prefixedSetting;
        writePrefixed(&prefixedSetting);
        setting->setPrefixedProperties(prefix, &prefixedSetting);
    }
}

bool KisCurveOptionData::readPrefixed(const KisPropertiesConfiguration *setting)
{
    isChecked = !isCheckable || setting->getBool("Pressure" + id.id(), false);

    std::vector<KisSensorData*> sensors = this->sensors();
    QMap<QString, KisSensorData*> sensorById;

    Q_FOREACH (KisSensorData *sensor, sensors) {
        sensorById.insert(sensor->id.id(), sensor);
    }

    QSet<KisSensorData*> sensorsToReset =
        QSet<KisSensorData*>::fromList(sensorById.values());

    const QString sensorDefinition = setting->getString(id.id() + "Sensor");

    if (sensorDefinition.isEmpty()) {
        // noop
    } else if (!sensorDefinition.contains("sensorslist")) {
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

    strengthValue = setting->getDouble(id.id() + "Value", strengthMaxValue);

    if (valueFixUpReadCallback) {
        strengthValue = valueFixUpReadCallback(strengthValue, setting);
    }

    useCurve = setting->getBool(id.id() + "UseCurve", true);
    curveMode = setting->getInt(id.id() + "curveMode", 0);

    return true;
}

void KisCurveOptionData::writePrefixed(KisPropertiesConfiguration *setting) const
{
    setting->setProperty("Pressure" + id.id(), isChecked || !isCheckable);

    QVector<const KisSensorData*> activeSensors;
    Q_FOREACH(const KisSensorData *sensor, sensors()) {
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
        Q_FOREACH (const KisSensorData *sensor, activeSensors) {
            QDomElement childelt = doc.createElement("ChildSensor");
            sensor->write(doc, childelt);
            root.appendChild(childelt);
        }
    }
    setting->setProperty(id.id() + "Sensor", doc.toString());

    setting->setProperty(id.id() + "UseCurve", useCurve);
    setting->setProperty(id.id() + "UseSameCurve", useSameCurve);
    setting->setProperty(id.id() + "Value", strengthValue);
    if (valueFixUpWriteCallback) {
        valueFixUpWriteCallback(strengthValue, setting);
    }
    setting->setProperty(id.id() + "curveMode", curveMode);
    setting->setProperty(id.id() + "commonCurve", commonCurve);
}
