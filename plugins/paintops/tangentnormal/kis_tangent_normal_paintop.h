/*
 *  Copyright (C) 2015 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
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

#ifndef _KIS_TANGENTNORMALPAINTOP_H_
#define _KIS_TANGENTNORMALPAINTOP_H_

#include <QRect>

#include <kis_brush_based_paintop.h>
#include <kis_types.h>

#include <kis_pressure_size_option.h>
#include <kis_tangent_tilt_option.h>
#include <kis_airbrush_option_widget.h>
#include <kis_pressure_flow_opacity_option.h>
#include <kis_pressure_spacing_option.h>
#include <kis_pressure_rate_option.h>
#include <kis_pressure_rotation_option.h>
#include <kis_pressure_scatter_option.h>
#include <kis_pressure_flow_option.h>
#include <kis_pressure_softness_option.h>
#include <kis_pressure_sharpness_option.h>

class KisBrushBasedPaintOpSettings;
class KisPainter;

class KisTangentNormalPaintOp: public KisBrushBasedPaintOp
{
public:
    //public functions//

    /* Create a Tangent Normal Brush Operator*/
    KisTangentNormalPaintOp(const KisPaintOpSettingsSP settings, KisPainter* painter, KisNodeSP node, KisImageSP image);
    ~KisTangentNormalPaintOp() override;

    void paintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2, KisDistanceInformation *currentDistance) override;

protected:
    /*paint the dabs*/
    KisSpacingInformation paintAt(const KisPaintInformation& info) override;

    KisSpacingInformation updateSpacingImpl(const KisPaintInformation &info) const override;

    KisTimingInformation updateTimingImpl(const KisPaintInformation &info) const override;

private:
    KisSpacingInformation computeSpacing(const KisPaintInformation &info, qreal scale,
                                         qreal rotation) const;

private:
    //private functions//
    KisPressureSizeOption m_sizeOption;
    KisFlowOpacityOption m_opacityOption;
    KisPressureSpacingOption m_spacingOption;
    KisPressureRateOption m_rateOption;
    KisPressureRotationOption m_rotationOption;
    KisPressureScatterOption m_scatterOption;
    KisTangentTiltOption m_tangentTiltOption;
    KisAirbrushOptionProperties m_airbrushOption;
    KisPressureSoftnessOption m_softnessOption;
    KisPressureSharpnessOption m_sharpnessOption;
    KisPressureFlowOption m_flowOption;

    KisFixedPaintDeviceSP m_maskDab;
    KisPaintDeviceSP m_tempDev;
    QRect m_dstDabRect;

    KisPaintDeviceSP m_lineCacheDevice;
};
#endif // _KIS_TANGENTNORMALPAINTOP_H_
