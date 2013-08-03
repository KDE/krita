/*
 *  Copyright (c) 2007,2010 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_dynamic_sensors.h"
KisDynamicSensorFuzzy::KisDynamicSensorFuzzy() : KisDynamicSensor(FuzzyId)
{
    setMinimumLabel(QString()); // TODO use dice icon
    setMaximumLabel(QString());
}

KisDynamicSensorSpeed::KisDynamicSensorSpeed() : KisDynamicSensor(SpeedId)
{
    setMinimumLabel(i18n("Slow"));
    setMaximumLabel(i18n("Fast"));
}

qreal KisDynamicSensorSpeed::value(const KisPaintInformation& info) {
    /**
     * The value of maximum speed was measured empirically. This is
     * the speed that is quite easy to get with an A6 tablet and quite
     * a big image. If you need smaller speeds, just change the curve.
     */
    const qreal maxSpeed = 30.0; // px / ms
    const qreal blendExponent = 0.05;

    qreal currentSpeed = info.drawingSpeed() / maxSpeed;
    m_speed = qMin(1.0, (m_speed * (1 - blendExponent) +
                         currentSpeed * blendExponent));
    return m_speed;
}

KisDynamicSensorDrawingAngle::KisDynamicSensorDrawingAngle() : KisDynamicSensor(DrawingAngleId)
{
    setMinimumLabel(i18n("0°"));
    setMaximumLabel(i18n("360°"));
}

qreal KisDynamicSensorDrawingAngle::value(const KisPaintInformation& info)
{
    /* so that we are in 0.0..1.0 */
    return 0.5 + info.drawingAngle() / (2.0 * M_PI);
}

bool KisDynamicSensorDrawingAngle::dependsOnCanvasRotation() const
{
    return false;
}

KisDynamicSensorRotation::KisDynamicSensorRotation() : KisDynamicSensor(RotationId)
{
    setMinimumLabel(i18n("0°"));
    setMaximumLabel(i18n("360°"));
}

KisDynamicSensorPressure::KisDynamicSensorPressure() : KisDynamicSensor(PressureId)
{
    setMinimumLabel(i18n("Low"));
    setMaximumLabel(i18n("High"));
}

KisDynamicSensorXTilt::KisDynamicSensorXTilt() : KisDynamicSensor(XTiltId)
{
    setMinimumLabel(i18n("-30°"));
    setMaximumLabel(i18n("30°"));
}

KisDynamicSensorYTilt::KisDynamicSensorYTilt() : KisDynamicSensor(YTiltId)
{
    setMinimumLabel(i18n("-30°"));
    setMaximumLabel(i18n("30°"));
}

KisDynamicSensorAscension::KisDynamicSensorAscension() : KisDynamicSensor(AscensionId)
{
    setMinimumLabel(i18n("0°"));
    setMaximumLabel(i18n("360°"));
}

KisDynamicSensorDeclination::KisDynamicSensorDeclination() : KisDynamicSensor(DeclinationId)
{
    setMinimumLabel(i18n("90°"));
    setMaximumLabel(i18n("0°"));
}


KisDynamicSensorPerspective::KisDynamicSensorPerspective() : KisDynamicSensor(PerspectiveId)
{
    setMinimumLabel(i18n("Far"));
    setMaximumLabel(i18n("Near"));
}

KisDynamicSensorTangentialPressure::KisDynamicSensorTangentialPressure() : KisDynamicSensor(TangentialPressureId)
{
    setMinimumLabel(i18n("Low"));
    setMaximumLabel(i18n("High"));
}

