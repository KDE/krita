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

#ifndef STORYBOARD_ITEM
#define STORYBOARD_ITEM

#include <QVariant>
#include <QVector>

//each storyboardItem contains pointer to child data
class StoryboardItem;

class StoryboardChild
{
public:
    StoryboardChild(QVariant &data, StoryboardItem *parent)
        : m_data(data)
        , m_parentItem(parent)
    {}
    StoryboardItem parent(){ return *m_parentItem;}
    QVariant data(){ return m_data;}
    //returns the row number of this child relative to its parent
    int row() const{
        if (m_parentItem)
            return m_parentItem->m_childData.indexOf(const_cast<StoryboardChild*>(this));

        return 0;
    }
    void setData(QVariant &value){
        m_data = value;
    }

private:
    StoryboardItem *m_parentItem;
    QVariant m_data;
};

class StoryboardItem
{
public:
    //see later if the constructor needs data
    explicit StoryboardItem(/*const QVector<QVariant> &data*/);
    ~StoryboardItem();

    void appendChild(int row, QVariant &data);
    void insertChild(int row, QVariant &data);
    void removeChild(int row);
    void childCount() const;
    StoryboardChild *child(int row);
    QPointer parent(){
        return nullptr;
    }

private:
    QVector<StoryboardChild*> m_childData;
};
