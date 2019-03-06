/*
 *  Copyright (c) 2019 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KISRESOURCETYPES_H
#define KISRESOURCETYPES_H

#include <QString>

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
    static const QString FilterEffects {"ko_effects"};
    static const QString TaskSets {"tasksets"};
}

namespace ResourceSubType {
    static const QString GbrBrushes {"gbr_brushes"};
    static const QString GihBrushes {"gih_brushes"};
    static const QString SvgBrushes {"svg_brushes"};
    static const QString PngBrushes {"png_brushes"};
    static const QString SegmentedGradients {"segmented_gradients"};
    static const QString StopGradients {"stop_gradients"};
}

#endif // KISRESOURCETYPES_H
