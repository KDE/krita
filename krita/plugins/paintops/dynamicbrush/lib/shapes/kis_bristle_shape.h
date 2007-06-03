/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_BRISTLE_SHAPE_H_
#define _KIS_BRISTLE_SHAPE_H_

#include "kis_dynamic_shape.h"

// TODO: don't export

struct KisPaintBrush;
#include <kis_shared_ptr.h>
typedef KisSharedPtr<KisPaintBrush> KisPaintBrushSP;

class DYNAMIC_BRUSH_EXPORT KisBristleShape : public KisDynamicShape {
    public:
        KisBristleShape(double paintbrushMinRadius = 10.0, double paintbrushMaxRadius = 20.0, double bristlesDensity = 1.0, double bristlesMinRadius = 0.0, double bristlesMaxRadius = 3.0);
        virtual QRect rect();
        virtual KisDynamicShape* clone() const;
        virtual void resize(double xs, double ys);
        virtual void rotate(double r);
        virtual void paintAt(const QPointF &pos, const KisPaintInformation& info, KisDynamicColoring* coloringsrc, KisPainter* m_painter);
    private:
        KisPaintBrushSP m_paintBrush;
        double m_radius;
        double m_angle;
        QRect m_rect;
};

#endif
