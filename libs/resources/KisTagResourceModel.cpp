/*
 * SPDX-FileCopyrightText: 2020 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "KisTagResourceModel.h"

#include <QtSql>
#include <QMap>

#include <KisResourceLocator.h>
#include <KisResourceModel.h>
#include <KisResourceModelProvider.h>
#include <KisResourceQueryMapper.h>
#include <KisStorageModel.h>
#include <kis_assert.h>

struct KisAllTagResourceModel::Private {
    QString resourceType;
    QSqlQuery query;
    int columnCount { TagName + 1 };
    int cachedRowCount {-1};
};


KisAllTagResourceModel::KisAllTagResourceModel(const QString &resourceType, QObject *parent)
    : QAbstractTableModel(parent)
    , d(new Private())
{
    d->resourceType = resourceType;
    resetQuery();

    connect(KisResourceLocator::instance(), SIGNAL(storageAdded(const QString&)), this, SLOT(addStorage(const QString&)));
    connect(KisResourceLocator::instance(), SIGNAL(storageRemoved(const QString&)), this, SLOT(removeStorage(const QString&)));
    connect(KisStorageModel::instance(), SIGNAL(storageEnabled(const QString&)), this, SLOT(addStorage(const QString&)));
    connect(KisStorageModel::instance(), SIGNAL(storageDisabled(const QString&)), this, SLOT(removeStorage(const QString&)));
    connect(KisResourceLocator::instance(), SIGNAL(resourceActiveStateChanged(const QString&, int)), this, SLOT(slotResourceActiveStateChanged(const QString&, int)));

    /**
     * TODO: connect to beginExternalResourceImport() and beginExternalResourceRemove
     *       as well. It seems to work without them somehow, but I guess it is just a
     *       coincidence or UB
     */
}

KisAllTagResourceModel::~KisAllTagResourceModel()
{
    delete d;
}

int KisAllTagResourceModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    if (d->cachedRowCount < 0) {
        QSqlQuery q;
        q.prepare("SELECT COUNT(DISTINCT resource_tags.tag_id || resources.name || resources.filename || resources.md5sum)\n"
                  "FROM   resource_tags\n"
                  ",      resources\n"
                  ",      resource_types\n"
                  "WHERE  resource_tags.resource_id = resources.id\n"
                  "AND    resources.resource_type_id = resource_types.id\n"
                  "AND    resource_types.name = :resource_type\n"
                  "AND    resource_tags.active = 1\n");

        q.bindValue(":resource_type", d->resourceType);

        if (!q.exec()) {
            qWarning() << "Could not execute resource/tags rowcount query" << q.lastError();
        }

        q.first();

        const_cast<KisAllTagResourceModel*>(this)->d->cachedRowCount = q.value(0).toInt();
    }
    return d->cachedRowCount;
}

int KisAllTagResourceModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return d->columnCount;
}

QVariant KisAllTagResourceModel::data(const QModelIndex &index, int role) const
{
    QVariant v;

    if (!index.isValid()) { return v; }
    if (index.row() > rowCount()) { return v; }
    if (index.column() > d->columnCount) { return v;}

    bool pos = const_cast<KisAllTagResourceModel*>(this)->d->query.seek(index.row());
    if (!pos) {return v;}

    if (role < Qt::UserRole + TagId && index.column() < TagId) {
        v = KisResourceQueryMapper::variantFromResourceQuery(d->query, index.column(), role, true);
    }

    if (index.column() >= TagId) {
        // trick to get the correct value without writing everything again
        // this is used for example in case of sorting
        role = Qt::UserRole + index.column();
    }

    // These are not shown, but needed for the filter
    switch(role) {
    case Qt::UserRole + TagId:
    {
        v = d->query.value("tag_id");
        break;
    }
    case Qt::UserRole + ResourceId:
    {
        v = d->query.value("resource_id");
        break;
    }
    case Qt::UserRole + Tag:
    {
        KisTagSP tag = KisResourceLocator::instance()->tagForUrl(d->query.value("tag_url").toString(), d->resourceType);
        v = QVariant::fromValue(tag);
        break;
    }
    case Qt::UserRole + Resource:
    {
        v = QVariant::fromValue(KisResourceLocator::instance()->resourceForId(d->query.value("resource_id").toInt()));
        break;
    }
    case Qt::UserRole + ResourceActive:
    {
        v = d->query.value("resource_active");
        break;
    }
    case Qt::UserRole + TagActive:
    {
        v = d->query.value("tag_active");
        break;
    }
    case Qt::UserRole + ResourceStorageActive:
    {
        v = d->query.value("resource_storage_active");
        break;
    }
    case Qt::UserRole + ResourceName:
    {
        v = d->query.value("resource_name");
        break;
    }
    case Qt::UserRole + TagName:
    {
        v = d->query.value("translated_name");
        if (v.isNull()) {
            v = d->query.value("tag_name");
        }
        break;
    }

    default:
        ;
    }
    return v;
}

