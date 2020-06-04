/*
 *  Copyright (c) 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "storyboardModel.h"

#include <QDebug>

#include <kis_icon.h>

StoryboardModel::StoryboardModel(QObject *parent)
        : QAbstractItemModel(parent)
{}

QModelIndex StoryboardModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    if (row < 0 || row >= rowCount())
        return QModelIndex();
    if (column !=0)
        return QModelIndex();
    
    //1st level node has invalid parent
    if (!parent.isValid()){
        return createIndex(row, column, m_items.at(row));
    }
    else if (!parent.parent().isValid()){
        StoryboardItem *parentItem = static_cast<StoryboardItem*>(parent.internalPointer());
        StoryboardChild *childItem = parentItem->child(row);
        if (childItem)
            return createIndex(row, column, childItem);
    }
    return QModelIndex();
}

QModelIndex StoryboardModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    {
        //no parent for 1st level node
        StoryboardItem *childItem = static_cast<StoryboardItem*>(index.internalPointer());
        if (m_items.contains(childItem)){
            return QModelIndex();
        }
    }

    //return parent only for 2nd level nodes
    StoryboardChild *childItem = static_cast<StoryboardChild*>(index.internalPointer());
    StoryboardItem *parentItem = childItem->parent();
    int indexOfParent = m_items.indexOf(const_cast<StoryboardItem*>(parentItem));
    return createIndex(indexOfParent, 0, parentItem);
}

int StoryboardModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_items.count();
    else if (!parent.parent().isValid()){
        StoryboardItem *parentItem = static_cast<StoryboardItem*>(parent.internalPointer());
        return parentItem->childCount();
    }
    return 0;   //2nd level nodes have no child
}

int StoryboardModel::columnCount(const QModelIndex &parent) const
{
   if (!parent.isValid()){
       return 1;
   }
   //1st level nodes have 1 column
   if (!parent.parent().isValid()){
       return 1;
   }
   //end level nodes have no child
   return 0;
}

QVariant StoryboardModel::data(const QModelIndex &index, int role) const
{

    if (!index.isValid())
        return QVariant();
    if (index.row() >= m_items.size())
        return QVariant();

    if(role == Qt::SizeHintRole){
            return QSize(200,200);
    }
    //return data only for the storyboardChild i.e. 2nd level nodes
    if (!index.parent().isValid()){
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        StoryboardChild *child = static_cast<StoryboardChild*>(index.internalPointer());
        return child->data();
    }
    return QVariant();
}

bool StoryboardModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    //qDebug()<<"attempting data set"<<role;
    if (index.isValid() && (role == Qt::EditRole || role == Qt::DisplayRole))
    {
        if (!index.parent().isValid())
        return false;

        StoryboardChild *child = static_cast<StoryboardChild*>(index.internalPointer());
        if (child){
            child->setData(value);
            emit dataChanged(index, index);
            return true;
        }
    }
    return false;
}

Qt::ItemFlags StoryboardModel::flags(const QModelIndex & index) const
{
    //qDebug()<<"flags requested";
    if(!index.isValid())
        return Qt::ItemIsDropEnabled;

    //1st level nodes
    if (!index.parent().isValid())
        return Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    //2nd level nodes
    return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
}

bool StoryboardModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    qDebug()<<"row inserted";
    if (!parent.isValid()){
        beginInsertRows(QModelIndex(), position, position+rows-1);
        for (int row = 0; row < rows; ++row) {
            StoryboardItem *newItem = new StoryboardItem();
            m_items.insert(position, newItem);
        }
        endInsertRows();
        return true;
    }

    //insert 2nd level nodes
    if (!parent.parent().isValid()){
        StoryboardItem *item = static_cast<StoryboardItem*>(parent.internalPointer());
        beginInsertRows(QModelIndex(), position, position+rows-1);
        for (int row = 0; row < rows; ++row) {
            item->insertChild(position, QVariant());
        }
        endInsertRows();
        return true;
    }
    //we can't insert to 2nd level nodes as they are leaf nodes
    return false;
}

bool StoryboardModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    qDebug()<<"row removed";
    //remove 1st level nodes
    if (!parent.isValid()){
        beginRemoveRows(QModelIndex(), position, position+rows-1);
        for (int row = 0; row < rows; ++row) {
            delete m_items.at(row);
            m_items.removeAt(position);
        }
        endRemoveRows();
        return true;
    }

    //remove 2nd level nodes
    if (!parent.parent().isValid()){
        StoryboardItem *item = static_cast<StoryboardItem*>(parent.internalPointer());
        if (m_items.contains(item)){
            beginRemoveRows(QModelIndex(), position, position+rows-1);
            for (int row = 0; row < rows; ++row) {
                item->removeChild(position);
            }
            endRemoveRows();
            return true;
        }
    }
    //2nd level node has no child
    return false;
}

Qt::DropActions StoryboardModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::DropActions StoryboardModel::supportedDragActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}
