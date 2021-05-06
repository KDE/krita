/*
 *  SPDX-FileCopyrightText: 2019 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KISRESOURCETYPES_H
#define KISRESOURCETYPES_H

#include <klocalizedstring.h>
#include "kritaresources_export.h"
/**
 * These namespaces define the type keys and sub-type keys for resource types.
 * The type keys correspond to folders in the resource folder, the sub-type
 * keys to different types that have their own resource loader instance.
 */
namespace ResourceType {
    static const QString PaintOpPresets {"paintoppresets"};
    static const QString Brushes {"brushes"};
    static const QString Gradients {"gradients"};
    static const QString Palettes {"palettes"};
    static const QString Patterns {"patterns"};
    static const QString Workspaces {"workspaces"};
    static const QString Symbols {"symbols"};
    static const QString WindowLayouts {"windowlayouts"};
    static const QString Sessions {"sessions"};
    static const QString GamutMasks {"gamutmasks"};
    static const QString SeExprScripts {"seexpr_scripts"};
    static const QString FilterEffects {"ko_effects"};
    static const QString TaskSets {"tasksets"};
    static const QString LayerStyles {"layerstyles"};
}

namespace ResourceSubType {
    static const QString AbrBrushes {"abr_brushes"};
    static const QString GbrBrushes {"gbr_brushes"};
    static const QString GihBrushes {"gih_brushes"};
    static const QString SvgBrushes {"svg_brushes"};
    static const QString PngBrushes {"png_brushes"};
    static const QString SegmentedGradients {"segmented_gradients"};
    static const QString StopGradients {"stop_gradients"};
    static const QString KritaPaintOpPresets {"krita_paintop_presets"};
    static const QString MyPaintPaintOpPresets {"mypaint_paintop_presets"};
}

namespace ResourceName {
    static const KLocalizedString PaintOpPresets = ki18nc("resource type", "Brush Presets");
    static const KLocalizedString Brushes = ki18nc("resource type", "Brush Tips");
    static const KLocalizedString Gradients = ki18nc("resource type", "Gradients");
    static const KLocalizedString Palettes = ki18nc("resource type", "Palettes");
    static const KLocalizedString Patterns = ki18nc("resource type", "Patterns");
    static const KLocalizedString Workspaces = ki18nc("resource type", "Workspaces");
    static const KLocalizedString Symbols = ki18nc("resource type", "Symbol Libraries");
    static const KLocalizedString WindowLayouts = ki18nc("resource type", "Window Layouts");
    static const KLocalizedString Sessions = ki18nc("resource type", "Sessions");
    static const KLocalizedString GamutMasks = ki18nc("resource type", "Gamut Masks");
    static const KLocalizedString SeExprScripts = ki18nc("resource type", "SeExpr Scripts");
    static const KLocalizedString FilterEffects = ki18nc("resource type", "Filter Effects");
    static const KLocalizedString TaskSets = ki18nc("resource type", "Task Sets");
    static const KLocalizedString LayerStyles = ki18nc("resource type", "Layer Styles");

    KRITARESOURCES_EXPORT QString resourceTypeToName(const QString &resourceType);

}



#endif // KISRESOURCETYPES_H
