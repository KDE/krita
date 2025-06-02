
/*
 * SPDX-FileCopyrightText: 2006-2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoPluginLoader.h"

#include <KoJsonTrader.h>

#include <QJsonObject>
#include <QPluginLoader>

#include "kis_debug.h"

#include <kis_debug.h>

#include <KConfig>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KPluginFactory>

namespace
{
int versionFromPlugin(const KoJsonTrader::Plugin &plugin)
{
    QJsonObject json = plugin.metaData().value("MetaData").toObject();
    QVariant version = json.value("X-Krita-Version");
    return version.toString().toInt();
};

QString idFromPlugin(const KoJsonTrader::Plugin &plugin)
{
    QJsonObject json = plugin.metaData().value("MetaData").toObject();
    return json.value("Id").toString();
};

bool versionCompareLess(const KoJsonTrader::Plugin &lhs, const KoJsonTrader::Plugin &rhs)
{
    return versionFromPlugin(lhs) < versionFromPlugin(rhs);
};

bool idCompareEqual(const KoJsonTrader::Plugin &lhs, const KoJsonTrader::Plugin &rhs)
{
    return idFromPlugin(lhs) == idFromPlugin(rhs);
};

bool sortByIdAndReversedVersion(const KoJsonTrader::Plugin &lhs, const KoJsonTrader::Plugin &rhs)
{
    const QString lhsId = idFromPlugin(lhs);
    const QString rhsId = idFromPlugin(rhs);
    return lhsId < rhsId ||
        (lhsId == rhsId && versionCompareLess(rhs, lhs));
};

} // namespace


class Q_DECL_HIDDEN KoPluginLoader::Private
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

Q_GLOBAL_STATIC(KoPluginLoader, pluginLoaderInstance)

KoPluginLoader* KoPluginLoader::instance()
{
    return pluginLoaderInstance();
}

void KoPluginLoader::load(const QString & serviceType, const PluginsConfig &config, QObject* owner, bool cache)
{
    // Don't load the same plugins again
    if (cache && d->loadedServiceTypes.contains(serviceType)) {
        return;
    }
    d->loadedServiceTypes << serviceType;

    QList<KoJsonTrader::Plugin> plugins = KoJsonTrader::instance()->query(serviceType, QString());

    {
        /**
         * First, remove all the duplicated plugins and keep only the ones with
         * the highest version number
         */
        std::sort(plugins.begin(), plugins.end(), &sortByIdAndReversedVersion);
        auto it = plugins.begin();
        while ((it = std::adjacent_find(it, plugins.end(), &idCompareEqual)) != plugins.end()) {
            warnPlugins << "Skipping duplicated plugin, id:" << idFromPlugin(*it)
                        << "version:" << versionFromPlugin(*it) << "filename:" << it->fileName();
            /**
             * Erasing an element in QList in Qt5 may invalidate the existing
             * iterator, so we should derive the new position from the iterator
             * returned by erase() method
             */
            it = std::prev(plugins.erase(std::next(it)));
        }
    }

    if (config.isValid()) {
        /**
         * Then remove all the blacklisted plugins if necessary
         */
        KConfigGroup configGroup(KSharedConfig::openConfig(), config.group);
        QStringList blackList = configGroup.readEntry(config.blacklist, QStringList());

        auto it = plugins.begin();
        while (it != plugins.end()) {
            if (blackList.contains(idFromPlugin(*it))) {
                it = plugins.erase(it);
            } else {
                ++it;
            }
        }
    }

    /**
     * Now "load" all the plugins. If "owner" object is not provided, we just
     * create the plugin object and immediately destroy it. Usually the constructor
     * of this plugin will just populate some registry, so the object is not necessary
     * anymore.
     */
    for (KoJsonTrader::Plugin &plugin : plugins) {
        const QString pluginName = idFromPlugin(plugin);
        dbgPlugins << "loading" << pluginName;

        QObject *object = 0;

        KPluginFactory *factory = qobject_cast<KPluginFactory *>(plugin.instance());
        if (factory) {
            object = factory->create<QObject>(owner ? owner : this, QVariantList());
        }

        if (object) {
            dbgPlugins << "\tLoaded plugin" << plugin.fileName() << "owner:" << owner;
            if (!owner) {
                delete object;
            }
        } else {
            qWarning() << "\tLoading plugin" << plugin.fileName() << "failed, " << plugin.errorString();
        }
    }
}

KPluginFactory* KoPluginLoader::loadSinglePlugin(const std::vector<std::pair<QString, QString>> &predicates, const QString &serviceType)
{
    QList<KoJsonTrader::Plugin> offers = KoJsonTrader::instance()->query(serviceType, QString());

    offers.erase(std::remove_if(offers.begin(),
                                offers.end(),
                                [&](const KoJsonTrader::Plugin &plugin) {
                                    QJsonObject json = plugin.metaData().value("MetaData").toObject();
                                    Q_FOREACH(const auto &predicate, predicates) {
                                        if (json.value(predicate.first).toString() != predicate.second) {
                                            return true;
                                        }
                                    }
                                    return false;
                                }),
                 offers.end());

    auto it = std::max_element(offers.begin(), offers.end(), versionCompareLess);

    if (it != offers.end()) {
        KPluginFactory *factory = qobject_cast<KPluginFactory *>(it->instance());
        return factory;
    }

    return nullptr;
}

KPluginFactory* KoPluginLoader::loadSinglePlugin(const std::pair<QString, QString> &predicates, const QString & serviceType)
{
    return loadSinglePlugin(std::vector<std::pair<QString, QString>>{predicates}, serviceType);
}

KPluginFactory* KoPluginLoader::loadSinglePlugin(const QString &id, const QString &serviceType)
{
    return loadSinglePlugin({"Id", id}, serviceType);
}