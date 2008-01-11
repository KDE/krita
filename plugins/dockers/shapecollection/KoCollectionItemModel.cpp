/* This file is part of the KDE project
 * Copyright (C) 2008 Peter Simonsson <peter.simonsson@gmail.com>
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
#include "KoCollectionItemModel.h"

#include <KoShapeFactory.h>

#include <kdebug.h>

#include <QMimeData>

KoCollectionItemModel::KoCollectionItemModel(QObject* parent)
    : QAbstractListModel(parent)
{
    setSupportedDragActions(Qt::CopyAction);
}

QVariant KoCollectionItemModel::data(const QModelIndex& index, int role) const
{
    switch(role)
    {
        case Qt::ToolTipRole:
            return m_shapeTemplateList[index.row()].toolTip;

        case Qt::DecorationRole:
            return m_shapeTemplateList[index.row()].icon;

        case Qt::UserRole:
            return m_shapeTemplateList[index.row()].id;

        case Qt::UserRole + 1:
            return m_shapeTemplateList[index.row()].properties;

        default:
            return QVariant();
    }

    return QVariant();
}

int KoCollectionItemModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_shapeTemplateList.count();
}

void KoCollectionItemModel::setShapeTemplateList(const QList<KoCollectionItem>& newlist)
{
    m_shapeTemplateList = newlist;
    reset();
}

QMimeData* KoCollectionItemModel::mimeData(const QModelIndexList& indexes) const
{
    if(indexes.isEmpty())
        return 0;

    QModelIndex index = indexes.first();

    if(!index.isValid())
        return 0;

    if(m_shapeTemplateList.isEmpty())
        return 0;

    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    dataStream << m_shapeTemplateList[index.row()].id;
    KoProperties *props = m_shapeTemplateList[index.row()].properties;

    if(props)
        dataStream << props->store(); // is a QString
    else
        dataStream << QString();

    QMimeData* mimeData = new QMimeData;
    mimeData->setData(SHAPETEMPLATE_MIMETYPE, itemData);

    return mimeData;
}

QStringList KoCollectionItemModel::mimeTypes() const
{
    QStringList mimetypes;
    mimetypes << SHAPETEMPLATE_MIMETYPE;

    return mimetypes;
}

Qt::ItemFlags KoCollectionItemModel::flags(const QModelIndex& index) const
{
    if(index.isValid())
        return QAbstractListModel::flags(index) | Qt::ItemIsDragEnabled;

    return QAbstractListModel::flags(index);
}

#include "KoCollectionItemModel.moc"
