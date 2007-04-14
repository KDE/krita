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

#include "KoTextEditingRegistry.h"

#include <KoPluginLoader.h>

#include <kstaticdeleter.h>

void KoTextEditingRegistry::init() {
    KoPluginLoader::PluginsConfig config;
    config.whiteList = "TextEditingPlugins";
    config.blacklist = "TextEditingPluginsDisabled";
    config.group = "koffice";
    KoPluginLoader::instance()->load( QString::fromLatin1("KOffice/Text-EditingPlugin"),
                                      QString::fromLatin1("[X-KoText-Version] == 1"), config);
}

KoTextEditingRegistry *KoTextEditingRegistry::s_instance = 0;
static KStaticDeleter<KoTextEditingRegistry> staticShapeRegistryDeleter;

KoTextEditingRegistry* KoTextEditingRegistry::instance() {
    if(KoTextEditingRegistry::s_instance == 0) {
        staticShapeRegistryDeleter.setObject(s_instance, new KoTextEditingRegistry());
        KoTextEditingRegistry::s_instance->init();
    }
    return KoTextEditingRegistry::s_instance;
}

#include "KoTextEditingRegistry.moc"
