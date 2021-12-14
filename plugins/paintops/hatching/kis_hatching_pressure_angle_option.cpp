/*  By Idiomdrottning <sandra.snan@idiomdrottning.org> 2018, after a file that
 *  was SPDX-FileCopyrightText: 2010 José Luis Vergara <pentalis@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_hatching_pressure_angle_option.h"
#include <kis_pressure_opacity_option.h>

#include <klocalizedstring.h>
#include <kis_painter.h>
#include <KoColor.h>

KisHatchingPressureAngleOption::KisHatchingPressureAngleOption()
    : KisCurveOption(KoID("Angle", i18n("Angle")), KisPaintOpOption::GENERAL, false)
{
}

double KisHatchingPressureAngleOption::apply(const KisPaintInformation & info) const
{
    if (!isChecked()) return 0.5;
    return computeSizeLikeValue(info);
}
