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

#include <QElapsedTimer>
#include <QBuffer>
#include <QImage>
#include <QtSql>
#include <QStringList>

#include <KisResourceLocator.h>
#include <KisResourceCacheDb.h>

#include <KisTagModelProvider.h>



struct KisAllResourcesModel::Private {
    QSqlQuery resourcesQuery;
    QSqlQuery tagQuery;
    QString resourceType;
    int columnCount {10};
    int cachedRowCount {-1};
};


//static int s_i = 0;

KisAllResourcesModel::KisAllResourcesModel(const QString &resourceType, QObject *parent)
    : QAbstractTableModel(parent)
    , d(new Private)
{
    //qDebug() << "ResourceModel" << s_i << resourceType; s_i++;

    connect(KisResourceLocator::instance(), SIGNAL(storageAdded(const QString&)), this, SLOT(addStorage(const QString&)));
    connect(KisResourceLocator::instance(), SIGNAL(storageRemoved(const QString&)), this, SLOT(removeStorage(const QString&)));

    d->resourceType = resourceType;
    
    bool r = d->resourcesQuery.prepare("SELECT resources.id\n"
                                       ",      resources.storage_id\n"
                                       ",      resources.name\n"
                                       ",      resources.filename\n"
                                       ",      resources.tooltip\n"
                                       ",      resources.thumbnail\n"
                                       ",      resources.status\n"
                                       ",      storages.location\n"
                                       ",      resources.version\n"
                                       ",      resource_types.name as resource_type\n"
                                       ",      resources.status as resource_active\n"
                                       ",      storages.active as storage_active\n"
                                       "FROM   resources\n"
                                       ",      resource_types\n"
                                       ",      storages\n"
                                       "WHERE  resources.resource_type_id = resource_types.id\n"
                                       "AND    resources.storage_id = storages.id\n"
                                       "AND    resource_types.name = :resource_type\n"
                                       "ORDER BY resources.id");
    if (!r) {
        qWarning() << "Could not prepare KisAllResourcesModel query" << d->resourcesQuery.lastError();
    }
    d->resourcesQuery.bindValue(":resource_type", d->resourceType);
    
    resetQuery();
    
    r = d->tagQuery.prepare("SELECT tags.id\n"
                            ",      tags.url\n"
                            ",      tags.name\n"
                            ",      tags.comment\n"
                            "FROM   tags\n"
                            ",      resource_tags\n"
                            "WHERE  tags.active > 0\n"                               // make sure the tag is active
                            "AND    tags.id = resource_tags.tag_id\n"                // join tags + resource_tags by tag_id
                            "AND    resource_tags.resource_id = :resource_id\n"
                            "ORDER BY tags.id");    // make sure we're looking for tags for a specific resource
    if (!r)  {
        qWarning() << "Could not prepare TagsForResource query" << d->tagQuery.lastError();
    }
    
}

KisAllResourcesModel::~KisAllResourcesModel()
{
    delete d;
}

int KisAllResourcesModel::columnCount(const QModelIndex &/*parent*/) const
{
    return d->columnCount;
}

