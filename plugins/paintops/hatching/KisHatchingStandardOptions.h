/*
 *  SPDX-FileCopyrightText: 2010 José Luis Vergara <pentalis@gmail.com>
 *  SPDX-FileCopyrightText: 2018 Idiomdrottning <sandra.snan@idiomdrottning.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISHATCHINGSTANDARDOPTIONS_H
#define KISHATCHINGSTANDARDOPTIONS_H

#include <KisHatchingStandardOptionData.h>
#include <KisStandardOptions.h>

using KisAngleOption = KisStandardOption<KisAngleOptionData>;
using KisCrosshatchingOption = KisStandardOption<KisCrosshatchingOptionData>;
using KisSeparationOption = KisStandardOption<KisSeparationOptionData>;
using KisThicknessOption = KisStandardOption<KisThicknessOptionData>;

template <>
inline qreal KisAngleOption::apply(const KisPaintInformation & info) const
{
    if (!isChecked()) return 0.5;
    return computeSizeLikeValue(info);
}
template <>
inline qreal KisCrosshatchingOption::apply(const KisPaintInformation & info) const
{
    if (!isChecked()) return 0.5;
    return computeSizeLikeValue(info);
}
template <>
inline qreal KisSeparationOption::apply(const KisPaintInformation & info) const
{
    if (!isChecked()) return 0.5;
    return computeSizeLikeValue(info);
}
template <>
inline qreal KisThicknessOption::apply(const KisPaintInformation & info) const
{
    if (!isChecked()) return 0.5;
    return computeSizeLikeValue(info);
}

#endif // KISHATCHINGSTANDARDOPTIONS_H
