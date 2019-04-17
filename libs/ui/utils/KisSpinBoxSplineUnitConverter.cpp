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

#include "KisSpinBoxSplineUnitConverter.h"
#include <QtMath>
#include <kis_debug.h>



double KisSpinBoxSplineUnitConverter::io2sp(int x, int min, int max)
{
    int reversedRange = max - min > 0 ? 1 : -1; // tilt elevation has range (90; 0)
    int rangeLen = qAbs(max - min);

    double response = reversedRange * double(x - min) / rangeLen;
    return response;
}

int KisSpinBoxSplineUnitConverter::sp2io(double x, int min, int max)
{
    int rangeLen = max - min; // tilt elevation has range (90; 0)
    int response = qRound(x*rangeLen) + min;

    return response;
}




