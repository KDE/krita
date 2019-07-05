/*
 * Copyright (C) 2018 Boudewijn Rempt <boud@valdyas.org>
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

#include "KisResourceModel.h"

#include <QBuffer>
#include <QImage>
#include <QtSql>
#include <QStringList>

#include <KisResourceLocator.h>
#include <KisResourceCacheDb.h>

struct KisResourceModel::Private {
    QSqlQuery resourcesQuery;
    QSqlQuery tagQuery;
    QString resourceType;
    int columnCount {9};
    int cachedRowCount {-1};
};


KisResourceModel::KisResourceModel(const QString &resourceType, QObject *parent)
    : QAbstractTableModel(parent)
    , d(new Private)
{
    d->resourceType = resourceType;

    bool r = d->resourcesQuery.prepare("SELECT  resources.id\n"
                                        ",      resources.storage_id\n"
                                        ",      resources.name\n"
                                        ",      resources.filename\n"
                                        ",      resources.tooltip\n"
                                        ",      resources.thumbnail\n"
                                        ",      resources.status\n"
                                        ",      storages.location\n"
                                        ",      resource_types.name as resource_type\n"
                                        "FROM   resources\n"
                                        ",      resource_types\n"
                                        ",      storages\n"
                                        "WHERE  resources.resource_type_id = resource_types.id\n"
                                        "AND    resources.storage_id = storages.id\n"
                                        "AND    resource_types.name = :resource_type\n"
                                        "AND    resources.status = 1\n"
                                        "AND    storages.active = 1");
    if (!r) {
        qWarning() << "Could not prepare KisResourceModel query" << d->resourcesQuery.lastError();
    }
    d->resourcesQuery.bindValue(":resource_type", d->resourceType);

    resetQuery();

    r = d->tagQuery.prepare("SELECT tags.url\n"
                            ",      tags.name\n"
                            ",      tags.comment\n"
                            "FROM   tags\n"
                            ",      resource_tags\n"
                            "WHERE  tags.active > 0\n"
                            "AND    tags.id = resource_tags.tag_id\n"
                            "AND    resource_tags.resource_id = :resource_id\n");
    if (!r)  {
        qWarning() << "Could not prepare TagsForResource query" << d->tagQuery.lastError();
    }

}

KisResourceModel::~KisResourceModel()
{
    delete d;
}

int KisResourceModel::columnCount(const QModelIndex &/*parent*/) const
{
    return d->columnCount;
}

QVariant KisResourceModel::data(const QModelIndex &index, int role) const
{

    QVariant v;
    if (!index.isValid()) return v;

    if (index.row() > rowCount()) return v;
    if (index.column() > d->columnCount) return v;

    bool pos = const_cast<KisResourceModel*>(this)->d->resourcesQuery.seek(index.row());

    if (pos) {
        switch(role) {
        case Qt::DisplayRole:
        {
            switch(index.column()) {
            case Id:
                return d->resourcesQuery.value("id");
            case StorageId:
                return d->resourcesQuery.value("storage_id");
            case Name:
                return d->resourcesQuery.value("name");
            case Filename:
                return d->resourcesQuery.value("filename");
            case Tooltip:
                return d->resourcesQuery.value("tooltip");
            case Image:
                ;
            case Status:
                return d->resourcesQuery.value("status");
            case Location:
                return d->resourcesQuery.value("location");
            case ResourceType:
                return d->resourcesQuery.value("resource_type");
            default:
                ;
            };
        }
        case Qt::DecorationRole:
        {
            if (index.column() == Image) {
                QByteArray ba = d->resourcesQuery.value("thumbnail").toByteArray();
                QBuffer buf(&ba);
                buf.open(QBuffer::ReadOnly);
                QImage img;
                img.load(&buf, "PNG");
                return QVariant::fromValue<QImage>(img);
            }
            return QVariant();
        }
        case Qt::ToolTipRole:
            /* Falls through. */
        case Qt::StatusTipRole:
            /* Falls through. */
        case Qt::WhatsThisRole:
            return d->resourcesQuery.value("tooltip");
        case Qt::UserRole + Id:
            return d->resourcesQuery.value("id");
        case Qt::UserRole + StorageId:
            return d->resourcesQuery.value("storage_id");
        case Qt::UserRole + Name:
            return d->resourcesQuery.value("name");
        case Qt::UserRole + Filename:
            return d->resourcesQuery.value("filename");
        case Qt::UserRole + Tooltip:
            return d->resourcesQuery.value("tooltip");
        case Qt::UserRole + Image:
        {
            QByteArray ba = d->resourcesQuery.value("thumbnail").toByteArray();
            QBuffer buf(&ba);
            buf.open(QBuffer::ReadOnly);
            QImage img;
            img.load(&buf, "PNG");
            return QVariant::fromValue<QImage>(img);
        }
        case Qt::UserRole + Status:
            return d->resourcesQuery.value("status");
        case Qt::UserRole + Location:
            return d->resourcesQuery.value("location");
        case Qt::UserRole + ResourceType:
            return d->resourcesQuery.value("resource_type");
        case Qt::UserRole + Tags:
        {
            QStringList tags = tagsForResource(d->resourcesQuery.value("id").toInt());
            return tags;
        }
        case Qt::UserRole + Dirty:
        {
            QString storageLocation = d->resourcesQuery.value("location").toString();
            QString resourceLocation = d->resourcesQuery.value("filename").toString();

            // An uncached resource has not been loaded, so it cannot be dirty
            if (!KisResourceLocator::instance()->resourceCached(storageLocation, resourceLocation)) {
                return false;
            }
            else {
                // Now we have to check the resource, but that's cheap since it's been loaded in any case
                KoResourceSP resource = resourceForIndex(index);
                return resource->isDirty();
            }
        }
        case Qt::UserRole + MetaData:
        {
            qDebug() << "REMINDER: implement KoResourceMetadata properly!";
            QMap<QString, QVariant> r;
            r.insert("paintopid", "paintbrush");
            return r;
        }

        default:
            ;
        }
    }
    return v;
}

