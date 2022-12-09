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

const QString MYPAINT_JSON = "MyPaint/json";

const KoID MyPaintPressureId("mypaint_pressure", ki18n("Pressure"));
const KoID MyPaintFineSpeedId("mypaint_speed1", ki18n("Fine Speed"));
const KoID MyPaintGrossSpeedId("mypaint_speed2", ki18n("Gross Speed"));
const KoID MyPaintRandomId("mypaint_random", ki18n("Random"));
const KoID MyPaintStrokeId("mypaint_stroke", ki18nc("The duration of a brush stroke", "Stroke"));
const KoID MyPaintDirectionId("mypaint_direction", ki18nc("Drawing Angle", "Direction"));
const KoID MyPaintDeclinationId("mypaint_tilt_declination", ki18nc("Pen tilt declination", "Declination"));
const KoID MyPaintAscensionId("mypaint_tilt_ascension", ki18nc("Pen tilt ascension", "Ascension"));
const KoID MyPaintCustomId("mypaint_custom", ki18n("Custom"));

struct MyPaintSensorDataWithRange : KisSensorData, boost::equality_comparable<MyPaintSensorDataWithRange>
{
    MyPaintSensorDataWithRange(const KoID &id);

    inline friend bool operator==(const MyPaintSensorDataWithRange &lhs, const MyPaintSensorDataWithRange &rhs) {
        return lhs.curveRange == rhs.curveRange &&
            static_cast<const KisSensorData&>(lhs) == static_cast<const KisSensorData&>(rhs);
    }

    QRectF baseCurveRange() const override;
    void setBaseCurveRange(const QRectF &rect) override;
    void reset() override;

    void reshapeCurve();

    QRectF curveRange {0,-1,1,2};
};

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

    MyPaintSensorDataWithRange sensorPressure;
    MyPaintSensorDataWithRange sensorFineSpeed;
    MyPaintSensorDataWithRange sensorGrossSpeed;
    MyPaintSensorDataWithRange sensorRandom;
    MyPaintSensorDataWithRange sensorStroke;
    MyPaintSensorDataWithRange sensorDirection;
    MyPaintSensorDataWithRange sensorDeclination;
    MyPaintSensorDataWithRange sensorAscension;
    MyPaintSensorDataWithRange sensorCustom;
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
