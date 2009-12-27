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

#ifndef TRACKEDCHANGEMODEL_H
#define TRACKEDCHANGEMODEL_H

#include <QAbstractItemModel>
#include <QHash>
#include <QList>
#include <QMetaType>
#include <QObject>

class KoChangeTracker;
class KoTextDocumentLayout;

class QTextDocument;

struct ItemData
{
    int changeId;
    int changeStart;
    int changeEnd;
};

Q_DECLARE_METATYPE(ItemData)

class ModelItem
{
public:
    ModelItem(int changeId, ModelItem *parent = 0);
    ~ModelItem();

    void appendChild(ModelItem *child);

    ModelItem *child(int row);
    int childCount() const;
    int row() const;
    ModelItem *parent();

    ItemData itemData();

    void setChangeStart(int start);
    void setChangeEnd(int end);

private:
    QList<ModelItem*> m_childItems;
    ModelItem *m_parentItem;
    ItemData m_data;
};

class TrackedChangeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    TrackedChangeModel(QTextDocument *document, QObject *parent = 0);
    ~TrackedChangeModel();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    ItemData changeItemData(const QModelIndex &index, int role = Qt::DisplayRole) const;

private:
    void setupModelData(QTextDocument *document, ModelItem *parent);

    ModelItem *m_rootItem;
    KoChangeTracker *m_changeTracker;
    KoTextDocumentLayout *m_layout;

    QHash<int, int> m_changeOccurenceCounter;
};

#endif // TRACKEDCHANGEMODEL_H
