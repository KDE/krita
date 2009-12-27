/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#include "KoDeviceRegistry.h"
#include <KGlobal>

#include <KoPluginLoader.h>

KoDeviceRegistry::KoDeviceRegistry()
{
}

void KoDeviceRegistry::init()
{
    KoPluginLoader::PluginsConfig config;
    config.whiteList = "DevicePlugins";
    config.blacklist = "DevicePluginsDisabled";
    config.group = "koffice";
    KoPluginLoader::instance()->load(QString::fromLatin1("KOffice/Device"),
                                     QString::fromLatin1("[X-Flake-MinVersion] <= 0"), config);

    foreach(const QString & id, keys()) {
        KoDevice * d = value(id);
        if (d)
            d->start();
    }
}

KoDeviceRegistry::~KoDeviceRegistry()
{
    foreach(const QString & id, keys()) {
        KoDevice * d = value(id);
        if (d)
            d->stop();
    }
}

KoDeviceRegistry* KoDeviceRegistry::instance()
{
    K_GLOBAL_STATIC(KoDeviceRegistry, s_instance)
    if (!s_instance.exists()) {
        s_instance->init();
    }
    return s_instance;
}

#include <KoDeviceRegistry.moc>
