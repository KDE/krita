/* This file is part of the KDE project
 * Copyright (C) Nishant Rodrigues <nishantjr@gmail.com>, (C) 2016
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_pressure_ratio_option.h"

KisPressureRatioOption::KisPressureRatioOption()
    : KisCurveOption("Ratio", KisPaintOpOption::GENERAL, true)
{
}

double KisPressureRatioOption::apply(const KisPaintInformation & info) const
{
    if (!isChecked()) return 1.0;
    return computeSizeLikeValue(info);
}

