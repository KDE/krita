/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisScreentoneScreentoneFunctions.h"

#include <cmath>

#include <QStringList>
#include <klocalizedstring.h>

QStringList screentonePatternNames()
{
    return QStringList()
        << i18nc("Screentone Pattern Type - Dots", "Dots")
        << i18nc("Screentone Pattern Type - Lines", "Lines");
}

QStringList screentoneShapeNames(int pattern)
{
    if (pattern == KisScreentonePatternType_Dots) {
        return QStringList()
            << i18nc("Screentone Pattern - Round Dots", "Round")
            << i18nc("Screentone Pattern - Ellipse Dots", "Ellipse")
            << i18nc("Screentone Pattern - Diamond Dots", "Diamond")
            << i18nc("Screentone Pattern - Square Dots", "Square");
    } else if (pattern == KisScreentonePatternType_Lines) {
        return QStringList()
            << i18nc("Screentone Pattern - Straight Lines", "Straight")
            << i18nc("Screentone Pattern - Sine Wave Lines", "Sine Wave")
            << i18nc("Screentone Pattern - Trianular Wave Lines", "Triangular Wave")
            << i18nc("Screentone Pattern - Sawtooth Wave Lines", "Sawtooth Wave")
            << i18nc("Screentone Pattern - Curtains Lines", "Curtains");
    }
    
    return QStringList();
}

QStringList screentoneInterpolationNames(int pattern, int shape)
{
    if (pattern == KisScreentonePatternType_Dots) {
        if (shape == KisScreentoneShapeType_RoundDots || shape == KisScreentoneShapeType_EllipseDots) {
            return QStringList()
                << i18nc("Screentone Interpolation Method - Linear", "Linear")
                << i18nc("Screentone Interpolation Method - Sinusoidal", "Sinusoidal");
        }
    } else if (pattern == KisScreentonePatternType_Lines) {
        return QStringList()
            << i18nc("Screentone Interpolation Method - Linear", "Linear")
            << i18nc("Screentone Interpolation Method - Sinusoidal", "Sinusoidal");
    }

    return QStringList();
}

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

qreal DotsRoundSinusoidal::operator()(qreal x, qreal y) const
{
    return (sin(x) + sin(y)) / 2.;
}

qreal DotsEllipseLinear::operator()(qreal x, qreal y) const
{
    constexpr qreal ellipseRatioX = 0.4 / M_SQRT2;
    constexpr qreal ellipseRatioY = 0.6 / M_SQRT2;
    x = triangle(x) * ellipseRatioX;
    y = triangle(y) * ellipseRatioY;
    return std::sqrt(x * x + y * y) * M_SQRT2;
}

qreal DotsEllipseSinusoidal::operator()(qreal x, qreal y) const
{
    constexpr qreal ellipseRatioX = 0.4 * 2.0;
    constexpr qreal ellipseRatioY = 0.6 * 2.0;
    x = sin(x) * ellipseRatioX;
    y = sin(y) * ellipseRatioY;
    return (x + y) / 2.;
}

qreal DotsDiamond::operator()(qreal x, qreal y) const
{
    return (triangle(x) + triangle(y)) / 2.;
}

qreal DotsSquare::operator()(qreal x, qreal y) const
{
    return std::max(triangle(x), triangle(y));
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
