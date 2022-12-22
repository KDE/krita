/*
 *  SPDX-FileCopyrightText: 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSKETCHSTANDARDOPTIONDATA_H
#define KISSKETCHSTANDARDOPTIONDATA_H

#include <KisCurveOptionData.h>

struct KisOffsetScaleOptionData : KisCurveOptionData
{
    KisOffsetScaleOptionData()
        : KisCurveOptionData(
              KoID("Offset scale", i18n("Offset scale")),
              true, false, false,
              0.0, 1.0)
    {}
};

struct KisLineWidthOptionData : KisCurveOptionData
{
    KisLineWidthOptionData()
        : KisCurveOptionData(
              KoID("Line width", i18n("Line width")),
              true, false, false,
              0.0, 1.0)
    {}
};

struct KisDensityOptionData : KisCurveOptionData
{
    KisDensityOptionData()
        : KisCurveOptionData(
              KoID("Density", i18n("Density")),
              true, false, false,
              0.0, 1.0)
    {}
};

#endif // KISSKETCHSTANDARDOPTIONDATA_H