bool KisAllTagResourceModel::tagResources(const KisTagSP tag, const QVector<int>& resourceIds)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(tag && tag->valid() && tag->id() >= 0, false);

    // notes for performance:
    // the only two costly parts are:
    // - executing the query constructed by createQuery()
    // - running endInsertRows() (because it updates all the views and filter proxies etc...)

    QVector<int> resourceIdsToAdd;
    QVector<int> resourceIdsToUpdate;

    // looks expensive but actually isn't
    for (int i = 0; i < resourceIds.count(); i++) {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(resourceIds[i] >= 0, false);
        int taggedState = isResourceTagged(tag, resourceIds[i]);
        switch (taggedState) {
        case -1:
            // never tagged
            resourceIdsToAdd.append(resourceIds[i]);
            break;
        case 0:
            // tagged but then untagged
            resourceIdsToUpdate.append(resourceIds[i]);
            break;
        case 1:
            // it means the resource is already tagged; do nothing
            break;
        }
    }

    if (resourceIdsToAdd.isEmpty() && resourceIdsToUpdate.isEmpty()) {
        // Is already tagged, let's do nothing
        return true;
    }

    int howManyTimesBeginInsertedCalled = 0;

    if (resourceIdsToUpdate.count() > 0) {

        QSqlQuery allIndices;
        if (!allIndices.prepare(createQuery(false, true))) {
            qWarning() << "Could not prepare tagResource-allIndices query" << allIndices.lastError();
        }

        allIndices.bindValue(":resource_type", d->resourceType);
        allIndices.bindValue(":language", KisTag::currentLocale());

        if (!allIndices.exec()) {
            qWarning() << "Could not execute tagResource-allIndices query" << allIndices.lastError();
        }

        int activesRowId = -1;
        int lastActiveRowId = -1;

        // needed for beginInsertRows calculations
        QMap<int, int> resourcesCountForLastActiveRowId;

        int idd = 0;

        while (allIndices.next()) {
            idd++;
            bool isActive = allIndices.value("resource_tags_pair_active").toBool();
            if (isActive) {
                activesRowId++;
                lastActiveRowId = activesRowId;
            } else {
                bool variantSuccess = true;
                int rowTagId = allIndices.value("tag_id").toInt(&variantSuccess);
                KIS_SAFE_ASSERT_RECOVER(variantSuccess) { rowTagId = -1; }
                int rowResourceId = allIndices.value("resource_id").toInt(&variantSuccess);
                KIS_SAFE_ASSERT_RECOVER(variantSuccess) { rowResourceId = -1; }
                if (rowTagId == tag->id() && resourceIdsToUpdate.contains(rowResourceId)) {
                    if (!resourcesCountForLastActiveRowId.contains(lastActiveRowId)) {
                        resourcesCountForLastActiveRowId[lastActiveRowId] = 1;
                    } else {
                        resourcesCountForLastActiveRowId[lastActiveRowId] = resourcesCountForLastActiveRowId[lastActiveRowId] + 1;
                    }
                }
            }
         }

        Q_FOREACH(const int key, resourcesCountForLastActiveRowId.keys()) {
            // when having multiple beginInsertRows:
            // let's say you have this model:
            // 0  A
            // 1  A
            // 2  A
            //  <- put 2 Bs here
            // 3  A
            //  <- put one B here
            // 4  A
            // and you want to add the two Bs and then add another B
            // then the signals should be:
            // beginRemoveRows(3, 4) <- new indices of the two Bs
            // beginRemoveRows(4, 4) <- new index of the one B ignoring the action before


            beginInsertRows(QModelIndex(), key + 1, key + resourcesCountForLastActiveRowId[key]);
            howManyTimesBeginInsertedCalled++;
        }

        // Resource was tagged, then untagged. Tag again;
        QSqlQuery q;

        if (!q.prepare("UPDATE resource_tags\n"
                       "SET    active = 1\n"
                       "WHERE  resource_id = :resource_id\n"
                       "AND    tag_id      = :tag_id")) {


            qWarning() << "Could not prepare update resource_tags to active statement" << q.lastError();

            return false;
        }

        QVariantList resourceIdsVariants;
        QVariantList tagIdVariants;

        for (int i = 0; i < resourceIdsToUpdate.count(); i++) {
            resourceIdsVariants << QVariant(resourceIdsToUpdate[i]);
            tagIdVariants << QVariant(tag->id());
        }

        q.bindValue(":resource_id", resourceIdsVariants);
        q.bindValue(":tag_id", tagIdVariants);

        if (!q.execBatch()) {
            qWarning() << "Could not execute update resource_tags to active statement" << q.lastError();
            for (int i = 0; i < howManyTimesBeginInsertedCalled; i++) {
                endInsertRows();
            }
            return false;
        }

    }

    if (resourceIdsToAdd.count() > 0) {

        beginInsertRows(QModelIndex(), rowCount(), rowCount() + resourceIdsToAdd.count() - 1);
        howManyTimesBeginInsertedCalled++;

        // Resource was never tagged before, insert it. The active column is DEFAULT 1
        QSqlQuery q;


        QString values;
        for (int i = 0; i < resourceIdsToAdd.count(); i++) {
            if (i > 0) {
                values.append(", ");
            }
            values.append("(?, ?, ?)");
        }

        if (!q.prepare(QString("INSERT INTO resource_tags\n"
                       "(resource_id, tag_id, active)\n"
                       "VALUES ") + values + QString(";\n"))) {
            qWarning() << "Could not prepare insert into resource tags statement" << q.lastError();
            for (int i = 0; i < howManyTimesBeginInsertedCalled; i++) {
                endInsertRows();
            }
            return false;
        }

        for (int i = 0; i < resourceIdsToAdd.count(); i++) {
            q.addBindValue(resourceIdsToAdd[i]);
            q.addBindValue(tag->id());
            q.addBindValue(true);

        }

        if (!q.exec()) {
            qWarning() << "Could not execute insert into resource tags statement" << q.boundValues() << q.lastError();
            for (int i = 0; i < howManyTimesBeginInsertedCalled; i++) {
                endInsertRows();
            }
            return false;
        }
    }

    resetQuery();

    for (int i = 0; i < howManyTimesBeginInsertedCalled; i++) {
        endInsertRows();
    }

    return true;
}

