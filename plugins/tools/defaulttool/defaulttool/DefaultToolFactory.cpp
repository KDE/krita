/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
    setSection(ToolBoxSection::Main);
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
    actions << actionRegistry->makeQAction("object_order_front", this);
    actions << actionRegistry->makeQAction("object_order_raise", this);
    actions << actionRegistry->makeQAction("object_order_lower", this);
    actions << actionRegistry->makeQAction("object_order_back", this);
    actions << actionRegistry->makeQAction("object_align_horizontal_left", this);
    actions << actionRegistry->makeQAction("object_align_horizontal_center", this);
    actions << actionRegistry->makeQAction("object_align_horizontal_right", this);
    actions << actionRegistry->makeQAction("object_align_vertical_top", this);
    actions << actionRegistry->makeQAction("object_align_vertical_center", this);
    actions << actionRegistry->makeQAction("object_align_vertical_bottom", this);
    actions << actionRegistry->makeQAction("object_distribute_horizontal_left", this);
    actions << actionRegistry->makeQAction("object_distribute_horizontal_center", this);
    actions << actionRegistry->makeQAction("object_distribute_horizontal_right", this);
    actions << actionRegistry->makeQAction("object_distribute_horizontal_gaps", this);
    actions << actionRegistry->makeQAction("object_distribute_vertical_top", this);
    actions << actionRegistry->makeQAction("object_distribute_vertical_center", this);
    actions << actionRegistry->makeQAction("object_distribute_vertical_bottom", this);
    actions << actionRegistry->makeQAction("object_distribute_vertical_gaps", this);
    actions << actionRegistry->makeQAction("object_group", this);
    actions << actionRegistry->makeQAction("object_ungroup", this);
    actions << actionRegistry->makeQAction("object_transform_rotate_90_cw", this);
    actions << actionRegistry->makeQAction("object_transform_rotate_90_ccw", this);
    actions << actionRegistry->makeQAction("object_transform_rotate_180", this);
    actions << actionRegistry->makeQAction("object_transform_mirror_horizontally", this);
    actions << actionRegistry->makeQAction("object_transform_mirror_vertically", this);
    actions << actionRegistry->makeQAction("object_transform_reset", this);
    actions << actionRegistry->makeQAction("object_unite", this);
    actions << actionRegistry->makeQAction("object_intersect", this);
    actions << actionRegistry->makeQAction("object_subtract", this);
    actions << actionRegistry->makeQAction("object_split", this);

    actions << actionRegistry->makeQAction("text_type_preformatted", this);
    actions << actionRegistry->makeQAction("text_type_pre_positioned", this);
    actions << actionRegistry->makeQAction("text_type_inline_wrap", this);

    return actions;

}
