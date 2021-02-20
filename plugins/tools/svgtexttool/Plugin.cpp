/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "Plugin.h"

// KDE
#include <kpluginfactory.h>

// Calligra libs
#include <KoShapeRegistry.h>
#include <KoToolRegistry.h>

#include "SvgTextToolFactory.h"

K_PLUGIN_FACTORY_WITH_JSON(PluginFactory, "krita_tool_svgtext.json", registerPlugin<Plugin>();)

Plugin::Plugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoToolRegistry::instance()->add(new SvgTextToolFactory());
}

#include <Plugin.moc>

