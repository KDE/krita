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

    static QList<KoResourceSP> prepareLinkedResources(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface);

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