bool KisAllTagResourceModel::untagResources(const KisTagSP tag, const QVector<int> &resourceIds)
{
    if (!tag || !tag->valid()) return false;
    if (!d->query.isSelect()) return false;
    if (rowCount() < 1) return false;

    int beginRemoveRowsCount = 0;

    QSqlQuery q;

    if (!q.prepare("UPDATE resource_tags\n"
                   "SET    active      = 0\n"
                   "WHERE  tag_id      = :tag_id\n"
                   "AND    resource_id = :resource_id")) {
        qWarning() << "Could not prepare untagResource-update query" << q.lastError();
        return false;
    }

    QSqlQuery allIndices;
    if (!allIndices.prepare(createQuery(true, true))) {
        qWarning() << "Coult not prepare untagResource-allIndices query " << allIndices.lastError();
    }

    allIndices.bindValue(":resource_type", d->resourceType);
    allIndices.bindValue(":language", KisTag::currentLocale());

    if (!allIndices.exec()) {
        qCritical() << "Could not exec untagResource-allIndices query " << allIndices.lastError();
    }

    int activesRowId = -1;
    int lastActiveRowId = -1;

    // needed for beginInsertRows indices calculations
    QMap<int, int> resourcesCountForLastActiveRowId;

    int idd = 0;

    while (allIndices.next()) {
        idd++;
        bool variantSuccess = true;

        bool isActive = true; // all of them are active!
        KIS_SAFE_ASSERT_RECOVER(variantSuccess) { isActive = false; }

        int rowTagId = allIndices.value("tag_id").toInt(&variantSuccess);
        KIS_SAFE_ASSERT_RECOVER(variantSuccess) { rowTagId = -1; }
        int rowResourceId = allIndices.value("resource_id").toInt(&variantSuccess);
        KIS_SAFE_ASSERT_RECOVER(variantSuccess) { rowResourceId = -1; }

        bool willStayActive = isActive && (rowTagId != tag->id() || !resourceIds.contains(rowResourceId));
        activesRowId++;
        if (willStayActive) {
            lastActiveRowId = activesRowId;
        } else if (isActive) {
            // means we're removing it
            if (!resourcesCountForLastActiveRowId.contains(lastActiveRowId)) {
                resourcesCountForLastActiveRowId[lastActiveRowId] = 0;
            }
            resourcesCountForLastActiveRowId[lastActiveRowId]++;
        }
    }

    Q_FOREACH(const int key, resourcesCountForLastActiveRowId.keys()) {
        // when having multiple beginRemoveRows:
        // let's say you have this model:
        // 0  A
        // 1  A
        // 2  A
        // 3  *B*
        // 4  *B*
        // 5  A
        // 6  *B*
        // 7  A
        // and you want to remove all Bs
        // then the signals should be:
        // beginRemoveRows(3, 4) <- first two indices in the obvious way
        // beginRemoveRows(4, 4) <- next index as if the first action was already done
        //  (so `5  A` already became `3  A`, so the *B* is `4  B`, not `6  B`)

        beginRemoveRows(QModelIndex(), key + 1, key + resourcesCountForLastActiveRowId[key]);
        beginRemoveRowsCount++;
    }

    QSqlDatabase::database().transaction();
    for (int i = 0; i < resourceIds.count(); i++) {
        int resourceId = resourceIds[i];

        if (resourceId < 0) continue;
        if (isResourceTagged(tag, resourceId) < 1) continue;

        q.bindValue(":tag_id", tag->id());
        q.bindValue(":resource_id", resourceId);

        if (!q.exec()) {
            qWarning() << "Could not execute untagResource-update query" << q.lastError() << q.boundValues();
            for (int i = 0; i < beginRemoveRowsCount; i++) {
                endRemoveRows();
            }
            QSqlDatabase::database().rollback();
            return false;
        }
    }
    QSqlDatabase::database().commit();

    if (beginRemoveRowsCount > 0) {
        resetQuery();
        for (int i = 0; i < beginRemoveRowsCount; i++) {
            endRemoveRows();
        }
    }

    return true;
}

