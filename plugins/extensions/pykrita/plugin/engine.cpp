// This file is part of PyKrita, Krita' Python scripting plugin.
//
// Copyright (C) 2006 Paul Giannaros <paul@giannaros.org>
// Copyright (C) 2012, 2013 Shaheed Haque <srhaque@theiet.org>
// Copyright (C) 2013 Alex Turbov <i.zaufi@gmail.com>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) version 3, or any
// later version accepted by the membership of KDE e.V. (or its
// successor approved by the membership of KDE e.V.), which shall
// act as a proxy defined in Section 6 of version 3 of the license.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library.  If not, see <http://www.gnu.org/licenses/>.
//

#include "engine.h"

// config.h defines PYKRITA_PYTHON_LIBRARY, the path to libpython.so
// on the build system

#include "config.h"
#include "utilities.h"

#include <cmath>
#include <Python.h>

#include <QSettings>
#include <QLibrary>
#include <QFileInfo>

#include <kconfig.h>
#include <klocalizedstring.h>
#include <kcolorscheme.h>

//#include <KoServiceLocator.h>
#include <KoResourcePaths.h>

#include <kis_debug.h>

/// Name of the file where per-plugin configuration is stored.
#define CONFIG_FILE "kritapykritarc"

#if PY_MAJOR_VERSION < 3
# define PYKRITA_INIT initpykrita
#else
# define PYKRITA_INIT PyInit_pykrita
#endif

PyMODINIT_FUNC PYKRITA_INIT();                                 // fwd decl

/// \note Namespace name written in uppercase intentionally!
/// It will appear in debug output from Python plugins...
namespace PYKRITA
{
PyObject* debug(PyObject* /*self*/, PyObject* args)
{
    const char* text;

    if (PyArg_ParseTuple(args, "s", &text))
        dbgScript << text;
    Py_INCREF(Py_None);
    return Py_None;
}
}                                                           // namespace PYKRITA

namespace
{
PyObject* s_pykrita;
/**
 * \attention Krita has embedded Python, so init function \b never will be called
 * automatically! We can use this fact to initialize a pointer to an instance
 * of the \c Engine class (which is a part of the \c Plugin), so exported
 * functions will know it (yep, from Python's side they should be static).
 */
PyKrita::Engine* s_engine_instance = 0;

/**
 * Wrapper function, called explicitly from \c Engine::Engine
 * to initialize pointer to the only (by design) instance of the engine,
 * so exported (to Python) functions get know it... Then invoke
 * a real initialization sequence...
 */
void pythonInitwrapper(PyKrita::Engine* const engine)
{
    Q_ASSERT("Sanity check" && !s_engine_instance);
    s_engine_instance = engine;
    // Call initialize explicitly to initialize embedded interpreter.
    PYKRITA_INIT();
}

/**
 * Functions for the Python module called pykrita.
 * \todo Does it \b REALLY needed? Configuration data will be flushed
 * on exit anyway! Why to write it (and even allow to plugins to call this)
 * \b before krita really going to exit? It would be better to \b deprecate
 * this (harmful) function!
 */
PyObject* pykritaSaveConfiguration(PyObject* /*self*/, PyObject* /*unused*/)
{
    if (s_engine_instance)
        s_engine_instance->saveGlobalPluginsConfiguration();
    Py_INCREF(Py_None);
    return Py_None;
}

PyMethodDef pykritaMethods[] = {
    {
        "saveConfiguration"
        , &pykritaSaveConfiguration
        , METH_NOARGS
        , "Save the configuration of the plugin into " CONFIG_FILE
    }
    , {
        "qDebug"
        , &PYKRITA::debug
        , METH_VARARGS
        , "True KDE way to show debug info"
    }
    , { 0, 0, 0, 0 }
};

}                                                           // anonymous namespace

