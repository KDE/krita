/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISDYNAMICSENSORDISTANCE_H
#define KISDYNAMICSENSORDISTANCE_H

#include "KisDynamicSensor.h"

struct KisSensorWithLengthData;

class KisDynamicSensorDistance : public KisDynamicSensor
{
public:
    KisDynamicSensorDistance(const KisSensorWithLengthData &data, std::optional<KisCubicCurve> curveOverride);

    qreal value(const KisPaintInformation &pi) const override;

private:
    bool m_periodic;
    int m_length;
};

#endif // KISDYNAMICSENSORDISTANCE_H
