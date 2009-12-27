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

#include <QModelIndex>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFragment>

////ModelItem

ModelItem::ModelItem(int changeId, ModelItem* parent)
{
    m_data.changeId = changeId;
    m_parentItem = parent;
}

ModelItem::~ModelItem()
{
    qDeleteAll(m_childItems);
}

void ModelItem::appendChild(ModelItem* child)
{
    m_childItems.append(child);
}

ModelItem* ModelItem::child(int row)
{
    return m_childItems.value(row);
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

void ModelItem::setChangeEnd(int end)
{
    m_data.changeEnd = end;
}

void ModelItem::setChangeStart(int start)
{
    m_data.changeStart = start;
}

ItemData ModelItem::itemData()
{
    return m_data;
}

////TrackedChangeModel


TrackedChangeModel::TrackedChangeModel(QTextDocument* document, QObject* parent)
    :QAbstractItemModel(parent)
{
    m_rootItem = new ModelItem(0);
    setupModelData(document, m_rootItem);
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
            return QVariant(item->itemData().changeStart);
            break;
        case 2:
            return QVariant(item->itemData().changeEnd);
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
                return QVariant(QString("changeStart"));
                break;
            case 2:
                return QVariant(QString("ChangeEnd"));
                break;
        }
    }

    return QVariant();
}

void TrackedChangeModel::setupModelData(QTextDocument* document, ModelItem* parent)
{
    m_changeTracker = KoTextDocument(document).changeTracker();
    m_layout = dynamic_cast<KoTextDocumentLayout*>(document->documentLayout());

    ModelItem *currentParent = parent;

    QTextBlock block = document->begin();
    while (block.isValid()) {
        QTextBlock::iterator it;
        for (it = block.begin(); !(it.atEnd()); ++it) {
            QTextFragment fragment = it.fragment();
            QTextCharFormat format = fragment.charFormat();
            int changeId = format.property(KoCharacterStyle::ChangeTrackerId).toInt();
            if (m_changeTracker->elementById(changeId) && m_changeTracker->elementById(changeId)->getChangeType() == KoGenChange::deleteChange)
                continue;
            if (KoDeleteChangeMarker *changeMarker = dynamic_cast<KoDeleteChangeMarker*>(m_layout->inlineTextObjectManager()->inlineTextObject(format)))
                changeId = changeMarker->changeId();
            if (changeId) {
                if (currentParent->itemData().changeId != changeId) {
                    while (currentParent != parent) {
                        if (!m_changeTracker->isParent(currentParent->itemData().changeId, changeId))
                            currentParent = currentParent->parent();
                        else
                            break;
                    }
                    if (changeId != currentParent->itemData().changeId) {
                        ModelItem *item = new ModelItem(changeId, currentParent);
                        item->setChangeStart(fragment.position());
                        item->setChangeEnd(fragment.position() + fragment.length());
                        currentParent->appendChild(item);
                        currentParent = item;
                    }
                }
            }
            else {
                currentParent = parent;
            }
        }
        block = block.next();
    }
}

#include "TrackedChangeModel.moc"
