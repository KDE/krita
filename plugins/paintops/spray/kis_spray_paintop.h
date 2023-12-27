/*
 *  SPDX-FileCopyrightText: 2008-2012 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_SPRAY_PAINTOP_H_
#define KIS_SPRAY_PAINTOP_H_

#include <brushengine/kis_paintop.h>
#include <kis_types.h>

#include "spray_brush.h"
#include "kis_spray_paintop_settings.h"
#include "kis_brush_option.h"
#include <KisAirbrushOptionData.h>
#include <KisOpacityOption.h>
#include <KisRotationOption.h>
#include <KisSprayShapeDynamicsOptionData.h>
#include "KisSprayOpOption.h"
#include "KisSprayShapeOptionData.h"


class KisPainter;


class KisSprayPaintOp : public KisPaintOp
{

public:

    KisSprayPaintOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisNodeSP node, KisImageSP image);
    ~KisSprayPaintOp() override;

    static QList<KoResourceLoadResult> prepareLinkedResources(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface);

protected:

    KisSpacingInformation paintAt(const KisPaintInformation& info) override;

    KisSpacingInformation updateSpacingImpl(const KisPaintInformation &info) const override;

    KisTimingInformation updateTimingImpl(const KisPaintInformation &info) const override;

private:
    KisSpacingInformation computeSpacing(const KisPaintInformation &info, qreal lodScale) const;

private:
    KisSprayShapeOptionData m_shapeProperties;
    KisSprayOpOption m_sprayOpOption;
    KisSprayShapeDynamicsOptionData m_shapeDynamicsProperties;
    KisColorOptionData m_colorProperties;
    KisBrushOptionProperties m_brushOption;

    KisPaintDeviceSP m_dab;
    SprayBrush m_sprayBrush;
    qreal m_xSpacing, m_ySpacing, m_spacing;
    bool m_isPresetValid;
    KisAirbrushOptionData m_airbrushData;

    KisRotationOption m_rotationOption;
    KisSizeOption m_sizeOption;
    KisOpacityOption m_opacityOption;
    KisRateOption m_rateOption;
    KisNodeSP m_node;
};

#endif // KIS_SPRAY_PAINTOP_H_
