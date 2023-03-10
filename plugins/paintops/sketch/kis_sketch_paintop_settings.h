/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_SKETCH_PAINTOP_SETTINGS_H_
#define KIS_SKETCH_PAINTOP_SETTINGS_H_

#include <kis_brush_based_paintop_settings.h>
#include <kis_types.h>

#include "kis_sketch_paintop_settings_widget.h"


class KisSketchPaintOpSettings : public KisBrushBasedPaintOpSettings
{

public:
    KisSketchPaintOpSettings(KisResourcesInterfaceSP resourcesInterface);
    ~KisSketchPaintOpSettings() override {}

    KisOptimizedBrushOutline brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom) override;

    bool paintIncremental() override;

    bool hasPatternSettings() const override;
};

typedef KisSharedPtr<KisSketchPaintOpSettings> KisSketchPaintOpSettingsSP;

#endif
