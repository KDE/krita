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

#include <kcolorscheme.h>
#include <KI18n/KLocalizedString>

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
        auto *plugin = m_pluginManager->plugin(row);
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
            case COl_NAME:
                return i18nc("@title:column", "Name");
            case COL_COMMENT:
                return i18nc("@title:column", "Comment");
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
                    case COl_NAME:
                        return plugin->name();
                    case COL_COMMENT:
                        return plugin->comment();
                    default:
                        break;
                }
                break;
            case Qt::CheckStateRole:
                if (index.column() == COl_NAME) {
                    const bool checked = plugin->isEnabled();
                    return checked ? Qt::Checked : Qt::Unchecked;
                }
                break;
            case Qt::ToolTipRole:
                {
                    auto error = plugin->errorReason();
                    if (!error.isEmpty()) {
                        return error;
                    }
                }
                break;
            case Qt::ForegroundRole:
                if (plugin->isUnstable()) {
                    KColorScheme scheme(QPalette::Inactive, KColorScheme::View);
                    return scheme.foreground(KColorScheme::NegativeText).color();
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

    int result = Qt::ItemIsSelectable;
    if (index.column() == COl_NAME) {
        result |= Qt::ItemIsUserCheckable;
    }

    // Disable UI for broken modules
    if (!plugin->isBroken()) {
        result |= Qt::ItemIsEnabled;
    }

    return static_cast<Qt::ItemFlag>(result);
}

bool PythonPluginsModel::setData(const QModelIndex& index, const QVariant& value, const int role)
{
    PythonPlugin *plugin = static_cast<PythonPlugin*>(index.internalPointer());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(plugin, false);

    if (role == Qt::CheckStateRole) {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!plugin->isBroken(), false);

        const bool enabled = value.toBool();
        m_pluginManager->setPluginEnabled(*plugin, enabled);
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
