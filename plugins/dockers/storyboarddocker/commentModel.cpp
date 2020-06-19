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
#include <QMimeData>

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

        if (position < 0 || position > m_commentList.size()) return false;
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

bool CommentModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                            const QModelIndex &destinationParent, int destinationChild)
{
    if (destinationChild == sourceRow || destinationChild == sourceRow + 1){
        return false;
    }
    if (destinationChild > sourceRow + count - 1){
        //we adjust for the upward shift, see qt doc for why this is needed
        beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild + count - 1);
        destinationChild = destinationChild - count;
    }
    else {
        beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild);
    }
    for (int row = 0; row < count; row++){
        if (sourceRow < 0 || sourceRow >= m_commentList.size()) return false;
        if (destinationChild + row < 0 || destinationChild + row >= m_commentList.size()) return false;
        m_commentList.move(sourceRow, destinationChild + row);
    }
    endMoveRows();
    return true;
}

QMimeData *CommentModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodeData;

    QDataStream stream(&encodeData, QIODevice::WriteOnly);

    //take the row number of the index where drag started
    foreach (QModelIndex index, indexes){
        if (index.isValid()) {
            int row = index.row();
            stream << row;
        }
    }

    mimeData->setData("application/x-qabstractitemmodeldatalist", encodeData); //default mimetype
    return mimeData;
}

bool CommentModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                int row, int column, const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction)
        return false;

    if (action == Qt::MoveAction && data->hasFormat("application/x-qabstractitemmodeldatalist")){
        QByteArray bytes = data->data("application/x-qabstractitemmodeldatalist");
        QDataStream stream(&bytes, QIODevice::ReadOnly);

        if (parent.isValid()){
            return false;
        }
        int sourceRow;
        QModelIndexList moveRowIndexes;
        while (!stream.atEnd()) {
            stream >> sourceRow;
            QModelIndex index = createIndex(sourceRow, 0);
            moveRowIndexes.append(index);
        }
        moveRows(QModelIndex(), moveRowIndexes.at(0).row(), moveRowIndexes.count(), parent, row);

        //returning true deletes the source row
        return false;
    }
    return false;
}

Qt::DropActions CommentModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::DropActions CommentModel::supportedDragActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}
