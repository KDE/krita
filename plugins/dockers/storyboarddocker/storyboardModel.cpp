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
        , m_commentCount(0)
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
    if (index.isValid() && (role == Qt::EditRole || role == Qt::DisplayRole))
    {
        if (!index.parent().isValid())
        return false;

        StoryboardChild *child = static_cast<StoryboardChild*>(index.internalPointer());
        if (child){
            int fps = 24;
            if(index.row() == 3 && value.toInt() >= fps){         //TODO : set fps
                QModelIndex secondIndex = index.siblingAtRow(2);
                setData(secondIndex, secondIndex.data().toInt() + value.toInt() / fps, role);
                child->setData(value.toInt() % fps);
            }
            else {
                child->setData(value);
            }
            emit dataChanged(index, index);
            return true;
        }
    }
    return false;
}

Qt::ItemFlags StoryboardModel::flags(const QModelIndex & index) const
{
    if(!index.isValid())
        return Qt::ItemIsDropEnabled;

    //1st level nodes
    if (!index.parent().isValid())
        return Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    //2nd level nodes
    return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
}

bool StoryboardModel::insertRows(int position, int rows, const QModelIndex &parent)
{
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

bool StoryboardModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                                const QModelIndex &destinationParent, int destinationChild)
{
    if (sourceParent != destinationParent){
        return false;
    }
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

    //only implementing for moves within the 1st level nodes for comment nodes
    if (sourceParent == destinationParent && !sourceParent.parent().isValid()){
        const QModelIndex parent = sourceParent;
        for (int row = 0; row < count; row++){
            if (sourceRow < 4 || sourceRow >= rowCount(parent)) return false;
            if (destinationChild + row < 4 || destinationChild + row >= rowCount(parent)) return false;

            StoryboardItem *item = static_cast<StoryboardItem*>(parent.internalPointer());
            item->moveChild(sourceRow, destinationChild + row);
        }
        endMoveRows();
        return true;
    }
    else {
        return false;
    }
}

Qt::DropActions StoryboardModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::DropActions StoryboardModel::supportedDragActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

int StoryboardModel::visibleCommentCount() const
{
    int visibleComments = 0;
    foreach(Comment comment, m_commentList){
        if (comment.visibility){
            visibleComments++;
        }
    }
    return visibleComments;
}

int StoryboardModel::visibleCommentsUpto(QModelIndex index) const
{
    int commentRow = index.row() - 4;
    int visibleComments = 0;
    for (int row = 0; row < commentRow; row++){
        if (m_commentList.at(row).visibility){
            visibleComments++;
        }
    }
    return visibleComments;
}

void StoryboardModel::setCommentModel(CommentModel *commentModel)
{
    m_commentModel = commentModel;
    connect(m_commentModel, SIGNAL(dataChanged(const QModelIndex ,const QModelIndex)),
                this, SLOT(slotCommentDataChanged()));
    connect(m_commentModel, SIGNAL(rowsRemoved(const QModelIndex ,int, int)),
                this, SLOT(slotCommentRowRemoved(const QModelIndex ,int, int)));
    connect(m_commentModel, SIGNAL(rowsInserted(const QModelIndex, int, int)),
                this, SLOT(slotCommentRowInserted(const QModelIndex, int, int)));
    connect(m_commentModel, SIGNAL(rowsMoved(const QModelIndex, int, int, const QModelIndex, int)),
                this, SLOT(slotCommentRowMoved(const QModelIndex, int, int, const QModelIndex, int)));
}

Comment StoryboardModel::getComment(int row) const
{
    return m_commentList.at(row);
}

void StoryboardModel::slotCommentDataChanged()
{
    m_commentList = m_commentModel->m_commentList;
    emit(dataChanged(QModelIndex(), QModelIndex()));
}

void StoryboardModel::slotCommentRowInserted(const QModelIndex parent, int first, int last)
{
    int numItems = rowCount();
    for(int row = 0; row < numItems; row++){
        QModelIndex parentIndex = index(row, 0);
        insertRows(4 + first, last - first + 1, parentIndex);       //four indices are already there
    }
    slotCommentDataChanged();
}

void StoryboardModel::slotCommentRowRemoved(const QModelIndex parent, int first, int last)
{
    int numItems = rowCount();
    for(int row = 0; row < numItems; row++){
        QModelIndex parentIndex = index(row, 0);
        removeRows(4 + first, last - first + 1, parentIndex);
    }
    slotCommentDataChanged();
}

void StoryboardModel::slotCommentRowMoved(const QModelIndex &sourceParent, int start, int end,
                            const QModelIndex &destinationParent, int destinationRow)
{
    int numItems = rowCount();
    for(int row = 0; row < numItems; row++){
        QModelIndex parentIndex = index(row, 0);
        moveRows(parentIndex, start + 4, end - start + 1, parentIndex, destinationRow + 4);
    }
    slotCommentDataChanged();
}