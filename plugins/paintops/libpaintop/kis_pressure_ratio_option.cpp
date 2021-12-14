/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2016 Nishant Rodrigues <nishantjr@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_pressure_ratio_option.h"

KisPressureRatioOption::KisPressureRatioOption()
    : KisCurveOption(KoID("Ratio", i18n("Ratio")), KisPaintOpOption::GENERAL, true)
{
}

double KisPressureRatioOption::apply(const KisPaintInformation & info) const
{
    if (!isChecked()) return 1.0;
    return computeSizeLikeValue(info);
}

