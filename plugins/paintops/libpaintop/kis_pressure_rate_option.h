/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef KIS_PRESSURE_RATE_OPTION_H
#define KIS_PRESSURE_RATE_OPTION_H

#include "kis_curve_option.h"
#include <brushengine/kis_paint_information.h>
#include <kritapaintop_export.h>

class PAINTOP_EXPORT KisPressureRateOption : public KisCurveOption
{
public:
    KisPressureRateOption();
    double apply(const KisPaintInformation &info) const;
};

#endif // KIS_PRESSURE_RATE_OPTION_H
