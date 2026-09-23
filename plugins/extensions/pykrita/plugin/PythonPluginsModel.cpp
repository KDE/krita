/*
 * This file is part of PyKrita, Krita' Python scripting plugin.
 *
 * SPDX-FileCopyrightText: 2013 Alex Turbov <i.zaufi@gmail.com>
 * SPDX-FileCopyrightText: 2014-2016 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2017 Jouni Pentikäinen (joupent@gmail.com)
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "PythonPluginsModel.h"

#include <QApplication>

#include <kcolorscheme.h>
#include <KLocalizedString>

#include "PythonPluginManager.h"

PythonPluginsModel::PythonPluginsModel(QObject *parent, PythonPluginManager *pluginManager)
    : QAbstractTableModel(parent)
    , m_pluginManager(pluginManager)
{
}

int PythonPluginsModel::columnCount(const QModelIndex&) const
{
    return COLUMN_COUNT;
}

int PythonPluginsModel::rowCount(const QModelIndex&) const
{
    return m_pluginManager->plugins().size();
}

QModelIndex PythonPluginsModel::index(const int row, const int column, const QModelIndex& parent) const
{
    if (!parent.isValid() && column < COLUMN_COUNT) {
        PythonPlugin *plugin = m_pluginManager->plugin(row);
        if (plugin) {
            return createIndex(row, column, plugin);
        }
    }

    return QModelIndex();
}

QVariant PythonPluginsModel::headerData(const int section, const Qt::Orientation orientation, const int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case COL_NAME:
            return i18nc("@title:column", "Name");
        case COL_COMMENT:
            return i18nc("@title:column", "Comment");
        case COL_STATUS:
            return i18nc("@title:column", "Status");
        default:
            break;
        }
    }
    return QVariant();
}

QVariant PythonPluginsModel::data(const QModelIndex& index, const int role) const
{
    if (index.isValid()) {
        PythonPlugin *plugin = static_cast<PythonPlugin*>(index.internalPointer());
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(plugin, QVariant());

        switch (role) {
        case Qt::DisplayRole:
            switch (index.column()) {
            case COL_NAME:
                return plugin->name();
            case COL_COMMENT:
                return plugin->comment();
            case COL_STATUS:
                return plugin->statusText();
            default:
                break;
            }
            break;
        case Qt::CheckStateRole:
            if (index.column() == COL_ENABLED) {
                return plugin->isEnabled() ? Qt::Checked : Qt::Unchecked;
            }
            break;
        case Qt::ToolTipRole:
            if (index.column() == COL_COMMENT) {
                // Show comment in case it was elided
                return plugin->comment();
            } else {
                const QString error = plugin->errorReason();
                if (!error.isEmpty()) {
                    return error;
                }
            }
            break;
        case Qt::ForegroundRole:
            if (plugin->isUnstable() || !plugin->errorReason().isEmpty()) {
                if (plugin->isEnabled()) {
                    KColorScheme scheme(QPalette::Active, KColorScheme::View);
                    return scheme.foreground(KColorScheme::NegativeText).color();
                } else {
                    KColorScheme scheme(QPalette::Disabled, KColorScheme::View);
                    return scheme.foreground(KColorScheme::NegativeText).color();
                }
            } else if (!plugin->isEnabled()) {
                // Show disabled plugins with a disabled color,
                // without actually disabling the widget (which prevents selecting it)
                return qApp->palette().color(QPalette::Disabled, QPalette::WindowText);
            }
            break;
        default:
            break;
        }
    }

    return QVariant();
}

Qt::ItemFlags PythonPluginsModel::flags(const QModelIndex& index) const
{
    PythonPlugin *plugin = static_cast<PythonPlugin*>(index.internalPointer());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(plugin, Qt::ItemIsSelectable);

    int result = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (index.column() == COL_ENABLED) {
        result |= Qt::ItemIsUserCheckable;
    }

    return static_cast<Qt::ItemFlag>(result);
}

bool PythonPluginsModel::setData(const QModelIndex& index, const QVariant& value, const int role)
{
    PythonPlugin *plugin = static_cast<PythonPlugin*>(index.internalPointer());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(plugin, false);

    if (role == Qt::CheckStateRole) {
        m_pluginManager->setPluginEnabled(*plugin, value.toBool());
    }
    return true;
}

PythonPlugin *PythonPluginsModel::plugin(const QModelIndex &index) const
{
    if (index.isValid()) {
        PythonPlugin * plugin = static_cast<PythonPlugin*>(index.internalPointer());
        return plugin;
    }
    return 0;
}
