/*
 *  Copyright (c) 2008,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef KIS_DEFORMPAINTOP_H_
#define KIS_DEFORMPAINTOP_H_

#include <brushengine/kis_paintop.h>
#include <kis_types.h>

#include <kis_airbrush_option_widget.h>
#include <kis_pressure_size_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_rotation_option.h>
#include <kis_pressure_rate_option.h>

#include "deform_brush.h"

#include "kis_deform_paintop_settings.h"
#include "kis_deform_option.h"

class KisPainter;

class KisDeformPaintOp : public KisPaintOp
{

public:
    KisDeformPaintOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisNodeSP node, KisImageSP image);
    ~KisDeformPaintOp() override;

protected:
    KisSpacingInformation paintAt(const KisPaintInformation& info) override;

    KisSpacingInformation updateSpacingImpl(const KisPaintInformation &info) const override;

    KisTimingInformation updateTimingImpl(const KisPaintInformation &info) const override;

private:

    KisPaintDeviceSP m_dab;
    KisPaintDeviceSP m_dev;

    DeformBrush m_deformBrush;
    DeformOption m_properties;
    KisBrushSizeOptionProperties m_sizeProperties;

    KisAirbrushOptionProperties m_airbrushOption;

    KisPressureSizeOption m_sizeOption;
    KisPressureOpacityOption m_opacityOption;
    KisPressureRotationOption m_rotationOption;
    KisPressureRateOption m_rateOption;

    qreal m_xSpacing;
    qreal m_ySpacing;
    qreal m_spacing;
};

#endif // KIS_DEFORMPAINTOP_H_
