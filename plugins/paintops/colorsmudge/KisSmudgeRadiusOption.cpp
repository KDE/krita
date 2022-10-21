/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSmudgeRadiusOption.h"

#include <KisColorSmudgeStandardOptionData.h>
#include <KisSmudgeLengthOptionData.h>


KisSmudgeRadiusOption2::KisSmudgeRadiusOption2(const KisPropertiesConfiguration *setting)
    : KisCurveOption2(initializeData(setting))
{
}

KisSmudgeRadiusOptionData KisSmudgeRadiusOption2::initializeData(const KisPropertiesConfiguration *setting) {
    KisSmudgeLengthOptionData lengthData;
    lengthData.read(setting);

    KisSmudgeRadiusOptionData data;
    data.read(setting);

    data.strengthMaxValue = lengthData.useNewEngine ? 1.0 : 3.0;
    data.strengthValue = qBound(data.strengthMinValue, data.strengthValue, data.strengthMaxValue);

    return data;
}
