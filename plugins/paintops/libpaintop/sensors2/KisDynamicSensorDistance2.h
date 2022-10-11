/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISDYNAMICSENSORDISTANCE2_H
#define KISDYNAMICSENSORDISTANCE2_H

#include "KisDynamicSensor2.h"

class KisSensorWithLengthData;

class KisDynamicSensorDistance2 : public KisDynamicSensor2
{
public:
    KisDynamicSensorDistance2(const KisSensorWithLengthData &data, std::optional<KisCubicCurve> curveOverride);

    qreal value(const KisPaintInformation &pi) const override;

private:
    bool m_periodic;
    int m_length;
};

#endif // KISDYNAMICSENSORDISTANCE2_H
