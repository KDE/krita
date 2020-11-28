/*
 *  SPDX-FileCopyrightText: 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_DENSITY_OPTION_H_
#define KIS_DENSITY_OPTION_H_

#include "kis_curve_option.h"
#include <brushengine/kis_paint_information.h>

class KisDensityOption : public KisCurveOption
{
public:
    KisDensityOption();
    double apply(const KisPaintInformation & info, double probability) const;
};

#endif
