/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_LS_OVERLAY_FILTER_H
#define KIS_LS_OVERLAY_FILTER_H

#include <QSharedPointer>

#include "kis_layer_style_filter.h"
#include <kritaimage_export.h>

struct psd_layer_effects_overlay_base;


class KRITAIMAGE_EXPORT KisLsOverlayFilter : public KisLayerStyleFilter
{
public:
    enum Mode {
        Color,
        Gradient,
        Pattern
    };

public:
    KisLsOverlayFilter(Mode mode);

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
    KisLsOverlayFilter(const KisLsOverlayFilter &rhs);

    const psd_layer_effects_overlay_base* getOverlayStruct(KisPSDLayerStyleSP style) const;

    void applyOverlay(KisPaintDeviceSP srcDevice,
                      KisMultipleProjection *dst,
                      const QRect &applyRect,
                      const psd_layer_effects_overlay_base *config, KisResourcesInterfaceSP resourcesInterface,
                      KisLayerStyleFilterEnvironment *env) const;

private:
    Mode m_mode;
};

#endif