//BEGIN Python module registration
PyMODINIT_FUNC PYKRITA_INIT()
{
#if PY_MAJOR_VERSION < 3
    s_pykrita = Py_InitModule3("pykrita", pykritaMethods, "The pykrita module");
    PyModule_AddStringConstant(s_pykrita, "__file__", __FILE__);
#else
    static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT
        , "pykrita"
        , "The pykrita module"
        , -1
        , pykritaMethods
        , 0
        , 0
        , 0
        , 0
    };
    s_pykrita = PyModule_Create(&moduledef);
    PyModule_AddStringConstant(s_pykrita, "__file__", __FILE__);
    return s_pykrita;
#endif
}
//END Python module registration


//BEGIN PyKrita::Engine::PluginState
PyKrita::Engine::PluginState::PluginState()
    : m_enabled(false)
    , m_broken(false)
    , m_unstable(false)
    , m_isDir(false)
{
}
//END PyKrita::Engine::PluginState


/**
 * Just initialize some members. The second (most important) part
 * is to call \c Engine::tryInitializeGetFailureReason()!
 * W/o that call instance is invalid and using it lead to UB!
 */
PyKrita::Engine::Engine()
    : m_configuration(0)
    , m_sessionConfiguration(0)
    , m_engineIsUsable(false)
{
}

/// \todo More accurate shutdown required:
/// need to keep track what exactly was broken on
/// initialize attempt...
PyKrita::Engine::~Engine()
{
    dbgScript << "Going to destroy the Python engine";

    // Notify Python that engine going to die
    {
        Python py = Python();
        py.functionCall("_pykritaUnloading");
    }
    unloadAllModules();

    // Clean internal configuration dicts
    // NOTE Do not need to save anything! It's already done!
    if (m_configuration) {
        Py_DECREF(m_configuration);
    }
    if (m_sessionConfiguration) {
        Py_DECREF(m_sessionConfiguration);
    }

    Python::maybeFinalize();
    Python::libraryUnload();
    s_engine_instance = 0;
}

void PyKrita::Engine::unloadAllModules()
{
    // Unload all modules
    for (int i = 0; i < m_plugins.size(); ++i) {
        if (m_plugins[i].isEnabled() && !m_plugins[i].isBroken()) {
            unloadModule(i);
        }
    }
}

/**
 * \todo Make sure noone tries to use uninitialized engine!
 * (Or enable exceptions for this module, so this case wouldn't even araise?)
 */
QString PyKrita::Engine::tryInitializeGetFailureReason()
{
    dbgScript << "Construct the Python engine for Python" << PY_MAJOR_VERSION << "," << PY_MINOR_VERSION;

    if (!Python::libraryLoad()) {
        return i18nc("@info:tooltip ", "Cannot load Python library");
    }

    // Update PYTHONPATH
    // 0) custom plugin directories (prefer local dir over systems')
    // 1) shipped krita module's dir
    QStringList pluginDirectories = KoResourcePaths::findDirs("pythonscripts");
    dbgScript << "Plugin Directories: " << pluginDirectories;
    if (!Python::setPath(pluginDirectories)) {
        return i18nc("@info:tooltip ", "Cannot set Python paths");
    }

    if (0 != PyImport_AppendInittab(Python::PYKRITA_ENGINE, PYKRITA_INIT)) {
        return i18nc("@info:tooltip ", "Cannot load built-in <icode>pykrita</icode> module");
    }

    Python::ensureInitialized();
    Python py = Python();

    PyRun_SimpleString(
        "import sip\n"
        "sip.setapi('QDate', 2)\n"
        "sip.setapi('QTime', 2)\n"
        "sip.setapi('QDateTime', 2)\n"
        "sip.setapi('QUrl', 2)\n"
        "sip.setapi('QTextStream', 2)\n"
        "sip.setapi('QString', 2)\n"
        "sip.setapi('QVariant', 2)\n"
    );

    // Initialize our built-in module.
    pythonInitwrapper(this);
    if (!s_pykrita) {
        return i18nc("@info:tooltip ", "No <icode>pykrita</icode> built-in module");
    }

    // Setup global configuration
    m_configuration = PyDict_New();
    /// \todo Check \c m_configuration ?
    // Host the configuration dictionary.
    py.itemStringSet("configuration", m_configuration);

    // Setup per session configuration
    m_sessionConfiguration = PyDict_New();
    py.itemStringSet("sessionConfiguration", m_sessionConfiguration);

    // Initialize 'plugins' dict of module 'pykrita'
    PyObject* plugins = PyDict_New();
    py.itemStringSet("plugins", plugins);

    // Get plugins available
    scanPlugins();

    // NOTE Empty failure reson string indicates success!
    m_engineIsUsable = true;
    return QString();
}

