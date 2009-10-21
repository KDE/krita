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

#include <KoColor.h>

#include "kis_global.h"
#include "kis_types.h"
#include "kis_painter.h"

#include <krita_export.h>

class KoAbstractGradient;

/**
 *  XXX: Docs!
 */
class KRITAIMAGE_EXPORT KisGradientPainter : public KisPainter
{

public:

    KisGradientPainter();
    KisGradientPainter(KisPaintDeviceSP device);
    KisGradientPainter(KisPaintDeviceSP device, KisSelectionSP selection);

    enum enumGradientShape {
        GradientShapeLinear,
        GradientShapeBiLinear,
        GradientShapeRadial,
        GradientShapeSquare,
        GradientShapeConical,
        GradientShapeConicalSymetric
    };

    enum enumGradientRepeat {
        GradientRepeatNone,
        GradientRepeatForwards,
        GradientRepeatAlternate
    };

    class Configuration
    {

    public:
        const KoAbstractGradient* gradient;

        KoColor fgColor;
        quint8 opacity;
        const KoCompositeOp* compositeOp;
        KisTransaction* transaction;

        QPointF vectorStart;
        QPointF vectorEnd;

        KisGradientPainter::enumGradientShape shape;
        KisGradientPainter::enumGradientRepeat repeat;

        double antiAliasThreshold;
        bool reverse;
    };

    /**
     * Paint a gradient in the rect between startx, starty, width and height.
     */
    bool paintGradient(const QPointF& gradientVectorStart,
                       const QPointF& gradientVectorEnd,
                       enumGradientShape shape,
                       enumGradientRepeat repeat,
                       double antiAliasThreshold,
                       bool reverseGradient,
                       qint32 startx,
                       qint32 starty,
                       qint32 width,
                       qint32 height);

};
#endif //KIS_GRADIENT_PAINTER_H_
