/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_LS_STROKE_FILTER_H
#define KIS_LS_STROKE_FILTER_H

#include <QSharedPointer>
#include <kritaimage_export.h>

#include "kis_layer_style_filter.h"
#include "krita_utils.h"

struct psd_layer_effects_stroke;


class KRITAIMAGE_EXPORT KisLsStrokeFilter : public KisLayerStyleFilter
{
public:
    KisLsStrokeFilter();

    KisLayerStyleFilter* clone() const override;

    void processDirectly(KisPaintDeviceSP src,
                         KisMultipleProjection *dst,
                         KisLayerStyleKnockoutBlower *blower,
                         const QRect &applyRect,
                         KisPSDLayerStyleSP style,
                         KisLayerStyleFilterEnvironment *env) const override;

    QRect neededRect(const QRect & rect, KisPSDLayerStyleSP style, KisLayerStyleFilterEnvironment *env) const override;
    QRect changedRect(const QRect & rect, KisPSDLayerStyleSP style, KisLayerStyleFilterEnvironment *env) const override;

    KritaUtils::ThresholdMode sourcePlaneOpacityThresholdRequirement(KisPSDLayerStyleSP style) const;

private:
    KisLsStrokeFilter(const KisLsStrokeFilter &rhs);

    void applyStroke(KisPaintDeviceSP srcDevice,
                     KisMultipleProjection *dst,
                     KisLayerStyleKnockoutBlower *blower,
                     const QRect &applyRect,
                     const psd_layer_effects_stroke *config,
                     KisResourcesInterfaceSP resourcesInterface,
                     KisLayerStyleFilterEnvironment *env) const;
};

#endif