int PyKrita::Engine::columnCount(const QModelIndex&) const
{
    return Column::LAST__;
}

int PyKrita::Engine::rowCount(const QModelIndex&) const
{
    return m_plugins.size();
}

QModelIndex PyKrita::Engine::index(const int row, const int column, const QModelIndex& parent) const
{
    if (!parent.isValid() && row < m_plugins.size() && column < Column::LAST__)
        return createIndex(row, column);
    return QModelIndex();
}

QModelIndex PyKrita::Engine::parent(const QModelIndex&) const
{
    return QModelIndex();
}

QVariant PyKrita::Engine::headerData(
    const int section
    , const Qt::Orientation orientation
    , const int role
) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case Column::NAME:
            return i18nc("@title:column", "Name");
        case Column::COMMENT:
            return i18nc("@title:column", "Comment");
        default:
            break;
        }
    }
    return QVariant();
}

QVariant PyKrita::Engine::data(const QModelIndex& index, const int role) const
{
    Q_ASSERT("Sanity check" && index.row() < m_plugins.size());
    Q_ASSERT("Sanity check" && index.column() < Column::LAST__);
    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case Column::NAME:
            return m_plugins[index.row()].m_pythonPlugin.name();
        case Column::COMMENT:
            return m_plugins[index.row()].m_pythonPlugin.comment();
        default:
            break;
        }
        break;
    case Qt::CheckStateRole: {
        if (index.column() == Column::NAME) {
            const bool checked = m_plugins[index.row()].isEnabled();
            return checked ? Qt::Checked : Qt::Unchecked;
        }
        break;
    }
    case Qt::ToolTipRole:
        if (!m_plugins[index.row()].m_errorReason.isEmpty())
            return m_plugins[index.row()].m_errorReason;
        break;
    case Qt::ForegroundRole:
        if (m_plugins[index.row()].isUnstable()) {
            KColorScheme scheme(QPalette::Inactive, KColorScheme::View);
            return scheme.foreground(KColorScheme::NegativeText).color();
        }
    default:
        break;
    }
    return QVariant();
}

Qt::ItemFlags PyKrita::Engine::flags(const QModelIndex& index) const
{
    Q_ASSERT("Sanity check" && index.row() < m_plugins.size());
    Q_ASSERT("Sanity check" && index.column() < Column::LAST__);

    int result = Qt::ItemIsSelectable;
    if (index.column() == Column::NAME)
        result |= Qt::ItemIsUserCheckable;
    // Disable to select/check broken modules
    if (!m_plugins[index.row()].isBroken())
        result |= Qt::ItemIsEnabled;
    return static_cast<Qt::ItemFlag>(result);
}

bool PyKrita::Engine::setData(const QModelIndex& index, const QVariant& value, const int role)
{
    Q_ASSERT("Sanity check" && index.row() < m_plugins.size());

    if (role == Qt::CheckStateRole) {
        Q_ASSERT("Sanity check" && !m_plugins[index.row()].isBroken());

        const bool enabled = value.toBool();
        m_plugins[index.row()].m_enabled = enabled;
        if (enabled)
            loadModule(index.row());
        else
            unloadModule(index.row());
    }
    return true;
}

