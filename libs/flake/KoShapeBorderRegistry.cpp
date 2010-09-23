/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#include "KoShapeBorderRegistry.h"
#include <KoPluginLoader.h>

#include <KGlobal>

KoShapeBorderRegistry::KoShapeBorderRegistry()
{
}

void KoShapeBorderRegistry::init()
{
    KoPluginLoader::PluginsConfig config;
    config.whiteList = "FlakeBorderPlugins";
    config.blacklist = "FlakeBorderPluginsDisabled";
    KoPluginLoader::instance()->load(QString::fromLatin1("KOffice/Border"),
                                     QString::fromLatin1("[X-Flake-MinVersion] <= 0"),
                                     config);

    // Also add our hard-coded basic border
    // add(new KoPathShapeFactory(this, QStringList()));  TODO
}


KoShapeBorderRegistry::~KoShapeBorderRegistry()
{
}

KoShapeBorderRegistry* KoShapeBorderRegistry::instance()
{
    K_GLOBAL_STATIC(KoShapeBorderRegistry, s_instance)
    if (!s_instance.exists()) {
        s_instance->init();
    }
    return s_instance;
}

#include <KoShapeBorderRegistry.moc>