int KisAllTagResourceModel::isResourceTagged(const KisTagSP tag, const int resourceId)
{
    QSqlQuery query;
    bool r = query.prepare("SELECT resource_tags.active\n"
                           "FROM   resource_tags\n"
                           "WHERE  resource_tags.resource_id = :resource_id\n"
                           "AND    resource_tags.tag_id = :tag_id\n");

    if (!r) {
        qWarning() << "Could not prepare bool KisAllTagResourceModel::checkResourceTaggedState query" << query.lastError();
        return false;
    }

    query.bindValue(":resource_id", resourceId);
    query.bindValue(":tag_id", tag->id());

    if (!query.exec()) {
        qWarning() << "Could not execute is resource tagged with a specific tag query" << query.boundValues() << query.lastError();
        return false;
    }

    r = query.first();
    if (!r) {
        // Resource was not tagged
        return -1;
    }

    return query.value(0).toInt() > 0;
}

void KisAllTagResourceModel::addStorage(const QString &location)
{
    Q_UNUSED(location);
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    resetQuery();
    endInsertRows();
}

void KisAllTagResourceModel::removeStorage(const QString &location)
{
    Q_UNUSED(location);
    beginRemoveRows(QModelIndex(), rowCount(), rowCount());
    resetQuery();
    endRemoveRows();
}