QStringList PyKrita::Engine::enabledPlugins() const
{
    /// \todo \c std::transform + lambda or even better to use
    /// filtered and transformed view from boost
    QStringList result;
    Q_FOREACH(const PluginState & plugin, m_plugins)
    if (plugin.isEnabled()) {
        result.append(plugin.m_pythonPlugin.name());
    }
    return result;
}

void PyKrita::Engine::readGlobalPluginsConfiguration()
{
    Python py = Python();
    PyDict_Clear(m_configuration);
    KConfig config(CONFIG_FILE, KConfig::SimpleConfig);
    config.sync();
    py.updateDictionaryFromConfiguration(m_configuration, &config);
}

void PyKrita::Engine::saveGlobalPluginsConfiguration()
{
    Python py = Python();
    KConfig config(CONFIG_FILE, KConfig::SimpleConfig);
    py.updateConfigurationFromDictionary(&config, m_configuration);
    config.sync();
}

bool PyKrita::Engine::isPythonPluginUsable(const PyPlugin *pythonPlugin)
{
    dbgScript << "Got Krita/PythonPlugin: " << pythonPlugin->name()
              << ", module-path=" << pythonPlugin->library()
              ;
    // Make sure mandatory properties are here
    if (pythonPlugin->name().isEmpty()) {
        dbgScript << "Ignore desktop file w/o a name";
        return false;
    }
    if (pythonPlugin->library().isEmpty()) {
        dbgScript << "Ignore desktop file w/o a module to import";
        return false;
    }
    return true;
}

bool PyKrita::Engine::setModuleProperties(PluginState& plugin)
{
    // Find the module:
    // 0) try to locate directory based plugin first
    QString rel_path = plugin.moduleFilePathPart();
    rel_path = rel_path + "/" + "__init__.py";
    dbgScript << "Finding Pyrhon module with rel_path:" << rel_path;

    QString module_path = KoResourcePaths::findResource("pythonscripts", rel_path);

    dbgScript << "module_path:" << module_path;

    if (module_path.isEmpty()) {
        // 1) Nothing found, then try file based plugin
        rel_path = plugin.moduleFilePathPart() + ".py";
        dbgScript << "Finding Pyrhon module with rel_path:" << rel_path;
        module_path = KoResourcePaths::findResource("pythonscripts", rel_path);
        dbgScript << "module_path:" << module_path;
    } else {
        plugin.m_isDir = true;
    }

    // Is anything found at all?
    if (module_path.isEmpty()) {
        plugin.m_broken = true;
        plugin.m_errorReason = i18nc(
                                   "@info:tooltip"
                                   , "Unable to find the module specified <application>%1</application>"
                                   , plugin.m_pythonPlugin.library()
                               );
        dbgScript << "Cannot load module:" << plugin.m_errorReason;
        return false;
    }
    dbgScript << "Found module path:" << module_path;
    return true;
}

QPair<QString, PyKrita::version_checker> PyKrita::Engine::parseDependency(const QString& d)
{
    // Check if dependency has package info attached
    const int pnfo = d.indexOf('(');
    if (pnfo != -1) {
        QString dependency = d.mid(0, pnfo);
        QString version_str = d.mid(pnfo + 1, d.size() - pnfo - 2).trimmed();
        dbgScript << "Desired version spec [" << dependency << "]:" << version_str;
        version_checker checker = version_checker::fromString(version_str);
        if (!(checker.isValid() && d.endsWith(')'))) {
            dbgScript << "Invalid version spec " << d;
            QString reason = i18nc(
                                 "@info:tooltip"
                                 , "<p>Specified version has invalid format for dependency <application>%1</application>: "
                                 "<icode>%2</icode>. Skipped</p>"
                                 , dependency
                                 , version_str
                             );
            return qMakePair(reason, version_checker());
        }
        return qMakePair(dependency, checker);
    }
    return qMakePair(d, version_checker(version_checker::undefined));
}

