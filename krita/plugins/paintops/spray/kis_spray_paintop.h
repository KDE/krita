/*
 *  Copyright (c) 2008-2012 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef KIS_SPRAY_PAINTOP_H_
#define KIS_SPRAY_PAINTOP_H_

#include <kis_paintop.h>
#include <kis_types.h>

#include "spray_brush.h"
#include "kis_spray_paintop_settings.h"
#include "kis_brush_option.h"

class QPointF;
class KisPainter;


class KisSprayPaintOp : public KisPaintOp
{

public:

    KisSprayPaintOp(const KisSprayPaintOpSettings *settings, KisPainter * painter, KisImageWSP image);
    virtual ~KisSprayPaintOp();

    qreal paintAt(const KisPaintInformation& info);

private:
    KisShapeProperties m_shapeProperties;
    KisSprayProperties m_properties;
    KisShapeDynamicsProperties m_shapeDynamicsProperties;
    KisColorProperties m_colorProperties;
    KisBrushOption m_brushOption;

    const KisSprayPaintOpSettings *m_settings;

    KisPaintDeviceSP m_dab;
    SprayBrush m_sprayBrush;
    qreal m_xSpacing, m_ySpacing, m_spacing;
    bool m_isPresetValid;
    KisPressureRotationOption m_rotationOption;
    KisPressureSizeOption m_sizeOption;
    KisPressureOpacityOption m_opacityOption;
};

#endif // KIS_SPRAY_PAINTOP_H_
