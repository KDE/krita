/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisScreentoneScreentoneFunctions.h"

#include <cmath>

namespace KisScreentoneScreentoneFunctions {

qreal sin(qreal x)
{
    x = std::cos(x * M_PI);
    return  x * x;
}

qreal triangle(qreal x)
{
    return 1.0 - 2.0 * std::abs(x - std::floor(x + 0.5));
}

qreal sawTooth(qreal x)
{
    constexpr qreal peakXOffset = 0.9;
    constexpr qreal peakYOffset = 0.5;
    x = x - std::floor(x);
    return (x < peakXOffset ? 1.0 / peakXOffset * x : -1.0 / (1.0 - peakXOffset) * (x - 1.0)) * peakYOffset;
}

qreal DotsRoundLinear::operator()(qreal x, qreal y) const
{
    x = triangle(x);
    y = triangle(y);
    return std::sqrt(x * x + y * y) / M_SQRT2;
}

qreal DotsRoundLinearEqualized::operator()(qreal x, qreal y) const
{
    // In theory, the cumulative function for this spot function is:
    // "coverage = area of the intersection between the disk formed by the value,
    // and the screen cell".
    // This uses a piecewise cumulative function obtained analytically. If
    // the value is less than or equal to "sqrt(2) / 2" (value at which the
    // disk touches the screen cell borders) then the coverage for that value
    // is obtained simply by getting the area of the disk. If the value is
    // greater than "sqrt(2) / 2" then the coverage is obtained by computing
    // the area of the intersection between the disk and the screen cell (area
    // of the disk minus area of the chords of the disk outside the screen cell)
    const qreal z = DotsRoundLinear::operator()(x, y);
    const qreal zOverSqrt2 = z / M_SQRT2;
    const qreal zOverSqrt2Squared = zOverSqrt2 * zOverSqrt2;
    if (z <= M_SQRT2 / 2.0) {
        return M_PI * zOverSqrt2Squared;
    } else {
        return M_PI * zOverSqrt2Squared -
               4.0 * (zOverSqrt2Squared * std::acos(M_SQRT2 / (2.0 * z)) -
                      0.5 * std::sqrt(zOverSqrt2Squared - 0.25));
    }
}

qreal DotsRoundSinusoidal::operator()(qreal x, qreal y) const
{
    return (sin(x) + sin(y)) / 2.;
}

qreal DotsRoundSinusoidalEqualized::operator()(qreal x, qreal y) const
{
    // Here the cumulative function is a piecewise function obtained empirically
    // by fitting some simple curves to a list of points
    const qreal z = DotsRoundSinusoidal::operator()(x, y);
    if (z <= 0.5) {
        return M_SQRT2 / 2.0 - std::sqrt(-(z - 0.5469) / 1.0938);
    } else {
        return (1.0 - M_SQRT2 / 2.0) + std::sqrt((z - (1.0 - 0.5469)) / 1.0938);
    }
}

qreal DotsEllipseLinear::operator()(qreal x, qreal y) const
{
    constexpr qreal aspectRatio = 1.25;
    // The folowing magic number makes the function go to 1.0 in
    // the corners of the cell (normalizes it)
    constexpr qreal factor = 0.625;
    x = triangle(x);
    y = triangle(y) * aspectRatio;
    return std::sqrt(x * x + y * y) * factor;
}

qreal DotsEllipseLinearEqualized::operator()(qreal x, qreal y) const
{
    // In theory, the cumulative function for this spot function is:
    // "coverage = area of the intersection between the elliptical disk formed
    // by the value, and the screen cell".
    // This uses a piecewise cumulative function obtained analytically. First,
    // the area of the elliptical disk is obtained. If the value is greater than
    // "0.625" (value at which the elliptical disk touches the left and right
    // screen cell borders) then the area of the left and right elliptical
    // chords is subtracted; and if the value is greater than "0.78125" then the
    // area of the top and bottom elliptical chords is also subtracted
    const qreal z = DotsEllipseLinear::operator()(x, y);
    constexpr qreal factor = 0.625;
    constexpr qreal factorTimes2 = factor * 2.0;
    const qreal zOverFactorTimes2 = z / factorTimes2;
    const qreal zTimesPoint8OverFactorTimes2 = 0.8 * zOverFactorTimes2;
    qreal result = M_PI * zOverFactorTimes2 * zTimesPoint8OverFactorTimes2;
    if (z > 0.625) {
        const qreal zOverFactorTimes2Squared = zOverFactorTimes2 * zOverFactorTimes2;
        result -= 2.0 * (zOverFactorTimes2Squared * std::acos(factor / z) -
                         0.5 * std::sqrt(zOverFactorTimes2Squared - 0.25));
    }
    if (z > 0.78125) {
        const qreal zTimesPoint8OverFactorTimes2Squared =
            zTimesPoint8OverFactorTimes2 * zTimesPoint8OverFactorTimes2;
        result -= 2.0 * (zTimesPoint8OverFactorTimes2Squared * std::acos(factor / (0.8 * z)) -
                         0.5 * std::sqrt(zTimesPoint8OverFactorTimes2Squared - 0.25));
    }
    return result;
}

qreal DotsEllipseSinusoidal::operator()(qreal x, qreal y) const
{
    // The "0.4" and "0.6" values make a function such that if one thresholds it
    // at 0.4 the resulting shape touches the borders of the cell horizontally
    // and if one thresholds it at "0.6" it touches the cell vertically. That is
    // the standard convention
    x = sin(x) * 0.4;
    y = sin(y) * 0.6;
    // We would need to divide the following by ("0.4" + "0.6"), but since that is
    // equal to 1, we skip it. The division is required to normalize the values
    // of the function. If those magic numbers change, and they don't sum to 1,
    // then we must divide
    return (x + y);
}

qreal DotsEllipseSinusoidalEqualized::operator()(qreal x, qreal y) const
{
    // Here the cumulative function is a piecewise function obtained empirically
    // by fitting some simple cubic curves to a list of points
    const qreal z = DotsEllipseSinusoidal::operator()(x, y);
    const qreal z2 = z * z;
    const qreal z3 = z * z2;
    if (z <= 0.3) {
        return 0.8795 * z3 + 0.1825 * z2 + 0.6649 * z + 0.0008;
    } else if (z <= 0.4) {
        return 32.0507 * z3 - 30.3781 * z2 + 10.6756 * z - 1.0937;
    } else if (z <= 0.5) {
        return 27.8089 * z3 - 39.4726 * z2 + 19.8992 * z - 3.0553;
    } else if (z <= 0.6) {
        return 35.1490 * z3 - 55.6810 * z2 + 30.6244 * z - 5.2839;
    } else if (z <= 0.7) {
        return 24.3210 * z3 - 50.1381 * z2 + 35.6452 * z - 7.9322;
    } else {
        return 0.7457 * z3 - 2.4792 * z2 + 3.3748 * z - 0.6402;
    }
}

qreal DotsEllipseLinear_Legacy::operator()(qreal x, qreal y) const
{
    // This is the function used for the elliptical spots in Krita 4.*
    // It is wrong because it produces too dark values. The function should
    // produce a value of 1 at the corners of the screen cell
    constexpr qreal ellipseRatioX = 0.4 / M_SQRT2;
    constexpr qreal ellipseRatioY = 0.6 / M_SQRT2;
    x = triangle(x) * ellipseRatioX;
    y = triangle(y) * ellipseRatioY;
    return std::sqrt(x * x + y * y) * M_SQRT2;
}

qreal DotsDiamond::operator()(qreal x, qreal y) const
{
    return (triangle(x) + triangle(y)) / 2.;
}

qreal DotsDiamondEqualized::operator()(qreal x, qreal y) const
{
    // Here the cumulative function is a piecewise function obtained
    // analytically. If the value is less than or equal to "0.5" then the
    // coverage is simply the area of the diamond and if the value is greater
    // than "0.5" then the coverage is the area of the intersection between the
    // diamond and the screen cell
    const qreal z = DotsDiamond::operator()(x, y);
    if (z <= 0.5) {
        return 2.0 * z * z;
    } else {
        return -2.0 * z * z + 4.0 * z - 1.0;
    }
}

qreal DotsSquare::operator()(qreal x, qreal y) const
{
    return std::max(triangle(x), triangle(y));
}

qreal DotsSquareEqualized::operator()(qreal x, qreal y) const
{
    // Here the cumulative function was obtained analytically and it is just the
    // area of the square
    const qreal z = DotsSquare::operator()(x, y);
    return z * z;
}

qreal LinesStraightLinear::operator()(qreal x, qreal y) const
{
    Q_UNUSED(x);
    return triangle(y);
}

qreal LinesStraightSinusoidal::operator()(qreal x, qreal y) const
{
    Q_UNUSED(x);
    return sin(y);
}

qreal LinesSineWaveLinear::operator()(qreal x, qreal y) const
{
    return triangle(y + sin(x));
}

qreal LinesSineWaveSinusoidal::operator()(qreal x, qreal y) const
{
    return sin(y + sin(x));
}

qreal LinesTriangularWaveLinear::operator()(qreal x, qreal y) const
{
    return triangle(y + triangle(x));
}

qreal LinesTriangularWaveSinusoidal::operator()(qreal x, qreal y) const
{
    return sin(y + triangle(x));
}

qreal LinesSawToothWaveLinear::operator()(qreal x, qreal y) const
{
    return triangle(y + sawTooth(x));
}

qreal LinesSawToothWaveSinusoidal::operator()(qreal x, qreal y) const
{
    return sin(y + sawTooth(x));
}

qreal LinesCurtainsLinear::operator()(qreal x, qreal y) const
{
    x = triangle(x);
    return triangle(y + x * x);
}

qreal LinesCurtainsSinusoidal::operator()(qreal x, qreal y) const
{
    x = triangle(x);
    return sin(y + x * x);
}

}
