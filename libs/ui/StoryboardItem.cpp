/*
 *  SPDX-FileCopyrightText: 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "StoryboardItem.h"

#include <QDomElement>
#include <QDomDocument>

#include "kis_pointer_utils.h"

StoryboardItem::StoryboardItem()
    : m_childData()
{}

StoryboardItem::StoryboardItem(const StoryboardItem& other)
    : QEnableSharedFromThis<StoryboardItem>()
    , m_childData()
{
    cloneChildrenFrom(other);
}

StoryboardItem::~StoryboardItem()
{
    m_childData.clear();
}

void StoryboardItem::appendChild(QVariant data)
{
    QSharedPointer<StoryboardChild> child = toQShared( new StoryboardChild(data) );
    child->setParent(sharedFromThis());
    m_childData.append(child);
}

void StoryboardItem::cloneChildrenFrom(const StoryboardItem& other)
{
    m_childData.clear();
    for (int i = 0; i < other.m_childData.count(); i++) {
        QSharedPointer<StoryboardChild> child = toQShared( new StoryboardChild(*other.m_childData.at(i)));
        child->setParent(sharedFromThis());
        m_childData.append(child);
    }
}

void StoryboardItem::insertChild(int row, QVariant data)
{
    QSharedPointer<StoryboardChild> child = toQShared( new StoryboardChild(data) );
    child->setParent(sharedFromThis());
    m_childData.insert(row, child);
}

void StoryboardItem::removeChild(int row)
{
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

QSharedPointer<StoryboardChild> StoryboardItem::child(int row) const
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
        StoryboardItemSP item = toQShared( new StoryboardItem(*list.at(i)) );
        item->cloneChildrenFrom(*list.at(i));
        clonedList.append(item);
    }
    return clonedList;
}
