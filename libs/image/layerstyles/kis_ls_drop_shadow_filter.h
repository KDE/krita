/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_LS_DROP_SHADOW_FILTER_H
#define KIS_LS_DROP_SHADOW_FILTER_H

#include <QSharedPointer>

#include "kis_layer_style_filter.h"
#include <kritaimage_export.h>

class psd_layer_effects_shadow_base;


class KRITAIMAGE_EXPORT KisLsDropShadowFilter : public KisLayerStyleFilter
{
public:

    enum Mode {
        DropShadow,
        InnerShadow,
        OuterGlow,
        InnerGlow
    };

    KisLsDropShadowFilter(Mode mode = DropShadow);

    KisLayerStyleFilter* clone() const override;

    void processDirectly(KisPaintDeviceSP src,
                         KisMultipleProjection *dst,
                         KisLayerStyleKnockoutBlower *blower,
                         const QRect &applyRect,
                         KisPSDLayerStyleSP style,
                         KisLayerStyleFilterEnvironment *env) const override;

    QRect neededRect(const QRect & rect, KisPSDLayerStyleSP style, KisLayerStyleFilterEnvironment *env) const override;
    QRect changedRect(const QRect & rect, KisPSDLayerStyleSP style, KisLayerStyleFilterEnvironment *env) const override;

private:
    KisLsDropShadowFilter(const KisLsDropShadowFilter &rhs);
    const psd_layer_effects_shadow_base* getShadowStruct(KisPSDLayerStyleSP style) const;

    void applyDropShadow(KisPaintDeviceSP srcDevice,
                         KisMultipleProjection *dst,
                         const QRect &applyRect,
                         const psd_layer_effects_context *context,
                         const psd_layer_effects_shadow_base *shadow, KisResourcesInterfaceSP resourcesIntrerface,
                         KisLayerStyleFilterEnvironment *env) const;

private:
    const Mode m_mode;
};

#endif
