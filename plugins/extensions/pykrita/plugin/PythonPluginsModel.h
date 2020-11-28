/*
 * This file is part of PyKrita, Krita' Python scripting plugin.
 *
 * SPDX-FileCopyrightText: 2013 Alex Turbov <i.zaufi@gmail.com>
 * SPDX-FileCopyrightText: 2014-2016 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2017 Jouni Pentikäinen (joupent@gmail.com)
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KRITA_PYTHONPLUGINSMODEL_H
#define KRITA_PYTHONPLUGINSMODEL_H

#include <QtCore/QAbstractTableModel>

class PythonPluginManager;
class PythonPlugin;

class PythonPluginsModel : public QAbstractTableModel
{
public:
    PythonPluginsModel(QObject *parent, PythonPluginManager *pluginManager);
    PythonPlugin *plugin(const QModelIndex &) const;

protected:
    enum Column {COl_NAME, COL_COMMENT, COLUMN_COUNT};

    int columnCount(const QModelIndex&) const override;
    int rowCount(const QModelIndex&) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QVariant headerData(int, Qt::Orientation, int) const override;
    QVariant data(const QModelIndex&, int) const override;
    Qt::ItemFlags flags(const QModelIndex&) const override;
    bool setData(const QModelIndex&, const QVariant&, int) override;


private:
    PythonPluginManager *m_pluginManager;
};

#endif //KRITA_PYTHONPLUGINSMODEL_H
