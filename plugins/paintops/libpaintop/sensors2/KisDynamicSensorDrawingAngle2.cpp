/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisDynamicSensorDrawingAngle2.h"

// TODO: remove!
#include <kis_dynamic_sensor.h>

#include <kis_paint_information.h>
#include <KisCurveOptionData.h>

KisDynamicSensorDrawingAngle2::KisDynamicSensorDrawingAngle2(const KisDrawingAngleSensorData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensor2(DrawingAngleId, data, curveOverride)
    , m_lockedAngleMode(data.lockedAngleMode)
    , m_angleOffset(data.angleOffset)
{
}

qreal KisDynamicSensorDrawingAngle2::value(const KisPaintInformation &info) const
{
    /* so that we are in 0.0..1.0 */
    qreal ret = 0.5 + info.drawingAngle(m_lockedAngleMode) / (2.0 * M_PI) + m_angleOffset / 360.0;

    // check if m_angleOffset pushed us out of bounds
    if (ret > 1.0)
        ret -= 1.0;

    return ret;
}

bool KisDynamicSensorDrawingAngle2::isAbsoluteRotation() const
{
    return true;
}
