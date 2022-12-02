/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISCURVEOPTIONDATA_H
#define KISCURVEOPTIONDATA_H

#include <KisCurveOptionDataCommon.h>
#include <KisKritaSensorPack.h>


struct PAINTOP_EXPORT KisCurveOptionData : KisCurveOptionDataCommon
{
    KisCurveOptionData(const QString &prefix,
                       const KoID &id,
                       bool isCheckable = true,
                       bool isChecked = false,
                       bool separateCurveValue = false,
                       qreal minValue = 0.0,
                       qreal maxValue = 1.0);

    KisCurveOptionData(const KoID &id,
                       bool isCheckable = true,
                       bool isChecked = false,
                       bool separateCurveValue = false,
                       qreal minValue = 0.0,
                       qreal maxValue = 1.0);
        
    KisKritaSensorData& sensorStruct();
    const KisKritaSensorData& sensorStruct() const;
};

#endif // KISCURVEOPTIONDATA_H