QVariant KisAllResourcesModel::data(const QModelIndex &index, int role) const
{
    
    QVariant v;
    if (!index.isValid()) return v;
    
    if (index.row() > rowCount()) return v;
    if (index.column() > d->columnCount) return v;
    
    bool pos = const_cast<KisAllResourcesModel*>(this)->d->resourcesQuery.seek(index.row());
    
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
            case Thumbnail:
            {
                QByteArray ba = d->resourcesQuery.value("thumbnail").toByteArray();
                QBuffer buf(&ba);
                buf.open(QBuffer::ReadOnly);
                QImage img;
                img.load(&buf, "PNG");
                return QVariant::fromValue<QImage>(img);
            }
            case Status:
                return d->resourcesQuery.value("status");
            case Location:
                return d->resourcesQuery.value("location");
            case ResourceType:
                return d->resourcesQuery.value("resource_type");
            case ResourceActive:
                return d->resourcesQuery.value("resource_active");
            case StorageActive:
                return d->resourcesQuery.value("storage_active");
            default:
                ;
            };
            Q_FALLTHROUGH();
        }
        case Qt::DecorationRole:
        {
            if (index.column() == Thumbnail) {
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
            Q_FALLTHROUGH();
        case Qt::StatusTipRole:
            Q_FALLTHROUGH();
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
        case Qt::UserRole + Thumbnail:
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
            QVector<KisTagSP> tags = tagsForResource(d->resourcesQuery.value("id").toInt());
            QStringList tagNames;
            Q_FOREACH(const KisTagSP tag, tags) {
                tagNames << tag->name();
            }
            return tagNames;
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
            QMap<QString, QVariant> r = KisResourceLocator::instance()->metaDataForResource(d->resourcesQuery.value("id").toInt());
            return r;
        }
        case Qt::UserRole + KoResourceRole:
        {
            KoResourceSP tag = resourceForIndex(index);
            QVariant response;
            response.setValue(tag);
            return response;
        }
        case Qt::UserRole + ResourceActive:
        {
            return d->resourcesQuery.value("resource_active");
        }
        case Qt::UserRole + StorageActive:
        {
              return d->resourcesQuery.value("storage_active");
        }
        default:
            ;
        }
    }
    return v;
}

