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

#ifndef KO_PLUGIN_LOADER_H
#define KO_PLUGIN_LOADER_H

#include <QObject>
#include <QStringList>

#include <kofficecore_export.h>

/**
 * The pluginloader singleton is responsible for loading the plugins
 * that it's asked to load. It keeps track of which servicetypes it
 * has seen and doesn't reload them. The plugins need to inherit
 * a QObject with a default constructor. Inside the default
 * constructor you can create whatever object you want and add it to
 * whatever registry you prefer. After having been constructed, your plugin
 * will be deleted, so do all you need in the constructor.  Things like
 * adding a factory to a registry make sense there.
 * Example header file;
@code
#include <QObject>

class MyPlugin : public QObject {
    Q_OBJECT
public:
    MyPlugin(QObject *parent, const QStringList & );
    ~MyPlugin() {}
};
@endcode
 * Example cpp file;
@code
#include "MyPlugin.h"
#include <kgenericfactory.h>

K_EXPORT_COMPONENT_FACTORY(mylibrary,
                           KGenericFactory<MyPlugin>( "DaPlugin" ) )

MyPlugin::MyPlugin( QObject *parent, const QStringList& ) : QObject(parent) {
    // do stuff like creating a factory and adding it to the
    // registry instance.
}
#include "MyPlugin.moc"
@endcode
 *
 * TODO: make plugins manageable.
 */
class KOFFICECORE_EXPORT KoPluginLoader : public QObject
{

    Q_OBJECT

public:

    ~KoPluginLoader();

    /**
     * Return an instance of the KoPluginLoader
     * Creates an instance if that has never happened before and returns the singleton instance.
     */
    static KoPluginLoader * instance();

    /**
     * Load all plugins that conform to the versiontype and versionstring,
     * for instance:
     * KoPluginLoader::instance()->load("KOffice/Flake", "([X-Flake-Version] == 3)");
     */
    void load(const QString & serviceType, const QString & versionString = QString());

private:
    KoPluginLoader();
    KoPluginLoader(const KoPluginLoader&);
    KoPluginLoader operator=(const KoPluginLoader&);

private:
    static KoPluginLoader *m_singleton;
    class Private;
    Private * const d;
};

#endif // KO_PLUGIN_LOADER_H
