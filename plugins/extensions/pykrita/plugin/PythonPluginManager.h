/*
 * This file is part of PyKrita, Krita' Python scripting plugin.
 *
 * SPDX-FileCopyrightText: 2013 Alex Turbov <i.zaufi@gmail.com>
 * SPDX-FileCopyrightText: 2014-2016 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2017 Jouni Pentikäinen (joupent@gmail.com)
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef PYTHONMODULEMANAGER_H
#define PYTHONMODULEMANAGER_H

#include <QObject>
#include "version_checker.h"
#include "PythonPluginsModel.h"

class PythonPluginsModel;

/**
 * Represents a Python described in the plugin's .desktop file.
 */
class PythonPlugin
{
public:
    /**
     * Transforms the Python module name into a file path part
     */
    QString moduleFilePathPart() const;

    bool isValid() const;

    inline const QString& errorReason() const
    {
        return m_errorReason;
    }

    inline bool isEnabled() const
    {
        return m_enabled;
    }

    inline bool isBroken() const
    {
        return m_broken;
    }

    inline bool isUnstable() const
    {
        return m_unstable;
    }

    QString name() const
    {
        return m_name;
    }

    QString moduleName() const
    {
        return m_moduleName;
    }

    QVariant property(const QString &name) const
    {
        return m_properties.value(name, "");
    }

    QString comment() const
    {
        return m_comment;
    }

    QString manual() const
    {
        return m_manual;
    }

private:
    friend class PythonPluginManager;

    PythonPlugin() {
        m_properties["X-Python-Dependencies"] = QStringList();
        m_properties["X-Python-2-Dependencies"] = QStringList();
    }

    QString m_errorReason;
    bool m_enabled{false};
    bool m_broken{false};
    bool m_unstable{false};
    bool m_loaded{false};

    QString m_name;
    QString m_moduleName;
    QString m_comment;
    QString m_manual;

    QMap<QString, QVariant> m_properties;
};

/**
 * The Python plugin manager handles discovery, loading and unloading of Python plugins.
 * To get a reference to the manager, use PyKrita::pluginManager().
 */
class PythonPluginManager : public QObject
{
    Q_OBJECT

public:
    PythonPluginManager();

    const QList<PythonPlugin>& plugins() const;
    PythonPlugin *plugin(int index);

    void scanPlugins();
    void tryLoadEnabledPlugins();
    void setPluginEnabled(PythonPlugin &plugin, bool enabled);

    PythonPluginsModel *model();

public Q_SLOTS:
    void unloadAllModules();

private:
    void loadModule(PythonPlugin &plugin);
    void unloadModule(PythonPlugin &plugin);

    static bool verifyModuleExists(PythonPlugin &);
    static void verifyDependenciesSetStatus(PythonPlugin&);
    static QPair<QString, PyKrita::version_checker> parseDependency(const QString&);

    QList<PythonPlugin> m_plugins;

    PythonPluginsModel m_model;
};

#endif //PYTHONMODULEMANAGER_H
