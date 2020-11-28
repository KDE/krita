/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_gradient_shape_strategy.h"


KisGradientShapeStrategy::KisGradientShapeStrategy()
{
}

KisGradientShapeStrategy::KisGradientShapeStrategy(const QPointF& gradientVectorStart, const QPointF& gradientVectorEnd)
    : m_gradientVectorStart(gradientVectorStart),
      m_gradientVectorEnd(gradientVectorEnd)
{
}

KisGradientShapeStrategy::~KisGradientShapeStrategy()
{
}
