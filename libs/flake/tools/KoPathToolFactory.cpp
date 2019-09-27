/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
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

#include "KoPathToolFactory.h"
#include "KoPathTool.h"
#include "KoPathShape.h"
#include <kis_action_registry.h>

#include <KoIcon.h>
#include <klocalizedstring.h>

KoPathToolFactory::KoPathToolFactory()
        : KoToolFactoryBase("PathTool")
{
    setToolTip(i18n("Edit Shapes Tool"));
    setSection(mainToolType());
    setIconName(koIconNameCStr("shape_handling"));
    setPriority(2);
    setActivationShapeId("flake/always,KoPathShape");
}

KoPathToolFactory::~KoPathToolFactory()
{
}

KoToolBase * KoPathToolFactory::createTool(KoCanvasBase *canvas)
{
    return new KoPathTool(canvas);
}

QList<QAction *> KoPathToolFactory::createActionsImpl()
{
    KisActionRegistry *actionRegistry = KisActionRegistry::instance();
    QList<QAction *> actions;
    actions << actionRegistry->makeQAction("pathpoint-corner");
    actions << actionRegistry->makeQAction("pathpoint-smooth");
    actions << actionRegistry->makeQAction("pathpoint-symmetric");
    actions << actionRegistry->makeQAction("pathpoint-curve");
    actions << actionRegistry->makeQAction("pathpoint-line");
    actions << actionRegistry->makeQAction("pathsegment-line");
    actions << actionRegistry->makeQAction("pathsegment-curve");
    actions << actionRegistry->makeQAction("pathpoint-insert");
    actions << actionRegistry->makeQAction("pathpoint-remove");
    actions << actionRegistry->makeQAction("path-break-point");
    actions << actionRegistry->makeQAction("path-break-segment");
    actions << actionRegistry->makeQAction("pathpoint-join");
    actions << actionRegistry->makeQAction("pathpoint-merge");
    actions << actionRegistry->makeQAction("convert-to-path");
    return actions;
}
