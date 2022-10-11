/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISDYNAMICSENSORFUZZY2_H
#define KISDYNAMICSENSORFUZZY2_H

#include "KisDynamicSensor2.h"

class KisDynamicSensorData;

class KisDynamicSensorFuzzyBase2 : public KisDynamicSensor2
{
public:
    bool isAdditive() const override;
    qreal value(const KisPaintInformation &info) const override;

protected:
    KisDynamicSensorFuzzyBase2(const KoID &id, bool fuzzyPerStroke, const QString &parentOptionName, const KisSensorData &data, std::optional<KisCubicCurve> curveOverride);

private:
    const bool m_fuzzyPerStroke;
    QString m_perStrokeRandomSourceKey;
};

class KisDynamicSensorFuzzyPerDab2 : public KisDynamicSensorFuzzyBase2
{
public:
    KisDynamicSensorFuzzyPerDab2(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride);
};

class KisDynamicSensorFuzzyPerStroke2 : public KisDynamicSensorFuzzyBase2
{
public:
    KisDynamicSensorFuzzyPerStroke2(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride, const QString &parentOptionName);
};

#endif // KISDYNAMICSENSORFUZZY2_H
