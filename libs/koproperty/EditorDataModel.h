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

#include "Set.h"

namespace KoProperty
{

class Property;

/*! @short A data model for using Set objects within the Qt's model/view API.
 @see EditorView
*/
class KOPROPERTY_EXPORT EditorDataModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    EditorDataModel(Set &propertySet, QObject *parent = 0,
                    Set::Order order = Set::InsertionOrder);
    ~EditorDataModel();

    enum Role {
        PropertyModifiedRole = Qt::UserRole + 0
    };
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

    QModelIndex buddy(const QModelIndex & index) const;

    //! @return property set object for this model.
    Set& propertySet() const;

    //! @return property object for model index @a index
    //! or 0 for invalid index or index without property assigned.
    Property *propertyForItem(const QModelIndex& index) const;

    //! @return model index for property named @a propertyName
    //! or invalid index if such property could not be found.
    QModelIndex indexForPropertyName(const QByteArray& propertyName) const;

    //! @return a sibling for model index @a index and columnd @a column
    QModelIndex indexForColumn(const QModelIndex& index, int column) const;

    //! Sets order for properties. Restarts the iterator.
    void setOrder(Set::Order order);

    //! @return order for properties.
    Set::Order order() const;

    //! Reimplemented for optimization.
    bool hasChildren(const QModelIndex & parent = QModelIndex()) const;
private:
//    void setupModelData(const QStringList &lines, TreeItem *parent);
    void collectIndices() const;

    class Private;
    Private * const d;
};

}
#endif
