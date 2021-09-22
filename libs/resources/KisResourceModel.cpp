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
#include <KisResourceTypes.h>
#include <kis_debug.h>
#include <KisGlobalResourcesInterface.h>

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
                                       ",      resources.md5sum\n"
                                       ",      storages.location\n"
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

int KisAllResourcesModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

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
        case MD5:
            return i18n("md5sum");
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
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable | Qt::ItemNeverHasChildren;
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

bool KisAllResourcesModel::resourceExists(const QString &md5, const QString &filename, const QString &name)
{
    QSqlQuery q;

    // md5

    if (!md5.isEmpty()) {

        bool r = q.prepare("SELECT resources.id AS id\n"
                           "FROM   resources\n"
                           "WHERE  md5sum = :md5sum");
        if (!r) {
            qWarning() << "Could not prepare find resourceExists by md5 query"  << q.lastError();
        }

        q.bindValue(":mdsum", md5);

        r = q.exec();

        if (!r) {
            qWarning() << "Could not execute resourceExists by md5 query" << q.lastError();
        }

        if (q.first()) {
            return true;
        }
    }

    // filename

    if (!filename.isEmpty()) {

        bool r = q.prepare("SELECT resources.id AS id\n"
                      "FROM   resources\n"
                      "WHERE  filename = :filename");
        if (!r) {
            qWarning() << "Could not prepare find resourceExists by filename query"  << q.lastError();
        }

        q.bindValue(":filename", filename);

        r = q.exec();

        if (!r) {
            qWarning() << "Could not execute resourceExists by filename query" << q.lastError();
        }

        if (q.first()) {
            return true;
        }
    }

    // name

    if (!name.isEmpty()) {

        bool r = q.prepare("SELECT resources.id AS id\n"
                      "FROM   resources\n"
                      "WHERE  name = :name");
        if (!r) {
            qWarning() << "Could not prepare find resourceExists by name query"  << q.lastError();
        }

        q.bindValue(":name", name);

        r = q.exec();
        if (!r) {
            qWarning() << "Could not execute resourceExists by name query" << q.lastError();
        }

        if (q.first()) {
            return true;
        }
    }

    // failure

    return false;
}

QVector<KoResourceSP> KisAllResourcesModel::resourcesForFilename(QString filename, bool checkDependentResources) const
{
    QVector<KoResourceSP> resources;

    if (filename.isEmpty()) return resources;

    QSqlQuery q;
    bool r = q.prepare("SELECT resources.id AS id\n"
                       "FROM   resources\n"
                       ",      resource_types\n"
                       "WHERE  resources.resource_type_id = resource_types.id\n"
                       "AND    resources.filename = :resource_filename\n"
                       "AND    resource_types.name = :resource_type\n");
    if (!r) {
        qWarning() << "Could not prepare KisAllResourcesModel query for resource name" << q.lastError();
    }
    q.bindValue(":resource_filename", filename);
    q.bindValue(":resource_type", d->resourceType);

    r = q.exec();
    if (!r) {
        qWarning() << "Could not select" << d->resourceType << "resources by filename" << q.lastError() << q.boundValues();
    }

    while (q.next()) {
        int id = q.value("id").toInt();
        KoResourceSP resource = KisResourceLocator::instance()->resourceForId(id);
        if (resource) {
            resources << resource;
        }

    }

    if (resources.isEmpty() && checkDependentResources) {
        // Check whether the requested resource was embedded in another resource, which has not been loaded so the embedded resource is not available.
        r = q.prepare("SELECT value"
                      ",      foreign_id\n"
                      "FROM   metadata\n"
                      "WHERE  key = \"dependent_resources_filenames\"\n"
                      "AND    table_name = \"resources\"\n");

        r = q.exec();
        if (!r) {
            qWarning() << "Could not execute metadata query" << q.lastError();
        }

        while (q.next()) {
            QByteArray ba = q.value(0).toByteArray();
            int id = q.value(1).toInt();
            if (!ba.isEmpty()) {
                QDataStream ds(QByteArray::fromBase64(ba));
                QVariant value;
                ds >> value;
                QStringList l = value.toStringList();
                if (l.contains(filename)) {
                    // This will load the embedded resource and make it available
                    KoResourceSP res = KisResourceLocator::instance()->resourceForId(id);
                    Q_FOREACH(KoResourceSP embeddedRes, res->embeddedResources(KisGlobalResourcesInterface::instance())) {
                        // This is the best we can do because Krita4 only checked filename and resource type, too.
                        if (embeddedRes->filename() == filename && embeddedRes->resourceType().first == d->resourceType) {
                            resources << embeddedRes;
                        }
                    }
                }
            }
        }
    }
    return resources;
}

