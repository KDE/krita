/*
 * This file is part of PyKrita, Krita' Python scripting plugin.
 *
 * Copyright (C) 2013 Alex Turbov <i.zaufi@gmail.com>
 * Copyright (C) 2014-2016 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2017 Jouni Pentikäinen (joupent@gmail.com)
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

#include "PythonPluginManager.h"

#include <QFile>
#include <QFileInfo>
#include <KoResourcePaths.h>
#include <KConfigCore/KConfig>
#include <KConfigCore/KDesktopFile>
#include <KI18n/KLocalizedString>
#include <KConfigCore/KSharedConfig>
#include <KConfigCore/KConfigGroup>

#include "config.h"
#include "version_checker.h"

PythonPluginManager* instance = 0;

// PythonPlugin implementation

QString PythonPlugin::moduleFilePathPart() const
{
    QString filePath = m_moduleName;
    return filePath.replace(".", "/");
}

bool PythonPlugin::isValid() const
{
    dbgScript << "Got Krita/PythonPlugin: " << name()
              << ", module-path=" << moduleName()
                ;
    // Make sure mandatory properties are here
    if (m_name.isEmpty()) {
        dbgScript << "Ignore desktop file w/o a name";
        return false;
    }
    if (m_moduleName.isEmpty()) {
        dbgScript << "Ignore desktop file w/o a module to import";
        return false;
    }
#if PY_MAJOR_VERSION == 2
    // Check if the plug-in is compatible with Python 2 or not.
    if (m_properties["X-Python-2-Compatible"].toBool() != true) {
        dbgScript << "Ignoring plug-in. It is marked incompatible with Python 2.";
        return false;
    }
#endif

    return true;
}

// PythonPluginManager implementation

PythonPluginManager::PythonPluginManager()
        : QObject(0)
        , m_model(0, this)
{}

const QList<PythonPlugin>& PythonPluginManager::plugins() const
{
    return m_plugins;
}

PythonPlugin * PythonPluginManager::plugin(int index) {
    if (index >= 0 && index < m_plugins.count()) {
        return &m_plugins[index];
    }

    return nullptr;
}

PythonPluginsModel * PythonPluginManager::model()
{
    return &m_model;
}

void PythonPluginManager::unloadAllModules()
{
    Q_FOREACH(PythonPlugin plugin, m_plugins) {
        if (plugin.m_loaded) {
            unloadModule(plugin);
        }
    }
}

bool PythonPluginManager::verifyModuleExists(PythonPlugin &plugin)
{
    // Find the module:
    // 0) try to locate directory based plugin first
    QString rel_path = plugin.moduleFilePathPart();
    rel_path = rel_path + "/" + "__init__.py";
    dbgScript << "Finding Python module with rel_path:" << rel_path;

    QString module_path = KoResourcePaths::findResource("pythonscripts", rel_path);

    dbgScript << "module_path:" << module_path;

    if (module_path.isEmpty()) {
        // 1) Nothing found, then try file based plugin
        rel_path = plugin.moduleFilePathPart() + ".py";
        dbgScript << "Finding Python module with rel_path:" << rel_path;
        module_path = KoResourcePaths::findResource("pythonscripts", rel_path);
        dbgScript << "module_path:" << module_path;
    }

    // Is anything found at all?
    if (module_path.isEmpty()) {
        plugin.m_broken = true;
        plugin.m_errorReason = i18nc(
                                   "@info:tooltip"
                                   , "Unable to find the module specified <application>%1</application>"
                                   , plugin.moduleName()
                               );
        dbgScript << "Cannot load module:" << plugin.m_errorReason;
        return false;
    }
    dbgScript << "Found module path:" << module_path;
    return true;
}

QPair<QString, PyKrita::version_checker> PythonPluginManager::parseDependency(const QString& d)
{
    // Check if dependency has package info attached
    const int pnfo = d.indexOf('(');
    if (pnfo != -1) {
        QString dependency = d.mid(0, pnfo);
        QString version_str = d.mid(pnfo + 1, d.size() - pnfo - 2).trimmed();
        dbgScript << "Desired version spec [" << dependency << "]:" << version_str;
        PyKrita::version_checker checker = PyKrita::version_checker::fromString(version_str);
        if (!(checker.isValid() && d.endsWith(')'))) {
            dbgScript << "Invalid version spec " << d;
            QString reason = i18nc(
                                 "@info:tooltip"
                                 , "<p>Specified version has invalid format for dependency <application>%1</application>: "
                                 "<icode>%2</icode>. Skipped</p>"
                                 , dependency
                                 , version_str
                             );
            return qMakePair(reason, PyKrita::version_checker());
        }
        return qMakePair(dependency, checker);
    }
    return qMakePair(d, PyKrita::version_checker(PyKrita::version_checker::undefined));
}

/**
 * Collect dependencies and check them. To do it
 * just try to import a module... when unload it ;)
 *
 * \c X-Python-Dependencies property of \c .desktop file has the following format:
 * <tt>python-module(version-info)</tt>, where <tt>python-module</tt>
 * a python module name to be imported, <tt>version-spec</tt>
 * is a version triplet delimited by dots, possible w/ leading compare
 * operator: \c =, \c <, \c >, \c <=, \c >=
 */
