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
