/*
 *  SPDX-FileCopyrightText: 2020 Wolthera van Hövell tot Westerflier
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOCOLORTRANSFERFUNCTIONS_H
#define KOCOLORTRANSFERFUNCTIONS_H

#include "KoAlwaysInline.h"
#include "kritapigment_export.h"
#include <cmath>
#include <QtGlobal>

/**
 * @brief The KoColorTransferFunctions class
 *
 * A number of often used transferFunctions.
 * 
 * These functions can, at the time of writing, not be implemented
 * in ICC profiles, so instead, we apply or remove the curve as
 * necessary.
 */

ALWAYS_INLINE KRITAPIGMENT_EXPORT float applySmpte2048Curve(float x) {
    const float m1 = 2610.0 / 4096.0 / 4.0;
    const float m2 = 2523.0 / 4096.0 * 128.0;
    const float a1 = 3424.0 / 4096.0;
    const float c2 = 2413.0 / 4096.0 * 32.0;
    const float c3 = 2392.0 / 4096.0 * 32.0;
    const float a4 = 1.0;
    const float x_p = powf(0.008 * std::max(0.0f, x), m1);
    const float res = powf((a1 + c2 * x_p) / (a4 + c3 * x_p), m2);
    return res;
}

ALWAYS_INLINE KRITAPIGMENT_EXPORT float removeSmpte2048Curve(float x) {
    const float m1_r = 4096.0 * 4.0 / 2610.0;
    const float m2_r = 4096.0 / 2523.0 / 128.0;
    const float a1 = 3424.0 / 4096.0;
    const float c2 = 2413.0 / 4096.0 * 32.0;
    const float c3 = 2392.0 / 4096.0 * 32.0;

    const float x_p = powf(x, m2_r);
    const float res = powf(qMax(0.0f, x_p - a1) / (c2 - c3 * x_p), m1_r);
    return res * 125.0f;
}

ALWAYS_INLINE KRITAPIGMENT_EXPORT float applyHLGCurve(float x) {
    const float a = 0.17883277;
    const float b = 0.28466892;
    const float c = 0.55991073;

    if (x > 1.0/12.0) {
        return (a*log(12.0*x-b) + c);
    } else {
        return (sqrt(3.0) * powf(x, 0.5));
    }
}

ALWAYS_INLINE KRITAPIGMENT_EXPORT float removeHLGCurve(float x) {
    const float a = 0.17883277;
    const float b = 0.28466892;
    const float c = 0.55991073;
    if (x <= 0.5) {
        return (powf(x, 2.0) / 3.0);
    } else {
        return ( (exp(((x - c) / a)) + b) / 12.0);
    }
}

ALWAYS_INLINE KRITAPIGMENT_EXPORT float applySMPTE_ST_428Curve(float x) {
    return powf(48.0 * x/ 52.37, (1/2.6));
}

ALWAYS_INLINE KRITAPIGMENT_EXPORT float removeSMPTE_ST_428Curve(float x) {
    return (52.37/48.0 * powf( x , 2.6 ));
}

#endif // KOCOLORTRANSFERFUNCTIONS_H
