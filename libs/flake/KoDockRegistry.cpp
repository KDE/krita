/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoDockRegistry.h"

#include <QGlobalStatic>
#include <QDebug>
#include <QApplication>

#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include "KoPluginLoader.h"

Q_GLOBAL_STATIC(KoDockRegistry, s_instance)

KoDockRegistry::KoDockRegistry()
    : d(0)
{
}

void KoDockRegistry::init()
{
    KoPluginLoader::PluginsConfig config;
    config.whiteList = "DockerPlugins";
    config.blacklist = "DockerPluginsDisabled";
    config.group = "krita";
    KoPluginLoader::instance()->load(QString::fromLatin1("Krita/Dock"),
                                     QString::fromLatin1("[X-Flake-PluginVersion] == 28"),
                                     config);
}

KoDockRegistry::~KoDockRegistry()
{
    // XXX: Intentionally leak the dockwidget factories to work around, for now, a bug in SIP
    //      See https://bugs.kde.org/show_bug.cgi?id=391992
    //    qDeleteAll(doubleEntries());
    //    qDeleteAll(values());
}

KoDockRegistry* KoDockRegistry::instance()
{

    if (!s_instance.exists()) {
        s_instance->init();
    }
    return s_instance;
}
