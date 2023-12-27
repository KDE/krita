/*
 *  SPDX-FileCopyrightText: 2007, 2010 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisDynamicSensors.h"

#include <KisDynamicSensorIds.h>


KisDynamicSensorSpeed::KisDynamicSensorSpeed(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensor(SpeedId, data, curveOverride)
{
}

KisDynamicSensorRotation::KisDynamicSensorRotation(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensor(RotationId, data, curveOverride)
{
}


KisDynamicSensorPressure::KisDynamicSensorPressure(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensor(PressureId, data, curveOverride)
{
}

KisDynamicSensorPressureIn::KisDynamicSensorPressureIn(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensor(PressureInId, data, curveOverride)
{
}

KisDynamicSensorXTilt::KisDynamicSensorXTilt(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensor(XTiltId, data, curveOverride)
{
}

KisDynamicSensorYTilt::KisDynamicSensorYTilt(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensor(YTiltId, data, curveOverride)
{
}

KisDynamicSensorTiltDirection::KisDynamicSensorTiltDirection(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensor(TiltDirectionId, data, curveOverride)
{
}

KisDynamicSensorTiltElevation::KisDynamicSensorTiltElevation(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensor(TiltElevationId, data, curveOverride)
{
}

KisDynamicSensorPerspective::KisDynamicSensorPerspective(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensor(PerspectiveId, data, curveOverride)
{
}

KisDynamicSensorTangentialPressure::KisDynamicSensorTangentialPressure(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensor(TangentialPressureId, data, curveOverride)
{
}