QVector<KoResourceSP> KisAllResourcesModel::resourcesForName(const QString &name) const
{
    QVector<KoResourceSP> resources;

    if (name.isEmpty()) return resources;

    KoResourceSP resource = 0;

    QSqlQuery q;
    bool r = q.prepare("SELECT resources.id AS id\n"
                       "FROM   resources\n"
                       ",      resource_types\n"
                       "WHERE  resources.resource_type_id = resource_types.id\n"
                       "AND    resources.name = :resource_name\n"
                       "AND    resource_types.name = :resource_type\n");
    if (!r) {
        qWarning() << "Could not prepare KisAllResourcesModel query for resource name" << q.lastError();
    }

    q.bindValue(":resource_type", d->resourceType);
    q.bindValue(":resource_name", name);

    r = q.exec();
    if (!r) {
        qWarning() << "Could not select" << d->resourceType << "resources by name" << q.lastError() << q.boundValues();
    }

    while (q.next()) {
        int id = q.value("id").toInt();
        resource = KisResourceLocator::instance()->resourceForId(id);
        if (resource) {
            resources << resource;
        }
    }

    return resources;
}


QVector<KoResourceSP> KisAllResourcesModel::resourcesForMD5(const QString &md5sum) const
{
    QVector<KoResourceSP> resources;

    if (md5sum.isEmpty()) return resources;

    KoResourceSP resource = 0;

    QSqlQuery q;
    bool r = q.prepare("SELECT resource_id AS id\n"
                       "FROM   versioned_resources\n"
                       "WHERE  md5sum = :md5sum");
    if (!r) {
        qWarning() << "Could not prepare KisAllResourcesModel query for resource md5" << q.lastError();
    }
    q.bindValue(":md5sum", md5sum);

    r = q.exec();
    if (!r) {
        qWarning() << "Could not select" << d->resourceType << "resources by md5" << q.lastError() << q.boundValues();
    }

    while (q.next()) {
        int id = q.value("id").toInt();
        resource = KisResourceLocator::instance()->resourceForId(id);
        if (resource) {
            resources << resource;
        }
    }
    return resources;
}

QModelIndex KisAllResourcesModel::indexForResource(KoResourceSP resource) const
{
    if (!resource || !resource->valid() || resource->resourceId() < 0) return QModelIndex();

    // For now a linear seek to find the first resource with the right id
    return indexForResourceId(resource->resourceId());
}

QModelIndex KisAllResourcesModel::indexForResourceId(int resourceId) const
{
    d->resourcesQuery.first();
    do {
        if (d->resourcesQuery.value("id").toInt() == resourceId) {
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

KoResourceSP KisAllResourcesModel::importResourceFile(const QString &filename, const bool allowOverwrite, const QString &storageId)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    KoResourceSP res = KisResourceLocator::instance()->importResourceFromFile(d->resourceType, filename, allowOverwrite, storageId);
    if (!res) {
        qWarning() << "Failed to import resource" << filename;
    }
    resetQuery();
    endInsertRows();

    return res;
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


int KisAllResourcesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

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


void KisAllResourcesModel::addStorage(const QString &location)
{
    Q_UNUSED(location)
    beginResetModel();
    resetQuery();
    endResetModel();
}

void KisAllResourcesModel::removeStorage(const QString &location)
{
    Q_UNUSED(location)
    beginResetModel();
    resetQuery();
    endResetModel();
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

QModelIndex KisResourceModel::indexForResourceId(int resourceId) const
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return mapFromSource(source->indexForResourceId(resourceId));
    }
    return QModelIndex();
}

bool KisResourceModel::setResourceInactive(const QModelIndex &index)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->setResourceInactive(mapToSource(index));
    }
    if (d->resourceFilter != KisResourceModel::ResourceFilter::ShowAllResources) {
        invalidate();
    }
    return false;
}

KoResourceSP KisResourceModel::importResourceFile(const QString &filename, const bool allowOverwrite, const QString &storageId)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    KoResourceSP res;
    if (source) {
        res = source->importResourceFile(filename, allowOverwrite, storageId);
    }
    invalidate();
    return res;
}

