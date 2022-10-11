/*
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisCurveOption2.h"
#include "kis_algebra_2d.h"

#include <sensors2/KisDynamicSensors2.h>
#include <sensors2/KisDynamicSensorDrawingAngle2.h>
#include <sensors2/KisDynamicSensorDistance2.h>
#include <sensors2/KisDynamicSensorFade2.h>
#include <sensors2/KisDynamicSensorTime2.h>
#include <sensors2/KisDynamicSensorFuzzy2.h>

namespace {
template <typename Sensor, typename Data, typename... Args>
void addSensor(std::vector<std::unique_ptr<KisDynamicSensor2>> &sensors,
               const Data &data, std::optional<KisCubicCurve> commonCurve, Args... args)
{
    if (data.isActive) {
        sensors.push_back(std::unique_ptr<KisDynamicSensor2>(new Sensor(data, commonCurve, args...)));
    }
}

std::vector<std::unique_ptr<KisDynamicSensor2>> generateSensors(const KisCurveOptionData &data)
{
    std::vector<std::unique_ptr<KisDynamicSensor2>> result;

    std::optional<KisCubicCurve> commonCurve;
    if (data.useSameCurve) {
        commonCurve = KisCubicCurve::createFromString(data.commonCurve);
    }

    addSensor<KisDynamicSensorPressure2>(result, data.sensorPressure, commonCurve);
    addSensor<KisDynamicSensorPressureIn2>(result, data.sensorPressureIn, commonCurve);
    addSensor<KisDynamicSensorTangentialPressure2>(result, data.sensorTangentialPressure, commonCurve);
    addSensor<KisDynamicSensorDrawingAngle2>(result, data.sensorDrawingAngle, commonCurve);
    addSensor<KisDynamicSensorXTilt2>(result, data.sensorXTilt, commonCurve);
    addSensor<KisDynamicSensorYTilt2>(result, data.sensorYTilt, commonCurve);
    addSensor<KisDynamicSensorTiltDirection2>(result, data.sensorTiltDirection, commonCurve);
    addSensor<KisDynamicSensorTiltElevation2>(result, data.sensorTiltElevation, commonCurve);
    addSensor<KisDynamicSensorRotation2>(result, data.sensorRotation, commonCurve);
    addSensor<KisDynamicSensorFuzzyPerDab2>(result, data.sensorFuzzyPerDab, commonCurve);
    addSensor<KisDynamicSensorFuzzyPerStroke2>(result, data.sensorFuzzyPerStroke, commonCurve, data.id.id());
    addSensor<KisDynamicSensorSpeed2>(result, data.sensorSpeed, commonCurve);
    addSensor<KisDynamicSensorFade2>(result, data.sensorFade, commonCurve);
    addSensor<KisDynamicSensorDistance2>(result, data.sensorDistance, commonCurve);
    addSensor<KisDynamicSensorTime2>(result, data.sensorTime, commonCurve);
    addSensor<KisDynamicSensorPerspective2>(result, data.sensorPerspective, commonCurve);

    return result;
}
}

qreal KisCurveOption2::ValueComponents::rotationLikeValue(qreal normalizedBaseAngle, bool absoluteAxesFlipped, qreal scalingPartCoeff, bool disableScalingPart) const {
    const qreal offset =
            !hasAbsoluteOffset ? normalizedBaseAngle :
                                 absoluteAxesFlipped ? 0.5 - absoluteOffset :
                                                       absoluteOffset;

    const qreal realScalingPart = hasScaling && !disableScalingPart ? KisDynamicSensor::scalingToAdditive(scaling) : 0.0;
    const qreal realAdditivePart = hasAdditive ? additive : 0;

    qreal value = KisAlgebra2D::wrapValue(2 * offset + constant * (scalingPartCoeff * realScalingPart + realAdditivePart), -1.0, 1.0);
    if (qIsNaN(value)) {
        qWarning() << "rotationLikeValue returns NaN!" << normalizedBaseAngle << absoluteAxesFlipped;
        value = 0;
    }
    return value;
}

qreal KisCurveOption2::ValueComponents::sizeLikeValue() const {
    const qreal offset =
            hasAbsoluteOffset ? absoluteOffset : 1.0;

    const qreal realScalingPart = hasScaling ? scaling : 1.0;
    const qreal realAdditivePart = hasAdditive ? KisDynamicSensor::additiveToScaling(additive) : 1.0;

    return qBound(minSizeLikeValue,
                  constant * offset * realScalingPart * realAdditivePart,
                  maxSizeLikeValue);
}


KisCurveOption2::KisCurveOption2(const KisCurveOptionData &data)
    : m_isChecked(data.isChecked)
    , m_useCurve(data.useCurve)
    , m_curveMode(data.curveMode)
    , m_separateCurveValue(data.separateCurveValue)
    , m_strengthValue(data.strengthValue)
    , m_strengthMinValue(data.strengthMinValue)
    , m_strengthMaxValue(data.strengthMaxValue)
    , m_sensors(generateSensors(data))
{
}

KisCurveOption2::ValueComponents KisCurveOption2::computeValueComponents(const KisPaintInformation& info, bool useStrengthValue) const
{
    ValueComponents components;

    if (m_useCurve) {
        QList<double> sensorValues;
        for (auto i = m_sensors.cbegin(); i != m_sensors.cend(); ++i) {
            KisDynamicSensor2 *s(i->get());

            qreal valueFromCurve = s->parameter(info);
            if (s->isAdditive()) {
                components.additive += valueFromCurve;
                components.hasAdditive = true;
            } else if (s->isAbsoluteRotation()) {
                components.absoluteOffset = valueFromCurve;
                components.hasAbsoluteOffset =true;
            } else {
                sensorValues << valueFromCurve;
                components.hasScaling = true;
            }
        }

        if (sensorValues.count() == 1) {
            components.scaling = sensorValues.first();
        } else {

            if (m_curveMode == 1){           // add
                components.scaling = 0;
                double i;
                foreach (i, sensorValues) {
                    components.scaling += i;
                }
            } else if (m_curveMode == 2){    //max
                components.scaling = *std::max_element(sensorValues.begin(), sensorValues.end());

            } else if (m_curveMode == 3){    //min
                components.scaling = *std::min_element(sensorValues.begin(), sensorValues.end());

            } else if (m_curveMode == 4){    //difference
                double max = *std::max_element(sensorValues.begin(), sensorValues.end());
                double min = *std::min_element(sensorValues.begin(), sensorValues.end());
                components.scaling = max-min;

            } else {                         //multuply - default
                double i;
                foreach (i, sensorValues) {
                    components.scaling *= i;
                }
            }
        }

    }

    if (useStrengthValue) {
        components.constant = m_strengthValue;
    }

    components.minSizeLikeValue = m_strengthMinValue;
    components.maxSizeLikeValue = m_strengthMaxValue;

    return components;
}

qreal KisCurveOption2::computeSizeLikeValue(const KisPaintInformation& info, bool useStrengthValue) const
{
    const ValueComponents components = computeValueComponents(info, useStrengthValue);
    return components.sizeLikeValue();
}

qreal KisCurveOption2::computeRotationLikeValue(const KisPaintInformation& info, qreal baseValue, bool absoluteAxesFlipped, qreal scalingPartCoeff, bool disableScalingPart) const
{
    const ValueComponents components = computeValueComponents(info, true);
    return components.rotationLikeValue(baseValue, absoluteAxesFlipped, scalingPartCoeff, disableScalingPart);
}

qreal KisCurveOption2::strengthValue() const
{
    return m_strengthValue;
}

qreal KisCurveOption2::strengthMinValue() const
{
    return m_strengthMinValue;
}

qreal KisCurveOption2::strengthMaxValue() const
{
    return m_strengthMaxValue;
}

bool KisCurveOption2::isChecked() const
{
    return m_isChecked;
}
