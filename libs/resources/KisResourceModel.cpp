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
    QSqlQuery query;
    QString resourceType;
    int columnCount {9};
    int cachedRowCount {-1};
};


KisResourceModel::KisResourceModel(const QString &resourceType, QObject *parent)
    : QAbstractTableModel(parent)
    , d(new Private)
{
    d->resourceType = resourceType;
    prepareQuery();
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

    bool pos = const_cast<KisResourceModel*>(this)->d->query.seek(index.row());

    if (pos) {
        switch(role) {
        case Qt::DisplayRole:
        {
            switch(index.column()) {
            case Id:
                return d->query.value("id");
            case StorageId:
                return d->query.value("storage_id");
            case Name:
                return d->query.value("name");
            case Filename:
                return d->query.value("filename");
            case Tooltip:
                return d->query.value("tooltip");
            case Thumbnail:
                ;
            case Status:
                return d->query.value("status");
            case Location:
                return d->query.value("location");
            case ResourceType:
                return d->query.value("resource_type");
            default:
                ;
            };
        }
        case Qt::DecorationRole:
        {
            if (index.column() == 5) {
                QByteArray ba = d->query.value("thumbnail").toByteArray();
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
            return d->query.value("tooltip");
        case Qt::UserRole + Id:
            return d->query.value("id");
        case Qt::UserRole + StorageId:
            return d->query.value("storage_id");
        case Qt::UserRole + Name:
            return d->query.value("name");
        case Qt::UserRole + Filename:
            return d->query.value("filename");
        case Qt::UserRole + Tooltip:
            return d->query.value("tooltip");
        case Qt::UserRole + Status:
            return d->query.value("status");
        case Qt::UserRole + Location:
            return d->query.value("location");
        case Qt::UserRole + ResourceType:
            return d->query.value("resource_type");
        case Qt::UserRole + Tags:
        {
            QStringList tags;
            // XXX
            return QVariant::fromValue<QStringList>(tags);
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

    bool pos = const_cast<KisResourceModel*>(this)->d->query.seek(index.row());
    if (pos) {
        QString storageLocation = d->query.value("location").toString();
        QString resourceLocation = d->query.value("filename").toString();
        resource = KisResourceLocator::instance()->resource(storageLocation, resourceLocation);
    }
    return resource;

}

bool KisResourceModel::prepareQuery(const QStringList &tags)
{
    beginResetModel();
    bool r = d->query.prepare("SELECT resources.id\n"
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
        qWarning() << "Could not prepare KisResourceModel query" << d->query.lastError();
    }
    d->query.bindValue(":resource_type", d->resourceType);
    r = d->query.exec();
    if (!r) {
        qWarning() << "Could not select" << d->resourceType << "resources" << d->query.lastError() << d->query.boundValues();
    }
    d->cachedRowCount = -1;
    endResetModel();

    return r;

}

int KisResourceModel::rowCount(const QModelIndex &) const
{
    if (d->cachedRowCount < 0) {
        QSqlQuery q;
        q.prepare("SELECT count(*)\n"
                  "FROM   resources\n"
                  ",      resource_types\n"
                  "WHERE  resources.resource_type_id = resource_types.id\n"
                  "AND    resource_types.name = :resource_type");
        q.bindValue(":resource_type", d->resourceType);
        q.exec();
        q.first();

        const_cast<KisResourceModel*>(this)->d->cachedRowCount = q.value(0).toInt();
    }

    return d->cachedRowCount;
}
