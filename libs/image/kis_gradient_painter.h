/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
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
#ifndef KIS_GRADIENT_PAINTER_H_
#define KIS_GRADIENT_PAINTER_H_

#include <QScopedPointer>

#include <KoColor.h>

#include "kis_types.h"
#include "kis_painter.h"

#include <kritaimage_export.h>


/**
 *  XXX: Docs!
 */
class KRITAIMAGE_EXPORT KisGradientPainter : public KisPainter
{

public:

    KisGradientPainter();
    KisGradientPainter(KisPaintDeviceSP device);
    KisGradientPainter(KisPaintDeviceSP device, KisSelectionSP selection);

    ~KisGradientPainter() override;

    enum enumGradientShape {
        GradientShapeLinear,
        GradientShapeBiLinear,
        GradientShapeRadial,
        GradientShapeSquare,
        GradientShapeConical,
        GradientShapeConicalSymetric,
        GradientShapeSpiral,
        GradientShapeReverseSpiral,
        GradientShapePolygonal
    };

    enum enumGradientRepeat {
        GradientRepeatNone,
        GradientRepeatForwards,
        GradientRepeatAlternate
    };

    void setGradientShape(enumGradientShape shape);

    void precalculateShape();

    /**
     * Paint a gradient in the rect between startx, starty, width and height.
     */
    bool paintGradient(const QPointF& gradientVectorStart,
                       const QPointF& gradientVectorEnd,
                       enumGradientRepeat repeat,
                       double antiAliasThreshold,
                       bool reverseGradient,
                       qint32 startx,
                       qint32 starty,
                       qint32 width,
                       qint32 height);

    // convenience overload
    bool paintGradient(const QPointF& gradientVectorStart,
                       const QPointF& gradientVectorEnd,
                       enumGradientRepeat repeat,
                       double antiAliasThreshold,
                       bool reverseGradient,
                       const QRect &applyRect);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};
#endif //KIS_GRADIENT_PAINTER_H_
