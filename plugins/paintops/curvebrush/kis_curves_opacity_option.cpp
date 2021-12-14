/*
 *  SPDX-FileCopyrightText: 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_curves_opacity_option.h"
#include <klocalizedstring.h>

KisCurvesOpacityOption::KisCurvesOpacityOption()
    : KisCurveOption(KoID("Curves opacity", i18n("Curves opacity")), KisPaintOpOption::GENERAL, false)
{
}


qreal KisCurvesOpacityOption::apply(const KisPaintInformation & info, qreal opacity) const
{
    if (!isChecked()) return opacity;
    return computeSizeLikeValue(info) * opacity;
}
