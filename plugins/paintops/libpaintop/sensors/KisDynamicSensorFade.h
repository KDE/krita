/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISDYNAMICSENSORFADE_H
#define KISDYNAMICSENSORFADE_H

#include "KisDynamicSensor.h"

struct KisSensorWithLengthData;

class KisDynamicSensorFade : public KisDynamicSensor
{
public:
    KisDynamicSensorFade(const KisSensorWithLengthData &data, std::optional<KisCubicCurve> curveOverride);

    qreal value(const KisPaintInformation &pi) const override;

private:
    bool m_periodic;
    int m_length;
};

#endif // KISDYNAMICSENSORFADE_H
