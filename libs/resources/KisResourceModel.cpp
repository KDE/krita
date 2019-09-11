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

#include <QTime>
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


//static int s_i = 0;

KisResourceModel::KisResourceModel(const QString &resourceType, QObject *parent)
    : QAbstractTableModel(parent)
    , d(new Private)
{
    //qDebug() << "ResourceModel" << s_i << resourceType; s_i++;

    d->resourceType = resourceType;
    
    bool r = d->resourcesQuery.prepare("SELECT resources.id\n"
                                       ",     resources.storage_id\n"
                                       ",     resources.name\n"
                                       ",     resources.filename\n"
                                       ",     resources.tooltip\n"
                                       ",     resources.thumbnail\n"
                                       ",     resources.status\n"
                                       ",     storages.location\n"
                                       ",     resources.version\n"
                                       ",     resource_types.name as resource_type\n"
                                       "FROM  resources\n"
                                       ",     resource_types\n"
                                       ",     storages\n"
                                       "WHERE resources.resource_type_id = resource_types.id\n"
                                       "AND   resources.storage_id = storages.id\n"
                                       "AND   resource_types.name = :resource_type\n"
                                       "AND   resources.status = 1\n"
                                       "AND   storages.active = 1");
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
            QString filename = d->resourcesQuery.value("filename").toString();
            
            // An uncached resource has not been loaded, so it cannot be dirty
            if (!KisResourceLocator::instance()->resourceCached(storageLocation, d->resourceType, filename)) {
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
            //qDebug() << "REMINDER: implement KoResource::Metadata properly!";
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

//static int s_i2 {0};

KoResourceSP KisResourceModel::resourceForIndex(QModelIndex index) const
{
    KoResourceSP resource = 0;
    
    if (!index.isValid()) return resource;
    if (index.row() > rowCount()) return resource;
    if (index.column() > d->columnCount) return resource;

    //qDebug() << "KisResourceModel::resourceForIndex" << s_i2 << d->resourceType; s_i2++;

    bool pos = const_cast<KisResourceModel*>(this)->d->resourcesQuery.seek(index.row());
    if (pos) {
        QString storageLocation = d->resourcesQuery.value("location").toString();
        QString filename = d->resourcesQuery.value("filename").toString();
        resource = KisResourceLocator::instance()->resource(storageLocation, d->resourceType, filename);
        resource->setResourceId(d->resourcesQuery.value("id").toInt());
        resource->setVersion(d->resourcesQuery.value("version").toInt());
        resource->setFilename(filename);
        resource->setStorageLocation(storageLocation);
    }
    return resource;
}

//static int s_i3 {0};

QModelIndex KisResourceModel::indexFromResource(KoResourceSP resource) const
{
    if (!resource || !resource->valid()) return QModelIndex();

    //qDebug() << "KisResourceModel::indexFromResource" << s_i3 << d->resourceType; s_i3++;
    
    // For now a linear seek to find the first resource with the right id
    d->resourcesQuery.first();
    do {
        if (d->resourcesQuery.value("id").toInt() == resource->resourceId()) {
            return createIndex(d->resourcesQuery.at(), 0);
        }
    } while (d->resourcesQuery.next());
    
    return QModelIndex();
}

//static int s_i4 {0};

bool KisResourceModel::removeResource(const QModelIndex &index)
{
    if (index.row() > rowCount()) return false;
    if (index.column() > d->columnCount) return false;
    
    //qDebug() << "KisResourceModel::removeResource" << s_i4 << d->resourceType; s_i4++;

    bool pos = d->resourcesQuery.seek(index.row());
    if (!pos) return false;
    
    int resourceId = d->resourcesQuery.value("id").toInt();
    if (!KisResourceLocator::instance()->removeResource(resourceId)) {
        qWarning() << "Failed to remove resource" << resourceId;
        return false;
    }
    return resetQuery();
}

//static int s_i5 {0};

bool KisResourceModel::removeResource(KoResourceSP resource)
{
    if (!resource || !resource->valid()) return false;

    //qDebug() << "KisResourceModel::remvoeResource 2" << s_i5 << d->resourceType; s_i5++;

    if (!KisResourceLocator::instance()->removeResource(resource->resourceId())) {
        qWarning() << "Failed to remove resource" << resource->resourceId();
        return false;
    }
    return resetQuery();
}

//static int s_i6 {0};

bool KisResourceModel::importResourceFile(const QString &filename)
{
    //qDebug() << "KisResourceModel::importResource" << s_i6 << d->resourceType; s_i6++;

    if (!KisResourceLocator::instance()->importResourceFromFile(d->resourceType, filename)) {
        qWarning() << "Failed to import resource" << filename;
        return false;
    }
    return resetQuery();
}

//static int s_i7 {0};

bool KisResourceModel::addResource(KoResourceSP resource, bool save)
{
    if (!resource || !resource->valid()) {
        qWarning() << "Cannot add resource. Resource is null or not valid";
        return false;
    }

    //qDebug() << "KisResourceModel::addResource" << s_i7 << d->resourceType; s_i7++;

    if (!KisResourceLocator::instance()->addResource(d->resourceType, resource, save)) {
        qWarning() << "Failed to add resource" << resource->name();
        return false;
    }
    return resetQuery();
}

//static int s_i8 {0};

bool KisResourceModel::updateResource(KoResourceSP resource)
{
    if (!resource || !resource->valid()) {
        qWarning() << "Cannot update resource. Resource is null or not valid";
        return false;
    }

    //qDebug() << "KisResourceModel::updateResource" << s_i8 << d->resourceType; s_i8++;

    if (!KisResourceLocator::instance()->updateResource(d->resourceType, resource)) {
        qWarning() << "Failed to update resource";
        return false;
    }
    return resetQuery();
}

//static int s_i9 {0};

bool KisResourceModel::setResourceMetaData(KoResourceSP resource, QMap<QString, QVariant> metadata)
{
    //qDebug() << "KisResourceModel::setResourceMetaData" << s_i9 << d->resourceType; s_i9++;

    return true;
}

bool KisResourceModel::resetQuery()
{
    QTime t;
    t.start();

    beginResetModel();
    bool r = d->resourcesQuery.exec();
    if (!r) {
        qWarning() << "Could not select" << d->resourceType << "resources" << d->resourcesQuery.lastError() << d->resourcesQuery.boundValues();
    }
    d->cachedRowCount = -1;
    endResetModel();
    
    qDebug() << "KisResourceModel::resetQuery for" << d->resourceType << "took" << t.elapsed() << "ms";

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
        //qDebug() << d->tagQuery.value(0).toString() << d->tagQuery.value(1).toString() << d->tagQuery.value(2).toString();
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
