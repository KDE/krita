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

#include <kstaticdeleter.h>

KoDockRegistry::KoDockRegistry() {
}

void KoDockRegistry::init() {
    KoPluginLoader::PluginsConfig config;
    config.whiteList = "docker-plugins";
    config.blacklist = "disabled-docker-plugins";
    config.group = "koffice";
    KoPluginLoader::instance()->load( QString::fromLatin1("KOffice/Dock"), QString(), config);
}

KoDockRegistry::~KoDockRegistry()
{
}

// static
KoDockRegistry *KoDockRegistry::s_instance = 0;
static KStaticDeleter<KoDockRegistry> staticToolRegistryDeleter;

KoDockRegistry* KoDockRegistry::instance()
{
    if(KoDockRegistry::s_instance == 0) {
        staticToolRegistryDeleter.setObject(s_instance, new KoDockRegistry());
        KoDockRegistry::s_instance->init();
    }
    return KoDockRegistry::s_instance;
}

#include "KoDockRegistry.moc"
