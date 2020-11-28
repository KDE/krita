/*
 *  SPDX-FileCopyrightText: 2019 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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




