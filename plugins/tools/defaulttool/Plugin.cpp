/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "Plugin.h"
#include "defaulttool/DefaultToolFactory.h"
#include "referenceimagestool/ToolReferenceImages.h"

#include <KoToolRegistry.h>

#include <kpluginfactory.h>

K_PLUGIN_FACTORY_WITH_JSON(PluginFactory, "calligra_tool_defaults.json", registerPlugin<Plugin>();)

Plugin::Plugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoToolRegistry::instance()->add(new DefaultToolFactory());
    KoToolRegistry::instance()->add(new ToolReferenceImagesFactory());
}

#include <Plugin.moc>
