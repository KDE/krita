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

#include "storyboardItem.h"

//create 3 rows by default
StoryboardItem::StoryboardItem()
{}

StoryboardItem::~StoryboardItem()
{
    qDeleteAll(m_childData);
}

void StoryboardItem::appendChild(QVariant &data)
{
    StoryboardChild* child = new StoryboardChild(data, this);
    m_childData.append(child);
}

void StoryboardItem::insertChild(int row, QVariant &data)
{
    StoryboardChild* child = new StoryboardChild(data, this);
    m_childData.insert(row, child);
}

void StoryboardItem::removeChild(int row)
{
    m_childData.removeAt(row);
}

int StoryboardItem::childCount() const
{
    return m_childData.count();
}

StoryboardChild* StoryboardItem::child(int row)
{
    if (row < 0 || row >= m_childData.size())
        return nullptr;
    return m_childData.at(row);
}
