/*
 * Copyright (c) 2018 boud <boud@valdyas.org>
 *    Copyright (c) 2020 Agata Cacko <cacko.azh@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "KisTagModel.h"

#include <QtSql>
#include <QStringList>

#include <klocalizedstring.h>

#include <KisResourceLocator.h>
#include <KisResourceCacheDb.h>
#include <KisTag.h>

#include <KisResourceModelProvider.h>


Q_DECLARE_METATYPE(QSharedPointer<KisTag>)


struct KisTagModel::Private {
    QSqlQuery query;

    QSqlQuery tagsForResourceQuery;
    QSqlQuery resourcesForTagQuery;

    QString resourceType;
    int columnCount {5};
    int cachedRowCount {-1};
    int fakeRowsCount {1};
};


KisTagModel::KisTagModel(const QString &resourceType, QObject *parent)
    : QAbstractTableModel(parent)
    , d(new Private())
{
    d->resourceType = resourceType;
    if (!d->resourceType.isEmpty()) {
        prepareQuery();
    }
}

KisTagModel::~KisTagModel()
{
    delete d;
}

int KisTagModel::rowCount(const QModelIndex &/*parent*/) const
{
    if (d->cachedRowCount < 0) {
        QSqlQuery q;
        q.prepare("SELECT count(*)\n"
                  "FROM   tags\n"
                  ",      resource_types\n"
                  "WHERE  active = 1\n"
                  "AND    tags.resource_type_id = resource_types.id\n"
                  "AND    resource_types.name = :resource_type\n"
                  "AND    tags.storage_id in (SELECT id\n"
                  "                      FROM   storages\n"
                  "                      WHERE  active  == 1)");
        q.bindValue(":resource_type", d->resourceType);
        q.exec();
        q.first();

        const_cast<KisTagModel*>(this)->d->cachedRowCount = q.value(0).toInt() + d->fakeRowsCount;
    }

    return d->cachedRowCount;
}

int KisTagModel::columnCount(const QModelIndex &/*parent*/) const
{
    return d->columnCount;
}

