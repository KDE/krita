
/*
*  SPDX-FileCopyrightText: 2010 José Luis Vergara <pentalis@gmail.com>
*
*  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kis_hatching_pressure_thickness_option.h"
#include <kis_pressure_opacity_option.h>

#include <klocalizedstring.h>
#include <kis_painter.h>
#include <KoColor.h>


KisHatchingPressureThicknessOption::KisHatchingPressureThicknessOption()
    : KisCurveOption("Thickness", KisPaintOpOption::GENERAL, false)
{
}

double KisHatchingPressureThicknessOption::apply(const KisPaintInformation & info) const
{
    if (!isChecked()) return 0.5;
    return computeSizeLikeValue(info);
}
