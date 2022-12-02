/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef MYPAINTSENSORPACK_H
#define MYPAINTSENSORPACK_H

#include <boost/operators.hpp>

#include <KisSensorData.h>
#include <KisSensorPackInterface.h>

const KoID MyPaintPressureId("pressure", ki18n("Pressure"));
const KoID MyPaintFineSpeedId("speed1", ki18n("Fine Speed"));
const KoID MyPaintGrossSpeedId("speed2", ki18n("Gross Speed"));
const KoID MyPaintRandomId("random", ki18n("Random"));
const KoID MyPaintStrokeId("stroke", ki18nc("The duration of a brush stroke", "Stroke"));
const KoID MyPaintDirectionId("direction", ki18nc("Drawing Angle", "Direction"));
const KoID MyPaintDeclinationId("tilt_declination", ki18nc("Pen tilt declination", "Declination"));
const KoID MyPaintAscensionId("tilt_ascension", ki18nc("Pen tilt ascension", "Ascension"));
const KoID MyPaintCustomId("custom", ki18n("Custom"));

struct MyPaintSensorData : boost::equality_comparable<MyPaintSensorData>
{
    MyPaintSensorData();

    inline friend bool operator==(const MyPaintSensorData &lhs, const MyPaintSensorData &rhs) {   
        return lhs.sensorPressure == rhs.sensorPressure &&
            lhs.sensorFineSpeed == rhs.sensorFineSpeed &&
            lhs.sensorGrossSpeed == rhs.sensorGrossSpeed &&
            lhs.sensorRandom == rhs.sensorRandom &&
            lhs.sensorStroke == rhs.sensorStroke &&
            lhs.sensorDirection == rhs.sensorDirection &&
            lhs.sensorDeclination == rhs.sensorDeclination &&
            lhs.sensorAscension == rhs.sensorAscension &&
            lhs.sensorCustom == rhs.sensorCustom;
    }

    KisSensorData sensorPressure;
    KisSensorData sensorFineSpeed;
    KisSensorData sensorGrossSpeed;
    KisSensorData sensorRandom;
    KisSensorData sensorStroke;
    KisSensorData sensorDirection;
    KisSensorData sensorDeclination;
    KisSensorData sensorAscension;
    KisSensorData sensorCustom;
};

class MyPaintSensorPack : public KisSensorPackInterface
{
public:
    MyPaintSensorPack() = default;
    MyPaintSensorPack(const MyPaintSensorPack &rhs) = default;

    KisSensorPackInterface * clone() const override;
    
    std::vector<const KisSensorData *> constSensors() const override;
    std::vector<KisSensorData *> sensors() override;

    const MyPaintSensorData& constSensorsStruct() const;
    MyPaintSensorData& sensorsStruct();

    bool compare(const KisSensorPackInterface *rhs) const override;
    bool read(KisCurveOptionDataCommon &data, const KisPropertiesConfiguration *setting) const override;
    void write(const KisCurveOptionDataCommon &data, KisPropertiesConfiguration *setting) const override;


private:
    MyPaintSensorData m_data;
};

#endif // MYPAINTSENSORPACK_H