void PythonPluginManager::verifyDependenciesSetStatus(PythonPlugin& plugin)
{
    QStringList dependencies = plugin.property("X-Python-Dependencies").toStringList();

    PyKrita::Python py = PyKrita::Python();
    QString reason = i18nc("@info:tooltip", "<title>Dependency check</title>");
    Q_FOREACH(const QString & d, dependencies) {
        QPair<QString, PyKrita::version_checker> info_pair = parseDependency(d);
        PyKrita::version_checker& checker = info_pair.second;
        if (!checker.isValid()) {
            plugin.m_broken = true;
            reason += info_pair.first;
            continue;
        }

        dbgScript << "Try to import dependency module/package:" << d;

        // Try to import a module
        const QString& dependency = info_pair.first;
        PyObject* module = py.moduleImport(PQ(dependency));
        if (module) {
            if (checker.isEmpty()) {                        // Need to check smth?
                dbgScript << "No version to check, just make sure it's loaded:" << dependency;
                Py_DECREF(module);
                continue;
            }
            // Try to get __version__ from module
            // See PEP396: http://www.python.org/dev/peps/pep-0396/
            PyObject* version_obj = py.itemString("__version__", PQ(dependency));
            if (!version_obj) {
                dbgScript << "No __version__ for " << dependency
                          << "[" << plugin.name() << "]:\n" << py.lastTraceback()
                          ;
                plugin.m_unstable = true;
                reason += i18nc(
                              "@info:tooltip"
                              , "<p>Failed to check version of dependency <application>%1</application>: "
                              "Module do not have PEP396 <code>__version__</code> attribute. "
                              "It is not disabled, but behaviour is unpredictable...</p>"
                              , dependency
                          );
            }
            PyKrita::version dep_version = PyKrita::version::fromPythonObject(version_obj);

            if (!dep_version.isValid()) {
                // Dunno what is this... Giving up!
                dbgScript << "***: Can't parse module version for" << dependency;
                plugin.m_unstable = true;
                reason += i18nc(
                              "@info:tooltip"
                              , "<p><application>%1</application>: Unexpected module's version format"
                              , dependency
                          );
            } else if (!checker(dep_version)) {
                dbgScript << "Version requirement check failed ["
                          << plugin.name() << "] for "
                          << dependency << ": wanted " << checker.operationToString()
                          << QString(checker.required())
                          << ", but found" << QString(dep_version)
                          ;
                plugin.m_broken = true;
                reason += i18nc(
                              "@info:tooltip"
                              , "<p><application>%1</application>: No suitable version found. "
                              "Required version %2 %3, but found %4</p>"
                              , dependency
                              , checker.operationToString()
                              , QString(checker.required())
                              , QString(dep_version)
                          );
            }
            // Do not need this module anymore...
            Py_DECREF(module);
        } else {
            dbgScript << "Load failure [" << plugin.name() << "]:\n" << py.lastTraceback();
            plugin.m_broken = true;
            reason += i18nc(
                          "@info:tooltip"
                          , "<p>Failure on module load <application>%1</application>:</p><pre>%2</pre>"
                          , dependency
                          , py.lastTraceback()
                      );
        }
    }

    if (plugin.isBroken() || plugin.isUnstable()) {
        plugin.m_errorReason = reason;
    }
}

void PythonPluginManager::scanPlugins()
{
    m_plugins.clear();

    KConfigGroup pluginSettings(KSharedConfig::openConfig(), "python");

    QStringList desktopFiles = KoResourcePaths::findAllResources("data", "pykrita/*desktop");

    Q_FOREACH(const QString &desktopFile, desktopFiles) {

        const KDesktopFile df(desktopFile);
        const KConfigGroup dg = df.desktopGroup();
        if (dg.readEntry("ServiceTypes") == "Krita/PythonPlugin") {
            PythonPlugin plugin;
            plugin.m_comment = df.readComment();
            plugin.m_name = df.readName();
            plugin.m_moduleName = dg.readEntry("X-KDE-Library");
            plugin.m_properties["X-Python-2-Compatible"] = dg.readEntry("X-Python-2-Compatible", false);

            QString manual = dg.readEntry("X-Krita-Manual");
            if (!manual.isEmpty()) {
                QFile f(QFileInfo(desktopFile).path() + "/" + plugin.m_moduleName + "/" + manual);
                if (f.exists()) {
                    f.open(QFile::ReadOnly);
                    QByteArray ba = f.readAll();
                    f.close();
                    plugin.m_manual = QString::fromUtf8(ba);
                }
            }
            if (!plugin.isValid()) {
                dbgScript << plugin.name() << "is not usable";
                continue;
            }

            if (!verifyModuleExists(plugin)) {
                dbgScript << "Cannot load" << plugin.name() << ": broken"
                          << plugin.isBroken()
                          << "because:" << plugin.errorReason();
                continue;
            }

            verifyDependenciesSetStatus(plugin);

            plugin.m_enabled = pluginSettings.readEntry(QString("enable_") + plugin.moduleName(), false);

            m_plugins.append(plugin);
        }
    }
}

