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

#include <KoGenChange.h>

#include <QAbstractItemModel>
#include <QHash>
#include <QList>
#include <QMetaType>
#include <QObject>
#include <QPair>

class KoChangeTracker;
class KoTextDocumentLayout;

class QTextDocument;

struct ItemData {
    int changeId;
    QList<QPair<int, int> > changeRanges;
    KoGenChange::Type changeType;
    QString title;
    QString author;
};

Q_DECLARE_METATYPE(ItemData)

class ModelItem
{
public:
    explicit ModelItem(ModelItem *parent = 0);
    ~ModelItem();

    void setChangeId(int changeId);
    void setChangeType(KoGenChange::Type type);
    void setChangeTitle(const QString &title);
    void setChangeAuthor(const QString &author);

    void appendChild(ModelItem *child);

    ModelItem *child(int row);
    QList<ModelItem *> children();
    int childCount() const;
    int row() const;
    ModelItem *parent();

    ItemData itemData();

    void setChangeRange(int start, int end);

    void removeChildren();

private:
    QList<ModelItem *> m_childItems;
    ModelItem *m_parentItem;
    ItemData m_data;
};

class TrackedChangeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit TrackedChangeModel(QTextDocument *document, QObject *parent = 0);
    ~TrackedChangeModel();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex indexForChangeId(int changeId);
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    ItemData changeItemData(const QModelIndex &index, int role = Qt::DisplayRole) const;

public Q_SLOTS:
    void setupModel();

private:
    void setupModelData(QTextDocument *document, ModelItem *parent);

    QTextDocument *m_document;
    ModelItem *m_rootItem;
    KoChangeTracker *m_changeTracker;
    KoTextDocumentLayout *m_layout;

    QHash<int, int> m_changeOccurenceCounter;
    QHash<int, ModelItem *> m_changeItems;
};

#endif // TRACKEDCHANGEMODEL_H
