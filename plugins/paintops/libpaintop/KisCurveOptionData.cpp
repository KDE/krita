/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisCurveOptionData.h"

KisCurveOptionData::KisCurveOptionData(const QString &prefix,
                                       const KoID &id,
                                       bool isCheckable,
                                       bool isChecked,
                                       qreal minValue,
                                       qreal maxValue)
    : KisCurveOptionDataCommon(prefix,
                               id,
                               isCheckable,
                               isChecked,
                               minValue,
                               maxValue,
                               new KisKritaSensorPack())
{
}

KisCurveOptionData::KisCurveOptionData(const KoID &id,
                                       bool isCheckable,
                                       bool isChecked,
                                       qreal minValue,
                                       qreal maxValue)
    : KisCurveOptionData("", id, isCheckable, isChecked, minValue, maxValue)
{
}

KisKritaSensorData &KisCurveOptionData::sensorStruct()
{
    return dynamic_cast<KisKritaSensorPack *>(sensorData.data())->sensorsStruct();
}

const KisKritaSensorData &KisCurveOptionData::sensorStruct() const
{
    return dynamic_cast<const KisKritaSensorPack*>(sensorData.constData())->constSensorsStruct();
}
