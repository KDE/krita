/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISDYNAMICSENSORFUZZY_H
#define KISDYNAMICSENSORFUZZY_H

#include "KisDynamicSensor.h"

class KisDynamicSensorData;

class KisDynamicSensorFuzzyBase : public KisDynamicSensor
{
public:
    bool isAdditive() const override;
    qreal value(const KisPaintInformation &info) const override;

protected:
    KisDynamicSensorFuzzyBase(const KoID &id, bool fuzzyPerStroke, const QString &parentOptionName, const KisSensorData &data, std::optional<KisCubicCurve> curveOverride);

private:
    const bool m_fuzzyPerStroke;
    QString m_perStrokeRandomSourceKey;
};

class KisDynamicSensorFuzzyPerDab : public KisDynamicSensorFuzzyBase
{
public:
    KisDynamicSensorFuzzyPerDab(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride);
};

class KisDynamicSensorFuzzyPerStroke : public KisDynamicSensorFuzzyBase
{
public:
    KisDynamicSensorFuzzyPerStroke(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride, const QString &parentOptionName);
};

#endif // KISDYNAMICSENSORFUZZY_H
