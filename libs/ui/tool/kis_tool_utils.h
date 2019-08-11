/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2018 Emmet & Eoin O'Neill <emmetoneill.pdx@gmail.com>
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

#ifndef KIS_TOOL_UTILS_H
#define KIS_TOOL_UTILS_H

#include <kis_types.h>
#include <kritaui_export.h>

class QPoint;
class KoColor;

namespace KisToolUtils {

struct KRITAUI_EXPORT ColorPickerConfig {
    ColorPickerConfig();

    bool toForegroundColor;
    bool updateColor;
    bool addPalette;
    bool normaliseValues;
    bool sampleMerged;
    int radius;
    int blend;

    void save(bool defaultActivation = true) const;
    void load(bool defaultActivation = true);
private:
    static const QString CONFIG_GROUP_NAME;
};

/**
 * Pick a color based on the given position on the given paint device.
 *
 * out_color   - Output parameter returning newly picked color.
 * dev         - Paint device to pick from.
 * pos         - Position to pick from.
 * blendColor  - Optional color to be blended with.
 * radius      - Picking area radius in pixels.
 * blend       - Blend percentage. 100% all picked, 0% all blendColor.
 * pure        - Whether to bypass radius, blending, and active layer settings for pure picking.
 *
 * RETURN      - Returns TRUE whenever a valid color is picked.
 */
bool KRITAUI_EXPORT pickColor(KoColor &out_color, KisPaintDeviceSP dev, const QPoint &pos,
                              KoColor const *const blendColor = nullptr, int radius = 1,
                              int blend = 100, bool pure = false);

/**
 * Recursively search a node with a non-transparent pixel
 */
KisNodeSP KRITAUI_EXPORT findNode(KisNodeSP node, const QPoint &point, bool wholeGroup, bool editableOnly = true);

/**
 * return true if success
 * Clears the image. Selection is optional, use 0 to clear everything.
 */
bool KRITAUI_EXPORT clearImage(KisImageSP image, KisNodeSP node, KisSelectionSP selection);
}

#endif // KIS_TOOL_UTILS_H
