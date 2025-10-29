/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
    setSection(ToolBoxSection::Main);
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
    actions << actionRegistry->makeQAction("pathpoint-corner", this);
    actions << actionRegistry->makeQAction("pathpoint-smooth", this);
    actions << actionRegistry->makeQAction("pathpoint-symmetric", this);
    actions << actionRegistry->makeQAction("pathpoint-curve", this);
    actions << actionRegistry->makeQAction("pathpoint-line", this);
    actions << actionRegistry->makeQAction("pathsegment-line", this);
    actions << actionRegistry->makeQAction("pathsegment-curve", this);
    actions << actionRegistry->makeQAction("pathpoint-insert", this);
    actions << actionRegistry->makeQAction("pathpoint-remove", this);
    actions << actionRegistry->makeQAction("path-break-point", this);
    actions << actionRegistry->makeQAction("path-break-segment", this);
    actions << actionRegistry->makeQAction("path-break-selection", this);
    actions << actionRegistry->makeQAction("pathpoint-join", this);
    actions << actionRegistry->makeQAction("pathpoint-merge", this);
    actions << actionRegistry->makeQAction("convert-to-path", this);
    return actions;
}
