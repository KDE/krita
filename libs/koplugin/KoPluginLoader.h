/*
 * SPDX-FileCopyrightText: 2006-2016 Boudewijn Rempt (boud@valdyas.org)
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KO_PLUGIN_LOADER_H
#define KO_PLUGIN_LOADER_H

#include <QObject>
#include <QStringList>

#include "kritaplugin_export.h"

#ifndef Q_MOC_RUN
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
    MyPlugin(QObject *parent, const QVariantList & );
    ~MyPlugin() {}
};
@endcode
 * Example cpp file;
@code
#include "MyPlugin.h"
#include <kpluginfactory.h>

K_PLUGIN_FACTORY_WITH_JSON(MyPluginFactory, "myplugin.json", registerPlugin<MyPlugin>();)

MyPlugin::MyPlugin( QObject *parent, const QVariantList& ) : QObject(parent) {
    // do stuff like creating a factory and adding it to the
    // registry instance.
}
#include <MyPlugin.moc>
@endcode
 */
#endif
class KRITAPLUGIN_EXPORT KoPluginLoader : public QObject
{

    Q_OBJECT

public:
    /**
     * Config object for load()
     * It is possible to limit which plugins will be loaded in the KConfig configuration file by
     * stating explicitly which plugins are wanted.
     */
    struct PluginsConfig {
        PluginsConfig() : group(0), whiteList(0), blacklist(0) {}
        /**
         * The properties are retrieved from the config using the following construct;
         * /code
         *  KConfigGroup configGroup =  KSharedConfig::openConfig()->group(config.group);
         * /endcode
         * For most cases you can pass the string "krita" into this variable.
         */
        const char * group;
        /// This contains the variable name for the list of plugins (by library name) the user wants to load
        const char * whiteList;
        /// This contains the variable name for the list of plugins (by library name) that will not be loaded
        const char * blacklist;
        /// A registry can state it wants to load a default set of plugins instead of all plugins
        /// when the application starts the first time.  Append all such plugin (library) names to this list.
        QStringList defaults;
    };

    ~KoPluginLoader() override;

    /**
     * Return an instance of the KoPluginLoader
     * Creates an instance if that has never happened before and returns the singleton instance.
     */
    static KoPluginLoader * instance();

    /**
     * Load all plugins that conform to the versiontype and versionstring,
     * for instance:
     * KoPluginLoader::instance()->load("Krita/Flake", "([X-Flake-PluginVersion] == 28)");
     * This method allows you to optionally limit the plugins that are loaded by version, but also
     * using a user configurable set of config options.
     * If you pass a PluginsConfig struct only those plugins are loaded that are specified in the
     * application config file.  New plugins found since last start will be automatically loaded.
     * @param serviceType The string used to identify the plugins.
     * @param versionString A string match that allows you to check for a specific version
     * @param config when passing a valid config only the wanted plugins are actually loaded
     * #param owner if 0, the plugin will be deleted after instantiation, if not, the given qobject will own the plugin in its qobject hierarchy
     * @param cache: if true, the plugin will only be loaded once
     * @return a list of services (by library name) that were not know in the config
     */
    void load(const QString & serviceType, const QString & versionString = QString(), const PluginsConfig &config = PluginsConfig(), QObject* owner = 0, bool cache = true);

public:
    /// DO NOT USE! Use instance() instead
    // TODO: turn KoPluginLoader into namespace and do not expose object at all
    KoPluginLoader();
private:
    KoPluginLoader(const KoPluginLoader&);
    KoPluginLoader operator=(const KoPluginLoader&);

private:
    class Private;
    Private * const d;
};

#endif // KO_PLUGIN_LOADER_H
