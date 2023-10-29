/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSmudgeRadiusOptionData.h"
#include <KisSmudgeLengthOptionData.h>

KisSmudgeRadiusOptionData::KisSmudgeRadiusOptionData()
    : KisCurveOptionData(
          KoID("SmudgeRadius", i18n("Smudge Radius")),
          Checkability::Checkable, std::nullopt,
          std::make_pair(0.0, 3.0))
{
    valueFixUpReadCallback = [] (KisCurveOptionDataCommon *data, const KisPropertiesConfiguration *setting) {
        const int smudgeRadiusVersion = setting->getInt("SmudgeRadiusVersion", 1);
        if (smudgeRadiusVersion < 2) {
            data->strengthValue = data->strengthValue / 100.0;
        }

        KisSmudgeLengthOptionMixInImpl lengthData;
        lengthData.read(setting);

        data->strengthMaxValue = lengthData.useNewEngine ? 1.0 : 3.0;
        data->strengthValue = qMin(data->strengthValue, data->strengthMaxValue);
    };

    valueFixUpWriteCallback = [] (qreal, KisPropertiesConfiguration *setting) {
        setting->setProperty("SmudgeRadiusVersion", 2);
    };
}