KoResourceSP KisResourceModel::resourceForIndex(QModelIndex index) const
{
    KoResourceSP resource = 0;

    if (!index.isValid()) return resource;

    if (index.row() > rowCount()) return resource;
    if (index.column() > d->columnCount) return resource;

    bool pos = const_cast<KisResourceModel*>(this)->d->resourcesQuery.seek(index.row());
    if (pos) {
        QString storageLocation = d->resourcesQuery.value("location").toString();
        QString resourceLocation = d->resourcesQuery.value("filename").toString();
        resource = KisResourceLocator::instance()->resource(storageLocation, resourceLocation);
        resource->setResourceId(d->resourcesQuery.value("id").toInt());
        resource->setFilename(resourceLocation);
        resource->setStorageLocation(storageLocation);

    }
    return resource;
}

QModelIndex KisResourceModel::indexFromResource(KoResourceSP resource) const
{
    // For now a linear seek to find the first resource with the right id
    d->resourcesQuery.first();
    do {
        if (d->resourcesQuery.value("id").toInt() == resource->resourceId()) {
            return createIndex(d->resourcesQuery.at(), 0);
        }
    } while (d->resourcesQuery.next());

    return QModelIndex();
}


bool KisResourceModel::removeResource(const QModelIndex &index)
{
    if (index.row() > rowCount()) return false;
    if (index.column() > d->columnCount) return false;

    bool pos = d->resourcesQuery.seek(index.row());
    if (!pos) return false;

    int resourceId = d->resourcesQuery.value("id").toInt();
    if (!KisResourceLocator::instance()->removeResource(resourceId)) {
        qWarning() << "Failed to remove resource" << resourceId;
        return false;
    }
    return resetQuery();
}

bool KisResourceModel::removeResource(KoResourceSP resource)
{
    if (!KisResourceLocator::instance()->removeResource(resource->resourceId())) {
        qWarning() << "Failed to remove resource" << resource->resourceId();
        return false;
    }
    return resetQuery();
}

bool KisResourceModel::importResourceFile(const QString &filename)
{
    beginResetModel();
    endResetModel();
    return false;
}


bool KisResourceModel::addResource(KoResourceSP resource, bool save)
{
    beginResetModel();
    endResetModel();
    return false;
}

bool KisResourceModel::updateResource(KoResourceSP resource)
{
    beginResetModel();
    endResetModel();
    return false;
}


bool KisResourceModel::resetQuery()
{
    beginResetModel();
    bool r = d->resourcesQuery.exec();
    if (!r) {
        qWarning() << "Could not select" << d->resourceType << "resources" << d->resourcesQuery.lastError() << d->resourcesQuery.boundValues();
    }
    d->cachedRowCount = -1;
    endResetModel();

    return r;
}

QStringList KisResourceModel::tagsForResource(int resourceId) const
{
    d->tagQuery.bindValue(":resource_id", resourceId);
    bool r = d->tagQuery.exec();
    if (!r) {
        qWarning() << "Could not select tags for" << resourceId << d->tagQuery.lastError() << d->tagQuery.boundValues();
    }
    QStringList tags;
    while (d->tagQuery.next()) {
        qDebug() << d->tagQuery.value(0).toString()
                 << d->tagQuery.value(1).toString()
                 << d->tagQuery.value(2).toString();
        tags << d->tagQuery.value(1).toString();
    }
    return tags;
}

int KisResourceModel::rowCount(const QModelIndex &) const
{
    if (d->cachedRowCount < 0) {
        QSqlQuery q;
        q.prepare("SELECT count(*)\n"
                  "FROM   resources\n"
                  ",      resource_types\n"
                  ",      storages\n"
                  "WHERE  resources.resource_type_id = resource_types.id\n"
                  "AND    resource_types.name = :resource_type\n"
                  "AND    resources.storage_id = storages.id\n"
                  "AND    resources.status = 1\n"
                  "AND    storages.active = 1");
        q.bindValue(":resource_type", d->resourceType);
        q.exec();
        q.first();

        const_cast<KisResourceModel*>(this)->d->cachedRowCount = q.value(0).toInt();
    }

    return d->cachedRowCount;
}
