/*
 *  SPDX-FileCopyrightText: 2010 José Luis Vergara <pentalis@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_hatching_pressure_crosshatching_option.h"
#include <kis_pressure_opacity_option.h>

#include <klocalizedstring.h>
#include <kis_painter.h>
#include <KoColor.h>

KisHatchingPressureCrosshatchingOption::KisHatchingPressureCrosshatchingOption()
    : KisCurveOption(KoID("Crosshatching", i18n("Crosshatching")), KisPaintOpOption::GENERAL, false)
{
}

double KisHatchingPressureCrosshatchingOption::apply(const KisPaintInformation & info) const
{
    if (!isChecked()) return 0.5;
    return computeSizeLikeValue(info);
}
