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

#include "EditorDataModel.h"
#include "property.h"
#include "set.h"

#include <KLocale>

using namespace KoProperty;

class EditorDataModel::Private
{
public:
    Private() {}
    Set *set;
    Property rootItem;
};

// -------------------

EditorDataModel::EditorDataModel(Set &propertySet, QObject *parent)
        : QAbstractItemModel(parent)
        , d(new Private)
{
    d->set = &propertySet;
}

EditorDataModel::~EditorDataModel()
{
    delete d;
}

int EditorDataModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

QVariant EditorDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    Property *prop = getItem(index);
    const int col = index.column();
    if (col == 0)
        return prop->name();

    return prop->value();
}

Qt::ItemFlags EditorDataModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    const int col = index.column();
    if (col == 0)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

Property *EditorDataModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        Property *item = static_cast<Property*>(index.internalPointer());
        if (item)
            return item;
    }
    return &d->rootItem;
}

QVariant EditorDataModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return i18n("Name");
        } else {
            return i18n("Value");
        }
    }
    return QVariant();
}

QModelIndex EditorDataModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    Property *parentItem = getItem(parent);
    Property *childItem;
    if (parentItem == &d->rootItem) { // special case: top level
        int r = 0;
        Set::Iterator it(*d->set);
        for (; r < row && it.current(); r++, ++it)
            ;
        childItem = it.current();
    } else {
        const QList<Property*>* children = parentItem->children();
        if (!children)
            return QModelIndex();
        childItem = children->value(row);
    }
    if (!childItem)
        return QModelIndex();
    return createIndex(row, column, childItem);
}

/*bool EditorDataModel::insertColumns(int position, int columns, const QModelIndex &parent)
{
    bool success;

    beginInsertColumns(parent, position, position + columns - 1);
    success = rootItem->insertColumns(position, columns);
    endInsertColumns();

    return success;
}

bool EditorDataModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    Property *parentItem = getItem(parent);
    bool success;

    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(position, rows, rootItem->columnCount());
    endInsertRows();

    return success;
}
*/

QModelIndex EditorDataModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    Property *childItem = getItem(index);
    Property *parentItem = childItem->parent();

    if (!parentItem)
        return QModelIndex();

    const QList<Property*>* children = parentItem->children();
    Q_ASSERT(children);
    const int indexOfItem = children->indexOf(childItem);
    Q_ASSERT(indexOfItem != -1);

    return createIndex(indexOfItem, 0, parentItem);
}

/*bool EditorDataModel::removeColumns(int position, int columns, const QModelIndex &parent)
{
    bool success;

    beginRemoveColumns(parent, position, position + columns - 1);
    success = rootItem->removeColumns(position, columns);
    endRemoveColumns();

    if (rootItem->columnCount() == 0)
        removeRows(0, rowCount());

    return success;
}

bool EditorDataModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    Property *parentItem = getItem(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}*/

int EditorDataModel::rowCount(const QModelIndex &parent) const
{
    Property *parentItem = getItem(parent);
    if (!parentItem)
        return 0;
    if (parentItem == &d->rootItem) { // top level
        int count = 0;
        Set::Iterator it(*d->set);
        for (; it.current(); count++, ++it)
            ;
        return count;
    } else {
        const QList<Property*>* children = parentItem->children();
        return children ? children->count() : 0;
    }
}

bool EditorDataModel::setData(const QModelIndex &index, const QVariant &value,
                              int role)
{
    if (role != Qt::EditRole)
        return false;

    Property *item = getItem(index);
    if (item == &d->rootItem)
        return false;
    item->setValue(value);
    return true;
}

bool EditorDataModel::setHeaderData(int section, Qt::Orientation orientation,
                                    const QVariant &value, int role)
{
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(value);
    Q_UNUSED(role);
    return false;
    /*    if (role != Qt::EditRole || orientation != Qt::Horizontal)
            return false;

        return rootItem->setData(section, value);*/
}

/*
void EditorDataModel::setupModelData(const QStringList &lines, Property *parent)
{
    QList<TreeItem*> parents;
    QList<int> indentations;
    parents << parent;
    indentations << 0;

    int number = 0;

    while (number < lines.count()) {
        int position = 0;
        while (position < lines[number].length()) {
            if (lines[number].mid(position, 1) != " ")
                break;
            position++;
        }

        QString lineData = lines[number].mid(position).trimmed();

        if (!lineData.isEmpty()) {
            // Read the column data from the rest of the line.
            QStringList columnStrings = lineData.split("\t", QString::SkipEmptyParts);
            QVector<QVariant> columnData;
            for (int column = 0; column < columnStrings.count(); ++column)
                columnData << columnStrings[column];

            if (position > indentations.last()) {
                // The last child of the current parent is now the new parent
                // unless the current parent has no children.

                if (parents.last()->childCount() > 0) {
                    parents << parents.last()->child(parents.last()->childCount()-1);
                    indentations << position;
                }
            } else {
                while (position < indentations.last() && parents.count() > 0) {
                    parents.pop_back();
                    indentations.pop_back();
                }
            }

            // Append a new item to the current parent's list of children.
            Property *parent = parents.last();
            parent->insertChildren(parent->childCount(), 1, rootItem->columnCount());
            for (int column = 0; column < columnData.size(); ++column)
                parent->child(parent->childCount() - 1)->setData(column, columnData[column]);
        }

        number++;
    }
}
*/

#include "EditorDataModel.moc"
