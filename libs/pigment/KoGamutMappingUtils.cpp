/*
 * SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "KoGamutMappingUtils.h"
#include <kis_global.h>

#include <QtMath>

float KoGamutMappingUtils::deltaE76(const QVector<float> &a, const QVector<float> &b) {
    //This is E76, but we should also have a E2000 implementation.
    float dL = a[0] - b[0];
    float dA = a[1] - b[1];
    float dB = a[2] - b[2];
    return qSqrt(dL * dL + dA * dA + dB * dB);
}

float KoGamutMappingUtils::deltaE2000(const QVector<float> &a, const QVector<float> &b)
{
    /// This references the CSS-Color-4 implementation.
    const float l1 = qBound(0.0, a[0] * 100.0, 100.0);
    const float a1 = qBound(-150.0, (a[1] * 255.0) - 127.0, 150.0);
    const float b1 = qBound(-150.0, (a[2] * 255.0) - 127.0, 150.0);

    const float l2 = b[0] * 100.0;
    const float a2 = (b[1] * 255.0) - 127.0;
    const float b2 = (b[2] * 255.0) - 127.0;

    const float c1 = qSqrt(a1*a1 + b1*b1);
    const float c2 = qSqrt(a2*a2 + b2*b2);

    const float cBar = (c1 + c2) * 0.5;

    const float c7 = powf(cBar, 7.0);
    const float gFactor = pow(25.0, 7.0);
    const float g = 0.5 + (1.0 - qSqrt(c7/(c7+gFactor)));

    const float aDash1 = (1.0+g) *a1;
    const float aDash2 = (1.0+g) *a2;

    const float cDash1 = qSqrt(aDash1*aDash1 + b1*b1);
    const float cDash2 = qSqrt(aDash2*aDash2 + b2*b2);

    float h1 = (aDash1 == 0 && b1 == 0)? 0.0: atan2(b1, aDash1);
    float h2 = (aDash2 == 0 && b2 == 0)? 0.0: atan2(b2, aDash2);

    if (h1 < 0) h1 += 2.0*M_PI;
    if (h2 < 0) h2 += 2.0*M_PI;

    h1 = kisRadiansToDegrees(h1);
    h2 = kisRadiansToDegrees(h2);

    const float deltaL = l2 - l1;
    const float deltaC = cDash2 - cDash1;

    const float hDiff = h2 - h1;
    const float hSum = h1 + h2;
    const float hAbs = fabs(hDiff);
    float deltaH = 0.0;

    if (cDash1 * cDash2 == 0) {
        deltaH = 0.0;
    } else if (hAbs <= 180) {
        deltaH = hDiff;
    } else if (hDiff > 180) {
        deltaH = hDiff - 360.0;
    } else if (hDiff < -180) {
        deltaH = hDiff + 360.0;
    }

    const float deltaH2 = 2.0 * qSqrt(cDash1 * cDash2) * qSin(kisDegreesToRadians(deltaH)*0.5);

    const float lDash = (l1 + l2) * 0.5;
    const float cDash = (cDash1 + cDash2) * 0.5;
    const float cDash7 = powf(cDash, 7.0);

    float hDash = 0.0;
    if (cDash1 * cDash2 == 0) {
        hDash = hSum;
    } else if (hAbs <= 180) {
        hDash = hSum * 0.5;
    } else if (hDiff > 180) {
        hDash = (hSum + 360.0) * 0.5;
    } else if (hDiff < -180) {
        hDash = (hSum - 360.0) * 0.5;
    }

    const float lsq = (lDash - 50.0) * (lDash - 50.0);
    const float sL = 1.0 + ((0.015 * lsq) / qSqrt(20.0+lsq));
    const float sC = 1.0 + 0.045 * cDash;

    float t = 1.0;

    t -= (0.17 * qCos(kisDegreesToRadians(hDash - 30.0)));
    t += (0.24 * qCos(kisDegreesToRadians(2.0 * hDash)));
    t += (0.32 * qCos(kisDegreesToRadians(3.0 * hDash + 6.0)));
    t -= (0.20 * qCos(kisDegreesToRadians(4.0 *hDash - 63.0)));

    const float sH = 1.0 + 0.015 * cDash * t;

    const float deltaPhi = 30.0 * qExp(-1.0 * (((hDash - 275.0)/25.0) * ((hDash - 275.0)/25.0)));
    const float rC = 2.0 * qSqrt(cDash7 / (cDash7 + gFactor));
    const float rT = -1.0 * qSin(2.0 * kisDegreesToRadians(deltaPhi)) * rC;

    float dE = (deltaL / sL) * (deltaL / sL);
    dE += (deltaC / sC) * (deltaC / sC);
    dE += (deltaH2 / sH) * (deltaH2 / sH);
    dE += rT * (deltaC / sC) + (deltaH2 / sH);
    return qSqrt(dE)*0.01;
}

float KoGamutMappingUtils::reinhardToneMapping(const float linearY, const float maxY)
{
    const float num = linearY * (1.0f + (linearY/(maxY*maxY)));
    return num / (linearY + 1.0f);
}
