/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISDYNAMICSENSORFADE2_H
#define KISDYNAMICSENSORFADE2_H

#include "KisDynamicSensor2.h"

class KisSensorWithLengthData;

class KisDynamicSensorFade2 : public KisDynamicSensor2
{
public:
    KisDynamicSensorFade2(const KisSensorWithLengthData &data, std::optional<KisCubicCurve> curveOverride);

    qreal value(const KisPaintInformation &pi) const override;

private:
    bool m_periodic;
    int m_length;
};

#endif // KISDYNAMICSENSORFADE2_H
