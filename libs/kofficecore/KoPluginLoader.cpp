/* This file is part of the KDE project
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
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
#include <kstaticdeleter.h>
#include <KConfig>

class KoPluginLoader::Private {
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

KoPluginLoader *KoPluginLoader::Private::singleton = 0;
static KStaticDeleter<KoPluginLoader> pluginLoaderStatic;

KoPluginLoader* KoPluginLoader::instance()
{
    if(KoPluginLoader::Private::singleton == 0)
    {
        pluginLoaderStatic.setObject(Private::singleton, new KoPluginLoader());
    }
    return KoPluginLoader::Private::singleton;
}

void KoPluginLoader::load(const QString & serviceType, const QString & versionString, const PluginsConfig &config) {
    // Don't load the same plugins again
    if (d->loadedServiceTypes.contains(serviceType)) {
        return;
    }
    // kDebug() << "KoPluginLoader::load " << serviceType << kBacktrace() << endl;
    d->loadedServiceTypes << serviceType;
    QString query = QString::fromLatin1("(Type == 'Service')");
    if(! versionString.isEmpty())
        query += QString::fromLatin1(" and (%1)").arg(versionString);

    const KService::List offers = KServiceTypeTrader::self()->query(serviceType, query);
    KService::List plugins;
    bool configChanged = false;
    QList<QString> blacklist; // what we will save out afterwards
    if(config.whiteList && config.blacklist && config.group) {
        kDebug(30003) << "loading " << serviceType << " with checking the config\n";
        KConfigGroup configGroup = KGlobal::config()->group(config.group);
        QList<QString> whiteList = configGroup.readEntry(config.whiteList, config.defaults);
        QList<QString> knownList;

        // if there was no list of defaults; all plugins are loaded.
        const bool firstStart = !config.defaults.isEmpty() && !configGroup.hasKey(config.whiteList);
        knownList = configGroup.readEntry(config.blacklist, knownList);
        if(firstStart)
            configChanged = true;

        foreach(KSharedPtr<KService> service, offers) {
            QString lib = service->library();
            if(whiteList.contains(lib))
                plugins.append(service);
            else if(!firstStart && !knownList.contains(lib)) { // also load newly installed plugins.
                plugins.append(service);
                configChanged = true;
            }
            else
                blacklist << service->library();
        }
    }
    else
        plugins = offers;

    QList<QString> whiteList;
    foreach(KSharedPtr<KService> service, plugins) {
        int errCode = 0;
        QObject * plugin = KService::createInstance<QObject>(service, this, QStringList(), &errCode );
        if ( plugin ) {
            whiteList << service->library();
            kDebug(30003) << "Loaded plugin " << service->name() << endl;
            delete plugin;
        }
        else {
            kWarning(30003) <<"loading plugin '" << service->name() << "' failed, "<< KLibLoader::errorString( errCode ) << " ("<< errCode << ")\n";
        }
    }

    if(configChanged && config.whiteList && config.blacklist && config.group) {
        KConfigGroup configGroup = KGlobal::config()->group(config.group);
        configGroup.writeEntry(config.whiteList, whiteList);
        configGroup.writeEntry(config.blacklist, blacklist);
    }
}

#include "KoPluginLoader.moc"