void KisAllTagResourceModel::slotResourceActiveStateChanged(const QString &resourceType, int resourceId)
{
    if (resourceType != d->resourceType) return;
    if (resourceId < 0) return;

    resetQuery();

    /// The model has multiple rows for every resource, one row per tag,
    /// so we need to notify about the changes in all the tags
    QVector<QModelIndex> indexes;

    for (int i = 0; i < rowCount(); ++i)  {
        const QModelIndex idx = this->index(i, 0);
        KIS_ASSERT_RECOVER(idx.isValid()) { continue; }

        if (idx.data(Qt::UserRole + KisAllTagResourceModel::ResourceId).toInt() == resourceId) {
            indexes << idx;
        }
    }

    Q_FOREACH(const QModelIndex &index, indexes) {
        Q_EMIT dataChanged(index, index, {Qt::CheckStateRole, Qt::UserRole + KisAllTagResourceModel::ResourceActive});
    }
}

QString KisAllTagResourceModel::createQuery(bool onlyActive, bool returnADbIndexToo)
{
    QString query = QString("WITH initial_selection AS (\n"
                            "    SELECT   tags.id\n"
                            "    ,        resources.name\n"
                            "    ,        resources.filename\n"
                            "    ,        resources.md5sum\n"
                            "    ,        resource_types.id            as    resource_type_id\n"
                            "    ,        resource_types.name          as    resource_type_name\n"
                            "    ,        min(resources.id)            as    resource_id\n"
                            ) + (returnADbIndexToo ? QString(", resource_tags.id   as   resource_tags_row_id\n") : QString("")) + QString( // include r_t row id
                            ) + (onlyActive ? QString("") : QString(", resource_tags.active   as   resource_tags_pair_active\n")) + QString( // include r_t row active info
                            "    FROM     resource_types\n"
                            "    JOIN     resource_tags\n   ON       resource_tags.resource_id    = resources.id\n"
                            ) + (onlyActive ? QString("    AND       resource_tags.active         = 1\n") : QString("")) + QString( // make sure only active tags are used
                            "    JOIN     resources         ON       resources.resource_type_id   = resource_types.id\n"
                            "    JOIN     tags              ON       tags.id                      = resource_tags.tag_id\n"
                            "                              AND       tags.resource_type_id        = resource_types.id\n"
                            "    WHERE    resource_types.name          = :resource_type\n"
                            "    GROUP BY tags.id\n"
                            "    ,        resources.name\n"
                            "    ,        resources.filename\n"
                            "    ,        resources.md5sum\n"
                            "    ,        resource_types.id\n"
                            "    ORDER BY resource_tags.id\n"
                            ")\n"
                            "SELECT \n"
                            "       initial_selection.id           as tag_id\n"
                            ",      initial_selection.name         as resource_name\n"
                            ",      initial_selection.filename     as resource_filename\n"
                            ",      initial_selection.md5sum       as resource_md5sum\n"
                            ",      initial_selection.resource_id  as resource_id\n"
                            ",      tags.url                       as tag_url"
                            ",      tags.active                    as tag_active"
                            ",      tags.name                      as tag_name"
                            ",      tags.comment                   as tag_comment"
                            ",      resources.status               as resource_active\n"
                            ",      resources.tooltip              as resource_tooltip\n"
                            ",      resources.thumbnail            as resource_thumbnail\n"
                            ",      resources.status               as resource_active\n"
                            ",      resources.storage_id           as storage_id\n"
                            ",      storages.active                as resource_storage_active\n"
                            ",      storages.location              as location\n"
                            ",      tag_translations.name          as translated_name\n"
                            ",      tag_translations.comment       as translated_comment\n"
                            ",      initial_selection.resource_type_name as resource_type\n"
                            ) + (returnADbIndexToo ? QString(", initial_selection.resource_tags_row_id   as   resource_tags_row_id\n") : QString("")) + QString(
                            ) + (onlyActive ? QString("") : QString(", initial_selection.resource_tags_pair_active   as   resource_tags_pair_active\n")) + QString(
                            "FROM      initial_selection\n"
                            "JOIN      tags               ON   tags.id                     = initial_selection.id\n"
                            "                            AND   tags.resource_type_id       = initial_selection.resource_type_id\n"
                            "JOIN      resources          ON   resources.id                = resource_id\n"
                            "JOIN      storages           ON   storages.id                 = resources.storage_id\n"
                            "LEFT JOIN tag_translations   ON   tag_translations.tag_id     = initial_selection.id\n"
                            "                            AND   tag_translations.language   = :language\n");

    return query;


}

