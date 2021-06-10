/*
 * SPDX-FileCopyrightText: 2021 Halla Rempt <halla@valdyas.org>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "KisResourceTypes.h"
#include <QCoreApplication>
#include <QMap>
#include <QDebug>

#include <ResourceDebug.h>

namespace ResourceType {
    const QString PaintOpPresets {QStringLiteral("paintoppresets")};
    const QString Brushes {QStringLiteral("brushes")};
    const QString Gradients {QStringLiteral("gradients")};
    const QString Palettes {QStringLiteral("palettes")};
    const QString Patterns {QStringLiteral("patterns")};
    const QString Workspaces {QStringLiteral("workspaces")};
    const QString Symbols {QStringLiteral("symbols")};
    const QString WindowLayouts {QStringLiteral("windowlayouts")};
    const QString Sessions {QStringLiteral("sessions")};
    const QString GamutMasks {QStringLiteral("gamutmasks")};
    const QString SeExprScripts {QStringLiteral("seexpr_scripts")};
    const QString FilterEffects {QStringLiteral("ko_effects")};
    const QString TaskSets {QStringLiteral("tasksets")};
    const QString LayerStyles {QStringLiteral("layerstyles")};
}

namespace ResourceSubType {
    const QString AbrBrushes {QStringLiteral("abr_brushes")};
    const QString GbrBrushes {QStringLiteral("gbr_brushes")};
    const QString GihBrushes {QStringLiteral("gih_brushes")};
    const QString SvgBrushes {QStringLiteral("svg_brushes")};
    const QString PngBrushes {QStringLiteral("png_brushes")};
    const QString SegmentedGradients {QStringLiteral("segmented_gradients")};
    const QString StopGradients {QStringLiteral("stop_gradients")};
    const QString KritaPaintOpPresets {QStringLiteral("krita_paintop_presets")};
    const QString MyPaintPaintOpPresets {QStringLiteral("mypaint_paintop_presets")};
}

namespace ResourceName {
    const KLocalizedString PaintOpPresets = ki18nc("resource type", "Brush Presets");
    const KLocalizedString Brushes = ki18nc("resource type", "Brush Tips");
    const KLocalizedString Gradients = ki18nc("resource type", "Gradients");
    const KLocalizedString Palettes = ki18nc("resource type", "Palettes");
    const KLocalizedString Patterns = ki18nc("resource type", "Patterns");
    const KLocalizedString Workspaces = ki18nc("resource type", "Workspaces");
    const KLocalizedString Symbols = ki18nc("resource type", "Symbol Libraries");
    const KLocalizedString WindowLayouts = ki18nc("resource type", "Window Layouts");
    const KLocalizedString Sessions = ki18nc("resource type", "Sessions");
    const KLocalizedString GamutMasks = ki18nc("resource type", "Gamut Masks");
    const KLocalizedString SeExprScripts = ki18nc("resource type", "SeExpr Scripts");
    const KLocalizedString FilterEffects = ki18nc("resource type", "Filter Effects");
    const KLocalizedString TaskSets = ki18nc("resource type", "Task Sets");
    const KLocalizedString LayerStyles = ki18nc("resource type", "Layer Styles");
}

QString ResourceName::resourceTypeToName(const QString &resourceType)
{
    static const QMap<QString, QString> typeMap = []() {
        if (!QCoreApplication::instance()) {
            warnResource << "QCoreApplication not valid when initializing resourceTypeNameMap in" << __FILE__ << "line" << __LINE__;
        }
        QMap<QString, QString> typeMap;
        typeMap[ResourceType::PaintOpPresets] = ResourceName::PaintOpPresets.toString();
        typeMap[ResourceType::Brushes] = ResourceName::Brushes.toString();
        typeMap[ResourceType::Gradients] = ResourceName::Gradients.toString();
        typeMap[ResourceType::Palettes] = ResourceName::Palettes.toString();
        typeMap[ResourceType::Patterns] = ResourceName::Patterns.toString();
        typeMap[ResourceType::Workspaces] = ResourceName::Workspaces.toString();
        typeMap[ResourceType::Symbols] = ResourceName::Symbols.toString();
        typeMap[ResourceType::WindowLayouts] = ResourceName::WindowLayouts.toString();
        typeMap[ResourceType::Sessions] = ResourceName::Sessions.toString();
        typeMap[ResourceType::GamutMasks] = ResourceName::GamutMasks.toString();
        typeMap[ResourceType::SeExprScripts] = ResourceName::SeExprScripts.toString();
        typeMap[ResourceType::FilterEffects] = ResourceName::FilterEffects.toString();
        typeMap[ResourceType::TaskSets] = ResourceName::TaskSets.toString();
        typeMap[ResourceType::LayerStyles] = ResourceName::LayerStyles.toString();
        return typeMap;
    }();

    Q_ASSERT(typeMap.contains(resourceType));

    return typeMap[resourceType];

}
