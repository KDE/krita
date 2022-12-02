/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISKRITASENSORPACK_H
#define KISKRITASENSORPACK_H

#include "kritapaintop_export.h"
#include <boost/operators.hpp>

#include <KisSensorData.h>
#include <KisSensorPackInterface.h>


struct PAINTOP_EXPORT KisKritaSensorData : boost::equality_comparable<KisKritaSensorData>
{
    KisKritaSensorData();
    
    inline friend bool operator==(const KisKritaSensorData &lhs, const KisKritaSensorData &rhs) {   
        return lhs.sensorPressure == rhs.sensorPressure &&
            lhs.sensorPressureIn == rhs.sensorPressureIn &&
            lhs.sensorXTilt == rhs.sensorXTilt &&
            lhs.sensorYTilt == rhs.sensorYTilt &&
            lhs.sensorTiltDirection == rhs.sensorTiltDirection &&
            lhs.sensorTiltElevation == rhs.sensorTiltElevation &&
            lhs.sensorSpeed == rhs.sensorSpeed &&
            lhs.sensorDrawingAngle == rhs.sensorDrawingAngle &&
            lhs.sensorRotation == rhs.sensorRotation &&
            lhs.sensorDistance == rhs.sensorDistance &&
            lhs.sensorTime == rhs.sensorTime &&
            lhs.sensorFuzzyPerDab == rhs.sensorFuzzyPerDab &&
            lhs.sensorFuzzyPerStroke == rhs.sensorFuzzyPerStroke &&
            lhs.sensorFade == rhs.sensorFade &&
            lhs.sensorPerspective == rhs.sensorPerspective &&
            lhs.sensorTangentialPressure == rhs.sensorTangentialPressure;   
    }

    KisSensorData sensorPressure;
    KisSensorData sensorPressureIn;
    KisSensorData sensorXTilt;
    KisSensorData sensorYTilt;
    KisSensorData sensorTiltDirection;
    KisSensorData sensorTiltElevation;
    KisSensorData sensorSpeed;
    KisDrawingAngleSensorData sensorDrawingAngle;
    KisSensorData sensorRotation;
    KisSensorWithLengthData sensorDistance;
    KisSensorWithLengthData sensorTime;
    KisSensorData sensorFuzzyPerDab;
    KisSensorData sensorFuzzyPerStroke;
    KisSensorWithLengthData sensorFade;
    KisSensorData sensorPerspective;
    KisSensorData sensorTangentialPressure;
};

class PAINTOP_EXPORT KisKritaSensorPack : public KisSensorPackInterface
{
public:
    KisKritaSensorPack() = default;
    KisKritaSensorPack(const KisKritaSensorPack &rhs) = default;

    KisSensorPackInterface * clone() const override;
    
    std::vector<const KisSensorData *> constSensors() const override;
    std::vector<KisSensorData *> sensors() override;

    const KisKritaSensorData& constSensorsStruct() const;
    KisKritaSensorData& sensorsStruct();

    bool compare(const KisSensorPackInterface *rhs) const override;
    bool read(KisCurveOptionDataCommon &data, const KisPropertiesConfiguration *setting) const override;
    void write(const KisCurveOptionDataCommon &data, KisPropertiesConfiguration *setting) const override;

    int calcActiveSensorLength(const QString &activeSensorId) const override;

private:
    KisKritaSensorData m_data;
};

#endif // KISKRITASENSORPACK_H