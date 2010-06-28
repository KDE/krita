/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef KIS_SKETCH_PAINTOP_H_
#define KIS_SKETCH_PAINTOP_H_

#include <kis_paintop.h>
#include <kis_types.h>

#include "kis_sketchop_option.h"
#include "kis_sketch_paintop_settings.h"

#include "kis_painter.h"

class KisSketchPaintOp : public KisPaintOp
{

public:

    KisSketchPaintOp(const KisSketchPaintOpSettings *settings, KisPainter * painter, KisImageWSP image);
    virtual ~KisSketchPaintOp();
    virtual KisDistanceInformation paintLine(const KisPaintInformation& pi1, const KisPaintInformation& pi2, const KisDistanceInformation& savedDist = KisDistanceInformation());
    double paintAt(const KisPaintInformation& info);

private:
    // pixel buffer
    KisPaintDeviceSP m_dab;
    KisPressureOpacityOption m_opacityOption;
    SketchProperties m_sketchProperties;
    
    QVector<QPointF> m_points;
    int m_count;
    KisPainter * m_painter;
};

#endif // KIS_SKETCH_PAINTOP_H_