QVariant KisAllResourcesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant v = QVariant();
    if (role != Qt::DisplayRole) {
        return v;
    }
    if (orientation == Qt::Horizontal) {
        switch(section) {
        case Id:
            v = i18n("Id");
            break;
        case StorageId:
            v = i18n("Storage ID");
            break;
        case Name:
            v = i18n("Name");
            break;
        case Filename:
            v = i18n("File Name");
            break;
        case Tooltip:
            v = i18n("Tooltip");
            break;
        case Thumbnail:
            v = i18n("Image");
            break;
        case Status:
            v = i18n("Status");
            break;
        case Location:
            v = i18n("Location");
            break;
        case ResourceType:
            v = i18n("Resource Type");
            break;
        case ResourceActive:
            v = i18n("Active");
        case StorageActive:
            v = i18n("Storage Active");
        default:
            v = QString::number(section);
        }
        return v;
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

//static int s_i2 {0};

KoResourceSP KisAllResourcesModel::resourceForIndex(QModelIndex index) const
{
    KoResourceSP resource = 0;
    
    if (!index.isValid()) return resource;
    if (index.row() > rowCount()) return resource;
    if (index.column() > d->columnCount) return resource;

    //qDebug() << "KisAllResourcesModel::resourceForIndex" << s_i2 << d->resourceType; s_i2++;

    bool pos = const_cast<KisAllResourcesModel*>(this)->d->resourcesQuery.seek(index.row());
    if (pos) {
        int id = d->resourcesQuery.value("id").toInt();
        resource = resourceForId(id);
    }
    return resource;
}

KoResourceSP KisAllResourcesModel::resourceForId(int id) const
{
    return KisResourceLocator::instance()->resourceForId(id);
}

KoResourceSP KisAllResourcesModel::resourceForFilename(QString filename) const
{
    KoResourceSP resource = 0;

    QSqlQuery q;
    bool r = q.prepare("SELECT resources.id AS id\n"
                       "FROM   resources\n"
                       ",      resource_types\n"
                       ",      storages\n"
                       "WHERE  resources.resource_type_id = resource_types.id\n"
                       "AND    resources.storage_id = storages.id\n"
                       "AND    resources.filename = :resource_filename\n"
                       "AND    resource_types.name = :resource_type\n"
                       "AND    resources.status = 1\n"
                       "AND    storages.active = 1");
    if (!r) {
        qWarning() << "Could not prepare KisAllResourcesModel query for resource name" << q.lastError();
    }
    q.bindValue(":resource_filename", filename);
    q.bindValue(":resource_type", d->resourceType);

    r = q.exec();
    if (!r) {
        qWarning() << "Could not select" << d->resourceType << "resources by filename" << q.lastError() << q.boundValues();
    }

    if (q.first()) {
        int id = q.value("id").toInt();
        resource = KisResourceLocator::instance()->resourceForId(id);
    }
    return resource;
}

KoResourceSP KisAllResourcesModel::resourceForName(QString name) const
{
    KoResourceSP resource = 0;

    QSqlQuery q;
    bool r = q.prepare("SELECT resources.id AS id\n"
                       "FROM   resources\n"
                       ",      resource_types\n"
                       ",      storages\n"
                       "WHERE  resources.resource_type_id = resource_types.id\n"
                       "AND    resources.storage_id = storages.id\n"
                       "AND    resources.name = :resource_name\n"
                       "AND    resource_types.name = :resource_type\n"
                       "AND    resources.status = 1\n"
                       "AND    storages.active = 1");
    if (!r) {
        qWarning() << "Could not prepare KisAllResourcesModel query for resource name" << q.lastError();
    }
    q.bindValue(":resource_type", d->resourceType);
    q.bindValue(":resource_name", name);

    r = q.exec();
    if (!r) {
        qWarning() << "Could not select" << d->resourceType << "resources by name" << q.lastError() << q.boundValues();
    }

    if (q.first()) {
        int id = q.value("id").toInt();
        resource = KisResourceLocator::instance()->resourceForId(id);
    }
    return resource;
}


KoResourceSP KisAllResourcesModel::resourceForMD5(const QByteArray md5sum) const
{
    KoResourceSP resource = 0;

    QSqlQuery q;
    bool r = q.prepare("SELECT resource_id AS id\n"
                       "FROM   versioned_resources\n"
                       "WHERE  md5sum = :md5sum");
    if (!r) {
        qWarning() << "Could not prepare KisAllResourcesModel query for resource md5" << q.lastError();
    }
    q.bindValue(":md5sum", md5sum.toHex());

    r = q.exec();
    if (!r) {
        qWarning() << "Could not select" << d->resourceType << "resources by md5" << q.lastError() << q.boundValues();
    }

    if (q.first()) {
        int id = q.value("id").toInt();
        resource = KisResourceLocator::instance()->resourceForId(id);
    }
    return resource;
}

//static int s_i3 {0};

QModelIndex KisAllResourcesModel::indexFromResource(KoResourceSP resource) const
{
    if (!resource || !resource->valid()) return QModelIndex();

    //qDebug() << "KisAllResourcesModel::indexFromResource" << s_i3 << d->resourceType; s_i3++;
    
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

bool KisAllResourcesModel::setResourceInactive(const QModelIndex &index)
{
    if (index.row() > rowCount()) return false;
    if (index.column() > d->columnCount) return false;
    
    //qDebug() << "KisAllResourcesModel::setResourceInactive" << s_i4 << d->resourceType; s_i4++;

    bool pos = d->resourcesQuery.seek(index.row());
    if (!pos) return false;
    
    int resourceId = d->resourcesQuery.value("id").toInt();
    if (!KisResourceLocator::instance()->setResourceInactive(resourceId)) {
        qWarning() << "Failed to remove resource" << resourceId;
        return false;
    }
    return resetQuery();
}

//static int s_i5 {0};

bool KisAllResourcesModel::setResourceInactive(KoResourceSP resource)
{
    if (!resource || !resource->valid()) return false;

    //qDebug() << "KisAllResourcesModel::remvoeResource 2" << s_i5 << d->resourceType; s_i5++;

    if (!KisResourceLocator::instance()->setResourceInactive(resource->resourceId())) {
        qWarning() << "Failed to remove resource" << resource->resourceId();
        return false;
    }
    return resetQuery();
}

//static int s_i6 {0};

bool KisAllResourcesModel::importResourceFile(const QString &filename)
{
    //qDebug() << "KisAllResourcesModel::importResource" << s_i6 << d->resourceType; s_i6++;

    if (!KisResourceLocator::instance()->importResourceFromFile(d->resourceType, filename)) {
        qWarning() << "Failed to import resource" << filename;
        return false;
    }
    return resetQuery();
}

//static int s_i7 {0};

bool KisAllResourcesModel::addResource(KoResourceSP resource, const QString &storageId)
{
    if (!resource || !resource->valid()) {
        qWarning() << "Cannot add resource. Resource is null or not valid";
        return false;
    }

    //qDebug() << "KisAllResourcesModel::addResource" << s_i7 << d->resourceType; s_i7++;

    if (!KisResourceLocator::instance()->addResource(d->resourceType, resource, storageId)) {
        qWarning() << "Failed to add resource" << resource->name();
        return false;
    }
    return resetQuery();
}

//static int s_i8 {0};

bool KisAllResourcesModel::updateResource(KoResourceSP resource)
{
    if (!resource || !resource->valid()) {
        qWarning() << "Cannot update resource. Resource is null or not valid";
        return false;
    }

    //qDebug() << "KisAllResourcesModel::updateResource" << s_i8 << d->resourceType; s_i8++;

    if (!KisResourceLocator::instance()->updateResource(d->resourceType, resource)) {
        qWarning() << "Failed to update resource" << resource;
        return false;
    }
    return resetQuery();
}

bool KisAllResourcesModel::renameResource(KoResourceSP resource, const QString &name)
{
    if (!resource || !resource->valid() || name.isEmpty()) {
        qWarning() << "Cannot rename resources. Resource is NULL or not valid or name is empty";
        return false;
    }
    resource->setName(name);
    if (!KisResourceLocator::instance()->updateResource(d->resourceType, resource)) {
        qWarning() << "Failed to rename resource" << resource << name;
        return false;
    }
    return resetQuery();
}

//static int s_i9 {0};

bool KisAllResourcesModel::setResourceMetaData(KoResourceSP resource, QMap<QString, QVariant> metadata)
{
    //qDebug() << "KisAllResourcesModel::setResourceMetaData" << s_i9 << d->resourceType; s_i9++;
    Q_ASSERT(resource->resourceId() > -1);
    return KisResourceLocator::instance()->setMetaDataForResource(resource->resourceId(), metadata);
}

bool KisAllResourcesModel::resetQuery()
{
//    QElapsedTimer t;
//    t.start();

    beginResetModel();
    bool r = d->resourcesQuery.exec();
    if (!r) {
        qWarning() << "Could not select" << d->resourceType << "resources" << d->resourcesQuery.lastError() << d->resourcesQuery.boundValues();
    }
    d->cachedRowCount = -1;

    endResetModel();

//    qDebug() << "KisAllResourcesModel::resetQuery for" << d->resourceType << "took" << t.elapsed() << "ms";

    return r;
}

QVector<KisTagSP> KisAllResourcesModel::tagsForResource(int resourceId) const
{
    return KisTagModelProvider::tagModel(d->resourceType)->tagsForResource(resourceId);
}

int KisAllResourcesModel::rowCount(const QModelIndex &) const
{
    if (d->cachedRowCount < 0) {
        QSqlQuery q;
        q.prepare("SELECT count(*)\n"
                  "FROM   resources\n"
                  ",      resource_types\n"
                  ",      storages\n"
                  "WHERE  resources.resource_type_id = resource_types.id\n"
                  "AND    resource_types.name = :resource_type\n"
                  "AND    resources.storage_id = storages.id\n");
        q.bindValue(":resource_type", d->resourceType);
        q.exec();
        q.first();
        
        const_cast<KisAllResourcesModel*>(this)->d->cachedRowCount = q.value(0).toInt();
    }
    
    return d->cachedRowCount;
}


void KisAllResourcesModel::addStorage(const QString &location)
{

}


void KisAllResourcesModel::removeStorage(const QString &location)
{

}

struct KisResourceModel::Private
{
    ResourceFilter resourceFilter {ShowActiveResources};
    StorageFilter storageFilter {ShowActiveStorages};
};

KisResourceModel::KisResourceModel(const QString &type, QObject *parent)
    : QSortFilterProxyModel(parent)
    , d(new Private)
{
    setSourceModel(new KisAllResourcesModel(type));
}

KisResourceModel::~KisResourceModel()
{
    delete d;
}

void KisResourceModel::setResourceFilter(ResourceFilter filter)
{
    d->resourceFilter = filter;
}

void KisResourceModel::setStorageFilter(StorageFilter filter)
{
    d->storageFilter = filter;
}

KoResourceSP KisResourceModel::resourceForIndex(QModelIndex index) const
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->resourceForIndex(mapToSource(index));
    }
    return 0;
}

