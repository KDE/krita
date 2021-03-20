/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
    config.group = "krita";
    config.whiteList = "ToolPlugins";
    config.blacklist = "ToolPluginsDisabled";
    KoPluginLoader::instance()->load(QString::fromLatin1("Krita/Tool"),
                                     QString::fromLatin1("[X-Flake-PluginVersion] == 28"),
                                     config);

    // register generic tools
    add(new KoPathToolFactory());
    add(new KoZoomToolFactory());

    KConfigGroup cfg =  KSharedConfig::openConfig()->group("krita");
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
