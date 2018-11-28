/*
 *  Copyright (c) 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
    : KisSelectionToolFactoryBase(id)
{

}

KisToolPolyLineFactoryBase::~KisToolPolyLineFactoryBase()
{

}

QList<QAction *> KisToolPolyLineFactoryBase::createActionsImpl()
{
    KisActionRegistry *actionRegistry = KisActionRegistry::instance();
    QList<QAction *> actions = KisSelectionToolFactoryBase::createActionsImpl();

    actions << actionRegistry->makeQAction("undo_polygon_selection");
    actions << actionRegistry->makeQAction("selection_tool_mode_add");

    return actions;
}
