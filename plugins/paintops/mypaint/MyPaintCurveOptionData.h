/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef MYPAINTCURVEOPTIONDATA_H
#define MYPAINTCURVEOPTIONDATA_H

#include <KisCurveOptionDataCommon.h>
#include "MyPaintSensorPack.h"


struct MyPaintCurveOptionData : KisCurveOptionDataCommon
{
    MyPaintCurveOptionData(const QString &prefix,
                              const KoID &id,
                              bool isCheckable = true,
                              bool isChecked = false,
                              qreal minValue = 0.0,
                              qreal maxValue = 1.0);
        
    MyPaintCurveOptionData(const KoID &id,
                              bool isCheckable = true,
                              bool isChecked = false,
                              qreal minValue = 0.0,
                              qreal maxValue = 1.0);

protected:
    MyPaintCurveOptionData(const QString &prefix,
                           const KoID &id,
                           bool isCheckable,
                           bool isChecked,
                           qreal minValue,
                           qreal maxValue,
                           MyPaintSensorPack *sensorInterface);
public:

    MyPaintSensorData& sensorStruct();
    const MyPaintSensorData& sensorStruct() const;
};

#endif // MYPAINTCURVEOPTIONDATA_H
