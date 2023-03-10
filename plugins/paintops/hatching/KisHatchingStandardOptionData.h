/*
 *  SPDX-FileCopyrightText: 2010 José Luis Vergara <pentalis@gmail.com>
 *  SPDX-FileCopyrightText: 2018 Idiomdrottning <sandra.snan@idiomdrottning.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISHATCHINGSTANDARDOPTIONDATA_H
#define KISHATCHINGSTANDARDOPTIONDATA_H

#include <KisCurveOptionData.h>

struct KisAngleOptionData : KisCurveOptionData
{
    KisAngleOptionData()
        : KisCurveOptionData(
              KoID("Angle", i18n("Angle")),
              true, false,
              0.0, 1.0)
    {}
};

struct KisCrosshatchingOptionData : KisCurveOptionData
{
    KisCrosshatchingOptionData()
        : KisCurveOptionData(
              KoID("Crosshatching", i18n("Crosshatching")),
              true, false,
              0.0, 1.0)
    {}
};

struct KisSeparationOptionData : KisCurveOptionData
{
    KisSeparationOptionData()
        : KisCurveOptionData(
              KoID("Separation", i18n("Separation")),
              true, true,
              0.0, 1.0)
    {}
};

struct KisThicknessOptionData : KisCurveOptionData
{
    KisThicknessOptionData()
        : KisCurveOptionData(
              KoID("Thickness", i18n("Thickness")),
              true, false,
              0.0, 1.0)
    {}
};

#endif // KISHATCHINGSTANDARDOPTIONDATA_H
