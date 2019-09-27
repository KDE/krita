/*
 *  Copyright (c) 2019 Agata Cacko <cacko.azh@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
