/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include "KoToolRegistry.h"

#include <FlakeDebug.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include "tools/KoPathToolFactory.h"
#include "tools/KoZoomTool.h"
#include "tools/KoZoomToolFactory.h"
#include "KoToolManager.h"
#include <KoPluginLoader.h>

#include <QGlobalStatic>

Q_GLOBAL_STATIC(KoToolRegistry, s_instance)

KoToolRegistry::KoToolRegistry()
  : d(0)
{
}

void KoToolRegistry::init()
{
    KoPluginLoader::PluginsConfig config;
    config.group = "calligra";
    config.whiteList = "ToolPlugins";
    config.blacklist = "ToolPluginsDisabled";
    KoPluginLoader::instance()->load(QString::fromLatin1("Calligra/Tool"),
                                     QString::fromLatin1("[X-Flake-PluginVersion] == 28"),
                                     config);

    // register generic tools
    add(new KoPathToolFactory());
    add(new KoZoomToolFactory());

    KConfigGroup cfg =  KSharedConfig::openConfig()->group("calligra");
    QStringList toolsBlacklist = cfg.readEntry("ToolsBlacklist", QStringList());
    foreach (const QString& toolID, toolsBlacklist) {
        delete value(toolID);
        remove(toolID);
    }
}

KoToolRegistry::~KoToolRegistry()
{
    qDeleteAll(doubleEntries());
    qDeleteAll(values());
}

KoToolRegistry* KoToolRegistry::instance()
{
    if (!s_instance.exists()) {
        s_instance->init();
    }
    return s_instance;
}