bool KisAllTagResourceModel::resetQuery()
{
    bool r = d->query.prepare(createQuery(true));

    if (!r) {
        qWarning() << "Could not prepare KisAllTagResourcesModel query" << d->query.lastError();
    }

    d->query.bindValue(":resource_type", d->resourceType);
    d->query.bindValue(":language", KisTag::currentLocale());

    r = d->query.exec();

    if (!r) {
        qWarning() << "Could not execute KisAllTagResourcesModel query" << d->query.lastError();
    }

    d->cachedRowCount = -1;

    return r;
}


struct KisTagResourceModel::Private {
    QString resourceType;
    KisAllTagResourceModel *sourceModel {0};
    QVector<int> tagIds;
    QVector<int> resourceIds;
    TagFilter tagFilter {ShowActiveTags};
    StorageFilter storageFilter {ShowActiveStorages};
    ResourceFilter resourceFilter {ShowActiveResources};
};


KisTagResourceModel::KisTagResourceModel(const QString &resourceType, QObject *parent)
    : QSortFilterProxyModel(parent)
    , d(new Private())
{
    d->resourceType = resourceType;
    d->sourceModel = KisResourceModelProvider::tagResourceModel(resourceType);
    setSourceModel(d->sourceModel);

    connect(KisResourceLocator::instance(), SIGNAL(storageAdded(const QString &)), this, SLOT(storageChanged(const QString &)));
    connect(KisResourceLocator::instance(), SIGNAL(storageRemoved(const QString &)), this, SLOT(storageChanged(const QString &)));
    connect(KisStorageModel::instance(), SIGNAL(storageEnabled(const QString &)), this, SLOT(storageChanged(const QString &)));
    connect(KisStorageModel::instance(), SIGNAL(storageDisabled(const QString &)), this, SLOT(storageChanged(const QString &)));
}

KisTagResourceModel::~KisTagResourceModel()
{
    delete d;
}

void KisTagResourceModel::setTagFilter(KisTagResourceModel::TagFilter filter)
{
    d->tagFilter = filter;
    invalidateFilter();
}

void KisTagResourceModel::setResourceFilter(KisTagResourceModel::ResourceFilter filter)
{
    d->resourceFilter = filter;
    invalidateFilter();
}

void KisTagResourceModel::setStorageFilter(KisTagResourceModel::StorageFilter filter)
{
    d->storageFilter = filter;
    invalidateFilter();
}

bool KisTagResourceModel::tagResources(const KisTagSP tag, const QVector<int> &resourceIds)
{
    bool r = d->sourceModel->tagResources(tag, resourceIds);
    return r;
}

bool KisTagResourceModel::untagResources(const KisTagSP tag, const QVector<int> &resourceIds)
{
    return d->sourceModel->untagResources(tag, resourceIds);
}

