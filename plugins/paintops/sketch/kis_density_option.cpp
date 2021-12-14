/*
 *  SPDX-FileCopyrightText: 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_density_option.h"
#include <klocalizedstring.h>

KisDensityOption::KisDensityOption()
    : KisCurveOption(KoID("Density", i18n("Density")), KisPaintOpOption::GENERAL, false)
{
}


double KisDensityOption::apply(const KisPaintInformation & info, double probability) const
{
    if (!isChecked()) return probability;
    return computeSizeLikeValue(info) * probability;
}
