/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_LS_BEVEL_EMBOSS_FILTER_H
#define KIS_LS_BEVEL_EMBOSS_FILTER_H

#include <QSharedPointer>

#include "kis_layer_style_filter.h"
#include <kritaimage_export.h>

struct psd_layer_effects_bevel_emboss;


class KRITAIMAGE_EXPORT KisLsBevelEmbossFilter : public KisLayerStyleFilter
{
public:
    KisLsBevelEmbossFilter();

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
    KisLsBevelEmbossFilter(const KisLsBevelEmbossFilter &rhs);

    void applyBevelEmboss(KisPaintDeviceSP srcDevice,
                          KisMultipleProjection *dst,
                          const QRect &applyRect,
                          const psd_layer_effects_bevel_emboss *config, KisResourcesInterfaceSP resourcesInterface,
                          KisLayerStyleFilterEnvironment *env) const;
};

#endif
