/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisKritaSensorPack.h"
#include "KisCppQuirks.h"
#include "kis_assert.h"

#include <QDomDocument>
#include <QDomElement>

#include <KisCurveOptionData.h>

namespace detail {
template <typename Data,
          typename SensorData =
              std::copy_const_t<Data,
                               KisSensorData>>
inline std::vector<SensorData*> sensors(Data *data)
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
} // namespace detail

KisKritaSensorData::KisKritaSensorData()
    : sensorPressure(PressureId),
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

KisKritaSensorPack::KisKritaSensorPack(Checkability checkability)
    : m_checkability(checkability)
{
}

KisSensorPackInterface * KisKritaSensorPack::clone() const
{
    return new KisKritaSensorPack(*this);
}

std::vector<const KisSensorData *> KisKritaSensorPack::constSensors() const
{
    return detail::sensors(&m_data);
}

std::vector<KisSensorData *> KisKritaSensorPack::sensors()
{
    return detail::sensors(&m_data);
}

const KisKritaSensorData& KisKritaSensorPack::constSensorsStruct() const 
{
    return m_data;
}

KisKritaSensorData& KisKritaSensorPack::sensorsStruct()
{
    return m_data;
}

bool KisKritaSensorPack::compare(const KisSensorPackInterface *rhs) const
{
    const KisKritaSensorPack *pack = dynamic_cast<const KisKritaSensorPack*>(rhs);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(pack, false);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_checkability == pack->m_checkability, false);

    return m_data == pack->m_data;
}

bool KisKritaSensorPack::read(KisCurveOptionDataCommon &data, const KisPropertiesConfiguration *setting) const
{
    data.isCheckable = m_checkability == Checkability::Checkable ||
        (m_checkability == Checkability::CheckableIfHasPrefix &&
         (!data.prefix.isEmpty() ||
          !setting->getString(KisPropertiesConfiguration::extractedPrefixKey()).isEmpty()));

    data.isChecked = !data.isCheckable || setting->getBool("Pressure" + data.id.id(), false);

    std::vector<KisSensorData*> sensors = data.sensors();
    QMap<QString, KisSensorData*> sensorById;

    Q_FOREACH (KisSensorData *sensor, sensors) {
        sensorById.insert(sensor->id.id(), sensor);
    }

    QSet<KisSensorData*> sensorsToReset;

#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)

    QList l = sensorById.values();
    if (!l.isEmpty()) {
        sensorsToReset = QSet<KisSensorData*>(l.begin(), l.end());
    }
#else
    sensorsToReset =
        QSet<KisSensorData*>::fromList(sensorById.values());
#endif

    const QString sensorDefinition = setting->getString(data.id.id() + "Sensor");

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

        data.commonCurve = sensor->curve;

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

        data.commonCurve = !proposedCommonCurve.isEmpty() ? proposedCommonCurve : DEFAULT_CURVE_STRING;
    }

    data.useSameCurve = setting->getBool(data.id.id() + "UseSameCurve", true);

    // Only load the old curve format if the curve wasn't saved by the sensor
    // This will give every sensor the same curve.
    if (!sensorDefinition.contains("curve")) {
        if (setting->getBool("Custom" + data.id.id(), false)) {
            data.commonCurve = setting->getString("Curve" + data.id.id(), DEFAULT_CURVE_STRING);
            Q_FOREACH (KisSensorData *sensor, sensors) {
                sensor->curve = data.commonCurve;
                sensorsToReset.remove(sensor);
            }
        } else {
            data.commonCurve = DEFAULT_CURVE_STRING;
        }
    }

    if (data.useSameCurve) {
        data.commonCurve = setting->getString(data.id.id() + "commonCurve",
                                              !data.commonCurve.isEmpty() ?
                                                  data.commonCurve :
                                                  DEFAULT_CURVE_STRING);
        if (data.commonCurve.isEmpty()) {
            data.commonCurve = DEFAULT_CURVE_STRING;
        }
    }

    Q_FOREACH (KisSensorData *sensor, sensorsToReset) {
        sensor->reset();
    }

    // At least one sensor needs to be active
    if (std::find_if(sensors.begin(), sensors.end(),
                     std::mem_fn(&KisSensorData::isActive)) == sensors.end()) {

        sensorById[PressureId.id()]->isActive = true;
    }

    data.strengthValue = setting->getDouble(data.id.id() + "Value", data.strengthMaxValue);

    if (data.valueFixUpReadCallback) {
        data.valueFixUpReadCallback(&data, setting);
    }

    data.useCurve = setting->getBool(data.id.id() + "UseCurve", true);
    data.curveMode = setting->getInt(data.id.id() + "curveMode", 0);

    return true;
}

void KisKritaSensorPack::write(const KisCurveOptionDataCommon &data, KisPropertiesConfiguration *setting) const
{
    setting->setProperty("Pressure" + data.id.id(), data.isChecked || !data.isCheckable);

    QVector<const KisSensorData*> activeSensors;
    Q_FOREACH(const KisSensorData *sensor, data.sensors()) {
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
    setting->setProperty(data.id.id() + "Sensor", doc.toString());

    setting->setProperty(data.id.id() + "UseCurve", data.useCurve);
    setting->setProperty(data.id.id() + "UseSameCurve", data.useSameCurve);
    setting->setProperty(data.id.id() + "Value", data.strengthValue);
    if (data.valueFixUpWriteCallback) {
        data.valueFixUpWriteCallback(data.strengthValue, setting);
    }
    setting->setProperty(data.id.id() + "curveMode", data.curveMode);
    setting->setProperty(data.id.id() + "commonCurve", data.commonCurve);
}

int KisKritaSensorPack::calcActiveSensorLength(const QString &activeSensorId) const
{
    if (activeSensorId == FadeId.id()) {
        return m_data.sensorFade.length;
    } else if (activeSensorId == DistanceId.id()) {
        return m_data.sensorDistance.length;
    } else if (activeSensorId == TimeId.id()) {
        return m_data.sensorTime.length;
    }

    return -1;
}
