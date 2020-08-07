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

#include "StoryboardItem.h"

#include <QDomElement>
#include <QDomDocument>

StoryboardItem::StoryboardItem()
{}

StoryboardItem::StoryboardItem(const StoryboardItem& other)
{
    for (int i = 0; i < other.childCount(); i++) {
        appendChild(other.child(i)->data());
    }
}

StoryboardItem::~StoryboardItem()
{
    qDeleteAll(m_childData);
    m_childData.clear();
}

void StoryboardItem::appendChild(QVariant data)
{
    StoryboardChild* child = new StoryboardChild(data, this);
    m_childData.append(child);
}

void StoryboardItem::insertChild(int row, QVariant data)
{
    StoryboardChild* child = new StoryboardChild(data, this);
    m_childData.insert(row, child);
}

void StoryboardItem::removeChild(int row)
{
    delete m_childData.at(row);
    m_childData.removeAt(row);
}

void StoryboardItem::moveChild(int from, int to)
{
    m_childData.move(from, to);
}

int StoryboardItem::childCount() const
{
    return m_childData.count();
}

StoryboardChild* StoryboardItem::child(int row) const
{
    if (row < 0 || row >= m_childData.size()) {
        return nullptr;
    }
    return m_childData.at(row);
}

QDomElement StoryboardItem::toXML(QDomDocument doc)
{
    QDomElement itemElement = doc.createElement("storyboarditem");

    int frame = qvariant_cast<ThumbnailData>(child(FrameNumber)->data()).frameNum.toInt();
    itemElement.setAttribute("frame", frame);
    itemElement.setAttribute("item-name", child(ItemName)->data().toString());
    itemElement.setAttribute("duration-second", child(DurationSecond)->data().toInt());
    itemElement.setAttribute("duration-frame", child(DurationFrame)->data().toInt());

    for (int i = Comments; i < childCount(); i++) {
        CommentBox comment = qvariant_cast<CommentBox>(child(i)->data());
        QDomElement commentElement = doc.createElement("comment");

        commentElement.setAttribute("content", comment.content.toString());
        commentElement.setAttribute("scroll-value", comment.scrollValue.toInt());

        itemElement.appendChild(commentElement);
    }

    return itemElement;
}

void StoryboardItem::loadXML(const QDomElement &itemNode)
{
    ThumbnailData thumbnail;
    thumbnail.frameNum = itemNode.attribute("frame").toInt();
    appendChild(QVariant::fromValue<ThumbnailData>(thumbnail));
    appendChild(itemNode.attribute("item-name"));
    appendChild(itemNode.attribute("duration-second").toInt());
    appendChild(itemNode.attribute("duration-frame").toInt());

    for (QDomElement commentNode = itemNode.firstChildElement(); !commentNode.isNull(); commentNode = commentNode.nextSiblingElement()) {
        if (commentNode.nodeName().toUpper() != "COMMENT") continue;

        CommentBox comment;
        if (commentNode.hasAttribute("content")) {
            comment.content = commentNode.attribute("content");
        }
        if (commentNode.hasAttribute("scroll-value")) {
            comment.scrollValue = commentNode.attribute("scroll-value");
        }
        appendChild(QVariant::fromValue<CommentBox>(comment));
    }
}

StoryboardItemList StoryboardItem::cloneStoryboardItemList(const StoryboardItemList &list)
{
    StoryboardItemList clonedList;
    for (auto i = 0; i < list.count(); i++) {
        StoryboardItem *item = new StoryboardItem(*list.at(i));
        clonedList.append(item);
    }
    return clonedList;
}
