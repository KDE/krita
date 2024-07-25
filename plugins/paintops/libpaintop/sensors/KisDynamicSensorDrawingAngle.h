/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISDYNAMICSENSORDRAWINGANGLE_H
#define KISDYNAMICSENSORDRAWINGANGLE_H

#include "KisDynamicSensor.h"

struct KisDrawingAngleSensorData;

class KisDynamicSensorDrawingAngle : public KisDynamicSensor
{
public:
    KisDynamicSensorDrawingAngle(const KisDrawingAngleSensorData &data, std::optional<KisCubicCurve> curveOverride);

    qreal value(const KisPaintInformation& info) const override;
    bool isAbsoluteRotation() const override;

private:
    const bool m_lockedAngleMode;
    const int m_angleOffset;
};

#endif // KISDYNAMICSENSORDRAWINGANGLE_H
