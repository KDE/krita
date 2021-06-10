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
    KRITARESOURCES_EXPORT extern const QString PaintOpPresets;
    KRITARESOURCES_EXPORT extern const QString Brushes;
    KRITARESOURCES_EXPORT extern const QString Gradients;
    KRITARESOURCES_EXPORT extern const QString Palettes;
    KRITARESOURCES_EXPORT extern const QString Patterns;
    KRITARESOURCES_EXPORT extern const QString Workspaces;
    KRITARESOURCES_EXPORT extern const QString Symbols;
    KRITARESOURCES_EXPORT extern const QString WindowLayouts;
    KRITARESOURCES_EXPORT extern const QString Sessions;
    KRITARESOURCES_EXPORT extern const QString GamutMasks;
    KRITARESOURCES_EXPORT extern const QString SeExprScripts;
    KRITARESOURCES_EXPORT extern const QString FilterEffects;
    KRITARESOURCES_EXPORT extern const QString TaskSets;
    KRITARESOURCES_EXPORT extern const QString LayerStyles;
}

namespace ResourceSubType {
    KRITARESOURCES_EXPORT extern const QString AbrBrushes;
    KRITARESOURCES_EXPORT extern const QString GbrBrushes;
    KRITARESOURCES_EXPORT extern const QString GihBrushes;
    KRITARESOURCES_EXPORT extern const QString SvgBrushes;
    KRITARESOURCES_EXPORT extern const QString PngBrushes;
    KRITARESOURCES_EXPORT extern const QString SegmentedGradients;
    KRITARESOURCES_EXPORT extern const QString StopGradients;
    KRITARESOURCES_EXPORT extern const QString KritaPaintOpPresets;
    KRITARESOURCES_EXPORT extern const QString MyPaintPaintOpPresets;
}

namespace ResourceName {
    KRITARESOURCES_EXPORT extern const KLocalizedString PaintOpPresets;
    KRITARESOURCES_EXPORT extern const KLocalizedString Brushes;
    KRITARESOURCES_EXPORT extern const KLocalizedString Gradients;
    KRITARESOURCES_EXPORT extern const KLocalizedString Palettes;
    KRITARESOURCES_EXPORT extern const KLocalizedString Patterns;
    KRITARESOURCES_EXPORT extern const KLocalizedString Workspaces;
    KRITARESOURCES_EXPORT extern const KLocalizedString Symbols;
    KRITARESOURCES_EXPORT extern const KLocalizedString WindowLayouts;
    KRITARESOURCES_EXPORT extern const KLocalizedString Sessions;
    KRITARESOURCES_EXPORT extern const KLocalizedString GamutMasks;
    KRITARESOURCES_EXPORT extern const KLocalizedString SeExprScripts;
    KRITARESOURCES_EXPORT extern const KLocalizedString FilterEffects;
    KRITARESOURCES_EXPORT extern const KLocalizedString TaskSets;
    KRITARESOURCES_EXPORT extern const KLocalizedString LayerStyles;

    KRITARESOURCES_EXPORT QString resourceTypeToName(const QString &resourceType);

}



#endif // KISRESOURCETYPES_H
