/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISDYNAMICSENSORTIME2_H
#define KISDYNAMICSENSORTIME2_H

#include "KisDynamicSensor2.h"

struct KisSensorWithLengthData;

class KisDynamicSensorTime2 : public KisDynamicSensor2
{
public:
    KisDynamicSensorTime2(const KisSensorWithLengthData &data, std::optional<KisCubicCurve> curveOverride);

    qreal value(const KisPaintInformation &pi) const override;

private:
    bool m_periodic;
    int m_length;
};

#endif // KISDYNAMICSENSORTIME2_H
