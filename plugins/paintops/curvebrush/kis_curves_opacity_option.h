/*
 *  SPDX-FileCopyrightText: 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_CURVES_OPACITY_OPTION_H_
#define KIS_CURVES_OPACITY_OPTION_H_

#include "kis_curve_option.h"
#include <brushengine/kis_paint_information.h>

class KisCurvesOpacityOption : public KisCurveOption
{
public:
    KisCurvesOpacityOption();
    qreal apply(const KisPaintInformation & info, qreal opacity) const;
};

#endif
