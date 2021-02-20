/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_POLYGONAL_GRADIENT_SHAPE_STRATEGY_H
#define __KIS_POLYGONAL_GRADIENT_SHAPE_STRATEGY_H

#include "kis_gradient_shape_strategy.h"

#include <QPolygonF>
#include <QPainterPath>

#include "kritaimage_export.h"


class KRITAIMAGE_EXPORT KisPolygonalGradientShapeStrategy : public KisGradientShapeStrategy
{
public:
    KisPolygonalGradientShapeStrategy(const QPainterPath &selectionPath,
                                      qreal exponent);
    ~KisPolygonalGradientShapeStrategy() override;

    double valueAt(double x, double y) const override;

    static QPointF testingCalculatePathCenter(int numSamples, const QPainterPath &path, qreal exponent, bool searchForMax);

private:
    QPainterPath m_selectionPath;

    qreal m_exponent;
    qreal m_minWeight;
    qreal m_maxWeight;
    qreal m_scaleCoeff;
};

#endif /* __KIS_POLYGONAL_GRADIENT_SHAPE_STRATEGY_H */
