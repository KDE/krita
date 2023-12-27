/*
 *  SPDX-FileCopyrightText: 2008, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_DEFORMPAINTOP_H_
#define KIS_DEFORMPAINTOP_H_

#include <brushengine/kis_paintop.h>
#include <kis_types.h>

#include <KisAirbrushOptionData.h>
#include <KisOpacityOption.h>
#include <KisRotationOption.h>

#include "deform_brush.h"

#include "kis_deform_paintop_settings.h"
#include "KisDeformOptionData.h"
#include "KisBrushSizeOptionData.h"

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
    KisDeformOptionData m_deformData;
    KisBrushSizeOptionData m_brushSizeData;

    KisAirbrushOptionData m_airbrushData;

    KisSizeOption m_sizeOption;
    KisOpacityOption m_opacityOption;
    KisRotationOption m_rotationOption;
    KisRateOption m_rateOption;

    qreal m_xSpacing;
    qreal m_ySpacing;
    qreal m_spacing;
};

#endif // KIS_DEFORMPAINTOP_H_
