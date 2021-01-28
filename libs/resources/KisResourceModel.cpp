/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisResourceModel.h"

#include <QElapsedTimer>
#include <QBuffer>
#include <QImage>
#include <QtSql>
#include <QStringList>

#include <KisResourceLocator.h>
#include <KisResourceCacheDb.h>

#include <KisResourceModelProvider.h>
#include <KisStorageModel.h>
#include <KisTagModel.h>

#include <kis_debug.h>

#include "KisResourceQueryMapper.h"

struct KisAllResourcesModel::Private {
    QSqlQuery resourcesQuery;
    QString resourceType;
    int columnCount {StorageActive};
    int cachedRowCount {-1};
};

KisAllResourcesModel::KisAllResourcesModel(const QString &resourceType, QObject *parent)
    : QAbstractTableModel(parent)
    , d(new Private)
{

    connect(KisResourceLocator::instance(), SIGNAL(storageAdded(const QString&)), this, SLOT(addStorage(const QString&)));
    connect(KisResourceLocator::instance(), SIGNAL(storageRemoved(const QString&)), this, SLOT(removeStorage(const QString&)));
    connect(KisStorageModel::instance(), SIGNAL(storageEnabled(const QString&)), this, SLOT(addStorage(const QString&)));
    connect(KisStorageModel::instance(), SIGNAL(storageDisabled(const QString&)), this, SLOT(removeStorage(const QString&)));

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
        v = KisResourceQueryMapper::variantFromResourceQuery(d->resourcesQuery, index.column(), role);
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
            return i18n("Id");
        case StorageId:
            return i18n("Storage ID");
        case Name:
            return i18n("Name");
        case Filename:
            return i18n("File Name");
        case Tooltip:
            return i18n("Tooltip");
        case Thumbnail:
            return i18n("Image");
        case Status:
            return i18n("Status");
        case Location:
            return i18n("Location");
        case ResourceType:
            return i18n("Resource Type");
        case ResourceActive:
            return i18n("Active");
        case StorageActive:
            return i18n("Storage Active");
        default:
            return QString::number(section);
        }
    }
    return v;
}

bool KisAllResourcesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::CheckStateRole) {
        if (value.toBool()) {
            if (!KisResourceLocator::instance()->setResourceActive(index.data(Qt::UserRole + KisAbstractResourceModel::Id).toInt(), value.toBool())) {
                return false;
            }
        }
        resetQuery();
        emit dataChanged(index, index, {role});
    }
    return true;
}

Qt::ItemFlags KisAllResourcesModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

