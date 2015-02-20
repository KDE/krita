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

#ifndef KEXIDB_PLUGINLOADER_H
#define KEXIDB_PLUGINLOADER_H

#include "calligradb_export.h"

#include <KService>
#include <KPluginFactory>

//! Implementation of a plugin's factory and version for given @a prefix, @a class_name,
//! @a internal_name and version. This macro should be placed in plugin's implementation.
//! See K_EXPORT_KEXIDB_DRIVER for example use.
//! @note K_EXPORT_PLUGIN_VERSION (exporting static integer value) had resolve errors
//!       (at least) on Linux so has been replaced here by a plugin_version() "C" function.
#define KEXI_EXPORT_PLUGIN( prefix, class_name, internal_name, major_version, \
                            minor_version, release_version ) \
    K_PLUGIN_FACTORY(factory, registerPlugin<class_name>();) \
    K_EXPORT_PLUGIN(factory(prefix "_" # internal_name)) \
    Q_EXTERN_C KDE_EXPORT quint32 plugin_version() { \
        return KDE_MAKE_VERSION(major_version, minor_version, release_version); }

//! General-purpose plugin loader
class CALLIGRADB_EXPORT KexiPluginLoader {
public:
    KexiPluginLoader(KService::Ptr ptr, const QString &pluginNameProperty);

    ~KexiPluginLoader();

    //! @return major version of the plugin loaded
    uint majorVersion() const;

    //! @return minor version of the plugin loaded
    uint minorVersion() const;

    //! @return release version of the plugin loaded
    uint releaseVersion() const;

    //! @return instance of the plugin loaded. 0 is returned on error.
    template<typename T>
    T *createPlugin(QObject* parent = 0) {
        if (!factory())
            return 0;
        T* plugin = factory()->create<T>(parent);
        if (plugin) {
            plugin->setObjectName(pluginName());
        }
        return plugin;
    }

private:
    KPluginFactory *factory() const;
    QString pluginName() const;

    Q_DISABLE_COPY(KexiPluginLoader)
    class Private;
    Private * const d;
};

#endif
