/*
 * Copyright (c) 2019 Agata Cacko <cacko.azh@gmail.com>
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
#include "KisTagsResourcesModel.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QTime>

struct KisTagsResourcesModel::Private {
    QSqlQuery query;
    QString resourceType;
    int columnCount {3};
    int cachedRowCount {-1};
};


KisTagsResourcesModel::KisTagsResourcesModel(const QString &resourceType, QObject *parent)
    : QAbstractTableModel(parent)
    , d(new Private())
{
    d->resourceType = resourceType;
    if (!d->resourceType.isEmpty()) {
        prepareQuery();
    }
}

KisTagsResourcesModel::~KisTagsResourcesModel()
{
    delete d;
}

int KisTagsResourcesModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 0;

    if (d->cachedRowCount < 0) {
        QSqlQuery q;
        q.prepare("SELECT count(*)\n"
                  "FROM   resource_tags\n"
                  ",      resource_types\n"
                  ",      tags\n"
                  "WHERE  tags.resource_type_id = resource_types.id\n"
                  "AND    resource_types.name = :resource_type\n"
                  "AND    tags.id = resource_tags.id\n"
                  "AND    tags.storage_id in (SELECT id\n"
                  "                      FROM   storages\n"
                  "                      WHERE  active  == 1)");
        q.bindValue(":resource_type", d->resourceType);
        q.exec();
        q.first();

        const_cast<KisTagsResourcesModel*>(this)->d->cachedRowCount = q.value(0).toInt();
    }

    return d->cachedRowCount;
}

int KisTagsResourcesModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 0;
    return d->columnCount;
}

QVariant KisTagsResourcesModel::data(const QModelIndex &index, int role) const
{
    QVariant v;
    if (!index.isValid()) return v;
    if (index.row() > rowCount()) return v;
    if (index.column() > d->columnCount) return v;

    bool pos = const_cast<KisTagsResourcesModel*>(this)->d->query.seek(index.row());
    if (pos) {
        switch(role) {
        case Qt::DisplayRole:
            return QVariant(d->query.value("id").toString() + ": " + d->query.value("resource_id").toString() + ", "
                    + d->query.value("tag_id").toString());
        case Qt::ToolTipRole:   // fallthrough
        case Qt::StatusTipRole: // fallthrough
        case Qt::WhatsThisRole:
            return QVariant("Tag/Resource relationship: " + d->query.value("id").toString() + ": "
                    + d->query.value("resource_id").toString() + ", " + d->query.value("tag_id").toString());
        case Qt::UserRole + Id:
            return d->query.value("id");
        case Qt::UserRole + ResourceId:
            return d->query.value("resource_id");
        case Qt::UserRole + TagId:
            return d->query.value("tag_id");
        default:
            ;
        }
    }

    return v;
}



bool KisTagsResourcesModel::tagResource(const KisTagSP tag, const KoResourceSP resource)
{
    if (!tag) return false;
    if (!tag->valid()) return false;
    if (tag->id() < 0) return false;

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
                  "   WHERE  url = :url\n"
                  "   AND    name = :name\n"
                  "   AND    comment = :comment\n"
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
    q.bindValue(":url", tag->url());
    q.bindValue(":name", tag->name());
    q.bindValue(":comment", tag->comment());
    q.bindValue(":resource_type", d->resourceType);

    if (!q.exec()) {
        qWarning() << "Could not execute insert into resource tags statement" << q.boundValues() << q.lastError();
        return false;
    }

    return prepareQuery();
}

bool KisTagsResourcesModel::untagResource(const KisTagSP tag, const KoResourceSP resource)
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
        qWarning() << "Could not prepare KisTagsResourcesModel query" << query.lastError();
    }

    query.bindValue(":resource_id", resource->resourceId());
    query.bindValue(":tag_id", tag->id());


    r = query.exec();

    if (!r) {
        qWarning() << "Could not select tags" << query.lastError();
    }

    return prepareQuery();
}

QVector<KisTagSP> KisTagsResourcesModel::tagsForResource(int resourceId) const
{
    d->query.bindValue(":resource_id", resourceId);
    bool r = d->query.exec();
    if (!r) {
        qWarning() << "Could not select tags for" << resourceId << d->query.lastError() << d->query.boundValues();
    }

    QVector<KisTagSP> tags;
    while (d->query.next()) {
        //qDebug() << d->tagQuery.value(0).toString() << d->tagQuery.value(1).toString() << d->tagQuery.value(2).toString();
        KisTagSP tag(new KisTag());
        tag->setId(d->query.value("id").toInt());
        tag->setUrl(d->query.value("url").toString());
        tag->setName(d->query.value("name").toString());
        tag->setComment(d->query.value("comment").toString());
        tag->setValid(true);
        tag->setActive(true);
        tags << tag;
    }
    return tags;
}


void KisTagsResourcesModel::setResourceType(const QString &resourceType)
{
    d->resourceType = resourceType;
    prepareQuery();
}

bool KisTagsResourcesModel::resetQuery()
{
    QTime t;
    t.start();

    beginResetModel();
    bool r = d->query.exec();
    if (!r) {
        qWarning() << "Could not select" << d->resourceType << "resources" << d->query.lastError() << d->query.boundValues();
    }
    d->cachedRowCount = -1;
    endResetModel();
    //emit afterResourcesLayoutReset();

    qDebug() << "KisTagsResourcesModel::resetQuery for" << d->resourceType << "took" << t.elapsed() << "ms";


    return r;
}

bool KisTagsResourcesModel::prepareQuery()
{
    beginResetModel();

    bool r = d->query.prepare("SELECT tags.id\n"
                            ",      tags.url\n"
                            ",      tags.name\n"
                            ",      tags.comment\n"
                            "FROM   tags\n"
                            ",      resource_tags\n"
                            "WHERE  tags.active > 0\n"                               // make sure the tag is active
                            "AND    tags.id = resource_tags.tag_id\n"                // join tags + resource_tags by tag_id
                            "AND    resource_tags.resource_id = :resource_id\n");    // make sure we're looking for tags for a specific resource
    if (!r)  {
        qWarning() << "Could not prepare TagsForResource query" << d->query.lastError();
    }

    d->cachedRowCount = -1;
    endResetModel();

    return r;
}