KoResourceSP KisAllResourcesModel::resourceForIndex(QModelIndex index) const
{
    KoResourceSP resource = 0;

    if (!index.isValid()) return resource;
    if (index.row() > rowCount()) return resource;
    if (index.column() > d->columnCount) return resource;

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

QModelIndex KisAllResourcesModel::indexForResource(KoResourceSP resource) const
{
    if (!resource || !resource->valid() || resource->resourceId() < 0) return QModelIndex();

    // For now a linear seek to find the first resource with the right id
    d->resourcesQuery.first();
    do {
        if (d->resourcesQuery.value("id").toInt() == resource->resourceId()) {
            return index(d->resourcesQuery.at(), 0);
        }
    } while (d->resourcesQuery.next());
    
    return QModelIndex();
}

bool KisAllResourcesModel::setResourceInactive(const QModelIndex &index)
{
    if (index.row() > rowCount()) return false;
    if (index.column() > d->columnCount) return false;
    
    int resourceId = index.data(Qt::UserRole + Id).toInt();
    if (!KisResourceLocator::instance()->setResourceActive(resourceId)) {
        qWarning() << "Failed to remove resource" << resourceId;
        return false;
    }
    resetQuery();
    emit dataChanged(index, index, {Qt::EditRole, Qt::CheckStateRole});
    return true;
}
//static int s_i6 {0};

bool KisAllResourcesModel::importResourceFile(const QString &filename)
{
    bool r = true;
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    if (!KisResourceLocator::instance()->importResourceFromFile(d->resourceType, filename)) {
        r = false;
        qWarning() << "Failed to import resource" << filename;
    }
    resetQuery();
    endInsertRows();
    return r;
}

bool KisAllResourcesModel::addResource(KoResourceSP resource, const QString &storageId)
{

    if (!resource || !resource->valid()) {
        qWarning() << "Cannot add resource. Resource is null or not valid";
        return false;
    }

    bool r = true;
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    if (!KisResourceLocator::instance()->addResource(d->resourceType, resource, storageId)) {
        qWarning() << "Failed to add resource" << resource->name();
        r = false;
    }
    resetQuery();
    endInsertRows();
    return r;
}

bool KisAllResourcesModel::updateResource(KoResourceSP resource)
{
    if (!resource || !resource->valid()) {
        qWarning() << "Cannot update resource. Resource is null or not valid";
        return false;
    }

    if (!KisResourceLocator::instance()->updateResource(d->resourceType, resource)) {
        qWarning() << "Failed to update resource" << resource;
        return false;
    }
    bool r = resetQuery();
    QModelIndex index = indexForResource(resource);
    emit dataChanged(index, index, {Qt::EditRole});
    return r;
}

bool KisAllResourcesModel::reloadResource(KoResourceSP resource)
{
    if (!resource || !resource->valid()) {
        qWarning() << "Cannot reload resource. Resource is null or not valid";
        return false;
    }

    if (!KisResourceLocator::instance()->reloadResource(d->resourceType, resource)) {
        qWarning() << "Failed to reload resource" << resource;
        return false;
    }
    bool r = resetQuery();
    QModelIndex index = indexForResource(resource);
    emit dataChanged(index, index, {Qt::EditRole});
    return r;
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
    bool r = resetQuery();
    QModelIndex index = indexForResource(resource);
    emit dataChanged(index, index, {Qt::EditRole});
    return r;
}

//static int s_i9 {0};

bool KisAllResourcesModel::setResourceMetaData(KoResourceSP resource, QMap<QString, QVariant> metadata)
{
    Q_ASSERT(resource->resourceId() > -1);
    return KisResourceLocator::instance()->setMetaDataForResource(resource->resourceId(), metadata);
}

bool KisAllResourcesModel::resetQuery()
{
    bool r = d->resourcesQuery.exec();
    if (!r) {
        qWarning() << "Could not select" << d->resourceType << "resources" << d->resourcesQuery.lastError() << d->resourcesQuery.boundValues();
    }
    d->cachedRowCount = -1;

    return r;
}

QVector<KisTagSP> KisAllResourcesModel::tagsForResource(int resourceId) const
{
    bool r;

    QSqlQuery q;

    r = q.prepare("SELECT tags.id\n"
                  ",      tags.url\n"
                  ",      tags.name\n"
                  ",      tags.comment\n"
                  ",      resource_types.name as resource_type\n"
                  "FROM   tags\n"
                  ",      resource_tags\n"
                  ",      resource_types\n"
                  "WHERE  tags.active > 0\n"                               // make sure the tag is active
                  "AND    tags.id = resource_tags.tag_id\n"                // join tags + resource_tags by tag_id
                  "AND    resource_tags.resource_id = :resource_id\n"
                  "AND    resource_types.id = tags.resource_type_id\n");    // make sure we're looking for tags for a specific resource
    if (!r)  {
        qWarning() << "Could not prepare TagsForResource query" << q.lastError();
    }

    q.bindValue(":resource_id", resourceId);
    r = q.exec();
    if (!r) {
        qWarning() << "Could not select tags for" << resourceId << q.lastError() << q.boundValues();
    }

    QVector<KisTagSP> tags;
    while (q.next()) {
        KisTagSP tag(new KisTag());
        tag->setId(q.value("id").toInt());
        tag->setUrl(q.value("url").toString());
        tag->setName(q.value("name").toString());
        tag->setComment(q.value("comment").toString());
        tag->setResourceType(q.value("resource_type").toString());
        tag->setValid(true);
        tag->setActive(true);
        tags << tag;
    }
    return tags;
}


int KisAllResourcesModel::rowCount(const QModelIndex &) const
{
    if (d->cachedRowCount < 0) {
        QSqlQuery q;
        q.prepare("SELECT count(*)\n"
                  "FROM   resources\n"
                  ",      resource_types\n"
                  "WHERE  resources.resource_type_id = resource_types.id\n"
                  "AND    resource_types.name = :resource_type\n");
        q.bindValue(":resource_type", d->resourceType);
        q.exec();
        q.first();
        
        const_cast<KisAllResourcesModel*>(this)->d->cachedRowCount = q.value(0).toInt();
    }
    
    return d->cachedRowCount;
}


void KisAllResourcesModel::addStorage(const QString &/*location*/)
{
    qDebug() << "KisAllResourcesModel::addStorage" << rowCount();

    beginResetModel();
    resetQuery();
    endResetModel();

    qDebug() << "\t\t" << rowCount();
}



void KisAllResourcesModel::removeStorage(const QString &/*location*/)
{
    qDebug() << "KisAllResourcesModel::removeStorage" << rowCount();

    beginResetModel();
    resetQuery();
    endResetModel();

    qDebug() << "\t\t" << rowCount();
}

struct KisResourceModel::Private
{
    ResourceFilter resourceFilter {ShowActiveResources};
    StorageFilter storageFilter {ShowActiveStorages};
    bool showOnlyUntaggedResources {false};
};

KisResourceModel::KisResourceModel(const QString &type, QObject *parent)
    : QSortFilterProxyModel(parent)
    , d(new Private)
{
    setSourceModel(KisResourceModelProvider::resourceModel(type));
    setDynamicSortFilter(true);
}

KisResourceModel::~KisResourceModel()
{
    delete d;
}

void KisResourceModel::setResourceFilter(ResourceFilter filter)
{
    if (d->resourceFilter != filter) {
        d->resourceFilter = filter;
        invalidateFilter();
    }
}

void KisResourceModel::setStorageFilter(StorageFilter filter)
{
    if (d->storageFilter != filter) {
        d->storageFilter = filter;
        invalidateFilter();
    }
}

void KisResourceModel::showOnlyUntaggedResources(bool showOnlyUntagged)
{
    d->showOnlyUntaggedResources = showOnlyUntagged;
    invalidateFilter();
}

KoResourceSP KisResourceModel::resourceForIndex(QModelIndex index) const
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->resourceForIndex(mapToSource(index));
    }
    return 0;
}

