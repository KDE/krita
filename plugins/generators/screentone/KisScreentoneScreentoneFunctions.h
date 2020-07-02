/*
 * KDE. Krita Project.
 *
 * Copyright (c) 2020 Deif Lou <ginoba@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KISSCREENTONESCREENTONEFUNCTIONS_H
#define KISSCREENTONESCREENTONEFUNCTIONS_H

#include <QtGlobal>

class QStringList;

enum KisScreentonePatternType
{
    KisScreentonePatternType_Dots,
    KisScreentonePatternType_Lines
};

enum KisScreentoneShapeType
{
    // Dots
    KisScreentoneShapeType_RoundDots,
    KisScreentoneShapeType_EllipseDots,
    KisScreentoneShapeType_DiamondDots,
    KisScreentoneShapeType_SquareDots,

    // Lines
    KisScreentoneShapeType_StraightLines = 0,
    KisScreentoneShapeType_SineWaveLines,
    KisScreentoneShapeType_TriangularWaveLines,
    KisScreentoneShapeType_SawtoothWaveLines,
    KisScreentoneShapeType_CurtainsLines
};

enum KisScreentoneInterpolationType
{
    KisScreentoneInterpolationType_Linear,
    KisScreentoneInterpolationType_Sinusoidal
};

QStringList screentonePatternNames();
QStringList screentoneShapeNames(int pattern);
QStringList screentoneInterpolationNames(int pattern, int shape);

namespace KisScreentoneScreentoneFunctions {

// Screentone functions must return a value between 0.0 and 1.0
// 0 means the foreground is fully opaque
// 1 means the foreground is fully transparent so the background can be seen
// One cycle of the pattern in each direction should expand 1px. The size (scaling)
// in the transformations dictates the final scaling of the pattern (dots, lines, etc.)

qreal sin(qreal x);
qreal triangle(qreal x);
qreal sawTooth(qreal x);

class DotsRoundLinear
{
public:
    qreal operator()(qreal x, qreal y) const;
};

class DotsRoundSinusoidal
{
public:
    qreal operator()(qreal x, qreal y) const;
};

class DotsEllipseLinear
{
public:
    qreal operator()(qreal x, qreal y) const;
};

class DotsEllipseSinusoidal
{
public:
    qreal operator()(qreal x, qreal y) const;
};

class DotsDiamond
{
public:
    qreal operator()(qreal x, qreal y) const;
};

class DotsSquare
{
public:
    qreal operator()(qreal x, qreal y) const;
};

class LinesStraightLinear
{
public:
    qreal operator()(qreal x, qreal y) const;
};

class LinesStraightSinusoidal
{
public:
    qreal operator()(qreal x, qreal y) const;
};

class LinesSineWaveLinear
{
public:
    qreal operator()(qreal x, qreal y) const;
};

class LinesSineWaveSinusoidal
{
public:
    qreal operator()(qreal x, qreal y) const;
};

class LinesTriangularWaveLinear
{
public:
    qreal operator()(qreal x, qreal y) const;
};

class LinesTriangularWaveSinusoidal
{
public:
    qreal operator()(qreal x, qreal y) const;
};

class LinesSawToothWaveLinear
{
public:
    qreal operator()(qreal x, qreal y) const;
};

class LinesSawToothWaveSinusoidal
{
public:
    qreal operator()(qreal x, qreal y) const;
};

class LinesCurtainsLinear
{
public:
    qreal operator()(qreal x, qreal y) const;
};

class LinesCurtainsSinusoidal
{
public:
    qreal operator()(qreal x, qreal y) const;
};

}

#endif