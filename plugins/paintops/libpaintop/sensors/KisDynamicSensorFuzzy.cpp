/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisDynamicSensorFuzzy.h"

#include <KisDynamicSensorIds.h>

#include <kis_paint_information.h>
#include <KisCurveOptionData.h>

KisDynamicSensorFuzzyBase::KisDynamicSensorFuzzyBase(const KoID &id, bool fuzzyPerStroke, const QString &perStrokeRandomSourceKey, const KisSensorData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensor(id, data, curveOverride)
    , m_fuzzyPerStroke(fuzzyPerStroke)
    , m_perStrokeRandomSourceKey(perStrokeRandomSourceKey)
{
}

bool KisDynamicSensorFuzzyBase::isAdditive() const
{
    return true;
}

qreal KisDynamicSensorFuzzyBase::value(const KisPaintInformation &info) const
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

KisDynamicSensorFuzzyPerDab::KisDynamicSensorFuzzyPerDab(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensorFuzzyBase(FuzzyPerDabId, false, "", data, curveOverride)
{

}

KisDynamicSensorFuzzyPerStroke::KisDynamicSensorFuzzyPerStroke(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride, const QString &parentOptionName)
   : KisDynamicSensorFuzzyBase(FuzzyPerStrokeId, true, parentOptionName + "FuzzyStroke", data, curveOverride)
{
}
