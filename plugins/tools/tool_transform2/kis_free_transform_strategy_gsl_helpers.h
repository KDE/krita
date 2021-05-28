/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_FREE_TRANSFORM_STRATEGY_GSL_HELPERS_H
#define __KIS_FREE_TRANSFORM_STRATEGY_GSL_HELPERS_H

#include <QPointF>
class ToolTransformArgs;

namespace GSL
{

    struct ScaleResult1D {
        ScaleResult1D() : scale(1.0) {}

        QPointF transformedCenter;
        qreal scale;
        bool isValid = false;
    };

    ScaleResult1D calculateScaleX(const ToolTransformArgs &args,
                                  const QPointF &staticPointSrc,
                                  const QPointF &staticPointDst,
                                  const QPointF &movingPointSrc,
                                  const QPointF &movingPointDst);

    ScaleResult1D calculateScaleY(const ToolTransformArgs &args,
                                  const QPointF &staticPointSrc,
                                  const QPointF &staticPointDst,
                                  const QPointF &movingPointSrc,
                                  const QPointF &movingPointDst);

    struct ScaleResult2D {
        ScaleResult2D() : scaleX(1.0), scaleY(1.0) {}

        QPointF transformedCenter;
        qreal scaleX;
        qreal scaleY;

        bool isValid = false;
    };

    ScaleResult2D calculateScale2D(const ToolTransformArgs &args,
                                   const QPointF &staticPointSrc,
                                   const QPointF &staticPointDst,
                                   const QPointF &movingPointSrc,
                                   const QPointF &movingPointDst);

    ScaleResult2D calculateScale2DAffine(const ToolTransformArgs &args,
                                         const QPointF &staticPointSrc,
                                         const QPointF &staticPointDst,
                                         const QPointF &movingPointSrc,
                                         const QPointF &movingPointDst);
}

#endif /* __KIS_FREE_TRANSFORM_STRATEGY_GSL_HELPERS_H */
