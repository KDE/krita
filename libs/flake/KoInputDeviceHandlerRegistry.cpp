/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoInputDeviceHandlerRegistry.h"

#include <QGlobalStatic>

#include <KoPluginLoader.h>
#include "KoInputDeviceHandler.h"

Q_GLOBAL_STATIC(KoInputDeviceHandlerRegistry, s_instance)

KoInputDeviceHandlerRegistry::KoInputDeviceHandlerRegistry()
{
}

void KoInputDeviceHandlerRegistry::init()
{
    KoPluginLoader::PluginsConfig config;
    config.whiteList = "DevicePlugins";
    config.blacklist = "DevicePluginsDisabled";
    config.group = "krita";
    KoPluginLoader::instance()->load(QString::fromLatin1("Krita/Device"),
                                     QString::fromLatin1("[X-Flake-PluginVersion] == 28"), config);

    Q_FOREACH (const QString & id, keys()) {
        KoInputDeviceHandler * d = value(id);
        if (d)
            d->start();
    }
}

KoInputDeviceHandlerRegistry::~KoInputDeviceHandlerRegistry()
{
    Q_FOREACH (const QString & id, keys()) {
        KoInputDeviceHandler * d = value(id);
        if (d) {
            d->stop();
        }
    }
    Q_FOREACH (const QString &id, keys()) {
        get(id)->deleteLater();
    }
    // just leak on exit -- we get into trouble for explicitly
    // deleting stuff from static objects, like registries
    //qDeleteAll(doubleEntries());
}

KoInputDeviceHandlerRegistry* KoInputDeviceHandlerRegistry::instance()
{
    if (!s_instance.exists()) {
        s_instance->init();
    }
    return s_instance;
}
