/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISDYNAMICSENSORDRAWINGANGLE2_H
#define KISDYNAMICSENSORDRAWINGANGLE2_H

#include "KisDynamicSensor2.h"

struct KisDrawingAngleSensorData;

class KisDynamicSensorDrawingAngle2 : public KisDynamicSensor2
{
public:
    KisDynamicSensorDrawingAngle2(const KisDrawingAngleSensorData &data, std::optional<KisCubicCurve> curveOverride);

    qreal value(const KisPaintInformation& info) const override;
    bool isAbsoluteRotation() const override;

private:
    const bool m_lockedAngleMode;
    const bool m_angleOffset;
};

#endif // KISDYNAMICSENSORDRAWINGANGLE2_H
