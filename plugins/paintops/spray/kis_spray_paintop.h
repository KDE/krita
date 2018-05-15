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

#include <brushengine/kis_paintop.h>
#include <kis_types.h>

#include "spray_brush.h"
#include "kis_spray_paintop_settings.h"
#include "kis_brush_option.h"
#include <kis_airbrush_option_widget.h>
#include <kis_pressure_rotation_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_size_option.h>
#include <kis_pressure_rate_option.h>

class KisPainter;


class KisSprayPaintOp : public KisPaintOp
{

public:

    KisSprayPaintOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisNodeSP node, KisImageSP image);
    ~KisSprayPaintOp() override;

protected:

    KisSpacingInformation paintAt(const KisPaintInformation& info) override;

    KisSpacingInformation updateSpacingImpl(const KisPaintInformation &info) const override;

    KisTimingInformation updateTimingImpl(const KisPaintInformation &info) const override;

private:
    KisSpacingInformation computeSpacing(const KisPaintInformation &info, qreal lodScale) const;

private:
    KisShapeProperties m_shapeProperties;
    KisSprayOptionProperties m_properties;
    KisShapeDynamicsProperties m_shapeDynamicsProperties;
    KisColorProperties m_colorProperties;
    KisBrushOptionProperties m_brushOption;

    KisPaintDeviceSP m_dab;
    SprayBrush m_sprayBrush;
    qreal m_xSpacing, m_ySpacing, m_spacing;
    bool m_isPresetValid;
    KisAirbrushOptionProperties m_airbrushOption;
    KisPressureRotationOption m_rotationOption;
    KisPressureSizeOption m_sizeOption;
    KisPressureOpacityOption m_opacityOption;
    KisPressureRateOption m_rateOption;
    KisNodeSP m_node;
};

#endif // KIS_SPRAY_PAINTOP_H_
