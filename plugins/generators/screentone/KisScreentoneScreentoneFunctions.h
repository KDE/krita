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

#include "KisScreentoneGeneratorTemplate.h"

namespace KisScreentoneScreentoneFunctions {

// NOTE: Screentone functions must return a value in the range [0.0, 1.0]
// It must be a 2d function and you can think of it as a height map that is
// later transformed using the brightness and contrast parameters.

// NOTE: The current functions are periodic. They use the concepts of screen
// grid and grid cells. In each cell the shape is repeated. Each cell of the
// screen should have a size of one unit in each direction. The size (scaling)
// in the transformations dictates the final scaling of the pattern.

// NOTE: The normal spot functions just create a 2d function that makes the
// shape in a simple way, but the area of the shape when the function is
// thresholded may not have any connection with the threshold value itself
// (which would be derived from the brightness). This can result in the visual
// appearance of the screentone not corresponding with the lightness value
// chosen. To solve that, the area of the shape inside the cell (in other
// words, the coverage of the shape) must equal the threshold (lightness) value.
// This is achieved in two different ways:
//     1. The spot function is equalized. This is the same process as the one
//        used in image processing via histogram equalization: the histogram is
//        first computed and then a cumulative function is derived from it. If
//        we use the original image with this cumulative function, it would end
//        having a uniform value distribution.
//        Since we know here how the original functions look like before hand,
//        we also know their histogram so we don't have to compute it. Here the
//        the equalized versions of the functions just use the original
//        functions and pass them through a precomputed cumulative function
//        that equalizes them. Some of these cumulative functions were derived
//        analytically, but others, due to the complexity of the original
//        function, had to be empirically aproximated as piecewise functions
//        using cubic functions for each piece.
//     2. The second method is derived from traditional halftone screen
//        construction using a template. The template is filled with increasing
//        numbers from 0 to 1. For example, if the template is 10x10 pixels,
//        each of the 100 pixels will have a different value starting with 0 and
//        increasing by 1/100 through 1. This alone ensures the uniform
//        distribution, since there is exactly one pixel with the same value.
//        Now, to have the template resemble the original function's shape, we
//        have to evaluate the original function in each template pixel, and
//        then assign an index to each one based on the function's value (from
//        lower to higher; sort them in other words). Then the new value can be
//        computed as index/total_number_of_pixels_in_template.
// These two modes and the original function can be chosen by the user and each
// one has its pros/cons.

// NOTE: the linear variants of line patterns are already equalized in the
// original functions so a typedef is used. Also, the equalized sinusoidal
// variants of line patterns give the same results as the un-equalized linear
// variants, so a typedef is also used

qreal sin(qreal x);
qreal triangle(qreal x);
qreal sawTooth(qreal x);

class DotsRoundLinear
{
public:
    qreal operator()(qreal x, qreal y) const;
};

class DotsRoundLinearEqualized : public DotsRoundLinear
{
public:
    qreal operator()(qreal x, qreal y) const;
};

class DotsRoundSinusoidal
{
public:
    qreal operator()(qreal x, qreal y) const;
};

class DotsRoundSinusoidalEqualized : public DotsRoundSinusoidal
{
public:
    qreal operator()(qreal x, qreal y) const;
};

class DotsEllipseLinear
{
public:
    qreal operator()(qreal x, qreal y) const;
};

class DotsEllipseLinearEqualized : public DotsEllipseLinear
{
public:
    qreal operator()(qreal x, qreal y) const;
};

class DotsEllipseSinusoidal
{
public:
    qreal operator()(qreal x, qreal y) const;
};

class DotsEllipseSinusoidalEqualized : public DotsEllipseSinusoidal
{
public:
    qreal operator()(qreal x, qreal y) const;
};

class DotsEllipseLinear_Legacy
{
public:
    qreal operator()(qreal x, qreal y) const;
};

using DotsEllipseLinearEqualized_Legacy = DotsEllipseLinear_Legacy;

using DotsEllipseSinusoidal_Legacy = DotsEllipseSinusoidal;

using DotsEllipseSinusoidalEqualized_Legacy = DotsEllipseSinusoidal;

class DotsDiamond
{
public:
    qreal operator()(qreal x, qreal y) const;
};

class DotsDiamondEqualized : public DotsDiamond
{
public:
    qreal operator()(qreal x, qreal y) const;
};

class DotsSquare
{
public:
    qreal operator()(qreal x, qreal y) const;
};

class DotsSquareEqualized : public DotsSquare
{
public:
    qreal operator()(qreal x, qreal y) const;
};

class LinesStraightLinear
{
public:
    qreal operator()(qreal x, qreal y) const;
};

using LinesStraightLinearEqualized = LinesStraightLinear;

class LinesStraightSinusoidal
{
public:
    qreal operator()(qreal x, qreal y) const;
};

using LinesStraightSinusoidalEqualized = LinesStraightLinear;

class LinesSineWaveLinear
{
public:
    qreal operator()(qreal x, qreal y) const;
};

using LinesSineWaveLinearEqualized = LinesSineWaveLinear;

class LinesSineWaveSinusoidal
{
public:
    qreal operator()(qreal x, qreal y) const;
};

using LinesSineWaveSinusoidalEqualized = LinesSineWaveLinear;

class LinesTriangularWaveLinear
{
public:
    qreal operator()(qreal x, qreal y) const;
};

using LinesTriangularWaveLinearEqualized = LinesTriangularWaveLinear;

class LinesTriangularWaveSinusoidal
{
public:
    qreal operator()(qreal x, qreal y) const;
};

using LinesTriangularWaveSinusoidalEqualized = LinesTriangularWaveLinear;

class LinesSawToothWaveLinear
{
public:
    qreal operator()(qreal x, qreal y) const;
};

using LinesSawToothWaveLinearEqualized = LinesSawToothWaveLinear;

class LinesSawToothWaveSinusoidal
{
public:
    qreal operator()(qreal x, qreal y) const;
};

using LinesSawToothWaveSinusoidalEqualized = LinesSawToothWaveLinear;

class LinesCurtainsLinear
{
public:
    qreal operator()(qreal x, qreal y) const;
};

using LinesCurtainsLinearEqualized = LinesCurtainsLinear;

class LinesCurtainsSinusoidal
{
public:
    qreal operator()(qreal x, qreal y) const;
};

using LinesCurtainsSinusoidalEqualized = LinesCurtainsLinear;

// The name "TemplateBasedFunction" has nothing to do with c++ templates even if
// this class is templated. Here "Template" means that precomputed values are
// used somehow. The class is templated to allow extensibility in the future
template <typename T>
class TemplateBasedFunction
{
public:
    TemplateBasedFunction(const T &the_template)
        : m_template(the_template)
    {}

    qreal operator()(qreal x, qreal y) const
    {
        return m_template(x, y);
    }

private:
    const T& m_template;
};

}

#endif