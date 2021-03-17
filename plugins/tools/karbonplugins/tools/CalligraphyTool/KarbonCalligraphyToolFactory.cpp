/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KarbonCalligraphyToolFactory.h"
#include "KarbonCalligraphyTool.h"

#include <KoToolRegistry.h>
#include <kis_action_registry.h>

#include <KoIcon.h>
#include <klocalizedstring.h>
#include <QDebug>

KarbonCalligraphyToolFactory::KarbonCalligraphyToolFactory()
    : KoToolFactoryBase("KarbonCalligraphyTool")
{
    setToolTip(i18n("Calligraphy"));
    setSection(ToolBoxSection::Main);
    setIconName(koIconNameCStr("calligraphy"));
    setPriority(6);
    setActivationShapeId("flake/edit");
}

KarbonCalligraphyToolFactory::~KarbonCalligraphyToolFactory()
{
}

KoToolBase *KarbonCalligraphyToolFactory::createTool(KoCanvasBase *canvas)
{
    return new KarbonCalligraphyTool(canvas);
}

QList<QAction *> KarbonCalligraphyToolFactory::createActionsImpl()
{
    KisActionRegistry *actionRegistry = KisActionRegistry::instance();
    QList<QAction *> actions;

    actions << actionRegistry->makeQAction("calligraphy_increase_width");
    actions << actionRegistry->makeQAction("calligraphy_decrease_width");
    actions << actionRegistry->makeQAction("calligraphy_increase_angle");
    actions << actionRegistry->makeQAction("calligraphy_decrease_angle");

    return actions;
}
