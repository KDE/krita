/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_bookmarked_configuration_manager.h"
#include <QDomDocument>
#include <QDomElement>
#include <QString>

#include <kconfig.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>
#include <kconfiggroup.h>


#include <KoID.h>

#include "kis_debug.h"
#include "kis_serializable_configuration.h"


const char KisBookmarkedConfigurationManager::ConfigDefault[] = "Default";
const char KisBookmarkedConfigurationManager::ConfigLastUsed[] = "Last Used";

struct Q_DECL_HIDDEN KisBookmarkedConfigurationManager::Private {

    QString configEntryGroup;
    KisSerializableConfigurationFactory* configFactory;

};

KisBookmarkedConfigurationManager::KisBookmarkedConfigurationManager(const QString & configEntryGroup, KisSerializableConfigurationFactory* configFactory)
    : d(new Private)
{
    d->configEntryGroup = configEntryGroup;
    d->configFactory = configFactory;
}

KisBookmarkedConfigurationManager::~KisBookmarkedConfigurationManager()
{
    delete d->configFactory;
    delete d;
}

KisSerializableConfigurationSP KisBookmarkedConfigurationManager::load(const QString & configname) const
{
    if (!exists(configname)) {
        if (configname == KisBookmarkedConfigurationManager::ConfigDefault)
            return d->configFactory->createDefault();
        else
            return 0;
    }
    KConfigGroup cfg =  KSharedConfig::openConfig()->group(configEntryGroup());

    QDomDocument doc;
    doc.setContent(cfg.readEntry<QString>(configname, ""));
    QDomElement e = doc.documentElement();
    KisSerializableConfigurationSP config = d->configFactory->create(e);
    dbgImage << config << endl;
    return config;
}

void KisBookmarkedConfigurationManager::save(const QString & configname, const KisSerializableConfigurationSP config)
{
    dbgImage << "Saving configuration " << config << " to " << configname;
    if (!config) return;
    KConfigGroup cfg =  KSharedConfig::openConfig()->group(configEntryGroup());
    cfg.writeEntry(configname, config->toXML());
}

bool KisBookmarkedConfigurationManager::exists(const QString & configname) const
{
    KSharedConfig::Ptr cfg =  KSharedConfig::openConfig();
    QMap< QString, QString > m = cfg->entryMap(configEntryGroup());
    return (m.find(configname) != m.end());
}

QList<QString> KisBookmarkedConfigurationManager::configurations() const
{
    KSharedConfig::Ptr cfg =  KSharedConfig::openConfig();
    QMap< QString, QString > m = cfg->entryMap(configEntryGroup());
    QList<QString> keys = m.keys();
    QList<QString> configsKey;
    Q_FOREACH (const QString & key, keys) {
        if (key != KisBookmarkedConfigurationManager::ConfigDefault && key != KisBookmarkedConfigurationManager::ConfigLastUsed) {
            configsKey << key;
        }
    }
    return configsKey;
}

KisSerializableConfigurationSP KisBookmarkedConfigurationManager::defaultConfiguration() const
{
    if (exists(KisBookmarkedConfigurationManager::ConfigDefault)) {
        return load(KisBookmarkedConfigurationManager::ConfigDefault);
    }
    if (exists(KisBookmarkedConfigurationManager::ConfigLastUsed)) {
        return load(KisBookmarkedConfigurationManager::ConfigLastUsed);
    }
    return 0;
}

QString KisBookmarkedConfigurationManager::configEntryGroup() const
{
    return d->configEntryGroup;
}

void KisBookmarkedConfigurationManager::remove(const QString & name)
{
    KSharedConfig::Ptr cfg =  KSharedConfig::openConfig();
    KConfigGroup group = cfg->group(configEntryGroup());
    group.deleteEntry(name);
}

QString KisBookmarkedConfigurationManager::uniqueName(const KLocalizedString & base)
{
#ifndef QT_NO_DEBUG
    QString prev;
#endif
    int nb = 1;
    while (true) {
        QString cur = base.subs(nb++).toString();
        if (!exists(cur)) return cur;
#ifndef QT_NO_DEBUG
        Q_ASSERT(prev != cur);
        prev = cur;
#endif
    }
}
