/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "kis_pressure_rate_option.h"

KisPressureRateOption::KisPressureRateOption()
    : KisCurveOption(KoID("Rate", i18n("Rate")), KisPaintOpOption::COLOR, false)
{
}

double KisPressureRateOption::apply(const KisPaintInformation &info) const
{
    return isChecked() ? computeSizeLikeValue(info) : 1.0;
}