PyKrita::version PyKrita::Engine::tryObtainVersionFromTuple(PyObject* version_obj)
{
    Q_ASSERT("Sanity check" && version_obj);

    if (PyTuple_Check(version_obj) == 0)
        return version::invalid();

    int version_info[3] = {0, 0, 0};
    for (unsigned i = 0; i < PyTuple_Size(version_obj); ++i) {
        PyObject* v = PyTuple_GetItem(version_obj, i);
        if (v && PyLong_Check(v))
            version_info[i] = PyLong_AsLong(v);
        else
            version_info[i] = -1;
    }
    if (version_info[0] != -1 && version_info[1] != -1 && version_info[2] != -1)
        return version(version_info[0], version_info[1], version_info[2]);

    return version::invalid();
}

/**
 * Try to parse version string as a simple triplet X.Y.Z.
 *
 * \todo Some modules has letters in a version string...
 * For example current \c pytz version is \e "2013d".
 */
PyKrita::version PyKrita::Engine::tryObtainVersionFromString(PyObject* version_obj)
{
    Q_ASSERT("Sanity check" && version_obj);

    if (!Python::isUnicode(version_obj))
        return version::invalid();

    QString version_str = Python::unicode(version_obj);
    if (version_str.isEmpty())
        return version::invalid();

    return version::fromString(version_str);
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
void PyKrita::Engine::verifyDependenciesSetStatus(PluginState& plugin)
{
    QStringList dependencies = plugin.m_pythonPlugin.property("X-Python-Dependencies").toStringList();
#if PY_MAJOR_VERSION < 3
    {
        // Try to get Py2 only dependencies
        QStringList py2_dependencies = plugin.m_service->property("X-Python-2-Dependencies").toStringList();
        dependencies.append(py2_dependencies);
    }
#endif

    Python py = Python();
    QString reason = i18nc("@info:tooltip", "<title>Dependency check</title>");
    Q_FOREACH(const QString & d, dependencies) {
        QPair<QString, version_checker> info_pair = parseDependency(d);
        version_checker& checker = info_pair.second;
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
                          << "[" << plugin.m_pythonPlugin.name() << "]:\n" << py.lastTraceback()
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
            // PEP396 require __version__ to tuple of integers... try it!
            version dep_version = tryObtainVersionFromTuple(version_obj);
            if (!dep_version.isValid())
                // Second attempt: some "bad" modules have it as a string
                dep_version = tryObtainVersionFromString(version_obj);

            // Did we get it?
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
                          << plugin.m_pythonPlugin.name() << "] for "
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
            dbgScript << "Load failure [" << plugin.m_pythonPlugin.name() << "]:\n" << py.lastTraceback();
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

void PyKrita::Engine::scanPlugins()
{
    m_plugins.clear(); // Clear current state.

    QStringList desktopFiles = KoResourcePaths::findAllResources("data", "pykrita/*desktop");
    qDebug() << desktopFiles;

    Q_FOREACH(const QString &desktopFile, desktopFiles) {

        QSettings s(desktopFile, QSettings::IniFormat);
        s.beginGroup("Desktop Entry");
        if (s.value("ServiceTypes").toString() == "Krita/PythonPlugin") {
            PyPlugin pyplugin;
            pyplugin.m_comment = s.value("Comment").toString();
            pyplugin.m_name = s.value("Name").toString();
            pyplugin.m_libraryPath = s.value("X-KDE-Library").toString();
            pyplugin.m_properties["X-Python-2-Compatible"] = s.value("X-Python-2-Compatible", false).toBool();

            if (!isPythonPluginUsable(&pyplugin)) {
                dbgScript << pyplugin.name() << "is not usable";
                continue;
            }

            PluginState pluginState;
            pluginState.m_pythonPlugin = pyplugin;

            if (!setModuleProperties(pluginState)) {
                dbgScript << "Cannot load" << pyplugin.name() << ": broken"
                          << pluginState.isBroken()
                          << "because:" << pluginState.errorReason();
                continue;
            }

            verifyDependenciesSetStatus(pluginState);
            m_plugins.append(pluginState);
        }
    }
}

void PyKrita::Engine::setEnabledPlugins(const QStringList& enabled_plugins)
{
    for (int i = 0; i < m_plugins.size(); ++i) {
        m_plugins[i].m_enabled = enabled_plugins.indexOf(m_plugins[i].m_pythonPlugin.name()) != -1;
    }
}

void PyKrita::Engine::tryLoadEnabledPlugins()
{
    for (int i = 0; i < m_plugins.size(); ++i) {
        dbgScript << "Trying to load plugin" << m_plugins[i].pythonModuleName()
                  << ". Enabled:" << m_plugins[i].isEnabled()
                  << ". Broken: " << m_plugins[i].isBroken();
        if (!m_plugins[i].isBroken()) {
            m_plugins[i].m_enabled = true;
            loadModule(i);
        }
    }
}

void PyKrita::Engine::loadModule(const int idx)
{
    Q_ASSERT("Plugin index is out of range!" && 0 <= idx && idx < m_plugins.size());
    PluginState& plugin = m_plugins[idx];
    Q_ASSERT(
        "Why to call loadModule() for disabled/broken plugin?"
        && plugin.isEnabled()
        && !plugin.isBroken()
    );

    QString module_name = plugin.pythonModuleName();
    dbgScript << "Loading module: " << module_name;

    Python py = Python();

    // Get 'plugins' key from 'pykrita' module dictionary.
    // Every entry has a module name as a key and 2 elements tuple as a value
    PyObject* plugins = py.itemString("plugins");
    Q_ASSERT(
        "'plugins' dict expected to be alive, otherwise code review required!"
        && plugins
    );

    PyObject* module = py.moduleImport(PQ(module_name));
    if (module) {
        // Move just loaded module to the dict
        const int ins_result = PyDict_SetItemString(plugins, PQ(module_name), module);
        Q_ASSERT("expected successful insertion" && ins_result == 0);
        Py_DECREF(module);
        // Handle failure in release mode.
        if (ins_result == 0) {
            // Initialize the module from Python's side
            PyObject* const args = Py_BuildValue("(s)", PQ(module_name));
            PyObject* result = py.functionCall("_pluginLoaded", Python::PYKRITA_ENGINE, args);
            Py_DECREF(args);
            if (result) {
                dbgScript << "\t" << "success!";
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

void PyKrita::Engine::unloadModule(int idx)
{
    Q_ASSERT("Plugin index is out of range!" && 0 <= idx && idx < m_plugins.size());
    PluginState& plugin = m_plugins[idx];
    Q_ASSERT("Why to call unloadModule() for broken plugin?" && !plugin.isBroken());

    dbgScript << "Unloading module: " << plugin.pythonModuleName();

    Python py = Python();

    // Get 'plugins' key from 'pykrita' module dictionary
    PyObject* plugins = py.itemString("plugins");
    Q_ASSERT(
        "'plugins' dict expected to be alive, otherwise code review required!"
        && plugins
    );

    PyObject* const args = Py_BuildValue("(s)", PQ(plugin.pythonModuleName()));
    py.functionCall("_pluginUnloading", Python::PYKRITA_ENGINE, args);
    Py_DECREF(args);

    // This will just decrement a reference count for module instance
    PyDict_DelItemString(plugins, PQ(plugin.pythonModuleName()));

    // Remove the module also from 'sys.modules' dict to really unload it,
    // so if reloaded all @init actions will work again!
    PyObject* sys_modules = py.itemString("modules", "sys");
    Q_ASSERT("Sanity check" && sys_modules);
    PyDict_DelItemString(sys_modules, PQ(plugin.pythonModuleName()));
}

// krita: space-indent on; indent-width 4;
#undef PYKRITA_INIT
