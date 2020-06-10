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

#include "commentModel.h"

#include <QDebug>

#include <kis_icon.h>

CommentModel::CommentModel(QObject *parent)
        : QAbstractListModel(parent) 
{
    //initialize variables
}

int CommentModel::rowCount(const QModelIndex &parent) const
{
   return m_commentList.count();
}

QVariant CommentModel::data(const QModelIndex &index, int role) const
{
    
    if(!index.isValid())
        return QVariant();
    if(index.row() >= m_commentList.size())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_commentList.at(index.row()).name;
    }
    if (role == Qt::DecorationRole) {
        if (m_commentList.at(index.row()).visibility){
            return KisIconUtils::loadIcon("visible");
        }
        else {
            return KisIconUtils::loadIcon("novisible");
        }
    }
    return QVariant();
    
}

bool CommentModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (index.isValid() && (role == Qt::EditRole || role == Qt::DisplayRole))
    {
        
        m_commentList[index.row()].name = value.toString();
        emit dataChanged(index, index);
        return true;
    }
    
    if (index.isValid() && role == Qt::DecorationRole){
        m_commentList[index.row()].visibility = !m_commentList[index.row()].visibility;
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags CommentModel::flags(const QModelIndex & index) const
{
    if(!index.isValid())
        return Qt::ItemIsDropEnabled;

    return Qt::ItemIsDragEnabled | Qt::ItemIsSelectable |
           Qt::ItemIsEditable | Qt::ItemIsEnabled ;
}

bool CommentModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    beginInsertRows(QModelIndex(), position, position+rows-1);

    for (int row = 0; row < rows; ++row) {
        Comment newcomment;                       //maybe set a default name like comment 1
        newcomment.name = "";
        newcomment.visibility = true;

        if (position < 0 && position>=m_commentList.size()) return false;
        m_commentList.insert(position, newcomment);
    }

    endInsertRows();
    return true;
}

bool CommentModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    beginRemoveRows(QModelIndex(), position, position+rows-1);

    for (int row = 0; row < rows; ++row) {
        if (position < 0 || position >= m_commentList.size()) return false;
        m_commentList.removeAt(position);
    }

    endRemoveRows();
    return true;
}

Qt::DropActions CommentModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::DropActions CommentModel::supportedDragActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}
