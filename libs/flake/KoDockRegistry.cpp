/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#include "KoDockRegistry.h"

#include "KoPluginLoader.h"

#include <KGlobal>

KoDockRegistry::KoDockRegistry()
{
}

void KoDockRegistry::init()
{
    KoPluginLoader::PluginsConfig config;
    config.whiteList = "DockerPlugins";
    config.blacklist = "DockerPluginsDisabled";
    config.group = "koffice";
    KoPluginLoader::instance()->load(QString::fromLatin1("KOffice/Dock"),
                                     QString::fromLatin1("[X-Flake-MinVersion] <= 0"),
                                     config);
}

KoDockRegistry::~KoDockRegistry()
{
    foreach (const QString &id, keys()) {
        KoDockFactoryBase* dw = get(id);
        remove(id);
        delete dw;
    }
}

KoDockRegistry* KoDockRegistry::instance()
{
    K_GLOBAL_STATIC(KoDockRegistry, s_instance)
    if (!s_instance.exists()) {
        s_instance->init();
    }
    return s_instance;
}

#include <KoDockRegistry.moc>