QVariant KisTagModel::data(const QModelIndex &index, int role) const
{
    QVariant v;

    if (!index.isValid()) return v;
    if (index.row() > rowCount()) return v;
    if (index.column() > d->columnCount) return v;

    // The first row is All
    // XXX: Should we also add an "All Untagged"?
    if (index.row() < d->fakeRowsCount) {
        switch(role) {
        case Qt::DisplayRole:   // fallthrough
        case Qt::ToolTipRole:   // fallthrough
        case Qt::StatusTipRole: // fallthrough
        case Qt::WhatsThisRole:
            return i18n("All");
        case Qt::UserRole + Id:
            return "-1";
        case Qt::UserRole + Url: {
            return "All";
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
    else {
        bool pos = const_cast<KisTagModel*>(this)->d->query.seek(index.row() - d->fakeRowsCount);
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

void KisTagModel::setResourceType(const QString &resourceType)
{
    d->resourceType = resourceType;
    prepareQuery();
}

KisTagSP KisTagModel::tagForIndex(QModelIndex index) const
{
    KisTagSP tag = 0;
    if (!index.isValid()) return tag;
    if (index.row() > rowCount()) return tag;
    if (index.column() > columnCount()) return tag;


    if (index.row() < d->fakeRowsCount) {
        tag.reset(new KisTag());
        tag->setName("All");
        tag->setUrl("All");
        tag->setComment("All");
        tag->setId(-1);
        tag->setActive(true);
        tag->setValid(true);
    }
    else {
        bool pos = const_cast<KisTagModel*>(this)->d->query.seek(index.row() - d->fakeRowsCount);
        if (pos) {

            tag.reset(new KisTag());
            tag->setUrl(d->query.value("url").toString());
            tag->setName(d->query.value("name").toString());
            tag->setComment(d->query.value("comment").toString());
            tag->setId(d->query.value("id").toInt());
            tag->setActive(d->query.value("active").toBool());
            tag->setValid(true);
        }
    }

    return tag;
}

bool KisTagModel::addEmptyTag(const QString& tagName, QVector<KoResourceSP> taggedResouces)
{
    KisTagSP tag = KisTagSP(new KisTag());
    tag->setName(tagName);
    tag->setUrl(tagName);
    return addEmptyTag(tag, taggedResouces);
}

bool KisTagModel::addEmptyTag(const KisTagSP tag, QVector<KoResourceSP> taggedResouces)
{
    tag->setValid(true);
    tag->setActive(true);
    return addTag(tag, taggedResouces);
}

bool KisTagModel::addTag(const KisTagSP tag, QVector<KoResourceSP> taggedResouces)
{
    if (tag.isNull()) return false;
    if (!tag) return false;
    if (!tag->valid()) return false;
    // A new tag doesn't have an ID yet, that comes from the database
    if (tag->id() >= 0) return false;


    if (!KisResourceCacheDb::hasTag(tag->url(), d->resourceType)) {
         if (!KisResourceCacheDb::addTag(d->resourceType, "", tag->url(), tag->name(), tag->comment())) {
            qWarning() << "Could not add tag" << tag;
            return false;
        }
    } else {
        QSqlQuery q;
        if (!q.prepare("UPDATE tags\n"
                       "SET    active = 1\n"
                       "WHERE  url = :url\n"
                       "AND    resource_type_id = (SELECT id\n"
                       "                           FROM   resource_types\n"
                       "                           WHERE  name = :resource_type\n)")) {
            qWarning() << "Couild not prepare make existing tag active query" << tag << q.lastError();
            return false;
        }
        q.bindValue(":url", tag->url());
        q.bindValue(":resource_type", d->resourceType);

        if (!q.exec()) {
            qWarning() << "Couild not execute make existing tag active query" << q.boundValues(), q.lastError();
            return false;
        }
    }


    Q_FOREACH(const KoResourceSP resource, taggedResouces) {

        if (!resource) continue;
        if (!resource->valid()) continue;
        if (resource->resourceId() < 0) continue;

        tagResource(tag, resource);
    }


    return prepareQuery();
}

bool KisTagModel::removeTag(const KisTagSP tag)
{
    if (!tag) return false;
    if (!tag->valid()) return false;
    if (tag->id() < 0) return false;

    QSqlQuery q;
    if (!q.prepare("UPDATE tags\n"
                   "SET    active = 0\n"
                   "WHERE  id = :id")) {
        qWarning() << "Could not prepare remove tag query" << q.lastError();
        return false;
    }

    q.bindValue(":id", tag->id());

    if (!q.exec()) {
        qWarning() << "Could not execute remove tag query" << q.lastError();
        return false;
    }

    return prepareQuery();
}

bool KisTagModel::tagResource(const KisTagSP tag, const KoResourceSP resource)
{
    if (!tag) return false;
    if (!tag->valid()) return false;
    if (tag->id() < 0) return false;

    qDebug() << tag << " tag id " << tag->id();

    if (!resource) return false;
    if (!resource->valid()) return false;
    if (resource->resourceId() < 0) return false;

    QSqlQuery q;
    bool r = q.prepare("INSERT INTO resource_tags\n"
                  "(resource_id, tag_id)\n"
                  "VALUES\n"
                  "( (SELECT id\n"
                  "  FROM   resources\n"
                  "  WHERE  id = :resource_id)\n"
                      ", (SELECT id\n"
                  "   FROM   tags\n"
                  "   WHERE  id = :tag_id\n"
                  "   AND    resource_type_id = (SELECT id\n"
                  "                              FROM   resource_types\n"
                  "                              WHERE  name = :resource_type"
                  "                             \n)"
                  "  )\n"
                  ")\n");
    if (!r) {
        qWarning() << "Could not prepare insert into resource tags statement" << q.lastError();
        return false;
    }

    q.bindValue(":resource_id", resource->resourceId());
    q.bindValue(":tag_id", tag->id());
    q.bindValue(":resource_type", d->resourceType);

    if (!q.exec()) {
        qWarning() << "Could not execute insert into resource tags statement" << q.boundValues() << q.lastError();
        return false;
    }

    KisResourceModelProvider::resetModel(d->resourceType);
    return true;
}

bool KisTagModel::untagResource(const KisTagSP tag, const KoResourceSP resource)
{
    if (!tag) return false;
    if (!tag->valid()) return false;
    if (!tag->id()) return false;

    if (!resource) return false;
    if (!resource->valid()) return false;
    if (resource->resourceId() < 0) return false;

    // we need to delete an entry in resource_tags
    QSqlQuery query;
    bool r = query.prepare("DELETE FROM resource_tags\n"
                              "WHERE   resource_id = :resource_id\n"
                              "AND     tag_id = :tag_id");

    if (!r) {
        qWarning() << "Could not prepare KisTagModel query untagResource " << query.lastError();
    }

    query.bindValue(":resource_id", resource->resourceId());
    query.bindValue(":tag_id", tag->id());


    r = query.exec();

    if (!r) {
        qWarning() << "Could not select tags" << query.lastError();
    }

    KisResourceModelProvider::resetModel(d->resourceType);
    return true;
}

bool KisTagModel::renameTag(const KisTagSP tag, const QString &name)
{
    if (!tag) return false;
    if (!tag->valid()) return false;

    if (name.isEmpty()) return false;

    QSqlQuery q;
    if (!q.prepare("UPDATE tags\n"
                   "SET    name = :name\n"
                   "WHERE  url = :url\n"
                   "AND    resource_type_id = (SELECT id\n"
                   "                           FROM   resource_types\n"
                   "                           WHERE  name = :resource_type\n)")) {
        qWarning() << "Couild not prepare make existing tag active query" << tag << q.lastError();
        return false;
    }

    q.bindValue(":name", name);
    q.bindValue(":url", tag->url());
    q.bindValue(":resource_type", d->resourceType);

    if (!q.exec()) {
        qWarning() << "Couild not execute make existing tag active query" << q.boundValues(), q.lastError();
        return false;
    }

    return prepareQuery();

}

bool KisTagModel::changeTagActive(const KisTagSP tag, bool active)
{
    if (!tag) return false;
    if (!tag->valid()) return false;

    QSqlQuery q;
    if (!q.prepare("UPDATE tags\n"
                   "SET    active = :active\n"
                   "WHERE  url = :url\n"
                   "AND    resource_type_id = (SELECT id\n"
                   "                           FROM   resource_types\n"
                   "                           WHERE  name = :resource_type\n)")) {
        qWarning() << "Couild not prepare make existing tag active query" << tag << q.lastError();
        return false;
    }

    q.bindValue(":active", active);
    q.bindValue(":url", tag->url());
    q.bindValue(":resource_type", d->resourceType);

    if (!q.exec()) {
        qWarning() << "Couild not execute make existing tag active query" << q.boundValues(), q.lastError();
        return false;
    }

    return prepareQuery();

}

QVector<KisTagSP> KisTagModel::tagsForResource(int resourceId) const
{
    bool r = d->tagsForResourceQuery.prepare("SELECT tags.id\n"
                              ",      tags.url\n"
                              ",      tags.name\n"
                              ",      tags.comment\n"
                              "FROM   tags\n"
                              ",      resource_tags\n"
                              "WHERE  tags.active > 0\n"                               // make sure the tag is active
                              "AND    tags.id = resource_tags.tag_id\n"                // join tags + resource_tags by tag_id
                              "AND    resource_tags.resource_id = :resource_id\n");    // make sure we're looking for tags for a specific resource
    if (!r)  {
        qWarning() << "Could not prepare TagsForResource query" << d->tagsForResourceQuery.lastError();
    }

    d->tagsForResourceQuery.bindValue(":resource_id", resourceId);
    r = d->tagsForResourceQuery.exec();
    if (!r) {
        qWarning() << "Could not select tags for" << resourceId << d->tagsForResourceQuery.lastError() << d->tagsForResourceQuery.boundValues();
    }

    QVector<KisTagSP> tags;
    while (d->tagsForResourceQuery.next()) {
        //qDebug() << d->tagQuery.value(0).toString() << d->tagQuery.value(1).toString() << d->tagQuery.value(2).toString();
        KisTagSP tag(new KisTag());
        tag->setId(d->tagsForResourceQuery.value("id").toInt());
        tag->setUrl(d->tagsForResourceQuery.value("url").toString());
        tag->setName(d->tagsForResourceQuery.value("name").toString());
        tag->setComment(d->tagsForResourceQuery.value("comment").toString());
        tag->setValid(true);
        tag->setActive(true);
        tags << tag;
    }
    return tags;
}

QVector<KoResourceSP> KisTagModel::resourcesForTag(int tagId) const
{
    bool r = d->resourcesForTagQuery.prepare("SELECT DISTINCT resources.id\n"
                                        "FROM   resources\n"
                                        ",      resource_tags\n"
                                        ",      storages\n"
                                        ",      resource_types\n"
                                        "WHERE  resources.id = resource_tags.resource_id\n"                // join resources + resource_tags by resource_id
                                        "AND    resources.resource_type_id = resource_types.id\n" // join with resource types via resource type id
                                        "AND    resources.storage_id = storages.id\n"         // join with storages via storage id
                                        "AND    resource_tags.tag_id = :tag_id\n"     // must have the tag
                                        "AND    resource_types.name = :resource_type\n"       // the type must match the current type
                                        "AND    resources.status = 1\n"         // must be active itself
                                        "AND    storages.active = 1");          // must be from active storage
    if (!r)  {
        qWarning() << "Could not prepare ResourcesForTag query" << d->resourcesForTagQuery.lastError();
    }

    d->resourcesForTagQuery.bindValue(":tag_id", tagId);
    d->resourcesForTagQuery.bindValue(":resource_type", d->resourceType);

    r = d->resourcesForTagQuery.exec();
    if (!r) {
        qWarning() << "Could not select resources for tag " << tagId << d->resourcesForTagQuery.lastError() << d->resourcesForTagQuery.boundValues();
    }

    QVector<KoResourceSP> resources;
    qDebug() << "it's around: " << d->resourcesForTagQuery.size() << " resources for " << d->resourceType;
    while (d->resourcesForTagQuery.next()) {
        qDebug() << "KisTagModel::resourcesForTag(int tagId): ### query: " << d->resourcesForTagQuery.value("id").toInt();
        KoResourceSP resource = KisResourceLocator::instance()->resourceForId(d->resourcesForTagQuery.value("id").toInt());
        resources << resource;
    }

    return resources;

}

bool KisTagModel::prepareQuery()
{
    beginResetModel();
    bool r = d->query.prepare("SELECT  tags.id\n"
                              ",       tags.url\n"
                              ",       tags.name\n"
                              ",       tags.comment\n"
                              ",       resource_types.name as resource_type\n"
                              "FROM    tags\n"
                              ",       resource_types\n"
                              "WHERE   tags.resource_type_id = resource_types.id\n"
                              "AND     resource_types.name = :resource_type\n"
                              "AND     tags.active = 1\n"
                              "AND     tags.storage_id in (SELECT id\n"
                              "                      FROM   storages\n"
                              "                      WHERE  active  == 1)");

    if (!r) {
        qWarning() << "Could not prepare KisTagModel query" << d->query.lastError();
    }

    d->query.bindValue(":resource_type", d->resourceType);

    r = d->query.exec();

    if (!r) {
        qWarning() << "Could not select tags" << d->query.lastError();
    }

    d->cachedRowCount = -1;
    endResetModel();

    return r;
}
