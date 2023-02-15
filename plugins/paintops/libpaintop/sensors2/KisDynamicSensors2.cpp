/*
 *  SPDX-FileCopyrightText: 2007, 2010 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisDynamicSensors2.h"

#include <KisDynamicSensorIds.h>


KisDynamicSensorSpeed2::KisDynamicSensorSpeed2(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensor2(SpeedId, data, curveOverride)
{
}

KisDynamicSensorRotation2::KisDynamicSensorRotation2(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensor2(RotationId, data, curveOverride)
{
}


KisDynamicSensorPressure2::KisDynamicSensorPressure2(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensor2(PressureId, data, curveOverride)
{
}

KisDynamicSensorPressureIn2::KisDynamicSensorPressureIn2(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensor2(PressureInId, data, curveOverride)
{
}

KisDynamicSensorXTilt2::KisDynamicSensorXTilt2(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensor2(XTiltId, data, curveOverride)
{
}

KisDynamicSensorYTilt2::KisDynamicSensorYTilt2(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensor2(YTiltId, data, curveOverride)
{
}

KisDynamicSensorTiltDirection2::KisDynamicSensorTiltDirection2(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensor2(TiltDirectionId, data, curveOverride)
{
}

KisDynamicSensorTiltElevation2::KisDynamicSensorTiltElevation2(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensor2(TiltElevationId, data, curveOverride)
{
}

KisDynamicSensorPerspective2::KisDynamicSensorPerspective2(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensor2(PerspectiveId, data, curveOverride)
{
}

KisDynamicSensorTangentialPressure2::KisDynamicSensorTangentialPressure2(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride)
    : KisDynamicSensor2(TangentialPressureId, data, curveOverride)
{
}