bool KisResourceModel::addResource(KoResourceSP resource, const QString &storageId)
{
    KisAllResourcesModel *source = qobject_cast<KisAllResourcesModel*>(sourceModel());
    bool updateInsteadOfAdd = false;
    bool result = false;

    // Check whether the resource already existed, in that case, we will update
    // and possibly reactivate the resource
    QSqlQuery q;

    if (!q.prepare("SELECT resources.id\n"
                   ",      resources.md5sum\n"
                   ",      storages.location\n"
                   ",      resource_types.name\n"
                   "FROM   resources\n"
                   ",      storages\n"
                   ",      resource_types\n"
                   "WHERE  resources.name             = :name\n"
                   "AND    resources.storage_id       = storages.id\n"
                   "AND    resources.resource_type_id = resource_types.id\n"
                   "AND    resources.status           = 0")) {
        qWarning() << "Could not create KisResourceModel::addResource query" << q.lastError();
    }

    q.bindValue(":name", resource->name());

    if (!q.exec()) {
        qWarning() << "Could not execute KisResourceModel::addResource query" << q.lastError();
    }

    while (q.next()) {
        int id = q.value(0).toInt();
        QString md5sum = q.value(1).toString();
        QString storageLocation = q.value(2).toString();
        QString resourceType = q.value(3).toString();


        QSqlQuery q2;

        if (!q2.prepare("SELECT MAX(version)\n"
                       "FROM   versioned_resources\n"
                       "WHERE  resource_id = :id")) {
            qWarning() << "Could not prepare versioned_resources query" << q.lastError();
        }

        q2.bindValue(":id", id);

        if (!q2.exec()) {
            qWarning() << "Could not execute versioned_resources query" << q.lastError();
        }

        if (!q2.first()) {
            qWarning() << "No resource version found with id" << id;
        }

        q.first();

        int version = q2.value(0).toInt();

        if (resourceType == resource->resourceType().first) {
            resource->setResourceId(id);
            resource->setVersion(version);
            resource->setMD5Sum(md5sum);
            resource->setActive(true);
            resource->setStorageLocation(storageLocation);
            bool result = updateResource(resource);
            updateInsteadOfAdd = result;
            break;
        }
    }

    if (!updateInsteadOfAdd) {
        result = source->addResource(resource, storageId);
    }

    if (result) {
        invalidate();
    }

    return result;
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

    return filterResource(idx);
}

bool KisResourceModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    QString nameLeft = sourceModel()->data(source_left, Qt::UserRole + KisAbstractResourceModel::Name).toString();
    QString nameRight = sourceModel()->data(source_right, Qt::UserRole + KisAbstractResourceModel::Name).toString();

    return nameLeft.toLower() < nameRight.toLower();
}

QVector<KoResourceSP> KisResourceModel::filterByColumn(const QString filter, KisAbstractResourceModel::Columns column) const
{
    QVector<KoResourceSP> resources;
    for (int i = 0; i < rowCount(); ++i) {
        QModelIndex idx = index(i, 0);
        if (idx.isValid() && data(idx, Qt::UserRole + column).toString() == filter) {
            resources << resourceForIndex(idx);
        }
    }

    return resources;
}

bool KisResourceModel::filterResource(const QModelIndex &idx) const
{
    if (d->resourceFilter == ShowAllResources && d->storageFilter == ShowAllStorages) {
        return true;
    }

    ResourceFilter resourceActive = (ResourceFilter)sourceModel()->data(idx, Qt::UserRole + KisAbstractResourceModel::ResourceActive).toInt();
    StorageFilter storageActive =  (StorageFilter)sourceModel()->data(idx, Qt::UserRole + KisAbstractResourceModel::StorageActive).toInt();

    if (d->resourceFilter == ShowAllResources) {
        if (storageActive == d->storageFilter) {
            return true;
        }
    }

    if (d->storageFilter == ShowAllStorages) {
        if (resourceActive == d->resourceFilter) {
            return true;
        }
    }

    if ((storageActive == d->storageFilter) && (resourceActive == d->resourceFilter)) {
        return true;
    }

    return false;
}


KoResourceSP KisResourceModel::resourceForId(int id) const
{
    KoResourceSP res = static_cast<KisAllResourcesModel*>(sourceModel())->resourceForId(id);
    QModelIndex idx = indexForResource(res);
    if (idx.isValid()) {
        return res;
    }
    return 0;
}

QVector<KoResourceSP> KisResourceModel::resourcesForFilename(QString filename) const
{
    return filterByColumn(filename, KisAllResourcesModel::Filename);

}

QVector<KoResourceSP> KisResourceModel::resourcesForName(QString name) const
{
    return filterByColumn(name, KisAllResourcesModel::Name);
}

QVector<KoResourceSP> KisResourceModel::resourcesForMD5(const QString md5sum) const
{
    return filterByColumn(md5sum, KisAllResourcesModel::MD5);
}

QVector<KisTagSP> KisResourceModel::tagsForResource(int resourceId) const
{
    return static_cast<KisAllResourcesModel*>(sourceModel())->tagsForResource(resourceId);
}