QModelIndex KisResourceModel::indexFromResource(KoResourceSP resource) const
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return mapFromSource(source->indexFromResource(resource));
    }
    return QModelIndex();
}

bool KisResourceModel::setResourceInactive(const QModelIndex &index)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->setResourceInactive(mapToSource(index));
    }
    return false;
}

bool KisResourceModel::importResourceFile(const QString &filename)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->importResourceFile(filename);
    }
    return false;
}

bool KisResourceModel::addResource(KoResourceSP resource, const QString &storageId)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->addResource(resource, storageId);
    }
    return false;
}

bool KisResourceModel::updateResource(KoResourceSP resource)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->updateResource(resource);
    }
    return false;
}

bool KisResourceModel::renameResource(KoResourceSP resource, const QString &name)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->renameResource(resource, name);
    }
    return false;
}

bool KisResourceModel::setResourceInactive(KoResourceSP resource)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->setResourceInactive(resource);
    }
    return false;
}

bool KisResourceModel::setResourceMetaData(KoResourceSP resource, QMap<QString, QVariant> metadata)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->setResourceMetaData(resource, metadata);
    }
    return false;
}




bool KisResourceModel::filterAcceptsColumn(int /*source_column*/, const QModelIndex &/*source_parent*/) const
{
    return true;
}

