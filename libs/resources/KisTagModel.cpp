/*
 * Copyright (c) 2018 boud <boud@valdyas.org>
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

struct KisTagModel::Private {
    QSqlQuery query;
    QString resourceType;
    int columnCount {1};
    int cachedRowCount {-1};
    int fakeRowsCount {1};
};


KisTagModel::KisTagModel(const QString &resourceType, QObject *parent)
    : QAbstractTableModel(parent)
    , d(new Private())
{
    d->resourceType = resourceType;
    prepareQuery();
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
                  "AND    resource_types.name = :resource_type");
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
            default:
                ;
            }
        }
    }
    return v;
}

bool KisTagModel::prepareQuery()
{
    beginResetModel();
    bool r = d->query.prepare("SELECT tags.id\n"
                              ",       tags.url\n"
                              ",       tags.name\n"
                              ",       tags.comment\n"
                              ",       resource_types.name as resource_type\n"
                              "FROM    tags\n"
                              ",       resource_types\n"
                              "WHERE   tags.resource_type_id = resource_types.id\n"
                              "AND     resource_types.name = :resource_type\n"
                              "AND     tags.active = 1");

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
