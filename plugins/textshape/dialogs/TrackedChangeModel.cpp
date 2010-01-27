/* This file is part of the KDE project
* Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public License
* along with this library; see the file COPYING.LIB.  If not, write to
* the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
*/

#include "TrackedChangeModel.h"

#include <KoDeleteChangeMarker.h>
#include <KoTextDocument.h>
#include <KoTextDocumentLayout.h>
#include <KoGenChange.h>
#include <KoInlineTextObjectManager.h>
#include <changetracker/KoChangeTracker.h>
#include <changetracker/KoChangeTrackerElement.h>
#include <styles/KoCharacterStyle.h>

#include <QHash>
#include <QModelIndex>
#include <QStack>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFragment>

////ModelItem

ModelItem::ModelItem(ModelItem* parent)
{
    m_parentItem = parent;
    m_data.changeId = 0;
}

ModelItem::~ModelItem()
{
    qDeleteAll(m_childItems);
}

void ModelItem::setChangeId(int changeId)
{
    m_data.changeId = changeId;
}

void ModelItem::setChangeType(KoGenChange::Type type)
{
    m_data.changeType = type;
}

void ModelItem::setChangeTitle(QString title)
{
    m_data.title = title;
}

void ModelItem::setChangeAuthor(QString author)
{
    m_data.author = author;
}

void ModelItem::appendChild(ModelItem* child)
{
    m_childItems.append(child);
}

ModelItem* ModelItem::child(int row)
{
    return m_childItems.value(row);
}

QList< ModelItem* > ModelItem::children()
{
    return m_childItems;
}

int ModelItem::childCount() const
{
    return m_childItems.count();
}

ModelItem* ModelItem::parent()
{
    return m_parentItem;
}

int ModelItem::row() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<ModelItem*>(this));
    return 0;
}

void ModelItem::setChangeRange(int start, int end)
{
    m_data.changeRanges.append(QPair<int, int>(start, end));
}

ItemData ModelItem::itemData()
{
    return m_data;
}

void ModelItem::removeChildren()
{
    qDeleteAll(m_childItems);
    m_childItems.clear();
}

////TrackedChangeModel


TrackedChangeModel::TrackedChangeModel(QTextDocument* document, QObject* parent)
    :QAbstractItemModel(parent),
    m_document(document)
{
    m_rootItem = new ModelItem(0);
    setupModelData(m_document, m_rootItem);
}

TrackedChangeModel::~TrackedChangeModel()
{
    delete m_rootItem;
}

QModelIndex TrackedChangeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    ModelItem *parentItem;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<ModelItem*>(parent.internalPointer());

    ModelItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex TrackedChangeModel::indexForChangeId(int changeId)
{
    ModelItem *item = m_changeItems.value(changeId);
    if (!item)
        return QModelIndex();
    return createIndex(item->row(), 0, item);
}

QModelIndex TrackedChangeModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    ModelItem *childItem = static_cast<ModelItem*>(index.internalPointer());
    ModelItem *parentItem = childItem->parent();

    if (parentItem == m_rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int TrackedChangeModel::rowCount(const QModelIndex& parent) const
{
    ModelItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<ModelItem*>(parent.internalPointer());

    return parentItem->childCount();
}

int TrackedChangeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 3;
}

ItemData TrackedChangeModel::changeItemData(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return ItemData();

    if (role != Qt::DisplayRole)
        return ItemData();

    ModelItem *item = static_cast<ModelItem*>(index.internalPointer());

    return item->itemData();
}

QVariant TrackedChangeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    ModelItem *item = static_cast<ModelItem*>(index.internalPointer());

    ItemData data = item->itemData();

    switch(index.column()) {
        case 0:
            return QVariant(item->itemData().changeId);
            break;
        case 1:
            return QVariant(item->itemData().title);
            break;
        case 2:
            return QVariant(item->itemData().author);
            break;
        default:
            return QVariant();
    }
}

Qt::ItemFlags TrackedChangeModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant TrackedChangeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch(section) {
            case 0:
                return QVariant(QString("changeId"));
                break;
            case 1:
                return QVariant(QString("title"));
                break;
            case 2:
                return QVariant(QString("author"));
                break;
        }
    }

    return QVariant();
}

void TrackedChangeModel::setupModel()
{
    beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
    m_rootItem->removeChildren();
    endRemoveRows();
    setupModelData(m_document, m_rootItem);
    beginInsertRows(QModelIndex(), 0, m_rootItem->childCount() - 1);
    endInsertRows();
}

void TrackedChangeModel::setupModelData(QTextDocument* document, ModelItem* parent)
{
    m_changeTracker = KoTextDocument(document).changeTracker();
    m_layout = dynamic_cast<KoTextDocumentLayout*>(document->documentLayout());

    QStack<ModelItem*> itemStack;
    itemStack.push(parent);
    m_changeItems.clear();

    QTextBlock block = document->begin();
    while (block.isValid()) {
        QTextBlock::iterator it;
        for (it = block.begin(); !(it.atEnd()); ++it) {
            QTextFragment fragment = it.fragment();
            QTextCharFormat format = fragment.charFormat();
            int changeId = format.property(KoCharacterStyle::ChangeTrackerId).toInt();
//            if (m_changeTracker->elementById(changeId) && m_changeTracker->elementById(changeId)->getChangeType() == KoGenChange::deleteChange)
//                continue;
            if (KoDeleteChangeMarker *changeMarker = dynamic_cast<KoDeleteChangeMarker*>(m_layout->inlineTextObjectManager()->inlineTextObject(format)))
                changeId = changeMarker->changeId();
            if (changeId) {
                if (changeId != itemStack.top()->itemData().changeId) {
                    while (itemStack.top() != parent) {
                        if (!m_changeTracker->isParent(itemStack.top()->itemData().changeId, changeId))
                            itemStack.pop();
                        else
                            break;
                    }
                }
                ModelItem *item = m_changeItems.value(changeId);
                if (!item) {
                    item = new ModelItem(itemStack.top());
                    item->setChangeId(changeId);
                    item->setChangeType(m_changeTracker->elementById(changeId)->getChangeType());
                    item->setChangeTitle(m_changeTracker->elementById(changeId)->getChangeTitle());
                    item->setChangeAuthor(m_changeTracker->elementById(changeId)->getCreator());
                    itemStack.top()->appendChild(item);
                    m_changeItems.insert(changeId, item);
                }
                item->setChangeRange(fragment.position(), fragment.position() + fragment.length());
                ModelItem *parentItem = item->parent();
                while (parentItem->itemData().changeId) {
                    parentItem->setChangeRange(fragment.position(), fragment.position() + fragment.length());
                    parentItem = parentItem->parent();
                }
                itemStack.push(item);

            }
            else {
                itemStack.push(parent);
            }
        }
        block = block.next();
    }
}

#include <TrackedChangeModel.moc>
