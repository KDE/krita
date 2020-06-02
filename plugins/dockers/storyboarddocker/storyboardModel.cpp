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
        : QAbstractTableModel(parent)
{}

QModelIndex StoryboardModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    if (row < 0 || row >= rowCount())
        return QModelIndex();
    if (column !=0)
        return QModelIndex();
    
    //top level node has invalid parent
    if (!parent.isValid)
        return createIndex(row, column, m_items.at(row));
    else
        StoryboardItem *parentItem = static_cast<StoryboardItem*>(parent.internalPointer());

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex StoryboardModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    //return parent only for 2nd level nodes
    StoryboardChild *childItem = dynamic_cast<StoryboardChild*>(index.internalPointer());
    if(childItem){
        StoryboardItem *parentItem = childItem->parent();
        int indexOfParent = m_items.indexOf(const_cast<StoryboardItem*>(parentItem));
        return createIndex(indexOfParent, 0, parentItem);
    }
    return QModelIndex();
}

int StoryboardModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_items.count();
    else if (dynamic_cast<StoryboardItem*>(parent.internalPointer())){
        StoryboardItem parentItem = dynamic_cast<StoryboardItem*>(parent.internalPointer());
        return parentItem->childCount();
    }
    return 0;   //2nd level nodes have no child
}

int StoryboardModel::columnCount(const QModelIndex &parent) const
{
   //2nd level nodes have no child
   if (dynamic_cast<StoryboardChild*>(parent.internalPointer())){
       return 0;
   }
   return 1;
}

QVariant StoryboardModel::data(const QModelIndex &index, int role) const
{

    if (!index.isValid())
        return QVariant();
    if (index.row() >= m_items.size())
        return QVariant();

    //return data only for the storyboardChild i.e. 2nd level nodes
    if (dynamic_cast<StoryboardItem*>(index.internalPointer()))
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        StoryboardChild *child = static_cast<StoryboardChild*>(index.internalPointer());
        child->data().toS
        if (child){
            switch (index.row()):
            case 0:
                //frame number
                return child->data().toInt();
            case 1:
                //item name
                return child->data().toString();
            case 2:
                //duration
                return child->data().toInt();
            default:
                //comment(would it be string or a custom struct)
                return child->data().toInt();
        }
    }
    return QVariant();
}

bool StoryboardModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    //qDebug()<<"attempting data set"<<role;
    if (dynamic_cast<StoryboardItem*>(index.internalPointer()))
        return false;

    if (index.isValid() && (role == Qt::EditRole || role == Qt::DisplayRole))
    {
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
    if (dynamic_cast<StoryboardItem*>(index.internalPointer()))
        return Qt::ItemIsDragEnabled | Qt::ItemIsSelectable ;

    //2nd level nodes
    return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
}

bool StoryboardModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    //we can't insert to 2nd level nodes as they are leaf nodes
    if (dynamic_cast<StoryboardChild*>(parent.internalPointer())){
       return false;
    }
    //insert 1st level nodes
    if (!parent.isValid()){
        beginInsertRows(QModelIndex(), position, position+rows-1);
        for (int row = 0; row < rows; ++row) {
            StoryboardItem newItem = new StoryboardItem();
            m_items.insert(position, newItem);
        }
        endInsertRows();
        return true;
    }

    //insert 2nd level nodes
    StoryboardItem *item = dynamic_cast<StoryboardItem*>(index.internalPointer());
    if (item){
        beginInsertRows(QModelIndex(), position, position+rows-1);
        for (int row = 0; row < rows; ++row) {
            item->insertChild(position, QVariant());
        }
        endInsertRows();
        return true;
    }
    return false;
}

bool StoryboardModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    qDebug()<<"row removed";
    //2nd level node has no child
    if (dynamic_cast<StoryboardChild*>(parent.internalPointer())){
       return false;
    }
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
    StoryboardItem *item = dynamic_cast<StoryboardItem*>(index.internalPointer());
    if (item){
        beginRemoveRows(QModelIndex(), position, position+rows-1);
        for (int row = 0; row < rows; ++row) {
            item->removeChild(position);
        }
        endRemoveRows();
        return true;
    }
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
