/*
 *  SPDX-FileCopyrightText: 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_linewidth_option.h"

KisLineWidthOption::KisLineWidthOption()
    : KisCurveOption("Line width", KisPaintOpOption::GENERAL, false)
{
}


double KisLineWidthOption::apply(const KisPaintInformation & info, double lineWidth) const
{
    if (!isChecked()) return lineWidth;
    return computeSizeLikeValue(info) * lineWidth;
}
