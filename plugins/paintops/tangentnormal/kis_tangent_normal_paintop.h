/*
 *  SPDX-FileCopyrightText: 2015 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
