/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "MyPaintSensorPack.h"

#include <KisCppQuirks.h>
#include "kis_assert.h"

namespace detail {

template <typename Data,
          typename SensorData =
              std::add_const_if_t<std::is_const_v<Data>,
                                  KisSensorData>>
inline std::vector<SensorData*> sensors(Data *data)
{
    std::vector<SensorData*> result;

    result.reserve(9);

    result.push_back(&data->sensorPressure);
    result.push_back(&data->sensorFineSpeed);
    result.push_back(&data->sensorGrossSpeed);
    result.push_back(&data->sensorRandom);
    result.push_back(&data->sensorStroke);
    result.push_back(&data->sensorDirection);
    result.push_back(&data->sensorDeclination);
    result.push_back(&data->sensorAscension);
    result.push_back(&data->sensorCustom);

    return result;
}

} // namespace detail

MyPaintSensorData::MyPaintSensorData()
    : sensorPressure(MyPaintPressureId),
      sensorFineSpeed(MyPaintFineSpeedId),
      sensorGrossSpeed(MyPaintGrossSpeedId),
      sensorRandom(MyPaintRandomId),
      sensorStroke(MyPaintStrokeId),
      sensorDirection(MyPaintDirectionId),
      sensorDeclination(MyPaintDeclinationId),
      sensorAscension(MyPaintAscensionId),
      sensorCustom(MyPaintCustomId)
{
    sensorPressure.isActive = true;
}

KisSensorPackInterface * MyPaintSensorPack::clone() const
{
    return new MyPaintSensorPack(*this);
}

std::vector<const KisSensorData *> MyPaintSensorPack::constSensors() const
{
    return detail::sensors(&m_data);
}

std::vector<KisSensorData *> MyPaintSensorPack::sensors()
{
    return detail::sensors(&m_data);
}

const MyPaintSensorData& MyPaintSensorPack::constSensorsStruct() const 
{
    return m_data;
}

MyPaintSensorData& MyPaintSensorPack::sensorsStruct()
{
    return m_data;
}

bool MyPaintSensorPack::compare(const KisSensorPackInterface *rhs) const
{
    const MyPaintSensorPack *pack = dynamic_cast<const MyPaintSensorPack*>(rhs);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(pack, false);

    return m_data == pack->m_data;
}

bool MyPaintSensorPack::read(KisCurveOptionDataCommon &data, const KisPropertiesConfiguration *setting) const
{
    // TODO:
    return true;
}

void MyPaintSensorPack::write(const KisCurveOptionDataCommon &data, KisPropertiesConfiguration *setting) const
{
    // TODO:
}