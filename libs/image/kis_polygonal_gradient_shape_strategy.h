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
    ~KisPolygonalGradientShapeStrategy();

    double valueAt(double x, double y) const;

    static QPointF testingCalculatePathCenter(int numSamples, const QPainterPath &path, qreal exponent, bool searchForMax);

private:
    QPainterPath m_selectionPath;

    qreal m_exponent;
    qreal m_minWeight;
    qreal m_maxWeight;
    qreal m_scaleCoeff;
};

#endif /* __KIS_POLYGONAL_GRADIENT_SHAPE_STRATEGY_H */
