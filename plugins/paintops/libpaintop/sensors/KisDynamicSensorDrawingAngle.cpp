/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisDynamicSensorDrawingAngle.h"

#include <KisDynamicSensorIds.h>

#include <kis_paint_information.h>
#include <KisCurveOptionData.h>

KisDynamicSensorDrawingAngle::KisDynamicSensorDrawingAngle(const KisDrawingAngleSensorData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensor(DrawingAngleId, data, curveOverride)
    , m_lockedAngleMode(data.lockedAngleMode)
    , m_angleOffset(data.angleOffset)
{
}

qreal KisDynamicSensorDrawingAngle::value(const KisPaintInformation &info) const
{
    /* so that we are in 0.0..1.0 */
    qreal ret = 0.5 + info.drawingAngle(m_lockedAngleMode) / (2.0 * M_PI) + m_angleOffset / 360.0;

    // check if m_angleOffset pushed us out of bounds
    if (ret > 1.0)
        ret -= 1.0;

    return ret;
}

bool KisDynamicSensorDrawingAngle::isAbsoluteRotation() const
{
    return true;
}
