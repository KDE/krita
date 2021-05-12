/*
 * SPDX-FileCopyrightText: 2021 Halla Rempt <halla@valdyas.org>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "KisResourceTypes.h"
#include <QMap>
#include <QDebug>

QString ResourceName::resourceTypeToName(const QString &resourceType)
{
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

    Q_ASSERT(typeMap.contains(resourceType));

    return typeMap[resourceType];

}
