/*
 *  SPDX-FileCopyrightText: 2006-2007, 2010 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISDYNAMICSENSORS_H
#define KISDYNAMICSENSORS_H

#include "KisDynamicSensor.h"
#include <brushengine/kis_paint_information.h>

class KisDynamicSensorSpeed : public KisDynamicSensor
{
public:
    KisDynamicSensorSpeed(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride);

    qreal value(const KisPaintInformation& info) const override {
        return info.drawingSpeed();
    }
};

class KisDynamicSensorRotation : public KisDynamicSensor
{
public:
    KisDynamicSensorRotation(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride);

    bool isAdditive() const override {
        return true;
    }

    qreal value(const KisPaintInformation& info) const override {
        return info.rotation() / 180.0;
    }
};

class KisDynamicSensorPressure : public KisDynamicSensor
{
public:
    KisDynamicSensorPressure(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride);
    qreal value(const KisPaintInformation& info) const override {
        return info.pressure();
    }
};

class KisDynamicSensorPressureIn : public KisDynamicSensor
{
public:
    KisDynamicSensorPressureIn(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride);

    qreal value(const KisPaintInformation& info) const override {
        return !info.isHoveringMode() ? info.maxPressure() : 0.0;
    }
};

class KisDynamicSensorXTilt : public KisDynamicSensor
{
public:
    KisDynamicSensorXTilt(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride);

    qreal value(const KisPaintInformation& info) const override {
        return 1.0 - fabs(info.xTilt()) / 60.0;
    }
};

class KisDynamicSensorYTilt : public KisDynamicSensor
{
public:
    KisDynamicSensorYTilt(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride);
    qreal value(const KisPaintInformation& info) const override {
        return 1.0 - fabs(info.yTilt()) / 60.0;
    }
};

class KisDynamicSensorTiltDirection : public KisDynamicSensor
{
public:
    KisDynamicSensorTiltDirection(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride);

    bool isAdditive() const override {
        return true;
    }

    qreal value(const KisPaintInformation& info) const override {
        return scalingToAdditive(KisPaintInformation::tiltDirection(info, true));
    }
};

class KisDynamicSensorTiltElevation : public KisDynamicSensor
{
public:
    KisDynamicSensorTiltElevation(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride);

    qreal value(const KisPaintInformation& info) const override {
        return KisPaintInformation::tiltElevation(info, 60.0, 60.0, true);
    }
};

class KisDynamicSensorPerspective : public KisDynamicSensor
{
public:
    KisDynamicSensorPerspective(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride);

    qreal value(const KisPaintInformation& info) const override {
        return info.perspective();
    }
};

class KisDynamicSensorTangentialPressure : public KisDynamicSensor
{
public:
    KisDynamicSensorTangentialPressure(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride);

    qreal value(const KisPaintInformation& info) const override {
        return info.tangentialPressure();
    }
};

#endif // KISDYNAMICSENSORS_H
