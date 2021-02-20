/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_pressure_texture_strength_option.h"

#include <klocalizedstring.h>

KisPressureTextureStrengthOption::KisPressureTextureStrengthOption()
    : KisCurveOption("Texture/Strength/", KisPaintOpOption::TEXTURE, false)
{
}

double KisPressureTextureStrengthOption::apply(const KisPaintInformation & info) const
{
    if (!isChecked()) return 1.0;
    return computeSizeLikeValue(info);
}
