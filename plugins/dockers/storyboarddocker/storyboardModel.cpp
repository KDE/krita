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

struct CommentHeader 
{
    QString name; 
    bool visiblity;
};

struct StoryboardItem
{
    int startFrame;
    QString name;
    int duration;
    QStringList comments;

    StoryboardItem(int _startFrame = 0, QString _name = "", int _duration = 0)
        : startFrame(_startFrame)
        , name(_name)
        , duration(std::max(0, _duration)
};

struct StoryboardModel::Private
{
    Private()
        //initialize
    {}

    //KisImageWSP image;
    
    QVector<StoryboardItem> items;
    QVector<CommentHeader> commentHeader;

    //functions
};

StoryboardModel::StoryboardModel(const QStringList &strings, QObject *parent)
        : QAbstractTableModel(parent)
        , m_d(new Private()) 
{
    //initialize variables
}

int StoryboardModel::rowCount(const QModelIndex &parent) const
{
   return m_d->items.count();
}

int StoryboardModel::rowCount(const QModelIndex &parent) const
{
   return m_d->commentHeader.count() + 3;
}

QVariant StoryboardModel::data(const QModelIndex &index, int role) const
{
    /*
    if(!index.isValid())
        return QVariant();
    if(index.row() >= m_d->item.size())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
       return m_d->items.at(index.row());
    }
    */
    return QVariant();
    
}

QVariant KisMetaDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && 
        (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::DecorationRole) 
    {
        if(section > 3) {
            switch (role) {
            case Qt::DecorationRole:
                if (m_d->commentHeader[section-3].visibility){
                    //return no-visible icon
                }
                else{
                    //return visible icon
                }
            case (Qt::DisplayRole||Qt::EditRole):
                return m_d->commentHeader[section-3].name;
            }
        }
    }
    return QVariant();
}

bool StoryboardModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    /*
    qDebug()<<"attempting data set"<<role;

    if (index.isValid() && (role == Qt::EditRole || role == Qt::DisplayRole))
    {
        if( value.toInt)
        stringList.replace(index.row(), value.toString());
        emit dataChanged(index, index);
        return true;
    }
    */
    return false;
}

bool KisMetaDataModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (orientation == Qt::Horizontal && 
        (role == Qt::DisplayRole || role == Qt::EditRole) 
    {
        if(section > 3) {
            m_d->commentHeader.replace(section -3, (CommentHeader)value);
            return true;
        }
    }
    return false
}

Qt::ItemFlags StoryboardModel::flags(const QModelIndex & index) const
{
    //qDebug()<<"flags requested";
    if(!index.isValid())
        return Qt::ItemIsDropEnabled;

    return Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled|
           Qt::ItemIsEditable | Qt::ItemIsEnabled ;
}

bool StoryboardModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    qDebug()<<"row inserted";
    beginInsertRows(QModelIndex(), position, position+rows-1);

    for (int row = 0; row < rows; ++row) {
        StoryboardItem item()
        
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
