/* This file is part of the KDE project
 * Copyright (C)Peter Schatz <voronwe13@gmail.com>, (C) 2020
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "kis_pressure_lightness_strength_option.h"

KisPressureLightnessStrengthOption::KisPressureLightnessStrengthOption()
    : KisCurveOption("LightnessStrength", KisPaintOpOption::GENERAL, false)
{
}

double KisPressureLightnessStrengthOption::apply(const KisPaintInformation& info) const
{
    if (!isChecked()) return 1.0;
    return computeSizeLikeValue(info);
}