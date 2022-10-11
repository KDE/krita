/*
 *  SPDX-FileCopyrightText: 2006-2007, 2010 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISDYNAMICSENSORS2_H
#define KISDYNAMICSENSORS2_H

#include "KisDynamicSensor2.h"
#include <brushengine/kis_paint_information.h>

class KisDynamicSensorSpeed2 : public KisDynamicSensor2
{
public:
    KisDynamicSensorSpeed2(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride);

    qreal value(const KisPaintInformation& info) const override {
        return info.drawingSpeed();
    }
};

class KisDynamicSensorRotation2 : public KisDynamicSensor2
{
public:
    KisDynamicSensorRotation2(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride);

    bool isAdditive() const override {
        return true;
    }

    qreal value(const KisPaintInformation& info) const override {
        return info.rotation() / 180.0;
    }
};

class KisDynamicSensorPressure2 : public KisDynamicSensor2
{
public:
    KisDynamicSensorPressure2(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride);
    qreal value(const KisPaintInformation& info) const override {
        return info.pressure();
    }
};

class KisDynamicSensorPressureIn2 : public KisDynamicSensor2
{
public:
    KisDynamicSensorPressureIn2(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride);

    qreal value(const KisPaintInformation& info) const override {
        return !info.isHoveringMode() ? info.maxPressure() : 0.0;
    }
};

class KisDynamicSensorXTilt2 : public KisDynamicSensor2
{
public:
    KisDynamicSensorXTilt2(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride);

    qreal value(const KisPaintInformation& info) const override {
        return 1.0 - fabs(info.xTilt()) / 60.0;
    }
};

class KisDynamicSensorYTilt2 : public KisDynamicSensor2
{
public:
    KisDynamicSensorYTilt2(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride);
    qreal value(const KisPaintInformation& info) const override {
        return 1.0 - fabs(info.yTilt()) / 60.0;
    }
};

class KisDynamicSensorTiltDirection2 : public KisDynamicSensor2
{
public:
    KisDynamicSensorTiltDirection2(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride);

    bool isAdditive() const override {
        return true;
    }

    qreal value(const KisPaintInformation& info) const override {
        return scalingToAdditive(KisPaintInformation::tiltDirection(info, true));
    }
};

class KisDynamicSensorTiltElevation2 : public KisDynamicSensor2
{
public:
    KisDynamicSensorTiltElevation2(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride);

    qreal value(const KisPaintInformation& info) const override {
        return KisPaintInformation::tiltElevation(info, 60.0, 60.0, true);
    }
};

class KisDynamicSensorPerspective2 : public KisDynamicSensor2
{
public:
    KisDynamicSensorPerspective2(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride);

    qreal value(const KisPaintInformation& info) const override {
        return info.perspective();
    }
};

class KisDynamicSensorTangentialPressure2 : public KisDynamicSensor2
{
public:
    KisDynamicSensorTangentialPressure2(const KisSensorData &data, std::optional<KisCubicCurve> curveOverride);

    qreal value(const KisPaintInformation& info) const override {
        return info.tangentialPressure();
    }
};

#endif // KISDYNAMICSENSORS2_H