int KisTagResourceModel::isResourceTagged(const KisTagSP tag, const int resourceId)
{
    return d->sourceModel->isResourceTagged(tag, resourceId);
}

void KisTagResourceModel::setTagsFilter(const QVector<int> tagIds)
{
    d->tagIds = tagIds;
    invalidateFilter();
}

void KisTagResourceModel::setResourcesFilter(const QVector<int> resourceIds)
{
    d->resourceIds = resourceIds;
    invalidateFilter();
}

void KisTagResourceModel::setTagsFilter(const QVector<KisTagSP> tags)
{
    d->tagIds.clear();
    Q_FOREACH(const KisTagSP tag, tags) {
        if (tag && tag->valid() && tag->id() > -1) {
            d->tagIds << tag->id();
        }
    }
    invalidateFilter();
}

void KisTagResourceModel::setResourcesFilter(const QVector<KoResourceSP> resources)
{
    d->resourceIds.clear();
    Q_FOREACH(const KoResourceSP resource, resources) {
        if (resource->valid() && resource->resourceId() > -1) {
            d->resourceIds << resource->resourceId();
        }
    }
    invalidateFilter();
}

bool KisTagResourceModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
    if (!idx.isValid()) return false;

    int tagId = idx.data(Qt::UserRole + KisAllTagResourceModel::TagId).toInt();
    int resourceId = idx.data(Qt::UserRole + KisAllTagResourceModel::ResourceId).toInt();
    bool tagActive = idx.data(Qt::UserRole + KisAllTagResourceModel::TagActive).toBool();
    bool resourceActive = idx.data(Qt::UserRole + KisAllTagResourceModel::ResourceActive).toBool();
    bool resourceStorageActive = idx.data(Qt::UserRole + KisAllTagResourceModel::ResourceStorageActive).toBool();

    if (d->tagFilter == ShowAllTags && d->resourceFilter == ShowAllResources && d->storageFilter == ShowAllStorages) {
        return ((d->tagIds.contains(tagId) || d->tagIds.isEmpty()) &&
                (d->resourceIds.contains(resourceId) || d->resourceIds.isEmpty()));
    }

    if ((d->tagFilter == ShowActiveTags && !tagActive)
            || (d->tagFilter == ShowInactiveTags && tagActive)) {
        return false;
    }

    if ((d->resourceFilter == ShowActiveResources && !resourceActive)
            || (d->resourceFilter == ShowInactiveResources && resourceActive)) {
        return false;
    }

    if ((d->storageFilter == ShowActiveStorages && !resourceStorageActive)
            || (d->storageFilter == ShowInactiveStorages && resourceStorageActive)) {
        return false;
    }

    return ((d->tagIds.contains(tagId) || d->tagIds.isEmpty())
            && (d->resourceIds.contains(resourceId) || d->resourceIds.isEmpty()));
}

bool KisTagResourceModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    QString nameLeft = sourceModel()->data(source_left, Qt::UserRole + KisAllTagResourceModel::ResourceName).toString();
    QString nameRight = sourceModel()->data(source_right, Qt::UserRole + KisAllTagResourceModel::ResourceName).toString();
    return nameLeft.toLower() < nameRight.toLower();
}

void KisTagResourceModel::storageChanged(const QString &location)
{
    Q_UNUSED(location);
    invalidateFilter();
}

KoResourceSP KisTagResourceModel::resourceForIndex(QModelIndex index) const
{
    int id = data(index, Qt::UserRole + KisAllTagResourceModel::ResourceId).toInt();
    if (id < 1)  return nullptr;
    KoResourceSP res = KisResourceLocator::instance()->resourceForId(id);
    return res;
}

QModelIndex KisTagResourceModel::indexForResource(KoResourceSP resource) const
{
    if (!resource || !resource->valid() || resource->resourceId() < 0) return QModelIndex();

    for (int i = 0; i < rowCount(); ++i)  {
        QModelIndex idx = index(i, Qt::UserRole + KisAllTagResourceModel::ResourceId);
        Q_ASSERT(idx.isValid());
        if (idx.data(Qt::UserRole + KisAllTagResourceModel::ResourceId).toInt() == resource->resourceId()) {
            return idx;
        }
    }
    return QModelIndex();
}

