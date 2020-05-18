/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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
                      const psd_layer_effects_overlay_base *config,
                      KisLayerStyleFilterEnvironment *env) const;

private:
    Mode m_mode;
};

#endif
