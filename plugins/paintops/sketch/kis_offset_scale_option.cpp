/*
 *  SPDX-FileCopyrightText: 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_offset_scale_option.h"
#include <klocalizedstring.h>

KisOffsetScaleOption::KisOffsetScaleOption()
    : KisCurveOption("Offset scale", KisPaintOpOption::GENERAL, false)
{
}


double KisOffsetScaleOption::apply(const KisPaintInformation & info, double offsetScale) const
{
    if (!isChecked()) return offsetScale;
    return computeSizeLikeValue(info) * offsetScale;
}
