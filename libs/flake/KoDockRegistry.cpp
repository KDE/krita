/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoDockRegistry.h"

#include <QGlobalStatic>
#include <QFontDatabase>
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

QFont KoDockRegistry::dockFont()
{
    KConfigGroup config(KSharedConfig::openConfig(), "");

    QFont dockWidgetFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    QFont smallFont = QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont);


    if (config.readEntry<bool>("use_custom_system_font", false)) {
        QString fontName = config.readEntry<QString>("custom_system_font", "");
        int smallFontSize = config.readEntry<int>("custom_font_size", -1);

        if (smallFontSize <= 6) {
            smallFontSize = dockWidgetFont.pointSize();
        }

        if (!fontName.isEmpty()) {
            dockWidgetFont = QFont(fontName, dockWidgetFont.pointSize());
            smallFont = QFont(fontName, smallFontSize * 0.9);
        }
    }
    else {
        int pointSize = config.readEntry("palettefontsize", dockWidgetFont.pointSize());

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

    }

    return smallFont;
}
