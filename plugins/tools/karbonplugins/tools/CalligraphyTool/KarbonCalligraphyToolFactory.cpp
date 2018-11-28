/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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
    setSection(mainToolType());
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
