/* This file is part of the KDE project
 * Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
        : KisCurveOption("Rotation", KisPaintOpOption::GENERAL, false),
          m_defaultAngle(0.0),
          m_canvasAxisXMirrored(false),
          m_canvasAxisYMirrored(false)
{
}

double KisPressureRotationOption::apply(const KisPaintInformation & info) const
{
    if (!isChecked()) return m_defaultAngle;

    const bool absoluteAxesFlipped = m_canvasAxisXMirrored != m_canvasAxisYMirrored;
    const qreal normalizedBaseAngle = -m_defaultAngle / (2 * M_PI);

    qreal value = computeRotationLikeValue(info, normalizedBaseAngle, absoluteAxesFlipped);

    // flip to conform global legacy code
    value = 1.0 - value;

    return normalizeAngle(value * M_PI);
 }

void KisPressureRotationOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    m_defaultAngle = setting->getDouble("runtimeCanvasRotation", 0.0) * M_PI / 180.0;
    KisCurveOption::readOptionSetting(setting);

    m_canvasAxisXMirrored = setting->getBool("runtimeCanvasMirroredX", false);
    m_canvasAxisYMirrored = setting->getBool("runtimeCanvasMirroredY", false);
}

void KisPressureRotationOption::applyFanCornersInfo(KisPaintOp *op)
{
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
