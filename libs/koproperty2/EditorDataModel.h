/* This file is part of the KDE project
   Copyright (C) 2008 Jarosław Staniek <staniek@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KPROPERTY_EDITORDATAMODEL_H
#define KPROPERTY_EDITORDATAMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include "koproperty_export.h"

namespace KoProperty
{

class Property;
class Set;

class KOPROPERTY_EXPORT EditorDataModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    EditorDataModel(Set &propertySet, QObject *parent = 0);
    ~EditorDataModel();

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole);
    bool setHeaderData(int section, Qt::Orientation orientation,
                       const QVariant &value, int role = Qt::EditRole);

    /*  bool insertColumns(int position, int columns,
                         const QModelIndex &parent = QModelIndex());
      bool removeColumns(int position, int columns,
                         const QModelIndex &parent = QModelIndex());
      bool insertRows(int position, int rows,
                      const QModelIndex &parent = QModelIndex());
      bool removeRows(int position, int rows,
                      const QModelIndex &parent = QModelIndex());*/

private:
//    void setupModelData(const QStringList &lines, TreeItem *parent);
    Property *getItem(const QModelIndex &index) const;

    class Private;
    Private * const d;
};

}
#endif