QModelIndex KisTagResourceModel::indexForResourceId(int resourceId) const
{
    if (resourceId < 0) return QModelIndex();
    for (int i = 0; i < rowCount(); ++i)  {
        QModelIndex idx = index(i, Qt::UserRole + KisAllTagResourceModel::ResourceId);
        Q_ASSERT(idx.isValid());
        if (idx.data(Qt::UserRole + KisAllTagResourceModel::ResourceId).toInt() == resourceId) {
            return idx;
        }
    }
    return QModelIndex();
}

bool KisTagResourceModel::setResourceActive(const QModelIndex &index, bool value)
{
    KisResourceModel resourceModel(d->resourceType);
    QModelIndex idx = resourceModel.indexForResource(resourceForIndex(index));
    return resourceModel.setResourceActive(idx, value);
}

KoResourceSP KisTagResourceModel::importResourceFile(const QString &filename, const bool allowOverwrite, const QString &storageId)
{
    // Since we're importing the resource, there's no reason to add rows to the tags::resources table,
    // because the resource is untagged.
    KisResourceModel resourceModel(d->resourceType);
    return resourceModel.importResourceFile(filename, allowOverwrite, storageId);
}

KoResourceSP KisTagResourceModel::importResource(const QString &filename, QIODevice *device, const bool allowOverwrite, const QString &storageId)
{
    // Since we're importing the resource, there's no reason to add rows to the tags::resources table,
    // because the resource is untagged.
    KisResourceModel resourceModel(d->resourceType);
    return resourceModel.importResource(filename, device, allowOverwrite, storageId);
}

bool KisTagResourceModel::importWillOverwriteResource(const QString &fileName, const QString &storageLocation) const
{
    KisResourceModel resourceModel(d->resourceType);
    return resourceModel.importWillOverwriteResource(fileName, storageLocation);
}

bool KisTagResourceModel::exportResource(KoResourceSP resource, QIODevice *device)
{
    KisResourceModel resourceModel(d->resourceType);
    return resourceModel.exportResource(resource, device);
}

bool KisTagResourceModel::addResource(KoResourceSP resource, const QString &storageId)
{
    // Since we're importing the resource, there's no reason to add rows to the tags::resources table,
    // because the resource is untagged.
    KisResourceModel resourceModel(d->resourceType);
    return resourceModel.addResource(resource, storageId);
}

bool KisTagResourceModel::updateResource(KoResourceSP resource)
{
    KisResourceModel resourceModel(d->resourceType);
    bool r = resourceModel.updateResource(resource);
    if (r) {
        QModelIndex index = indexForResource(resource);
        if (index.isValid()) {
            emit dataChanged(index, index, {Qt::EditRole});
        }
    }
    return r;
}

bool KisTagResourceModel::reloadResource(KoResourceSP resource)
{
    KisResourceModel resourceModel(d->resourceType);
    bool r = resourceModel.reloadResource(resource);
    if (r) {
        QModelIndex index = indexForResource(resource);
        if (index.isValid()) {
            emit dataChanged(index, index, {Qt::EditRole});
        }
    }
    return r;
}

bool KisTagResourceModel::renameResource(KoResourceSP resource, const QString &name)
{
    KisResourceModel resourceModel(d->resourceType);
    bool r = resourceModel.renameResource(resource, name);
    if (r) {
        QModelIndex index = indexForResource(resource);
        if (index.isValid()) {
            emit dataChanged(index, index, {Qt::EditRole});
        }
    }
    return r;
}

bool KisTagResourceModel::setResourceMetaData(KoResourceSP resource, QMap<QString, QVariant> metadata)
{
    KisResourceModel resourceModel(d->resourceType);
    return resourceModel.setResourceMetaData(resource, metadata);
}
