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

//---------- Piecewise Filmic Curve -----------//

static void SolveAB(float & lnA, float & B, float x0, float y0, float m)
{
    B = (m*x0)/y0;
    lnA = logf(y0) - B*logf(x0);
}

static void AsSlopeIntercept(float & m, float & b, float x0, float x1, float y0, float y1)
{
    float dy = (y1-y0);
    float dx = (x1-x0);
    if (dx == 0)
        m = 1.0f;
    else
        m = dy/dx;

    b = y0 - x0*m;
}

static float EvalDerivativeLinearGamma(float m, float b, float g, float x)
{
    float ret = g*m*powf(m*x+b,g-1.0f);
    return ret;
}

KoGamutMappingUtils::PiecewiseFilmicCurve::PiecewiseFilmicCurve(float maxWhite, QPointF point0, QPointF point1, QPointF overshootPoint)
{
    segments = QVector<Segment>(3);

    p0 = point0;
    p1 = point1;
    QPointF overshoot = overshootPoint;

    inverseMaxWhite = 1.0/maxWhite;
    // normalize



    p0.setX(p0.x()/maxWhite);
    p1.setX(p1.x()/maxWhite);

    overshoot.setX(overshootPoint.x()/maxWhite);

    Segment linear;
    float m, b;
    AsSlopeIntercept(m, b, p0.x(), p1.x(), p0.y(), p1.y());
    const float g = 1.0;

    linear.offsetX = -(b / m);
    linear.lnA = g * logf(m);
    linear.b = g;

    segments[1] = linear;

    const float toeM = EvalDerivativeLinearGamma(m, b, g, p0.x());
    const float shoulderM = EvalDerivativeLinearGamma(m, b, g, p1.x());

    Segment toe;
    SolveAB(toe.lnA, toe.b, toe.offsetX, toe.offsetY, toeM);
    segments[0] = toe;

    Segment shoulder;
    const float x0 = (1.0f + overshoot.x()) - p1.x();
    const float y0 = (1.0f + overshoot.y()) - p1.y();
    SolveAB(shoulder.lnA, shoulder.b, x0, y0, shoulderM);

    shoulder.offsetX = (1.0f + overshoot.x());
    shoulder.offsetY = (1.0f + overshoot.y());
    shoulder.scaleX = -1.0;
    shoulder.scaleY = -1.0;

    segments[2] = shoulder;

    {
        const float scale = shoulder.evalSegment(1.0f);
        const float invScale = 1.0f / scale;

        segments[0].offsetY *= invScale;
        segments[0].scaleY *= invScale;

        segments[1].offsetY *= invScale;
        segments[1].scaleY *= invScale;

        segments[2].offsetY *= invScale;
        segments[2].scaleY *= invScale;
    }
}

float KoGamutMappingUtils::PiecewiseFilmicCurve::evalCurve(const float x)
{
    int index = (x < p0.x()) ? 0 : ((x < p1.x()) ? 1 : 2);
    Segment s = segments.at(index);
    return s.evalSegment(x*inverseMaxWhite);
}

float KoGamutMappingUtils::PiecewiseFilmicCurve::Segment::evalSegment(const float x)
{
    float x0 = (x - offsetX) * scaleX;
    float y0 = x0 > 0? expf(lnA + b * logf(x0)): 0.0;
    return y0 * scaleY + offsetY;
}

KoGamutMappingUtils::PiecewiseFilmicCurve KoGamutMappingUtils::PiecewiseFilmicCurve::fromUserParams(float toeStrength, float toeLength, float shoulderStrength, float shoulderLength, float shoulderAngle)
{
    const float uiGamma = 2.2f;

    const float normalizedToeLength = powf(qBound(0.0, toeLength, 1.0), uiGamma);
    const float normalizedToeStrength = qBound(0.0, toeStrength, 1.0);
    const float normalizedShoulderAngle = qBound(0.0, shoulderAngle, 1.0);

    const float x0 = normalizedToeLength * .5f;
    const float y0 = (1.0f - normalizedToeStrength) * x0;

    const float remainingY = 1.0f - y0;
    const float initialW = x0 + remainingY;

    const float y1_offset = (1.0f - shoulderLength) * remainingY;
    const float x1 = x0 + y1_offset;
    const float y1 = y0 + y1_offset;

    const float extraW = exp2f(shoulderStrength)-1.0f;
    const float W = initialW + extraW;

    QPointF point0(powf(x0, uiGamma), powf(y0, uiGamma));
    QPointF point1(powf(x1, uiGamma), powf(y1, uiGamma));
    float maxWhite = W;

    QPointF overshoot;
    overshoot.setX((W * 2.0f) * normalizedShoulderAngle * shoulderStrength);
    overshoot.setY(0.5f * normalizedShoulderAngle * shoulderStrength);

    return PiecewiseFilmicCurve(maxWhite, point0, point1, overshoot);
}
