/*
 *  SPDX-FileCopyrightText: 2019 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_SPIN_BOX_SPLINE_UNITS_CONVERTER_H
#define KIS_SPIN_BOX_SPLINE_UNITS_CONVERTER_H

#include <kritaui_export.h>



/**
 * KisSpinBoxSplineUnitConverter is a class that converts points
 *  from ranges(0, 1) to ranges (min, max).
 * In case of reverted ranges (min > max),
 *  smaller values from range (0, 1) correspond
 *  to bigger values from range (min, max)
 *  and the other way around.
 * Example:
 *  3 from (0, 10) is 0.3 from (0, 1)
 *  3 from (10, 0) is 0.7 from (0, 1)
 */
class KRITAUI_EXPORT KisSpinBoxSplineUnitConverter
{

public:
    double io2sp(int x, int min, int max);
    int sp2io(double x, int min, int max);

};



#endif // KIS_SPIN_BOX_SPLINE_UNITS_CONVERTER_H
