/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
