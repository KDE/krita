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

#include <QGlobalStatic>
#include <QFontDatabase>
#include <QDebug>

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
    config.group = "calligra";
    KoPluginLoader::instance()->load(QString::fromLatin1("Calligra/Dock"),
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

QFont KoDockRegistry::dockFont()
{
    KConfigGroup group( KSharedConfig::openConfig(), "GUI");
    QFont dockWidgetFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    QFont smallFont = QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont);

    int pointSize = group.readEntry("palettefontsize", dockWidgetFont.pointSize());

    // Not set by the user
    if (pointSize == dockWidgetFont.pointSize()) {
        // and there is no setting for the smallest readable font, calculate something small
        if (smallFont.pointSize() >= pointSize) {
            smallFont.setPointSizeF(pointSize * 0.9);
        }
    }
    else {
        // paletteFontSize was set, use that
        smallFont.setPointSize(pointSize);
    }
    return smallFont;
}
