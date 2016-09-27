/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
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

#include "DualBrushPaintopSettings.h"
#include "DualBrushOption.h"
#include "DualBrushProperties.h"
#include <kis_paint_action_type_option.h>
#include "kis_paintop_preset.h"

DualBrushPaintOpSettings::DualBrushPaintOpSettings()
{
}

bool DualBrushPaintOpSettings::paintIncremental()
{
    //return (enumPaintActionType)getInt("PaintOpAction", WASH) == BUILDUP;
    return false;
}

bool DualBrushPaintOpSettings::isAirbrushing() const
{
    // return true if one of the constituent brushes is an airbrush
    return false;
}

int DualBrushPaintOpSettings::rate() const
{
    return 100;
}

void DualBrushPaintOpSettings::setPaintOpSize(qreal value)
{
    DualBrushProperties prop;
    prop.readOptionSetting(this);
    if (prop.presetStack.isEmpty()) return;

    KisPaintOpPresetSP mainPreset = prop.presetStack.first().paintopPreset;
    const qreal oldSize = mainPreset->settings()->paintOpSize();
    const qreal scaleFactor = value / oldSize;

    auto it = prop.presetStack.begin();
    auto end = prop.presetStack.end();
    for (; it != end; ++it) {
        KisPaintOpSettingsSP settings = it->paintopPreset->settings();
        settings->setPaintOpSize(scaleFactor * settings->paintOpSize());
    }

    prop.writeOptionSetting(this);
}

qreal DualBrushPaintOpSettings::paintOpSize() const
{
    DualBrushProperties prop;
    prop.readOptionSetting(this);
    if (prop.presetStack.isEmpty()) return 1.0;

    KisPaintOpPresetSP mainPreset = prop.presetStack.first().paintopPreset;
    return mainPreset->settings()->paintOpSize();
}

QPainterPath DualBrushPaintOpSettings::brushOutline(const KisPaintInformation &info, OutlineMode mode)
{
    DualBrushProperties prop;
    prop.readOptionSetting(this);
    if (prop.presetStack.isEmpty()) return KisPaintOpSettings::brushOutline(info, mode);

    KisPaintOpPresetSP mainPreset = prop.presetStack.first().paintopPreset;
    return mainPreset->settings()->brushOutline(info, mode);
}
