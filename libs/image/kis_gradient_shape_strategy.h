/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_GRADIENT_SHAPE_STRATEGY_H
#define __KIS_GRADIENT_SHAPE_STRATEGY_H

#include <QPointF>


class KisGradientShapeStrategy
{
public:
    KisGradientShapeStrategy();
    KisGradientShapeStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd);
    virtual ~KisGradientShapeStrategy();

    virtual double valueAt(double x, double y) const = 0;

protected:
    QPointF m_gradientVectorStart;
    QPointF m_gradientVectorEnd;
};

#endif /* __KIS_GRADIENT_SHAPE_STRATEGY_H */
