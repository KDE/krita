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

#include <KisTangentTiltOption.h>
#include <KisTangentTiltOptionData.h>

#include <KisStandardOptions.h>
#include <KisFlowOpacityOption.h>
#include <KisSpacingOption.h>
#include <KisSharpnessOption.h>
#include <KisScatterOption.h>
#include <KisRotationOption.h>
#include <KisAirbrushOptionData.h>

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
    KisTangentTiltOption m_tangentTiltOption;

    KisFlowOpacityOption2 m_opacityOption;
    KisFlowOption m_flowOption;
    KisSizeOption m_sizeOption;
    KisSpacingOption m_spacingOption;
    KisSoftnessOption m_softnessOption;
    KisSharpnessOption m_sharpnessOption;
    KisScatterOption m_scatterOption;
    KisRotationOption m_rotationOption;
    KisAirbrushOptionData m_airbrushData;
    KisRateOption m_rateOption;

    KisFixedPaintDeviceSP m_maskDab;
    KisPaintDeviceSP m_tempDev;
    QRect m_dstDabRect;

    KisPaintDeviceSP m_lineCacheDevice;
};
#endif // _KIS_TANGENTNORMALPAINTOP_H_
