/*
 * This file is part of PyKrita, Krita' Python scripting plugin.
 *
 * Copyright (C) 2013 Alex Turbov <i.zaufi@gmail.com>
 * Copyright (C) 2014-2016 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2017 Jouni Pentikäinen (joupent@gmail.com)
​ *
​ * This library is free software; you can redistribute it and/or
​ * modify it under the terms of the GNU Library General Public
​ * License as published by the Free Software Foundation; either
​ * version 2 of the License, or (at your option) any later version.
​ *
​ * This library is distributed in the hope that it will be useful,
​ * but WITHOUT ANY WARRANTY; without even the implied warranty of
​ * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
​ * Library General Public License for more details.
​ *
​ * You should have received a copy of the GNU Library General Public License
​ * along with this library; see the file COPYING.LIB.  If not, write to
​ * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
​ * Boston, MA 02110-1301, USA.
​ */

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
