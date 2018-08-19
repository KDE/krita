/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef KIS_SKETCH_PAINTOP_SETTINGS_H_
#define KIS_SKETCH_PAINTOP_SETTINGS_H_

#include <kis_brush_based_paintop_settings.h>
#include <kis_types.h>

#include "kis_sketch_paintop_settings_widget.h"

#include <kis_pressure_opacity_option.h>


class KisSketchPaintOpSettings : public KisBrushBasedPaintOpSettings
{

public:
    KisSketchPaintOpSettings();
    ~KisSketchPaintOpSettings() override {}

    QPainterPath brushOutline(const KisPaintInformation &info, const OutlineMode &mode) override;

    bool paintIncremental() override;
};

typedef KisSharedPtr<KisSketchPaintOpSettings> KisSketchPaintOpSettingsSP;

#endif
