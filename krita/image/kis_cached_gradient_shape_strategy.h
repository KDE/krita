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

#ifndef __KIS_CACHED_GRADIENT_SHAPE_STRATEGY_H
#define __KIS_CACHED_GRADIENT_SHAPE_STRATEGY_H

#include "kis_gradient_shape_strategy.h"
#include "krita_export.h"

#include <QScopedPointer>

class QRect;


class KRITAIMAGE_EXPORT KisCachedGradientShapeStrategy : public KisGradientShapeStrategy
{
public:
    KisCachedGradientShapeStrategy(const QRect &rc, qreal xStep, qreal yStep, KisGradientShapeStrategy *baseStrategy);
    ~KisCachedGradientShapeStrategy();

    double valueAt(double x, double y) const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_CACHED_GRADIENT_SHAPE_STRATEGY_H */
