/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "kis_pressure_rotation_option.h"
#include "kis_pressure_opacity_option.h"
#include "sensors/kis_dynamic_sensor_drawing_angle.h"
#include "sensors/kis_dynamic_sensor_fuzzy.h"
#include <klocalizedstring.h>
#include <kis_painter.h>
#include <brushengine/kis_paintop.h>
#include <KoColor.h>

KisPressureRotationOption::KisPressureRotationOption()
        : KisCurveOption("Rotation", KisPaintOpOption::GENERAL, false)
{
}

double KisPressureRotationOption::apply(const KisPaintInformation & info) const
{
    if (!isChecked()) return kisDegreesToRadians(info.canvasRotation());

    const bool absoluteAxesFlipped = info.canvasMirroredH() != info.canvasMirroredV();

    const qreal normalizedBaseAngle = -info.canvasRotation() / 360.0;

    // we should invert scaling part because it is expected
    // to rotate the brush counterclockwise
    const qreal scalingPartCoeff = -1.0;

    qreal value = computeRotationLikeValue(info, normalizedBaseAngle, absoluteAxesFlipped, scalingPartCoeff, info.isHoveringMode());

    /// flip to conform global legacy code
    /// we measure rotation in the opposite direction relative Qt's way
    value = 1.0 - value;

    return normalizeAngle(value * M_PI);
 }

void KisPressureRotationOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisCurveOption::readOptionSetting(setting);
}

void KisPressureRotationOption::applyFanCornersInfo(KisPaintOp *op)
{
    if (!this->isChecked()) {
        return;
    }

    KisDynamicSensorDrawingAngle *sensor = dynamic_cast<KisDynamicSensorDrawingAngle*>(this->sensor(ANGLE, true).data());

    /**
     * A special case for the Drawing Angle sensor, because it
     * changes the behavior of KisPaintOp::paintLine()
     */
    if (sensor) {
        op->setFanCornersInfo(sensor->fanCornersEnabled(), qreal(sensor->fanCornersStep()) * M_PI / 180.0);
    }
}

int KisPressureRotationOption::intMinValue() const
{
    return -180;
}

int KisPressureRotationOption::intMaxValue() const
{
    return 180;
}

QString KisPressureRotationOption::valueSuffix() const
{
    return i18n("°");
}