bool KisResourceModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (d->resourceFilter == ShowAllResources && d->storageFilter == ShowAllStorages) {
        return true;
    }

    QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
    ResourceFilter resourceActive = (ResourceFilter)sourceModel()->data(idx, Qt::UserRole + KisAbstractResourceModel::ResourceActive).toInt();
    StorageFilter storageActive =  (StorageFilter)sourceModel()->data(idx, Qt::UserRole + KisAbstractResourceModel::StorageActive).toInt();

    if (d->resourceFilter == ShowAllResources) {
        return (storageActive == d->storageFilter);
    }

    if (d->storageFilter == ShowAllStorages) {
        return (resourceActive == d->resourceFilter);
    }

    return ((storageActive == d->storageFilter) && (resourceActive == d->resourceFilter));
}

bool KisResourceModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    QString nameLeft = sourceModel()->data(source_left, Qt::UserRole + KisAbstractResourceModel::Name).toString();
    QString nameRight = sourceModel()->data(source_right, Qt::UserRole + KisAbstractResourceModel::Name).toString();
    return nameLeft < nameRight;
}


KoResourceSP KisResourceModel::resourceForId(int id) const
{
    return static_cast<KisAllResourcesModel*>(sourceModel())->resourceForId(id);
}

KoResourceSP KisResourceModel::resourceForFilename(QString fileName) const
{
    return static_cast<KisAllResourcesModel*>(sourceModel())->resourceForFilename(fileName);
}

KoResourceSP KisResourceModel::resourceForName(QString name) const
{
    return static_cast<KisAllResourcesModel*>(sourceModel())->resourceForName(name);
}

KoResourceSP KisResourceModel::resourceForMD5(const QByteArray md5sum) const
{
    return static_cast<KisAllResourcesModel*>(sourceModel())->resourceForMD5(md5sum);
}

QVector<KisTagSP> KisResourceModel::tagsForResource(int resourceId) const
{
    return static_cast<KisAllResourcesModel*>(sourceModel())->tagsForResource(resourceId);
}



bool KisResourceModel::resetQuery()
{
    return static_cast<KisAllResourcesModel*>(sourceModel())->resetQuery();
}

