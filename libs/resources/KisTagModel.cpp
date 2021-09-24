/*
 * SPDX-FileCopyrightText: 2018 boud <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2020 Agata Cacko <cacko.azh@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisTagModel.h"

#include <QtSql>
#include <QStringList>
#include <QElapsedTimer>

#include <klocalizedstring.h>

#include <KisResourceLocator.h>
#include <KisResourceCacheDb.h>
#include <KisTag.h>

#include <KisResourceModelProvider.h>
#include <KisTagResourceModel.h>
#include <KisStorageModel.h>
#include <QVector>

#include <kis_assert.h>

static int s_fakeRowsCount {2};

struct KisAllTagsModel::Private {
    QSqlQuery query;

    QString resourceType;
    int columnCount {5};
    int cachedRowCount {-1};
};


KisAllTagsModel::KisAllTagsModel(const QString &resourceType, QObject *parent)
    : QAbstractTableModel(parent)
    , d(new Private())
{
    d->resourceType = resourceType;
    if (!d->resourceType.isEmpty()) {
        resetQuery();
    }

    connect(KisResourceLocator::instance(), SIGNAL(storageAdded(const QString&)), this, SLOT(addStorage(const QString&)));
    connect(KisResourceLocator::instance(), SIGNAL(storageRemoved(const QString&)), this, SLOT(removeStorage(const QString&)));
    connect(KisStorageModel::instance(), SIGNAL(storageEnabled(const QString&)), this, SLOT(addStorage(const QString&)));
    connect(KisStorageModel::instance(), SIGNAL(storageDisabled(const QString&)), this, SLOT(removeStorage(const QString&)));

}

void KisAllTagsModel::untagAllResources(KisTagSP tag)
{
    KisTagResourceModel model(d->resourceType);
    model.setTagsFilter(QVector<int>() << tag->id());
    QList<int> taggedResources;
    for (int i = 0; i < model.rowCount(); i++) {
        QModelIndex idx = model.index(i, 0);
        taggedResources.append(model.data(idx, Qt::UserRole + KisTagResourceModel::Id).toInt());
    }

    for (int i = 0; i < taggedResources.size(); i++) {
        model.untagResource(tag, taggedResources[i]);
    }


}

KisAllTagsModel::~KisAllTagsModel()
{
    delete d;
}

int KisAllTagsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    if (d->cachedRowCount < 0) {
        QSqlQuery q;
        q.prepare("SELECT count(*)\n"
                  "FROM   tags\n"
                  ",      resource_types\n"
                  "WHERE  tags.resource_type_id = resource_types.id\n"
                  "AND    resource_types.name = :resource_type\n");
        q.bindValue(":resource_type", d->resourceType);
        if (!q.exec()) {
            qWarning() << "Could not execute tags rowcount query" << q.lastError();
        }
        q.first();

        const_cast<KisAllTagsModel*>(this)->d->cachedRowCount = q.value(0).toInt() + s_fakeRowsCount;
    }

    return d->cachedRowCount;
}

int KisAllTagsModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return d->columnCount;
}

QVariant KisAllTagsModel::data(const QModelIndex &index, int role) const
{
    QVariant v;

    if (!index.isValid()) return v;
    if (index.row() > rowCount()) return v;
    if (index.column() > d->columnCount) return v;

    if (index.row() < s_fakeRowsCount) {
        if (index.row() == KisAllTagsModel::All + s_fakeRowsCount) {
            switch(role) {
            case Qt::DisplayRole:   // fallthrough
            case Qt::ToolTipRole:   // fallthrough
            case Qt::StatusTipRole: // fallthrough
            case Qt::WhatsThisRole:
            case Qt::UserRole + Name:
                return i18n("All");
            case Qt::UserRole + Id:
                return QString::number(KisAllTagsModel::All);
            case Qt::UserRole + Url: {
                return urlAll();
            }
            case Qt::UserRole + ResourceType:
                return d->resourceType;
            case Qt::UserRole + Active:
                return true;
            case Qt::UserRole + KisTagRole:
            {
                KisTagSP tag = tagForIndex(index);
                QVariant response;
                response.setValue(tag);
                return response;
            }
            default:
                ;
            }
        } else if (index.row() == KisAllTagsModel::AllUntagged + s_fakeRowsCount) {
            switch(role) {
            case Qt::DisplayRole:   // fallthrough
            case Qt::ToolTipRole:   // fallthrough
            case Qt::StatusTipRole: // fallthrough
            case Qt::WhatsThisRole:
            case Qt::UserRole + Name:
                return i18n("All Untagged");
            case Qt::UserRole + Id:
                return QString::number(KisAllTagsModel::AllUntagged);
            case Qt::UserRole + Url: {
                return urlAllUntagged();
            }
            case Qt::UserRole + ResourceType:
                return d->resourceType;
            case Qt::UserRole + Active:
                return true;
            case Qt::UserRole + KisTagRole:
            {
                KisTagSP tag = tagForIndex(index);
                QVariant response;
                response.setValue(tag);
                return response;
            }
            default:
                ;
            }
        }
    }
    else {
        bool pos = const_cast<KisAllTagsModel*>(this)->d->query.seek(index.row() - s_fakeRowsCount);
        if (pos) {
            switch(role) {
            case Qt::DisplayRole:
                return d->query.value("name");
            case Qt::ToolTipRole:   // fallthrough
            case Qt::StatusTipRole: // fallthrough
            case Qt::WhatsThisRole:
                return d->query.value("comment");
            case Qt::UserRole + Id:
                return d->query.value("id");
            case Qt::UserRole + Name:
                return d->query.value("name");
            case Qt::UserRole + Url:
                return d->query.value("url");
            case Qt::UserRole + ResourceType:
                return d->query.value("resource_type");
            case Qt::UserRole + Active:
                return d->query.value("active");
            case Qt::UserRole + KisTagRole:
            {
                KisTagSP tag = tagForIndex(index);
                QVariant response;
                response.setValue(tag);
                return response;
            }
            default:
                ;
            }
        }
    }
    return v;
}

bool KisAllTagsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    int id = data(index, Qt::UserRole + Id).toInt();

    if (index.isValid() && role == Qt::CheckStateRole) {
        QSqlQuery q;
        if (!q.prepare("UPDATE tags\n"
                       "SET    active = :active\n"
                       "WHERE  id = :id\n")) {
            qWarning() << "Could not prepare make existing tag active query" << q.lastError();
            return false;
        }
        q.bindValue(":active", value.toBool());
        q.bindValue(":id", id);

        if (!q.exec()) {
            qWarning() << "Could not execute make existing tag active query" << q.boundValues(), q.lastError();
            return false;
        }
    }
    resetQuery();
    emit dataChanged(index, index, {role});
    return true;
}

Qt::ItemFlags KisAllTagsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable | Qt::ItemNeverHasChildren;
}

QModelIndex KisAllTagsModel::indexForTag(KisTagSP tag) const
{
    if (!tag) return QModelIndex();
    // For now a linear seek to find the first tag
    if (tag->id() < 0 && (tag->url() == urlAll() || tag->url() == urlAllUntagged())) {
        // this must be either a fake tag id, or a "naked" tag
        // TODO: do we even use "naked tags"? won't it be better to just use QStrings?
        return index(tag->id() + s_fakeRowsCount, 0);
    }

    d->query.first();
    bool r = d->query.first();
    if (!r) {
        return QModelIndex();
    }
    do {
        if (tag->id() >= 0) {
            if (d->query.value("id").toInt() == tag->id()) {
                return index(d->query.at() + s_fakeRowsCount, 0);
            }
        }
        else {
            // This is a naked tag, one that didn't come from the
            // database.
            // But not a "fake" tag (All or AllUntagged)!
            if (d->query.value("url").toString() == tag->url()
                    && d->query.value("resource_type") == d->resourceType) {
                return index(d->query.at() + s_fakeRowsCount, 0);
            }
        }
    } while (d->query.next());

    return QModelIndex();
}

KisTagSP KisAllTagsModel::tagForIndex(QModelIndex index) const
{
    KisTagSP tag = 0;
    if (!index.isValid()) return tag;
    if (index.row() > rowCount()) return tag;
    if (index.column() > columnCount()) return tag;

    if (index.row() < s_fakeRowsCount) {
        if (index.row() == KisAllTagsModel::All + s_fakeRowsCount) {
            tag.reset(new KisTag());
            tag->setName(i18n("All"));
            tag->setResourceType(d->resourceType);
            tag->setUrl(urlAll());
            tag->setComment(i18n("All Resources"));
            tag->setId(KisAllTagsModel::All);
            tag->setActive(true);
            tag->setValid(true);
        }
        else if (index.row() == KisAllTagsModel::AllUntagged + s_fakeRowsCount) {
            tag.reset(new KisTag());
            tag->setName(i18n("All Untagged"));
            tag->setResourceType(d->resourceType);
            tag->setUrl(urlAllUntagged());
            tag->setComment(i18n("All Untagged Resources"));
            tag->setId(KisAllTagsModel::AllUntagged);
            tag->setActive(true);
            tag->setValid(true);
        }
    }
    else {
        bool pos = const_cast<KisAllTagsModel*>(this)->d->query.seek(index.row() - s_fakeRowsCount);
        if (pos) {
            tag.reset(new KisTag());
            tag->setUrl(d->query.value("url").toString());
            tag->setName(d->query.value("name").toString());
            tag->setComment(d->query.value("comment").toString());
            tag->setResourceType(d->resourceType);
            tag->setId(d->query.value("id").toInt());
            tag->setActive(d->query.value("active").toBool());
            tag->setValid(true);
        }
    }

    return tag;
}

KisTagSP KisAllTagsModel::addTag(const QString& tagName, const bool allowOverwrite, QVector<KoResourceSP> taggedResources)
{
    KisTagSP tag = KisTagSP(new KisTag());
    tag->setName(tagName);
    tag->setUrl(tagName);
    tag->setValid(true);
    tag->setActive(true);
    tag->setResourceType(d->resourceType);

    if (addTag(tag, allowOverwrite, taggedResources)) {
        return tag;
    }
    else {
        return 0;
    }
}


bool KisAllTagsModel::addTag(const KisTagSP tag, const bool allowOverwrite, QVector<KoResourceSP> taggedResouces)
{
    if (!tag) return false;
    if (!tag->valid()) return false;

    bool r = true;

    if (!KisResourceCacheDb::hasTag(tag->url(), d->resourceType)) {
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        if (!KisResourceCacheDb::addTag(d->resourceType, "", tag->url(), tag->name(), tag->comment(), tag->filename())) {
            qWarning() << "Could not add tag" << tag;
            return false;
        }
        resetQuery();
        endInsertRows();

    } else if (allowOverwrite) {
        KisTagSP trueTag = tagForUrl(tag->url());
        r = setData(indexForTag(trueTag), QVariant::fromValue(true), Qt::CheckStateRole);
        untagAllResources(trueTag);
        tag->setComment(trueTag->comment()); // id will be set later, comment and filename are the only thing left
        tag->setFilename(trueTag->filename());
    } else {
        return false;
    }

    tag->setId(data(indexForTag(tag), Qt::UserRole + KisAllTagsModel::Id).toInt());
    tag->setValid(true);
    tag->setActive(data(indexForTag(tag), Qt::UserRole + KisAllTagsModel::Active).toInt());

    if (!taggedResouces.isEmpty()) {
        KisTagSP tagFromDb = tagForUrl(tag->url());
        Q_FOREACH(const KoResourceSP resource, taggedResouces) {

            if (!resource) continue;
            if (!resource->valid()) continue;
            if (resource->resourceId() < 0) continue;

            KisTagResourceModel(d->resourceType).tagResource(tag, resource->resourceId());
        }
    }

    return r;
}

bool KisAllTagsModel::setTagActive(const KisTagSP tag)
{
    if (!tag) return false;
    if (!tag->valid()) return false;

    tag->setActive(true);

    return setData(indexForTag(tag), QVariant::fromValue(true), Qt::CheckStateRole);

}

bool KisAllTagsModel::setTagInactive(const KisTagSP tag)
{
    if (!tag) return false;
    if (!tag->valid()) return false;

    tag->setActive(false);

    return setData(indexForTag(tag), QVariant::fromValue(false), Qt::CheckStateRole);
}

bool KisAllTagsModel::renameTag(const KisTagSP tag, const bool allowOverwrite)
{
    if (!tag) return false;
    if (!tag->valid()) return false;

    QString name = tag->name();
    QString url = tag->name(); // let's set the url to be the same as the name
    int id = tag->id();

    if (tag->id() < 0) return false;

    if (name.isEmpty()) return false;

    KisTagSP tagWithTheSameUrl = tagForUrl(url);

    if (!tagWithTheSameUrl.isNull()) {
        if (!allowOverwrite) {
            // there is already a tag with this url
            return false;
        } else { // allowOverwrite = true
            // untag everything and remove the tag from the database
            this->untagAllResources(tagWithTheSameUrl);

            QModelIndex idxRemove = indexForTag(tagWithTheSameUrl);
            beginRemoveRows(QModelIndex(), idxRemove.row(), idxRemove.row());

            QSqlQuery qRemove;
            if (!qRemove.prepare("DELETE FROM tags\n"
                           "WHERE  id = :id\n"
                           "AND    url = :url\n"
                           "AND    resource_type_id = (SELECT id\n"
                           "                           FROM   resource_types\n"
                           "                           WHERE  name = :resource_type\n)")) {
                qWarning() << "Couild not prepare make query to remove a different tag with the same url" << tag << qRemove.lastError();
                endRemoveRows();
                return false;
            }

            qRemove.bindValue(":id", tagWithTheSameUrl->id());
            qRemove.bindValue(":url", tagWithTheSameUrl->url());
            qRemove.bindValue(":resource_type", d->resourceType);

            if (!qRemove.exec()) {
                qWarning() << "Couild not execute query to remove a different tag with the same url" << qRemove.boundValues(), qRemove.lastError();
                endRemoveRows();
                return false;
            }


            resetQuery();
            endRemoveRows();

        }
    }

    QSqlQuery q;
    if (!q.prepare("UPDATE tags\n"
                   "SET    name = :name\n"
                   ",      url = :url\n"
                   "WHERE  id = :id\n"
                   "AND    resource_type_id = (SELECT id\n"
                   "                           FROM   resource_types\n"
                   "                           WHERE  name = :resource_type\n)")) {
        qWarning() << "Couild not prepare make existing tag active query" << tag << q.lastError();
        return false;
    }

    q.bindValue(":name", name);
    q.bindValue(":url", url);
    q.bindValue(":id", id);
    q.bindValue(":resource_type", d->resourceType);

    if (!q.exec()) {
        qWarning() << "Couild not execute make existing tag active query" << q.boundValues(), q.lastError();
        return false;
    }

    bool r = resetQuery();
    QModelIndex idx = indexForTag(tag);
    emit dataChanged(idx, idx, {Qt::EditRole});

    // set the url of the provided tag to the new one
    tag->setUrl(name);

    return r;

}

bool KisAllTagsModel::changeTagActive(const KisTagSP tag, bool active)
{
    if (!tag) return false;
    if (!tag->valid()) return false;

    QModelIndex idx = indexForTag(tag);
    tag->setActive(active);
    return setData(idx, QVariant::fromValue(active), Qt::CheckStateRole);

}



KisTagSP KisAllTagsModel::tagForUrl(const QString& tagUrl) const
{
    if (tagUrl.isEmpty()) {
        return KisTagSP();
    }

    if (tagUrl == urlAll()) {
        return tagForIndex(index(Ids::All + s_fakeRowsCount, 0));
    } else if (tagUrl == urlAllUntagged()) {
        return tagForIndex(index(Ids::AllUntagged + s_fakeRowsCount, 0));
    }

    QSqlQuery query;
    bool r = query.prepare("SELECT tags.id\n"
                           ",      tags.url\n"
                           ",      tags.name\n"
                           ",      tags.comment\n"
                           ",      tags.active\n"
                           ",      resource_types.name as resource_type\n"
                           "FROM   tags\n"
                           ",      resource_types\n"
                           "WHERE  tags.resource_type_id = resource_types.id\n"
                           "AND    resource_types.name = :resource_type\n"
                           "AND    tags.url = :tag_url\n");

    if (!r) {
        qWarning() << "Could not prepare KisAllTagsModel::tagForUrl query" << query.lastError();
        return KisTagSP();
    }

    query.bindValue(":resource_type", d->resourceType);
    QString tagUrlForSql = tagUrl;
    query.bindValue(":tag_url", tagUrlForSql);

    r = query.exec();
    if (!r) {
        qWarning() << "Could not execute KisAllTagsModel::tagForUrl query" << query.lastError();
        return KisTagSP();
    }
    KisTagSP tag(new KisTag());
    r = query.first();
    if (!r) {
        return KisTagSP();
    }

    tag->setUrl(query.value("url").toString());
    tag->setName(query.value("name").toString());
    tag->setResourceType(d->resourceType);
    tag->setComment(query.value("comment").toString());
    tag->setId(query.value("id").toInt());
    tag->setActive(query.value("active").toBool());
    tag->setValid(true);

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(tagUrl.toLower() == tag->url().toLower(), KisTagSP());

    return tag;

}

bool KisAllTagsModel::resetQuery()
{
//    QElapsedTimer t;
//    t.start();
    bool r = d->query.prepare("SELECT tags.id\n"
                              ",      tags.url\n"
                              ",      tags.name\n"
                              ",      tags.comment\n"
                              ",      tags.active\n"
                              ",      resource_types.name as resource_type\n"
                              "FROM   tags\n"
                              ",      resource_types\n"
                              "WHERE  tags.resource_type_id = resource_types.id\n"
                              "AND    resource_types.name = :resource_type\n");

    if (!r) {
        qWarning() << "Could not prepare KisAllTagsModel query" << d->query.lastError();
    }

    d->query.bindValue(":resource_type", d->resourceType);

    r = d->query.exec();

    if (!r) {
        qWarning() << "Could not select tags" << d->query.lastError();
    }

    d->cachedRowCount = -1;
    return r;
}

void KisAllTagsModel::addStorage(const QString &location)
{
    Q_UNUSED(location)
    beginResetModel();
    resetQuery();
    endResetModel();
}



void KisAllTagsModel::removeStorage(const QString &location)
{
    Q_UNUSED(location)
    beginResetModel();
    resetQuery();
    endResetModel();
}


struct KisTagModel::Private {
    TagFilter tagFilter{KisTagModel::ShowActiveTags};
    StorageFilter storageFilter {KisTagModel::ShowActiveStorages};
};

KisTagModel::KisTagModel(const QString &type, QObject *parent)
    : QSortFilterProxyModel(parent)
    , d(new Private())
{
    setSourceModel(KisResourceModelProvider::tagModel(type));
    sort(KisAllTagsModel::Name);
}

KisTagModel::~KisTagModel()
{
    delete d;
}

void KisTagModel::setTagFilter(KisTagModel::TagFilter filter)
{
    if (d->tagFilter != filter) {
        d->tagFilter = filter;
        invalidateFilter();
    }
}

void KisTagModel::setStorageFilter(KisTagModel::StorageFilter filter)
{
    if (d->storageFilter != filter) {
        d->storageFilter = filter;
        invalidateFilter();
    }
}

QModelIndex KisTagModel::indexForTag(KisTagSP tag) const
{
    KisAbstractTagModel *source = dynamic_cast<KisAbstractTagModel*>(sourceModel());
    if (source) {
        return mapFromSource(source->indexForTag(tag));
    }
    return QModelIndex();

}

KisTagSP KisTagModel::tagForIndex(QModelIndex index) const
{
    KisAbstractTagModel *source = dynamic_cast<KisAbstractTagModel*>(sourceModel());
    if (source) {
        return source->tagForIndex(mapToSource(index));
    }
    return 0;
}


KisTagSP KisTagModel::addTag(const QString &tagName, const bool allowOverwrite, QVector<KoResourceSP> taggedResources)
{
    KisAbstractTagModel *source = dynamic_cast<KisAbstractTagModel*>(sourceModel());
    if (source) {
        return source->addTag(tagName, allowOverwrite, taggedResources);
    }
    return 0;
}

KisTagSP KisTagModel::tagForUrl(const QString& url) const
{
    KisAbstractTagModel *source = dynamic_cast<KisAbstractTagModel*>(sourceModel());
    if (source) {
        return source->tagForUrl(url);
    }
    return 0;
}


bool KisTagModel::addTag(const KisTagSP tag, const bool allowOverwrite, QVector<KoResourceSP> taggedResouces)
{
    KisAbstractTagModel *source = dynamic_cast<KisAbstractTagModel*>(sourceModel());
    if (source) {
        return source->addTag(tag, allowOverwrite, taggedResouces) ;
    }
    return false;
}

bool KisTagModel::setTagInactive(const KisTagSP tag)
{
    KisAbstractTagModel *source = dynamic_cast<KisAbstractTagModel*>(sourceModel());
    if (source) {
        return source->setTagInactive(tag) ;
    }
    return false;
}

bool KisTagModel::setTagActive(const KisTagSP tag)
{
    KisAbstractTagModel *source = dynamic_cast<KisAbstractTagModel*>(sourceModel());
    if (source) {
        return source->setTagActive(tag) ;
    }
    return false;

}

bool KisTagModel::renameTag(const KisTagSP tag, const bool allowOverwrite)
{
    KisAbstractTagModel *source = dynamic_cast<KisAbstractTagModel*>(sourceModel());
    if (source) {
        return source->renameTag(tag, allowOverwrite);
    }
    return false;
}

bool KisTagModel::changeTagActive(const KisTagSP tag, bool active)
{
    KisAbstractTagModel *source = dynamic_cast<KisAbstractTagModel*>(sourceModel());
    if (source) {
        return source->changeTagActive(tag, active);
    }
    return false;
}


bool KisTagModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (d->tagFilter == ShowAllTags && d->storageFilter == ShowAllStorages) {
        return true;
    }

    QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
    if (!idx.isValid()) {
        return false;
    }
    int tagId = sourceModel()->data(idx, Qt::UserRole + KisAllTagsModel::Id).toInt();

    if (tagId < 0) {
        return true;
    }

    TagFilter tagActive = (TagFilter)sourceModel()->data(idx, Qt::UserRole + KisAllTagsModel::Active).toInt();

    StorageFilter storageActive = ShowAllStorages;

    if (d->storageFilter == ShowAllStorages) {
        return (tagActive == d->tagFilter);
    }

    {
        if (tagId > 0) {
            QSqlQuery q;
            q.prepare("SELECT count(*)\n"
                      "FROM   tags_storages\n"
                      ",      storages\n"
                      "WHERE  tags_storages.tag_id = :tag_id\n"
                      "AND    tags_storages.storage_id = storages.id\n"
                      "AND    storages.active = 1\n");

            q.bindValue(":tag_id", tagId);

            if (!q.exec()) {
                qWarning() << "Could not execute tags in storages query" << q.lastError() << q.boundValues();
            }
            else {
                q.first();

                if (q.value(0).toInt() > 0) {
                    storageActive = ShowActiveStorages;
                }
            }
        }
    }

    if (d->tagFilter == ShowAllTags) {
        return (storageActive == d->storageFilter);
    }

    return ((storageActive == d->storageFilter) && (tagActive == d->tagFilter));
}

bool KisTagModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    const bool leftIsFakeRow = source_left.row() < s_fakeRowsCount;
    const bool rightIsFakeRow = source_right.row() < s_fakeRowsCount;
    // Always sort fake rows ("All" and "All Untagged") above the rest.
    if (leftIsFakeRow && rightIsFakeRow) {
        return source_left.row() < source_right.row();
    } else if (leftIsFakeRow) {
        return true;
    } else if (rightIsFakeRow) {
        return false;
    } else {
        QString nameLeft = sourceModel()->data(source_left, Qt::UserRole + KisAllTagsModel::Name).toString().toLower();
        QString nameRight = sourceModel()->data(source_right, Qt::UserRole + KisAllTagsModel::Name).toString().toLower();
        return (nameLeft < nameRight);
    }
}
