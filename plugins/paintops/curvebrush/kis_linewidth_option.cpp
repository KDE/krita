/*
 *  SPDX-FileCopyrightText: 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_linewidth_option.h"
#include "kis_paintop_option.h"

KisLineWidthOption::KisLineWidthOption()
    : KisCurveOption(KoID("Line width", i18n("Line width")), KisPaintOpOption::GENERAL, false)
{
}


double KisLineWidthOption::apply(const KisPaintInformation & info, double lineWidth) const
{
    if (!isChecked()) return lineWidth;
    return computeSizeLikeValue(info) * lineWidth;
}