QModelIndex KisResourceModel::indexForResource(KoResourceSP resource) const
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return mapFromSource(source->indexForResource(resource));
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

bool KisResourceModel::reloadResource(KoResourceSP resource)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->reloadResource(resource);
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
    QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
    if (idx.isValid()) {
        int id = idx.data(Qt::UserRole + KisAbstractResourceModel::Id).toInt();

        if (d->showOnlyUntaggedResources) {

            QString queryString = ("SELECT COUNT(*)\n"
                                   "FROM   resource_tags\n"
                                   ",      resources\n"
                                   ",      storages\n"
                                   "WHERE  resource_tags.resource_id = resources.id\n"
                                   "AND    storages.id               = resources.storage_id\n"
                                   "AND    resources.id              = :resource_id\n");

            if (d->resourceFilter == ShowActiveResources) {
                queryString.append("AND    resources.status > 0\n");
            }
            else if (d->resourceFilter == ShowInactiveResources) {
                queryString.append("AND    resources.status = 0\n");
            }

            if (d->storageFilter == ShowActiveStorages) {
                queryString.append("AND    storages.active > 0\n");
            }
            else if (d->storageFilter == ShowInactiveStorages) {
                queryString.append("AND    storages.active = 0\n");
            }

            QSqlQuery q;

            if (!q.prepare((queryString))) {
                qWarning() << "KisResourceModel: Could not prepare resource_tags query" << q.lastError();
            }

            q.bindValue(":resource_id", id);

            if (!q.exec()) {
                qWarning() << "KisResourceModel: Could not execute resource_tags query" << q.lastError() << q.boundValues();
            }

            q.first();
            if (q.value(0).toInt() > 0) {
                return false;
            }
        }
    }

    if (d->resourceFilter == ShowAllResources && d->storageFilter == ShowAllStorages) {
        return true;
    }

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
