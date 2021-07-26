/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSCREENTONESCREENTONEFUNCTIONS_H
#define KISSCREENTONESCREENTONEFUNCTIONS_H

#include <QtGlobal>

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