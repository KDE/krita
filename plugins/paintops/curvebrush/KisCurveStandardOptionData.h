/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCURVESTANDARDOPTIONDATA_H
#define KISCURVESTANDARDOPTIONDATA_H

#include <KisCurveOptionData.h>

struct KisLineWidthOptionData : KisCurveOptionData
{
    KisLineWidthOptionData()
        : KisCurveOptionData(
              KoID("Line width", i18n("Line width")),
              Checkability::Checkable,
              std::nullopt,
              std::make_pair(0.1, 1.0))
    {}
};

struct KisCurvesOpacityOptionData : KisCurveOptionData
{
    KisCurvesOpacityOptionData()
        : KisCurveOptionData(
              KoID("Curves opacity", i18n("Curves opacity")),
              Checkability::Checkable,
              std::nullopt,
              std::make_pair(0.1, 1.0))
    {}
};

#endif // KISCURVESTANDARDOPTIONDATA_H
