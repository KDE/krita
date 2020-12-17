/*
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2018 Emmet & Eoin O'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    bool addColorToCurrentPalette;
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
