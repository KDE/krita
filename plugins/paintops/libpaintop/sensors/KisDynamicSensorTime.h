/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISDYNAMICSENSORTIME_H
#define KISDYNAMICSENSORTIME_H

#include "KisDynamicSensor.h"

struct KisSensorWithLengthData;

class KisDynamicSensorTime : public KisDynamicSensor
{
public:
    KisDynamicSensorTime(const KisSensorWithLengthData &data, std::optional<KisCubicCurve> curveOverride);

    qreal value(const KisPaintInformation &pi) const override;

private:
    bool m_periodic;
    int m_length;
};

#endif // KISDYNAMICSENSORTIME_H
