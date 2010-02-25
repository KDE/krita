/* This file is part of the KDE project
   Copyright (C) 2008-2009 Jarosław Staniek <staniek@kde.org>

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
#include "Factory.h"
#include "Property.h"
#include "Set.h"

#include <QtCore/QHash>

#include <KLocale>
#include <kdebug.h>

using namespace KoProperty;

class EditorDataModel::Private
{
public:
    Private(Set *_set, Set::Order _order = Set::InsertionOrder) : set(_set), order(_order)
    {
    }
    Set *set;
    Property rootItem;
    QHash<QByteArray, QPersistentModelIndex> indicesForNames;
    Set::Order order; //!< order of properties
};

// -------------------

//! A property selector offering functor selecting only visible properties.
/*! Used e.g. in EditorDataModel::index(). */
class VisiblePropertySelector : public Set::PropertySelector
{
public:
    VisiblePropertySelector() {}
    virtual bool operator()(const Property& prop) const {
        return prop.isVisible();
    }
    Set::PropertySelector* clone() const { return new VisiblePropertySelector(); }
};

// -------------------

EditorDataModel::EditorDataModel(Set &propertySet, QObject *parent,
                                 Set::Order order)
        : QAbstractItemModel(parent)
        , d(new Private(&propertySet, order))
{
    collectIndices();
}

EditorDataModel::~EditorDataModel()
{
    delete d;
}

typedef QPair<QByteArray, QString> NameAndCaption;

bool nameAndCaptionLessThan(const NameAndCaption &n1, const NameAndCaption &n2)
{
    return QString::compare(n1.second, n2.second, Qt::CaseInsensitive) < 0;
}

void EditorDataModel::collectIndices() const
{
    Set::Iterator it(*d->set, VisiblePropertySelector());
    if (d->order == Set::AlphabeticalOrder) {
        it.setOrder(Set::AlphabeticalOrder);
    }
    d->indicesForNames.clear();
    for (int row = 0; it.current(); row++, ++it) {
        d->indicesForNames.insert( it.current()->name(), QPersistentModelIndex( createIndex(row, 0, it.current()) ) );
    }
}

QModelIndex EditorDataModel::indexForPropertyName(const QByteArray& propertyName) const
{
    return (const QModelIndex &)d->indicesForNames.value(propertyName);
}

QModelIndex EditorDataModel::indexForColumn(const QModelIndex& index, int column) const
{
    if (column == 0)
        return index;
    return createIndex(index.row(), column, propertyForItem(index));
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

    const int col = index.column();
    if (col == 0) {
        Property *prop = propertyForItem(index);
        if (role == Qt::DisplayRole) {
            if (!prop->caption().isEmpty())
                return prop->caption();
            return prop->name();
        }
        else if (role == PropertyModifiedRole) {
            return prop->isModified();
        }
    }
    else if (col == 1) {
        Property *prop = propertyForItem(index);
        if (role == Qt::EditRole) {
            return prop->value();
        }
        else if (role == Qt::DisplayRole) {
            return FactoryManager::self()->convertValueToText(prop);
        }
    }
    return QVariant();
}

Qt::ItemFlags EditorDataModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    const int col = index.column();
//    if (col == 0)
//buddy...        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;

    Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    Property *prop = propertyForItem(index);
    if (prop) {
//        if (!prop->children()) {
        if (col == 1) {
            f |= Qt::ItemIsEditable;
        }
//        if (col != 1 || !prop->children()) {
//            f |= Qt::ItemIsSelectable;
//        }
    }
    return f;
}

Property *EditorDataModel::propertyForItem(const QModelIndex &index) const
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
            return i18nc("Property name", "Name");
        } else {
            return i18nc("Property value", "Value");
        }
    }
    return QVariant();
}

QModelIndex EditorDataModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    Property *parentItem = propertyForItem(parent);
    Property *childItem;
    if (parentItem == &d->rootItem) { // special case: top level
        int visibleRows = 0;
        Set::Iterator it(*d->set, VisiblePropertySelector());
        if (d->order == Set::AlphabeticalOrder) {
            it.setOrder(Set::AlphabeticalOrder);
        }
//! @todo use qBinaryFind()?
        for (; visibleRows < row && it.current(); visibleRows++, ++it)
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
    Property *parentItem = propertyForItem(parent);
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

    Property *childItem = propertyForItem(index);
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
    Property *parentItem = propertyForItem(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}*/

int EditorDataModel::rowCount(const QModelIndex &parent) const
{
    Property *parentItem = propertyForItem(parent);
    if (!parentItem || parentItem == &d->rootItem) { // top level
        return d->set->count(VisiblePropertySelector());
    }
    const QList<Property*>* children = parentItem->children();
    return children ? children->count() : 0;
/* prev
    Property *parentItem = propertyForItem(parent);
    if (!parentItem)
        return 0;
    if (parentItem == &d->rootItem) { // top level
        int count = 0;
        Set::Iterator it(*d->set, VisiblePropertySelector());
        for (; it.current(); count++, ++it)
            ;
        return count;
    } else {
        const QList<Property*>* children = parentItem->children();
        return children ? children->count() : 0;
    }*/
}

bool EditorDataModel::setData(const QModelIndex &index, const QVariant &value,
                              int role)
{
    if (role != Qt::EditRole)
        return false;

    Property *item = propertyForItem(index);
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

QModelIndex EditorDataModel::buddy(const QModelIndex & idx) const
{
    if (idx.column() == 0)
        return index( idx.row(), 1, parent(idx));
    return idx;
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

Set& EditorDataModel::propertySet() const
{
    return *d->set;
}

void EditorDataModel::setOrder(Set::Order order)
{
    if (d->order != order) {
        d->order = order;
        collectIndices();
    }
}

Set::Order EditorDataModel::order() const
{
    return d->order;
}

bool EditorDataModel::hasChildren(const QModelIndex & parent) const
{
    Property *parentItem = propertyForItem(parent);
    if (!parentItem || parentItem == &d->rootItem) { // top level
        return d->set->hasVisibleProperties();
    }
    const QList<Property*>* children = parentItem->children();
    return children && !children->isEmpty();
}

#include "EditorDataModel.moc"
