/* This file is part of the KDE project
 * Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (c) 2007 Thomas Zander <zander@kde.org>
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

#include "KoPluginLoader.h"

#include <QString>
#include <QStringList>

#include <kdebug.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <KGlobal>
#include <KConfig>
#include <KConfigGroup>

class KoPluginLoader::Private
{
public:
    QStringList loadedServiceTypes;

    static KoPluginLoader *singleton;
};

KoPluginLoader::KoPluginLoader()
        : d(new Private())
{
}

KoPluginLoader::~KoPluginLoader()
{
    delete d;
}

KoPluginLoader* KoPluginLoader::instance()
{
    K_GLOBAL_STATIC(KoPluginLoader, s_instance)
    return s_instance;
}

void KoPluginLoader::load(const QString & serviceType, const QString & versionString, const PluginsConfig &config)
{
    // Don't load the same plugins again
    if (d->loadedServiceTypes.contains(serviceType)) {
        return;
    }
    // kDebug( 30003 ) <<"KoPluginLoader::load" << serviceType << kBacktrace();
    d->loadedServiceTypes << serviceType;
    QString query = QString::fromLatin1("(Type == 'Service')");
    if (! versionString.isEmpty())
        query += QString::fromLatin1(" and (%1)").arg(versionString);

    const KService::List offers = KServiceTypeTrader::self()->query(serviceType, query);
    KService::List plugins;
    bool configChanged = false;
    QList<QString> blacklist; // what we will save out afterwards
    if (config.whiteList && config.blacklist && config.group) {
        kDebug(30003) << "Loading" << serviceType << "with checking the config";
        KConfigGroup configGroup = KGlobal::config()->group(config.group);
        QList<QString> whiteList = configGroup.readEntry(config.whiteList, config.defaults);
        QList<QString> knownList;

        // if there was no list of defaults; all plugins are loaded.
        const bool firstStart = !config.defaults.isEmpty() && !configGroup.hasKey(config.whiteList);
        knownList = configGroup.readEntry(config.blacklist, knownList);
        if (firstStart)
            configChanged = true;

        foreach(KSharedPtr<KService> service, offers) {
            QString lib = service->library();
            if (whiteList.contains(lib))
                plugins.append(service);
            else if (!firstStart && !knownList.contains(lib)) { // also load newly installed plugins.
                plugins.append(service);
                configChanged = true;
            } else
                blacklist << service->library();
        }
    } else
        plugins = offers;

    QMap<QString, KSharedPtr<KService> > serviceNames;
    foreach(KSharedPtr<KService> service, plugins) {
        if (serviceNames.contains(service->name())) { // duplicate
            QVariant pluginVersion2 = service->property("X-Flake-PluginVersion");
            if (pluginVersion2.isNull()) // just take the first one found...
                continue;
            KSharedPtr<KService> otherService = serviceNames.value(service->name());
            QVariant pluginVersion = otherService->property("X-Flake-PluginVersion");
            if (!(pluginVersion.isNull() || pluginVersion.toInt() < pluginVersion2.toInt()))
                continue; // replace the old one with this one, since its newer.
        }
        serviceNames.insert(service->name(), service);
    }

    QList<QString> whiteList;
    foreach(KSharedPtr<KService> service, serviceNames) {
        int errCode = 0;
        QObject * plugin = KService::createInstance<QObject>(service, this, QStringList(), &errCode);
        if (plugin) {
            whiteList << service->library();
            kDebug(30003) << "Loaded plugin" << service->name();
            delete plugin;
        } else {
            kWarning(30003) << "Loading plugin" << service->name() << "failed, " << KLibLoader::errorString(errCode) << "(" << errCode << ")";
        }
    }

    if (configChanged && config.whiteList && config.blacklist && config.group) {
        KConfigGroup configGroup = KGlobal::config()->group(config.group);
        configGroup.writeEntry(config.whiteList, whiteList);
        configGroup.writeEntry(config.blacklist, blacklist);
    }
}

#include <KoPluginLoader.moc>
