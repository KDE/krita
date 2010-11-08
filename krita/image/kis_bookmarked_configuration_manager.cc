/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_bookmarked_configuration_manager.h"
#include <QDomDocument>
#include <QDomElement>
#include <QString>

#include <kconfig.h>
#include <klocale.h>
#include <kconfiggroup.h>
#include <kglobal.h>

#include <KoID.h>

#include "kis_debug.h"
#include "kis_serializable_configuration.h"

const KoID KisBookmarkedConfigurationManager::ConfigDefault = KoID("Default", ki18n("Default"));
const KoID KisBookmarkedConfigurationManager::ConfigLastUsed = KoID("Last Used", ki18n("Last used"));

struct KisBookmarkedConfigurationManager::Private {
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

KisSerializableConfiguration* KisBookmarkedConfigurationManager::load(const QString & configname) const
{
    if (!exists(configname)) {
        if (configname == ConfigDefault.id())
            return d->configFactory->createDefault();
        else
            return 0;
    }
    KConfigGroup cfg = KGlobal::config()->group(configEntryGroup());

    QDomDocument doc;
    doc.setContent(cfg.readEntry<QString>(configname, ""));
    QDomElement e = doc.documentElement();
    KisSerializableConfiguration* config = d->configFactory->create(e);
    dbgImage << config << endl;
    return config;
}

void KisBookmarkedConfigurationManager::save(const QString & configname, const KisSerializableConfiguration* config)
{
    dbgImage << "Saving configuration " << config << " to " << configname;
    if (!config) return;
    KConfigGroup cfg = KGlobal::config()->group(configEntryGroup());
    cfg.writeEntry(configname, config->toXML());
}

bool KisBookmarkedConfigurationManager::exists(const QString & configname) const
{
    KSharedConfig::Ptr cfg = KGlobal::config();
    QMap< QString, QString > m = cfg->entryMap(configEntryGroup());
    return (m.find(configname) != m.end());
}

QList<QString> KisBookmarkedConfigurationManager::configurations() const
{
    KSharedConfig::Ptr cfg = KGlobal::config();
    QMap< QString, QString > m = cfg->entryMap(configEntryGroup());
    QList<QString> keys = m.keys();
    QList<QString> configsKey;
    foreach(const QString & key, keys) {
        if (key != ConfigDefault.id() && key != ConfigLastUsed.id()) {
            configsKey << key;
        }
    }
    return configsKey;
}

KisSerializableConfiguration* KisBookmarkedConfigurationManager::defaultConfiguration() const
{
    if (exists(KisBookmarkedConfigurationManager::ConfigDefault.id())) {
        return load(KisBookmarkedConfigurationManager::ConfigDefault.id());
    }
    if (exists(KisBookmarkedConfigurationManager::ConfigLastUsed.id())) {
        return load(KisBookmarkedConfigurationManager::ConfigLastUsed.id());
    }
    return 0;
}

QString KisBookmarkedConfigurationManager::configEntryGroup() const
{
    return d->configEntryGroup;
}

void KisBookmarkedConfigurationManager::remove(const QString & name)
{
    KSharedConfig::Ptr cfg = KGlobal::config();
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
