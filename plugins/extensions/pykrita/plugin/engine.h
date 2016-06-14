/*
 * This file is part of PyKrita, Krita' Python scripting plugin.
 *
 * Copyright (C) 2013 Alex Turbov <i.zaufi@gmail.com>
 * Copyright (C) 2014-2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) version 3.
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

#ifndef __PYKRITA_ENGINE_H__
# define  __PYKRITA_ENGINE_H__

# include <Python.h>

# include "version_checker.h"

# include <QAbstractItemModel>
# include <QList>
# include <QStringList>

namespace PyKrita
{

/**
 * @brief The PyPlugin class describes a plugin written in Python and loaded into the system
 */
class PyPlugin {

public:

    QString name() const
    {
        return m_name;
    }

    QString library() const
    {
        return m_libraryPath;
    }

    QVariant property(const QString &name) const
    {
        return m_properties.value(name, "");
    }

    QString comment() const
    {
        return m_comment;
    }

private:

    QString m_name;
    QString m_libraryPath;
    QMap<QString, QVariant> m_properties;
    QString m_comment;
};

class Python;                                               // fwd decl

/**
 * The Engine class hosts the Python interpreter, loading
 * it into memory within Krita, and then with finding and
 * loading all of the PyKrita plugins.
 *
 * \attention Qt/KDE do not use exceptions (unfortunately),
 * so this class must be initialized in two steps:
 * - create an instance (via constructor)
 * - try to initialize the rest (via \c Engine::tryInitializeGetFailureReason())
 * If latter returns a non empty (failure reason) string, the only member
 * can be called is conversion to boolean! (which is implemented as safe-bool idiom [1])
 * Calling others leads to UB!
 *
 * \sa [1] http://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Safe_bool
 */
class Engine : public QAbstractItemModel
{
    Q_OBJECT

    typedef void (Engine::*bool_type)() const;
    void unspecified_true_bool_type() const {}

public:
    /// \todo Turn into a class w/ accessors
    class PluginState
    {
    public:
        /// \name Immutable accessors
        //@{
        QString pythonModuleName() const;
        const QString& errorReason() const;
        bool isEnabled() const;
        bool isBroken() const;
        bool isUnstable() const;
        //@}

    private:
        friend class Engine;

        PluginState();
        /// Transfort Python module name into a file path part
        QString moduleFilePathPart() const;

        PyPlugin *m_pythonPlugin;
        QString m_pythonModule;
        QString m_errorReason;
        bool m_enabled;
        bool m_broken;
        bool m_unstable;
        bool m_isDir;
    };

    /// Default constructor: initialize Python interpreter
    Engine();
    /// Cleanup everything on unload
    ~Engine();

    //BEGIN QAbstractItemModel interface
    virtual int columnCount(const QModelIndex&) const /*override*/;
    virtual int rowCount(const QModelIndex&) const /*override*/;
    virtual QModelIndex index(int, int, const QModelIndex&) const /*override*/;
    virtual QModelIndex parent(const QModelIndex&) const /*override*/;
    virtual QVariant headerData(int, Qt::Orientation, int) const /*override*/;
    virtual QVariant data(const QModelIndex&, int) const /*override*/;
    virtual Qt::ItemFlags flags(const QModelIndex&) const /*override*/;
    virtual bool setData(const QModelIndex&, const QVariant&, int) /*override*/;
    //END QAbstractItemModel interface

    void setEnabledPlugins(const QStringList&);             ///< Set enabled plugins to the model
    void tryLoadEnabledPlugins();                           ///< Try to load enabled plugins
    QStringList enabledPlugins() const;                     ///< Form a list of enabled plugins
    const QList<PluginState>& plugins() const;              ///< Provide immutable access to found plugins
    QString tryInitializeGetFailureReason();                ///< Try to initialize Python interpreter
    operator bool_type() const;                             ///< Check if instance is usable
    void setBroken();                                       ///< Make it broken by some external reason

public Q_SLOTS:
    void readGlobalPluginsConfiguration();                  ///< Load plugins' configuration.
    void saveGlobalPluginsConfiguration();                  ///< Write out plugins' configuration.
    void unloadAllModules();

protected:
    void scanPlugins();                                     ///< Search for available plugins
    void loadModule(int);                                   ///< Load module by index in \c m_plugins
    void unloadModule(int);                                 ///< Unload module by index in \c m_plugins

private:
    // Simulate strong typed enums from C++11
    struct Column {
        enum type {
            NAME
            , COMMENT
            , LAST__
        };
    };

    static bool isPythonPluginUsable(const PyPlugin *pyPlugin);      ///< Make sure that service is usable
    static bool setModuleProperties(PluginState&);
    static void verifyDependenciesSetStatus(PluginState&);
    static QPair<QString, version_checker> parseDependency(const QString&);
    static version tryObtainVersionFromTuple(PyObject*);
    static version tryObtainVersionFromString(PyObject*);

    PyObject* m_configuration;                              ///< Application-wide configuration data
    PyObject* m_sessionConfiguration;                       ///< Session-wide configuration data
    QList<PluginState> m_plugins;                           ///< List of available plugins
    bool m_engineIsUsable;                                  ///< Is engine loaded Ok?
};

inline QString Engine::PluginState::pythonModuleName() const
{
    return m_pythonPlugin->library();
}
inline QString PyKrita::Engine::PluginState::moduleFilePathPart() const
{
    return m_pythonPlugin->library().replace(".", "/");
}
inline const QString& Engine::PluginState::errorReason() const
{
    return m_errorReason;
}
inline bool Engine::PluginState::isEnabled() const
{
    return m_enabled;
}
inline bool Engine::PluginState::isBroken() const
{
    return m_broken;
}
inline bool Engine::PluginState::isUnstable() const
{
    return m_unstable;
}

inline const QList<Engine::PluginState>& Engine::plugins() const
{
    return m_plugins;
}

inline Engine::operator bool_type() const
{
    return m_engineIsUsable ? &Engine::unspecified_true_bool_type : 0;
}

inline void Engine::setBroken()
{
    m_engineIsUsable = false;
}

}                                                           // namespace PyKrita
#endif                                                      //  __PYKRITA_ENGINE_H__
