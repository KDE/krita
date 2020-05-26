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

StoryboardModel::StoryboardModel(const QStringList &strings, QObject *parent = 0)
        : QAbstractListModel(parent), stringList(strings) {}

int StoryboardModel::rowCount(const QModelIndex &parent) const
{
   return stringList.count();
}

QVariant StoryboardModel::data(const QModelIndex &index, int role) const
{

    //use Qt::DecorationRole to specify the icon
    if(!index.isValid())
        return QVariant();
    if(index.row() >= stringList.size())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
       return stringList.at(index.row());
    }
    return QVariant();
}

bool StoryboardModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    qDebug()<<"attempting data set"<<role;

    if (index.isValid() && (role == Qt::EditRole || role == Qt::DisplayRole))
    {
        //save value from editor to member m_gridData
        qDebug()<<"data set";
        stringList.replace(index.row(), value.toString());
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags StoryboardModel::flags(const QModelIndex & index) const
{
    //qDebug()<<"flags requested";
    if(!index.isValid())
        return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;

    return Qt::ItemIsDragEnabled | Qt::ItemIsSelectable |
           Qt::ItemIsEditable | Qt::ItemIsEnabled ;
}

bool StoryboardModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    qDebug()<<"row inserted";
    beginInsertRows(QModelIndex(), position, position+rows-1);

    for (int row = 0; row < rows; ++row) {
        stringList.insert(position, "");               //maybe set a default name like comment 1
    }

    endInsertRows();
    return true;
}

bool StoryboardModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    qDebug()<<"row removed";
    beginRemoveRows(QModelIndex(), position, position+rows-1);

    for (int row = 0; row < rows; ++row) {
        stringList.removeAt(position);
    }

    endRemoveRows();
    return true;
}

Qt::DropActions StoryboardModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::DropActions StoryboardModel::supportedDragActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}
