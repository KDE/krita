/* This file is part of the KDE project
 *
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#include "DefaultToolFactory.h"
#include "DefaultTool.h"

#include <kis_action_registry.h>

#include <KoIcon.h>
#include <klocalizedstring.h>

DefaultToolFactory::DefaultToolFactory()
    : KoToolFactoryBase(KoInteractionTool_ID)
{
    setToolTip(i18n("Select Shapes Tool"));
    setSection(mainToolType());
    setPriority(0);
    setIconName(koIconNameCStr("select"));
    setActivationShapeId("flake/always");
}

DefaultToolFactory::DefaultToolFactory(const QString &id)
    : KoToolFactoryBase(id)
{
}

DefaultToolFactory::~DefaultToolFactory()
{
}

KoToolBase *DefaultToolFactory::createTool(KoCanvasBase *canvas)
{
    return new DefaultTool(canvas, true);
}

QList<QAction *> DefaultToolFactory::createActionsImpl()
{
    KisActionRegistry *actionRegistry = KisActionRegistry::instance();

    QList<QAction *> actions;
    actions << actionRegistry->makeQAction("object_order_front");
    actions << actionRegistry->makeQAction("object_order_raise");
    actions << actionRegistry->makeQAction("object_order_lower");
    actions << actionRegistry->makeQAction("object_order_back");
    actions << actionRegistry->makeQAction("object_align_horizontal_left");
    actions << actionRegistry->makeQAction("object_align_horizontal_center");
    actions << actionRegistry->makeQAction("object_align_horizontal_right");
    actions << actionRegistry->makeQAction("object_align_vertical_top");
    actions << actionRegistry->makeQAction("object_align_vertical_center");
    actions << actionRegistry->makeQAction("object_align_vertical_bottom");
    actions << actionRegistry->makeQAction("object_distribute_horizontal_left");
    actions << actionRegistry->makeQAction("object_distribute_horizontal_center");
    actions << actionRegistry->makeQAction("object_distribute_horizontal_right");
    actions << actionRegistry->makeQAction("object_distribute_horizontal_gaps");
    actions << actionRegistry->makeQAction("object_distribute_vertical_top");
    actions << actionRegistry->makeQAction("object_distribute_vertical_center");
    actions << actionRegistry->makeQAction("object_distribute_vertical_bottom");
    actions << actionRegistry->makeQAction("object_distribute_vertical_gaps");
    actions << actionRegistry->makeQAction("object_group");
    actions << actionRegistry->makeQAction("object_ungroup");
    actions << actionRegistry->makeQAction("object_transform_rotate_90_cw");
    actions << actionRegistry->makeQAction("object_transform_rotate_90_ccw");
    actions << actionRegistry->makeQAction("object_transform_rotate_180");
    actions << actionRegistry->makeQAction("object_transform_mirror_horizontally");
    actions << actionRegistry->makeQAction("object_transform_mirror_vertically");
    actions << actionRegistry->makeQAction("object_transform_reset");
    actions << actionRegistry->makeQAction("object_unite");
    actions << actionRegistry->makeQAction("object_intersect");
    actions << actionRegistry->makeQAction("object_subtract");
    actions << actionRegistry->makeQAction("object_split");

    return actions;

}
