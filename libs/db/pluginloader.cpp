/* This file is part of the KDE project
   Copyright (C) 2003-2014 Jarosław Staniek <staniek@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "pluginloader.h"
#include <KDebug>
#include <KPluginLoader>
#include <QLibrary>
#include <QPointer>

class KexiPluginLoader::Private {
public:
    Private() : foundVersion(0) {}
    quint32 foundVersion;
    QPointer<KPluginFactory> factory;
    QString pluginName;
};

//! Typedef of plugin version matching the one exported by KEXI_EXPORT_PLUGIN.
typedef quint32 (*kexi_plugin_version_t)();

KexiPluginLoader::KexiPluginLoader(KService::Ptr ptr, const QString &pluginNameProperty)
    : d(new Private)
{
    if (ptr.isNull()) {
        kWarning() << "No service specified";
        return;
    }
    kDebug() << "library:" << ptr->library();
    KPluginLoader loader(ptr->library());
    QLibrary lib(loader.fileName());
    kexi_plugin_version_t plugin_version_function = (kexi_plugin_version_t)lib.resolve("plugin_version");
    if (!plugin_version_function) {
        kWarning() << "Plugin version not found for" << ptr->name();
        return;
    }
    d->foundVersion = plugin_version_function();
    kDebug() << "foundVersion:" << d->foundVersion;
    d->factory = loader.factory();
    if (!d->factory) {
        kWarning() << "Failed to create instance of factory for plugin" << ptr->name();
        return;
    }
    if (!pluginNameProperty.isEmpty()) {
        d->pluginName = ptr->property(pluginNameProperty).toString();
    }
}

KexiPluginLoader::~KexiPluginLoader()
{
    delete d;
}

uint KexiPluginLoader::majorVersion() const
{
    return (d->foundVersion >> 16) & 0xff;
}

uint KexiPluginLoader::minorVersion() const
{
    return (d->foundVersion >> 8) & 0xff;
}

uint KexiPluginLoader::releaseVersion() const
{
    return d->foundVersion & 0xff;
}

KPluginFactory* KexiPluginLoader::factory() const
{
    return d->factory;
}

QString KexiPluginLoader::pluginName() const
{
    return d->pluginName;
}
