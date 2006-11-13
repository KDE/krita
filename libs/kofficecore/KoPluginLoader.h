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

#include <koffice_export.h>

/**
 * The pluginloader singleton is responsible for loading the plugins
 * that it's asked to load. It keeps track of which servicetypes it 
 * has seen and doesn't reload them. The plugins need to inherit KoPlugin,
 * which is just a QObject with a default constructor. Inside the default
 * constructor you can create whatever object you want and add it to 
 * whatever registry you prefer. After having been constructed, your plugin
 * will be deleted, so do all you need in the constructor.
 *
 * TODO: make plugins manageable.
 */
class FLAKE_EXPORT KoPluginLoader : public QObject
{

    Q_OBJECT

public:

    virtual ~KoPluginLoader();

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
    void load(const QString & serviceType, const QString & versionString);

private:
    KoPluginLoader();
    KoPluginLoader(const KoPluginLoader&);
    KoPluginLoader operator=(const KoPluginLoader&);

private:
    static KoPluginLoader *m_singleton;

    QStringList m_loadedServiceTypes;
};

#endif // KO_PLUGIN_LOADER_H
