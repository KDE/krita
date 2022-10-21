/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCOLORSMUDGESTANDARDOPTIONDATA_H
#define KISCOLORSMUDGESTANDARDOPTIONDATA_H

#include <KisCurveOptionData.h>

struct KisColorRateOptionData : KisCurveOptionData
{
    KisColorRateOptionData()
        : KisCurveOptionData(
              KoID("ColorRate", i18nc("Color rate of active Foreground color", "Color Rate")))
    {}
};

struct KisGradientOptionData : KisCurveOptionData
{
    KisGradientOptionData()
        : KisCurveOptionData(
              KoID("Gradient", i18n("Gradient")))
    {}
};

struct KisSmudgeRadiusOptionData : KisCurveOptionData
{
    KisSmudgeRadiusOptionData()
        : KisCurveOptionData(
              KoID("SmudgeRadius", i18n("Smudge Radius")))
    {
        valueFixUpReadCallback = [] (qreal value, const KisPropertiesConfiguration *setting) {
            const int smudgeRadiusVersion = setting->getInt("SmudgeRadiusVersion", 1);
            if (smudgeRadiusVersion < 2) {
                value = value / 100.0;
            }
            return value;
        };

        valueFixUpWriteCallback = [] (qreal, KisPropertiesConfiguration *setting) {
            setting->setProperty("SmudgeRadiusVersion", 2);
        };
    }
};

#endif // KISCOLORSMUDGESTANDARDOPTIONDATA_H
