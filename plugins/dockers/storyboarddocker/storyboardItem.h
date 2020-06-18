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
    StoryboardChild(QVariant data, StoryboardItem *parent)
        : m_data(data)
        , m_parentItem(parent)
    {}
    StoryboardItem *parent(){ return m_parentItem;}
    QVariant data(){ return m_data;}
    void setData(QVariant value){
        m_data = value;
    }

private:
    QVariant m_data;
    StoryboardItem *m_parentItem;
};

class StoryboardItem
{
public:
    explicit StoryboardItem();
    ~StoryboardItem();

    void appendChild(QVariant data = QVariant());
    void insertChild(int row, QVariant data = QVariant());
    void removeChild(int row);
    void moveChild(int from, int to);
    int childCount() const;
    StoryboardChild *child(int row);

private:
    QVector<StoryboardChild*> m_childData;
};

#endif