void PythonPluginManager::tryLoadEnabledPlugins()
{
    for (PythonPlugin &plugin : m_plugins) {
        dbgScript << "Trying to load plugin" << plugin.moduleName()
                  << ". Enabled:" << plugin.isEnabled()
                  << ". Broken: " << plugin.isBroken();

        if (plugin.m_enabled && !plugin.isBroken()) {
            loadModule(plugin);
        }
    }
}

void PythonPluginManager::loadModule(PythonPlugin &plugin)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(plugin.isEnabled() && !plugin.isBroken());

    QString module_name = plugin.moduleName();
    dbgScript << "Loading module: " << module_name;

    PyKrita::Python py = PyKrita::Python();

    // Get 'plugins' key from 'pykrita' module dictionary.
    // Every entry has a module name as a key and 2 elements tuple as a value
    PyObject* plugins = py.itemString("plugins");
    KIS_SAFE_ASSERT_RECOVER_RETURN(plugins);

    PyObject* module = py.moduleImport(PQ(module_name));
    if (module) {
        // Move just loaded module to the dict
        const int ins_result = PyDict_SetItemString(plugins, PQ(module_name), module);
        KIS_SAFE_ASSERT_RECOVER_NOOP(ins_result == 0);
        Py_DECREF(module);
        // Handle failure in release mode.
        if (ins_result == 0) {
            // Initialize the module from Python's side
            PyObject* const args = Py_BuildValue("(s)", PQ(module_name));
            PyObject* result = py.functionCall("_pluginLoaded", PyKrita::Python::PYKRITA_ENGINE, args);
            Py_DECREF(args);
            if (result) {
                dbgScript << "\t" << "success!";
                plugin.m_loaded = true;
                return;
            }
        }
        plugin.m_errorReason = i18nc("@info:tooltip", "Internal engine failure");
    } else {
        plugin.m_errorReason = i18nc(
                                   "@info:tooltip"
                                   , "Module not loaded:<br/>%1"
                                   , py.lastTraceback().replace("\n", "<br/>")
                               );
    }
    plugin.m_broken = true;
    warnScript << "Error loading plugin" << module_name;
}

void PythonPluginManager::unloadModule(PythonPlugin &plugin)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(plugin.m_loaded);
    KIS_SAFE_ASSERT_RECOVER_RETURN(!plugin.isBroken());

    dbgScript << "Unloading module: " << plugin.moduleName();

    PyKrita::Python py = PyKrita::Python();

    // Get 'plugins' key from 'pykrita' module dictionary
    PyObject* plugins = py.itemString("plugins");
    KIS_SAFE_ASSERT_RECOVER_RETURN(plugins);

    PyObject* const args = Py_BuildValue("(s)", PQ(plugin.moduleName()));
    py.functionCall("_pluginUnloading", PyKrita::Python::PYKRITA_ENGINE, args);
    Py_DECREF(args);

    // This will just decrement a reference count for module instance
    PyDict_DelItemString(plugins, PQ(plugin.moduleName()));

    // Remove the module also from 'sys.modules' dict to really unload it,
    // so if reloaded all @init actions will work again!
    PyObject* sys_modules = py.itemString("modules", "sys");
    KIS_SAFE_ASSERT_RECOVER_RETURN(sys_modules);
    PyDict_DelItemString(sys_modules, PQ(plugin.moduleName()));

    plugin.m_loaded = false;
}

void PythonPluginManager::setPluginEnabled(PythonPlugin &plugin, bool enabled)
{
    bool wasEnabled = plugin.isEnabled();

    if (wasEnabled && !enabled) {
        unloadModule(plugin);
    }

    plugin.m_enabled = enabled;
    KConfigGroup pluginSettings(KSharedConfig::openConfig(), "python");
    pluginSettings.writeEntry(QString("enable_") + plugin.moduleName(), enabled);

    if (!wasEnabled && enabled) {
        loadModule(plugin);
    }
}
