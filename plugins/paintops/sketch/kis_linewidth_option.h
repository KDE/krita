/*
 *  SPDX-FileCopyrightText: 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_LINEWIDTH_OPTION_H_
#define KIS_LINEWIDTH_OPTION_H_

#include "kis_curve_option.h"
#include <brushengine/kis_paint_information.h>

class KisLineWidthOption : public KisCurveOption
{
public:
    KisLineWidthOption();
    double apply(const KisPaintInformation & info, double lineWidth) const;
};

#endif
