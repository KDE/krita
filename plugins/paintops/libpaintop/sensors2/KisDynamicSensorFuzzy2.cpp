/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisDynamicSensorFuzzy2.h"

// TODO
#include <kis_dynamic_sensor.h>

#include <kis_paint_information.h>
#include <KisCurveOptionData.h>

KisDynamicSensorFuzzyBase2::KisDynamicSensorFuzzyBase2(const KoID &id, bool fuzzyPerStroke, const QString &perStrokeRandomSourceKey, const KisSensorData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensor2(id, data, curveOverride)
    , m_fuzzyPerStroke(fuzzyPerStroke)
    , m_perStrokeRandomSourceKey(perStrokeRandomSourceKey)
{
}

bool KisDynamicSensorFuzzyBase2::isAdditive() const
{
    return true;
}

qreal KisDynamicSensorFuzzyBase2::value(const KisPaintInformation &info) const
{
    qreal result = 0.0;

    if (!info.isHoveringMode()) {
        result = m_fuzzyPerStroke ?
            info.perStrokeRandomSource()->generateNormalized(m_perStrokeRandomSourceKey) :
            info.randomSource()->generateNormalized();
        result = 2.0 * result - 1.0;
    }

    return result;
}

KisDynamicSensorFuzzyPerDab2::KisDynamicSensorFuzzyPerDab2(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensorFuzzyBase2(FuzzyPerDabId, false, "", data, curveOverride)
{

}

KisDynamicSensorFuzzyPerStroke2::KisDynamicSensorFuzzyPerStroke2(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride, const QString &parentOptionName)
   : KisDynamicSensorFuzzyBase2(FuzzyPerStrokeId, true, parentOptionName + "FuzzyStroke", data, curveOverride)
{
}
