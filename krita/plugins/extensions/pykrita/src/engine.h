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

#ifndef __PYKRITA_ENGINE_H__
# define  __PYKRITA_ENGINE_H__

# include "version_checker.h"

# include <kservice.h>
# include <kurl.h>

# include <Python.h>

# include <QAbstractItemModel>
# include <QList>
# include <QStringList>

namespace PyKrita
{
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

        KService::Ptr m_service;
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

    void readSessionPluginsConfiguration(KConfigBase*);
    void writeSessionPluginsConfiguration(KConfigBase*);

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

    static bool isServiceUsable(const KService::Ptr&);      ///< Make sure that service is usable
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
    return m_service->library();
}
inline QString PyKrita::Engine::PluginState::moduleFilePathPart() const
{
    /// \todo Use \c QString::split() and \c KUrl to form a valid path
    return m_service->library().replace(".", "/");
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
// krita: indent-width 4;
