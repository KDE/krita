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

#include <KoJsonTrader.h>

#include <QString>
#include <QStringList>

#include <QDebug>
#include <KConfig>
#include <KConfigGroup>

class KoPluginLoader::Private
{
public:
    QStringList loadedServiceTypes;
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
    d->loadedServiceTypes << serviceType;
    QString query = QString::fromLatin1("(Type == 'Service')");
    if (!versionString.isEmpty()) {
        query += QString::fromLatin1(" and (%1)").arg(versionString);
    }

    QList<QPluginLoader *> offers = KoJsonTrader::self()->query(serviceType, QString());
    QList<QPluginLoader *> plugins;
    bool configChanged = false;
    QList<QString> blacklist; // what we will save out afterwards
    if (config.whiteList && config.blacklist && config.group) {
        kDebug(30003) << "Loading" << serviceType << "with checking the config";
        KSharedConfigPtr configGroup = KSharedConfig::openConfig(config.group);
        QList<QString> whiteList = configGroup.readEntry(config.whiteList, config.defaults);
        QList<QString> knownList;

        // if there was no list of defaults; all plugins are loaded.
        const bool firstStart = !config.defaults.isEmpty() && !configGroup.hasKey(config.whiteList);
        knownList = configGroup.readEntry(config.blacklist, knownList);
        if (firstStart) {
            configChanged = true;
        }
        foreach(QPluginLoader *loader, offers) {
            QJsonObject json = loader->metaData().value("MetaData").toObject();
            const QString pluginName = json.value("X-KDE-PluginInfo-Name").toString();
            if (pluginName.isEmpty()) {
                kWarning(30003) << "Loading plugin" << loader->fileName() << "failed, has no X-KDE-PluginInfo-Name.";
                continue;
            }
            if (whiteList.contains(pluginName)) {
                plugins.append(loader);
            } else if (!firstStart && !knownList.contains(pluginName)) { // also load newly installed plugins.
                plugins.append(loader);
                configChanged = true;
            } else {
                blacklist << pluginName;
            }
        }
    } else {
        plugins = offers;
    }

    QMap<QString, QPluginLoader *> serviceNames;
    foreach(QPluginLoader *loader, plugins) {
        if (serviceNames.contains(loader->fileName())) { // duplicate
            QJsonObject json2 = loader->metaData().value("MetaData").toObject();
            QVariant pluginVersion2 = json2.value("X-Flake-PluginVersion").toVariant();
            if (pluginVersion2.isNull()) { // just take the first one found...
                continue;
            }
            QPluginLoader *currentLoader = serviceNames.value(loader->fileName());
            QJsonObject json = currentLoader->metaData().value("MetaData").toObject();
            QVariant pluginVersion = json.value("X-Flake-PluginVersion").toVariant();
            if (!(pluginVersion.isNull() || pluginVersion.toInt() < pluginVersion2.toInt())) {
                continue; // replace the old one with this one, since its newer.
            }
        }
        serviceNames.insert(loader->fileName(), loader);
    }

    QList<QString> whiteList;
    foreach(QPluginLoader *loader, serviceNames) {
        KPluginFactory *factory = qobject_cast<KPluginFactory *>(loader->instance());
        QObject *plugin = factory->create<QObject>(0, QVariantList());
        if (plugin) {
            QJsonObject json = loader->metaData().value("MetaData").toObject();
            const QString pluginName = json.value("X-KDE-PluginInfo-Name").toString();
            whiteList << pluginName;
            kDebug(30003) << "Loaded plugin" << loader->fileName();
            delete plugin;
        } else {
            kWarning(30003) << "Loading plugin" << loader->fileName() << "failed, " << loader->errorString();
        }
    }

    if (configChanged && config.whiteList && config.blacklist && config.group) {
        KSharedConfigPtr configGroup = KSharedConfig::openConfig(config.group);
        configGroup.writeEntry(config.whiteList, whiteList);
        configGroup.writeEntry(config.blacklist, blacklist);
    }
}
