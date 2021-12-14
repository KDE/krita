/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "kis_pressure_softness_option.h"
#include <klocalizedstring.h>

KisPressureSoftnessOption::KisPressureSoftnessOption()
    : KisCurveOption(KoID("Softness", i18n("Softness")), KisPaintOpOption::GENERAL, false, 1.0, 0.1, 1.0)
{
}


double KisPressureSoftnessOption::apply(const KisPaintInformation & info) const
{
    if (!isChecked()) return 1.0;
    return computeSizeLikeValue(info);
}
