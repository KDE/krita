/*
 *  SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "KisSelectionToolFactoryBase.h"

#include <kis_action_registry.h>

KisSelectionToolFactoryBase::KisSelectionToolFactoryBase(const QString &id)
    : KisToolPaintFactoryBase(id)
{
}

KisSelectionToolFactoryBase::~KisSelectionToolFactoryBase()
{
}

QList<QAction *> KisSelectionToolFactoryBase::createActionsImpl()
{
    KisActionRegistry *actionRegistry = KisActionRegistry::instance();
    QList<QAction *> actions = KisToolPaintFactoryBase::createActionsImpl();

    actions << actionRegistry->makeQAction("selection_tool_mode_add");
    actions << actionRegistry->makeQAction("selection_tool_mode_replace");
    actions << actionRegistry->makeQAction("selection_tool_mode_subtract");
    actions << actionRegistry->makeQAction("selection_tool_mode_intersect");

    return actions;
}

KisToolPolyLineFactoryBase::KisToolPolyLineFactoryBase(const QString &id)
    : KisToolPaintFactoryBase(id)
{
}

KisToolPolyLineFactoryBase::~KisToolPolyLineFactoryBase()
{

}

QList<QAction *> KisToolPolyLineFactoryBase::createActionsImpl()
{
    KisActionRegistry *actionRegistry = KisActionRegistry::instance();
    QList<QAction *> actions = KisToolPaintFactoryBase::createActionsImpl();

    actions << actionRegistry->makeQAction("undo_polygon_selection");
    actions << actionRegistry->makeQAction("selection_tool_mode_add");

    return actions;
}
