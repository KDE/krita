/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisCurveOptionModel.h"

#include <KisZug.h>

auto activeCurveLens = lager::lenses::getset(
    [](std::tuple<KisCurveOptionData, QString> data) -> QString {
        QString activeCurve;
        const bool useSameCurve = std::get<0>(data).useSameCurve;

        if (useSameCurve) {
            activeCurve = std::get<0>(data).commonCurve;
        } else {
            const QString activeSensorId = std::get<1>(data);
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!activeSensorId.isEmpty(), activeCurve);
            std::vector<KisSensorData*> srcSensors = std::get<0>(data).sensors();
            auto it =
                std::find_if(srcSensors.begin(), srcSensors.end(),
                    [activeSensorId] (const KisSensorData *sensor) {
                        return sensor->id.id() == activeSensorId;
                    });

            KIS_SAFE_ASSERT_RECOVER_NOOP(it != srcSensors.end());

            if (it != srcSensors.end()) {
                activeCurve = (*it)->curve;
            }
        }

        return activeCurve;
    },
    [](std::tuple<KisCurveOptionData, QString> data, QString curve) {
        const bool useSameCurve = std::get<0>(data).useSameCurve;

        if (useSameCurve) {
            std::get<0>(data).commonCurve = curve;
        } else {
            const QString activeSensorId = std::get<1>(data);
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!activeSensorId.isEmpty(), data);
            std::vector<KisSensorData*> srcSensors = std::get<0>(data).sensors();
            auto it =
                std::find_if(srcSensors.begin(), srcSensors.end(),
                    [activeSensorId] (const KisSensorData *sensor) {
                        return sensor->id.id() == activeSensorId;
                    });

            KIS_SAFE_ASSERT_RECOVER_NOOP(it != srcSensors.end());

            if (it != srcSensors.end()) {
                (*it)->curve = curve;
            }
        }

        return data;
    });

int calcActiveSensorLength(const KisCurveOptionData &data, const QString &activeSensorId) {
    int length = -1;

    if (activeSensorId == FadeId.id()) {
        return data.sensorFade.length;
    } else if (activeSensorId == DistanceId.id()) {
        return data.sensorDistance.length;
    } else if (activeSensorId == TimeId.id()) {
        return data.sensorTime.length;
    }

    return length;
}

KisCurveOptionModel::KisCurveOptionModel(lager::cursor<KisCurveOptionData> optionData)
    : m_optionData(optionData)
    , m_activeSensorIdData(PressureId.id())
    , LAGER_QT(isCheckable) {m_optionData[&KisCurveOptionData::isCheckable]}
    , LAGER_QT(isChecked) {m_optionData[&KisCurveOptionData::isChecked]}
    , LAGER_QT(value) {m_optionData[&KisCurveOptionData::value]
            .zoom(kiszug::lenses::scale<qreal>(100.0))}
    , LAGER_QT(range) {lager::with(m_optionData[&KisCurveOptionData::minValue],
                                   m_optionData[&KisCurveOptionData::maxValue])
            .xform(kiszug::foreach_arg(kiszug::map_mupliply<qreal>(100.0)))}

    , LAGER_QT(useCurve) {m_optionData[&KisCurveOptionData::useCurve]}
    , LAGER_QT(useSameCurve) {m_optionData[&KisCurveOptionData::useSameCurve]}
    , LAGER_QT(curveMode) {m_optionData[&KisCurveOptionData::curveMode]}
    , LAGER_QT(activeSensorId) {m_activeSensorIdData}
    , LAGER_QT(activeSensorLength) {lager::with(m_optionData, m_activeSensorIdData).map(&calcActiveSensorLength)}
    , LAGER_QT(labelsState) {lager::with(LAGER_QT(activeSensorId), LAGER_QT(activeSensorLength))}
    , LAGER_QT(activeCurve) {lager::with(m_optionData,
                                         LAGER_QT(activeSensorId))
            .zoom(activeCurveLens)}
{
}

KisCurveOptionModel::~KisCurveOptionModel()
{
}
