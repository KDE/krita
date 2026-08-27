/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoDockRegistry.h"

#include <QGlobalStatic>
#include <QDebug>
#include <QApplication>

#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include "KoPluginLoader.h"

Q_GLOBAL_STATIC(KoDockRegistry, s_instance)

namespace
{
struct SortableDockFactoryBase {
    QString id;
    KoDockFactoryBase *factory;
    int priority;

    bool operator<(const SortableDockFactoryBase &other) const
    {
        if (priority == other.priority) {
            return id.compare(other.id, Qt::CaseInsensitive) < 0;
        } else {
            return priority < other.priority;
        }
    }
};
} // namespace

KoDockRegistry::KoDockRegistry()
    : d(0)
{
}

void KoDockRegistry::init()
{
    KoPluginLoader::PluginsConfig config;
    config.blacklist = "DockerPluginsDisabled";
    config.group = "krita";
    KoPluginLoader::instance()->load(QString::fromLatin1("Krita/Dock"),
                                     config);
}

KoDockRegistry::~KoDockRegistry()
{
    Q_FOREACH(const KoDockFactoryBase *a, values()) {
        delete a;
    }
}

QList<KoDockFactoryBase *> KoDockRegistry::sortedDockWidgetFactories()
{
    QList<SortableDockFactoryBase> sortables;
    sortables.reserve(count());
    for (const QString &id : keys()) {
        KoDockFactoryBase *factory = value(id);
        sortables.append({id, factory, factory->priority()});
    }
    std::sort(sortables.begin(), sortables.end());

    QList<KoDockFactoryBase *> factories;
    factories.reserve(sortables.size());
    for (const SortableDockFactoryBase &s : sortables) {
        factories.append(s.factory);
    }
    return factories;
}

KoDockRegistry* KoDockRegistry::instance()
{

    if (!s_instance.exists()) {
        s_instance->init();
    }
    return s_instance;
}
